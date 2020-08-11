find_package(PkgConfig)

pkg_check_modules(PC_systemd QUIET systemd)

set(systemd_DEFINITIONS ${PC_systemd_CFLAGS_OTHER})
set(systemd_VERSION ${PC_systemd_VERSION})
if (${PC_systemd_SERVICES_INSTALL_DIR})
  set(systemd_SERVICES_INSTALL_DIR ${PC_systemd_SERVICES_INSTALL_DIR})
else ()
  execute_process(
    COMMAND "${PKG_CONFIG_EXECUTABLE}" --variable=systemdsystemunitdir systemd
    OUTPUT_VARIABLE systemd_SERVICES_INSTALL_DIR)
  string(STRIP "${systemd_SERVICES_INSTALL_DIR}" systemd_SERVICES_INSTALL_DIR)
endif ()

find_path(
  systemd_INCLUDE_DIR
  NAMES systemd/sd-bus.h systemd/sd-event.h
  HINTS ${PC_systemd_INCLUDE_DIRS})

if (VAST_STATIC_EXECUTABLE)
  list(APPEND systemd_LIB_NAMES
       "${CMAKE_STATIC_LIBRARY_PREFIX}systemd${CMAKE_STATIC_LIBRARY_SUFFIX}")
else ()
  list(APPEND systemd_LIB_NAMES systemd)
endif ()
find_library(
  systemd_LIBRARY
  NAMES ${systemd_LIB_NAMES}
  HINTS ${PC_systemd_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  systemd
  FOUND_VAR systemd_FOUND
  REQUIRED_VARS systemd_LIBRARY systemd_INCLUDE_DIR
  VERSION_VAR systemd_VERSION)

if (systemd_FOUND AND NOT TARGET systemd::systemd)
  add_library(systemd::systemd UNKNOWN IMPORTED)
  set_target_properties(
    systemd::systemd
    PROPERTIES
      IMPORTED_LOCATION "${systemd_LIBRARY}" INTERFACE_COMPILE_OPTIONS
                                             "${systemd_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${systemd_INCLUDE_DIR}")
endif ()

mark_as_advanced(systemd_INCLUDE_DIR systemd_LIBRARY)
