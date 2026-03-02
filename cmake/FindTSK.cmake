# cmake/FindTSK.cmake
find_path(TSK_INCLUDE_DIR NAMES tsk/libtsk.h)
find_library(TSK_LIBRARY NAMES tsk libtsk)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TSK DEFAULT_MSG TSK_LIBRARY TSK_INCLUDE_DIR)

if(TSK_FOUND AND NOT TARGET TSK::TSK)
    add_library(TSK::TSK UNKNOWN IMPORTED)
    set_target_properties(TSK::TSK PROPERTIES
            IMPORTED_LOCATION "${TSK_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${TSK_INCLUDE_DIR}"
    )
endif()