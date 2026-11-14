#!/bin/sh
# Runs inside the FreeBSD container test image. Boots FreeBSD under QEMU, copies the
# read-only jank checkout from /src into it, then provisions, builds, tests and packages.
#
# Mounts:
#   /src    jank repository (read-only, with submodules checked out)
#   /cache  persistent cache for the FreeBSD image
#   /out    results: the .pkg artifact and logs
#
# Environment:
#   FREEBSD_RELEASE   (default 14.4)
#   FREEBSD_IMAGE_URL (default: the official BASIC-CLOUDINIT ufs qcow2 for the release)
#   VM_CPUS, VM_MEM   (default 4, 8192 MiB)
#   VM_DISK           (default 60G)
#   KEEP_VM=1         leave the VM running on exit, for debugging over port 2222
#   BUILD_ONLY=1      build without the test suite or packaging
#   JOBS              parallel build jobs inside the VM (default: VM_CPUS)
set -eu

release="${FREEBSD_RELEASE:-14.4}"
base_url="https://download.freebsd.org/releases/VM-IMAGES/${release}-RELEASE/amd64/Latest"
image_name="FreeBSD-${release}-RELEASE-amd64-BASIC-CLOUDINIT-ufs.qcow2.xz"
image_url="${FREEBSD_IMAGE_URL:-$base_url/$image_name}"
cpus="${VM_CPUS:-4}"
mem="${VM_MEM:-8192}"
disk="${VM_DISK:-60G}"
jobs="${JOBS:-$cpus}"
build_only="${BUILD_ONLY:-0}"
ssh_port=2222

log() { printf '[freebsd-vm-test] %s\n' "$*" >&2; }
die() {
  log "error: $*"
  if [ -s /out/console.log ]; then
    log "last lines of the VM console:"
    tail -n 20 /out/console.log >&2 || true
  fi
  exit 1
}

[ -f /src/compiler+runtime/CMakeLists.txt ] || die "mount the jank repository at /src"
mkdir -p /cache /out
# Scratch space (download, decompressed image, VM overlay disk) lives under /cache rather
# than the container's writable layer, so --cache DIR moves all the bulk off the engine's
# storage, which is often a small root filesystem.
rm -rf /cache/work.* 2>/dev/null || true
work="$(mktemp -d /cache/work.XXXXXX)"

# ---- Image, verified against the release checksums. ----
qcow="/cache/${image_name%.xz}"
if [ ! -f "$qcow" ]; then
  log "downloading $image_url"
  curl -fSL --retry 3 --progress-bar -o "$work/$image_name" "$image_url"
  if [ -z "${FREEBSD_IMAGE_URL:-}" ]; then
    curl -fsSL --retry 3 -o "$work/CHECKSUM.SHA256" "$base_url/CHECKSUM.SHA256"
    expected="$(sed -n "s/^SHA256 ($image_name) = //p" "$work/CHECKSUM.SHA256")"
    [ -n "$expected" ] || die "no checksum for $image_name"
    actual="$(sha256sum "$work/$image_name" | cut -d' ' -f1)"
    [ "$expected" = "$actual" ] || die "checksum mismatch for $image_name"
    log "checksum ok"
  fi
  xz -d "$work/$image_name"
  mv "$work/${image_name%.xz}" "$qcow"
fi

# A throwaway overlay, so the cached image stays pristine.
qemu-img create -q -f qcow2 -F qcow2 -b "$qcow" "$work/disk.qcow2" "$disk"

# ---- cloud-init (nuageinit) seed: root SSH access with a fresh key. ----
ssh-keygen -q -t ed25519 -N '' -f "$work/id"
pubkey="$(cat "$work/id.pub")"
mkdir "$work/seed"
printf 'instance-id: jank-%s\nlocal-hostname: jank-freebsd\n' "$(date +%s)" >"$work/seed/meta-data"
cat >"$work/seed/user-data" <<EOF
#cloud-config
ssh_authorized_keys:
  - $pubkey
runcmd:
  - mkdir -p -m 700 /root/.ssh
  - echo '$pubkey' > /root/.ssh/authorized_keys
  - chmod 600 /root/.ssh/authorized_keys
  - echo 'PermitRootLogin prohibit-password' >> /etc/ssh/sshd_config
  - sysrc sshd_enable=YES
  - service sshd restart
EOF
genisoimage -quiet -output "$work/seed.iso" -volid cidata -joliet -rock "$work/seed"

# ---- Boot. ----
accel="tcg"
if [ -w /dev/kvm ]; then
  accel="kvm"
else
  log "warning: /dev/kvm unavailable; falling back to emulation, which is extremely slow"
fi

log "booting FreeBSD $release ($accel, ${cpus} cpus, ${mem} MiB)"
qemu-system-x86_64 \
  -accel "$accel" -cpu max -smp "$cpus" -m "$mem" \
  -drive "file=$work/disk.qcow2,if=virtio,format=qcow2" \
  -drive "file=$work/seed.iso,media=cdrom,readonly=on" \
  -netdev "user,id=net0,hostfwd=tcp:127.0.0.1:${ssh_port}-:22" \
  -device virtio-net-pci,netdev=net0 \
  -display none -serial "file:/out/console.log" \
  -daemonize -pidfile "$work/qemu.pid"

vm() {
  ssh -i "$work/id" -p "$ssh_port" -o StrictHostKeyChecking=no \
    -o UserKnownHostsFile=/dev/null -o LogLevel=ERROR -o ConnectTimeout=10 \
    -o ServerAliveInterval=30 root@127.0.0.1 "$@"
}

shutdown_vm() {
  status=$?
  if [ "${KEEP_VM:-0}" = "1" ]; then
    log "KEEP_VM=1: leaving the VM running (ssh -p $ssh_port root@127.0.0.1, key $work/id)"
    sleep infinity
  fi
  vm 'shutdown -p now' >/dev/null 2>&1 || true
  sleep 5
  [ -f "$work/qemu.pid" ] && kill "$(cat "$work/qemu.pid")" 2>/dev/null || true
  rm -rf "$work"
  exit "$status"
}
trap shutdown_vm EXIT

log "waiting for ssh"
tries=0
until vm true 2>/dev/null; do
  tries=$((tries + 1))
  [ "$tries" -lt 120 ] || die "VM never became reachable over ssh; see /out/console.log"
  sleep 5
done
log "VM is up: $(vm uname -sr)"

# ---- Copy the checkout in. The submodules' git metadata is needed for patching. ----
log "copying /src into the VM"
tar -C /src -cf - \
  --exclude='./compiler+runtime/build' --exclude='./compiler+runtime/build-*' \
  --exclude='./compiler+runtime/pkg-*' --exclude='./freebsd-out' . \
  | vm 'mkdir -p /root/jank && tar --no-same-owner -C /root/jank -xf -'

# ---- Provision, build, test, package. ----
# git refuses repositories owned by another uid ("dubious ownership"), which the patch
# applier would misreport as a missing worktree. git itself only exists after provisioning.
trust_checkout="git config --global --add safe.directory '*'"

run_in_vm() { # log-name command
  log "$1"
  vm "cd /root/jank/compiler+runtime && $2" 2>&1 | tee "/out/$1.log"
  # POSIX sh has no pipefail; fetch the remote status separately.
  vm "test -f /root/jank/.status-$1" || die "$1 failed; see /out/$1.log"
}

if [ "$build_only" = "1" ]; then
  run_in_vm provision 'bin/freebsd-provision --build-only && touch ../.status-provision'
  run_in_vm build "$trust_checkout && bin/freebsd-build --build-only --jobs $jobs && touch ../.status-build"
else
  run_in_vm provision 'bin/freebsd-provision && touch ../.status-provision'
  run_in_vm build "$trust_checkout && bin/freebsd-build --jobs $jobs && touch ../.status-build"
  run_in_vm smoke 'test/freebsd/smoke build-freebsd /root/pkg-out && touch ../.status-smoke'
  vm 'tar -C /root/pkg-out -cf - .' | tar -C /out -xf -
fi

log "PASS"
