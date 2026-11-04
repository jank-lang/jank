#!/usr/bin/env bash
# Fixture test for bin/apply-freebsd-patches.
# Each fixture repo records its OWN base commit in a fixture-private patch-dir
# (base.txt + copy of the patch file). The production script (bin/apply-freebsd-patches)
# runs with --patch-dir pointing at that fixture metadata, unchanged. Wrong-base
# refusal is exercised by moving the fixture HEAD AFTER the pin was written.
set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
src_root="$(cd "$here/../.." && pwd)"    # compiler+runtime
applier="$src_root/bin/apply-freebsd-patches"
tmp="$(mktemp -d "${TMPDIR:-/tmp}/freebsd-patch-fixture.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

fails=0
pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; fails=$((fails+1)); }

# build_sub NAME DIR FILE_BASE DIR_LIST content-dir file...
# Creates fixture repo at pristine base content of each file.
make_fixture_repo() { # dir files... (paths relative)
    local dir="$1"; shift
    mkdir -p "$dir"
    ( cd "$dir" || exit 1
      git init -q .
      git config user.email t@t; git config user.name t
      git add -A >/dev/null 2>&1
      git commit -qm base >/dev/null 2>&1 || true
    ) >/dev/null 2>&1
}

seed_files() { # dir base_commit base_sub path...
    local dir="$1" base_commit="$2" base_sub="$3"; shift 3
    local f
    for f in "$@"; do
        mkdir -p "$dir/$(dirname "$f")"
        git -C "$src_root/third-party/$base_sub" show "$base_commit:$f" > "$dir/$f"
    done
}

# make_patch_dir: fixture-private patch metadata for one dependency
# usage: make_patch_dir PDIR_DIR SUB_NAME FIXTURE_REPO_DIR   (writes base.txt from fixture HEAD, copies patch)
make_patch_dir() {
    local pdir="$1" sub="$2" repo="$3" realdir="$4"
    mkdir -p "$pdir/$sub"
    git -C "$repo" rev-parse HEAD > "$pdir/$sub/base.txt"
    cp "$realdir"/0001-*.patch "$pdir/$sub/"
}

cpptrace_files=(src/platform/platform.hpp src/binary/elf.cpp src/binary/elf.hpp
 src/binary/module_base.cpp src/binary/object.cpp src/binary/safe_dl.cpp
 src/jit/jit_objects.cpp src/jit/jit_objects.hpp
 src/platform/memory_mapping.cpp src/platform/program_name.hpp
 src/symbols/dwarf/dwarf_resolver.cpp src/symbols/dwarf/resolver.hpp
 src/symbols/symbols_with_addr2line.cpp src/symbols/symbols_with_libdwarf.cpp)
cppinterop_file=lib/CppInterOp/Compatibility.h
cpptrace_base="$(cat "$src_root/patches/freebsd/cpptrace/base.txt")"
cppinterop_base="$(cat "$src_root/patches/freebsd/cppinterop/base.txt")"
real_pdir="$src_root/patches/freebsd"

build_fixtures() {
    rm -rf "$tmp/third-party" "$tmp/patchdir"
    mkdir -p "$tmp/third-party"
    seed_files "$tmp/third-party/cpptrace"  "$cpptrace_base"  cpptrace  "${cpptrace_files[@]}"
    seed_files "$tmp/third-party/cppinterop" "$cppinterop_base" cppinterop "$cppinterop_file"
    make_fixture_repo "$tmp/third-party/cpptrace"
    make_fixture_repo "$tmp/third-party/cppinterop"
    make_patch_dir "$tmp/patchdir" cpptrace   "$tmp/third-party/cpptrace"   "$real_pdir/cpptrace"
    make_patch_dir "$tmp/patchdir" cppinterop "$tmp/third-party/cppinterop" "$real_pdir/cppinterop"
}

# Test 1: fresh apply (both deps validated, then applied)
build_fixtures
if "$applier" --source-root "$tmp" --patch-dir "$tmp/patchdir" >/dev/null 2>&1 \
   && grep -q "define IS_FREEBSD 1" "$tmp/third-party/cpptrace/src/platform/platform.hpp" \
   && grep -q "CLANG_VERSION_MAJOR >= 23" "$tmp/third-party/cppinterop/$cppinterop_file"; then
    pass "fresh apply"
else
    fail "fresh apply"
fi

# Test 2: idempotent reapply (no-op, notices "already applied" for both)
out="$("$applier" --source-root "$tmp" --patch-dir "$tmp/patchdir" 2>&1)"
if [ $? -eq 0 ] && [ "$(echo "$out" | grep -c "already applied")" -eq 2 ]; then
    pass "second apply no-op"
else
    fail "second apply no-op (output: $out)"
fi

# Test 3: wrong-base refusal. Fixture is at 2 ahead-of-pin HEAD (recommit), so
# its HEAD no longer matches its own recorded pin. Must refuse even though the
# touched files still match pristine base content exactly.
build_fixtures
git -C "$tmp/third-party/cpptrace" commit -q --allow-empty -m diverge >/dev/null 2>&1
[ "$(git -C "$tmp/third-party/cpptrace" rev-parse HEAD)" != "$(cat "$tmp/patchdir/cpptrace/base.txt")" ] \
    || fail "test3 setup: HEAD unexpectedly still matches pin"
out="$("$applier" --source-root "$tmp" --patch-dir "$tmp/patchdir" 2>&1)"
rc=$?
if [ $rc -ne 0 ] && echo "$out" | grep -q "refusing"; then
    pass "wrong base refusal"
else
    fail "wrong base refusal (rc=$rc output: $out)"
fi

# Test 4: even already-applied patches require the exact pinned HEAD.
build_fixtures
"$applier" --source-root "$tmp" --patch-dir "$tmp/patchdir" >/dev/null 2>&1 \
    || fail "test4 setup: initial patch application failed"
git -C "$tmp/third-party/cpptrace" commit -q --allow-empty -m diverge >/dev/null 2>&1
out="$("$applier" --source-root "$tmp" --patch-dir "$tmp/patchdir" 2>&1)"
rc=$?
if [ $rc -ne 0 ] && echo "$out" | grep -q "does not match pinned base"; then
    pass "wrong base refused when patches already applied"
else
    fail "wrong base refused when patches already applied (rc=$rc output: $out)"
fi

# Test 5: conflicting dirty file at correct base (content preserved)
build_fixtures
printf 'conflicting user contents\n' > "$tmp/third-party/cpptrace/src/binary/elf.cpp"
before="$(md5sum "$tmp/third-party/cpptrace/src/binary/elf.cpp" | cut -d' ' -f1)"
out="$("$applier" --source-root "$tmp" --patch-dir "$tmp/patchdir" 2>&1)"
rc=$?
after="$(md5sum "$tmp/third-party/cpptrace/src/binary/elf.cpp" | cut -d' ' -f1)"
if [ $rc -ne 0 ] && [ "$before" = "$after" ]; then
    pass "conflicting dirty state refusal (no edits lost)"
else
    fail "conflicting dirty state refusal (rc=$rc output: $out)"
fi

# Test 6: --check preflight refuses wrong state (dirty tree from test 5)
out="$("$applier" --check --source-root "$tmp" --patch-dir "$tmp/patchdir" 2>&1)"
if [ $? -ne 0 ]; then
    pass "check preflight refuses wrong state"
else
    fail "check preflight refuses wrong state"
fi

# Test 7: atomic preflight across both dependencies (cppinterop lane missing base fix;
# cpptrace untouched). Build at correct bases, then make cppinterop HEAD diverge.
build_fixtures
git -C "$tmp/third-party/cppinterop" commit -q --allow-empty -m diverge >/dev/null 2>&1
(cd "$tmp" && find third-party/cpptrace -type f ! -path '*/.git/*' | sort | xargs md5sum) > "$tmp/before.txt" 2>/dev/null
out="$("$applier" --source-root "$tmp" --patch-dir "$tmp/patchdir" --verbose 2>&1)"
rc=$?
(cd "$tmp" && find third-party/cpptrace -type f ! -path '*/.git/*' | sort | xargs md5sum) > "$tmp/after.txt" 2>/dev/null
if [ $rc -ne 0 ] && echo "$out" | grep -q "cppinterop" && diff -q "$tmp/before.txt" "$tmp/after.txt" >/dev/null; then
    pass "atomic preflight preserves untouched lane"
else
    fail "atomic preflight preserves untouched lane (rc=$rc output: $out)"
fi

echo
if [ $fails -eq 0 ]; then echo "ALL PATCH APPLIER TESTS PASS"; else echo "$fails TEST(S) FAILED"; exit 1; fi
