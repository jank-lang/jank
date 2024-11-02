set(FMT_DOC OFF CACHE BOOL "Generate docs")
set(FMT_INSTALL OFF CACHE BOOL "Generate the install target.")

add_subdirectory(third-party/fmt)

unset(FMT_DOC)
unset(FMT_INSTALL)
