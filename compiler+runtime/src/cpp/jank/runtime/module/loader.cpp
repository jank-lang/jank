#ifdef __MINGW64__
  // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
  #define WIN32_LEAN_AND_MEAN 1
  #include <windows.h>
#else
  #include <sys/mman.h>
#endif
#include <fcntl.h>
#include <unistd.h>

#include <filesystem>

#include <jank/type.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/atom.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/profile/time.hpp>
#include <jank/error/runtime.hpp>
#include <jank/error/report.hpp>
#include <jank/util/path.hpp>
#include <jank/util/environment.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime::module
{
  object_ref file_entry::to_runtime_data() const
  {
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("module::file_entry"),
      make_box("archive_path"),
      jank::detail::to_runtime_data(archive_path),
      make_box("path"),
      make_box(path));
  }

  std::filesystem::file_time_type file_entry::last_modified_at() const
  {
    auto const source_path{ archive_path.unwrap_or(path) };
    return std::filesystem::last_write_time(native_transient_string{ source_path });
  }

  file_view::file_view(file_view &&mf) noexcept
    : fd{ mf.fd }
    , head{ mf.head }
    , len{ mf.len }
    , buff{ jtl::move(mf.buff) }
    , file{ jtl::move(mf.file) }
  {
    mf.fd.reset();
    mf.head = nullptr;
  }

  file_view::file_view(jtl::immutable_string const &file,
                       file_handle const f,
                       char const * const h,
                       usize const s)
    : fd{ f }
    , head{ h }
    , len{ s }
    , file{ file }
  {
  }

  file_view::file_view(jtl::immutable_string const &file, jtl::immutable_string const &buff)
    : buff{ buff }
    , file{ file }
  {
  }

  file_view::~file_view()
  {
    reset();
  }

  file_view &file_view::operator=(file_view &&fv) noexcept
  {
    if(this == &fv)
    {
      return *this;
    }

    reset();
    fd = fv.fd;
    head = fv.head;
    len = fv.len;
    buff = jtl::move(fv.buff);
    file = jtl::move(fv.file);

    fv.reset();
    fv.head = nullptr;

    return *this;
  }

  char const *file_view::data() const
  {
    return buff.empty() ? head : buff.data();
  }

  usize file_view::size() const
  {
    return buff.empty() ? len : buff.size();
  }

  jtl::immutable_string const &file_view::file_path() const
  {
    return file;
  }

  jtl::immutable_string_view file_view::view() const
  {
    return { data(), size() };
  }

  void file_view::reset()
  {
    if(head != nullptr)
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): I want const everywhere else. */
    {
#ifdef JANK_WINDOWS_LIKE
      UnmapViewOfFile(head);
#else
      munmap(reinterpret_cast<void *>(const_cast<char *>(head)), len);
#endif
      head = nullptr;
    }
    if(fd)
    {
#ifdef JANK_WINDOWS_LIKE
      CloseHandle(fd->hMapping);
      CloseHandle(fd->hFile);
#else
      ::close(*fd);
#endif
      fd.reset();
    }
  }

  static jtl::result<file_view, error_ref> map_file(jtl::immutable_string const &path)
  {
    if(!std::filesystem::exists(path.c_str()))
    {
      return error::runtime_unable_to_open_file(util::format("File '{}' doesn't exist.", path));
    }

#ifdef JANK_WINDOWS_LIKE
    HANDLE hFile{ CreateFileA(path.c_str(),
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              nullptr,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr) };

    if(hFile == INVALID_HANDLE_VALUE)
    {
      return error::runtime_unable_to_open_file(util::format("Unable to open file '{}'.", path));
    }

    LARGE_INTEGER fileSize{};
    if(!GetFileSizeEx(hFile, &fileSize))
    {
      CloseHandle(hFile);
      return error::internal_runtime_failure("Failed to get file size");
    }

    HANDLE hMapping{ CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr) };

    if(!hMapping)
    {
      CloseHandle(hFile);
      return error::internal_runtime_failure("Failed to create file mapping");
    }

    auto head{ static_cast<char const *>(MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0)) };
    if(!head)
    {
      CloseHandle(hMapping);
      CloseHandle(hFile);
      return error::internal_runtime_failure("Failed to map view of file");
    }

    return ok(
      file_view{ path, HANDLES(hFile, hMapping), head, static_cast<size_t>(fileSize.QuadPart) });
#else
    auto const file_size(std::filesystem::file_size(path.c_str()));
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const fd(::open(path.c_str(), O_RDONLY));
    if(fd < 0)
    {
      return error::runtime_unable_to_open_file(util::format("Unable to open file '{}'.", path));
    }
    auto const head(
      reinterpret_cast<char const *>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0)));

    /* MAP_FAILED is a macro which does a C-style cast. */
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wold-style-cast"
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast,performance-no-int-to-ptr) */
    if(head == MAP_FAILED)
  #pragma clang diagnostic pop
    {
      return error::runtime_unable_to_open_file(util::format("Unable to map file '{}'.", path));
    }

    return ok(file_view{ path, fd, head, file_size });
#endif
  }

  jtl::result<file_view, error_ref> loader::read_file(jtl::immutable_string const &path)
  {
    if(path == read::no_source_path)
    {
      return error::internal_runtime_failure("No source to read.");
    }

    if(path.contains(".jar:"))
    {
      return read_jar_file(path);
    }
    return map_file(path);
  }

  bool loader::is_loaded(jtl::immutable_string const &module)
  {
    auto const atom{
      runtime::try_object<runtime::obj::atom>(__rt_ctx->loaded_libs_var->deref())->deref()
    };

    auto const loaded_libs{ runtime::try_object<runtime::obj::persistent_sorted_set>(atom) };
    auto const ret{ truthy(loaded_libs->contains(make_box<obj::symbol>(module))) };

    //util::println("is module loaded {}: {}", module, ret);

    return ret;
  }

  static native_set<jtl::immutable_string> const &core_modules()
  {
    /* TODO: Pass this in from CMake, so it's not duplicated. */
    static native_set<jtl::immutable_string> const modules{ "cpp",
                                                            "clojure.core",
                                                            "clojure.core-native",
                                                            "clojure.string",
                                                            "clojure.string-native",
                                                            "clojure.walk",
                                                            "jank.perf-native",
                                                            "jank.compiler-native",
                                                            "jank.nrepl.server.inspect",
                                                            "jank.nrepl.server.core",
                                                            "jank.nrepl.server.handler",
                                                            "jank.nrepl.server.bencode",
                                                            "jank.nrepl.server.capture",
                                                            "jank.nrepl.server.util",
                                                            "jank.nrepl.server.eval",
                                                            "jank.nrepl.server.parsec",
                                                            "jank.nrepl.server.handler.close",
                                                            "jank.nrepl.server.handler.clone",
                                                            "jank.nrepl.server.handler.describe",
                                                            "jank.nrepl.server.handler.completions",
                                                            "jank.nrepl.server.handler.eval",
                                                            "jank.nrepl.server.handler.lookup" };
    return modules;
  }

  bool is_core_module(jtl::immutable_string const &module)
  {
    return core_modules().contains(module);
  }

  void loader::set_is_loaded(jtl::immutable_string const &module)
  {
    auto const loaded_libs_atom{ runtime::try_object<runtime::obj::atom>(
      __rt_ctx->loaded_libs_var->deref()) };

    auto const swap_fn{ [&](object_ref const curr_val) {
      return runtime::try_object<runtime::obj::persistent_sorted_set>(curr_val)->conj(
        make_box<obj::symbol>(module));
    } };

    auto const swap_fn_wrapper{ make_box<runtime::obj::native_function_wrapper>(
      std::function<object_ref(object_ref const)>{ swap_fn }) };
    loaded_libs_atom->swap(swap_fn_wrapper);
  }

  void loader::add_load_fn(jtl::immutable_string const &module, jtl::ref<void()> const fn)
  {
    auto const locked_state{ state.lock() };
    locked_state->managed_load_fns.emplace(module, fn);
  }

  object_ref loader::to_runtime_data() const
  {
    auto const locked_state{ state.lock() };
    runtime::object_ref entry_maps(make_box<runtime::obj::persistent_array_map>());
    for(auto const &e : locked_state->entries)
    {
      entry_maps = runtime::assoc(entry_maps,
                                  make_box(e.first),
                                  runtime::obj::persistent_array_map::create_unique(
                                    make_box("jank"),
                                    jank::detail::to_runtime_data(e.second.jank),
                                    make_box("cljc"),
                                    jank::detail::to_runtime_data(e.second.cljc),
                                    make_box("cpp"),
                                    jank::detail::to_runtime_data(e.second.cpp),
                                    make_box("o"),
                                    jank::detail::to_runtime_data(e.second.o)));
    }

    return runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                             make_box("module::loader"),
                                                             make_box("entries"),
                                                             entry_maps,
                                                             make_box("paths"),
                                                             make_box(locked_state->paths));
  }
}
