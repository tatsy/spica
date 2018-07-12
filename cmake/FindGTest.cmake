include(FindPackageHandleStandardArgs)

set(GTEST_DIR "FFTW_ROOT_DIR" CACHE PATH "")

if (WIN32)
    find_path(GTEST_INCLUDE_DIR
              NAMES gtest/gtest.h
              PATHS
              ${GTEST_DIR}
              ${GTEST_DIR}/include)

    find_library(GTEST_LIBRARY
                 NAMES gtest
                 PATHS
                 ${GTEST_DIR}
                 ${GTEST_DIR}/lib
                 ${GTEST_DIR}/lib/Release)

    find_library(GTEST_MAIN_LIBRARY
                 NAMES gtest_main
                 PATHS
                 ${GTEST_DIR}
                 ${GTEST_DIR}/lib
                 ${GTEST_DIR}/lib/Release)
else()
    find_path(GTEST_INCLUDE_DIR
              NAMES gtest/gtest.h
              PATHS
              /usr/include
              /usr/local/include
              ${GTEST_DIR}
              ${GTEST_DIR}/include)

    find_library(GTEST_LIBRARY
                 NAMES gtest
                 PATHS
                 /usr/lib
                 /usr/local/lib
                 /usr/lib/x86_64-linux-gnu
                 ${GTEST_DIR}
                 ${GTEST_DIR}/lib)

    find_library(GTEST_MAIN_LIBRARY
                 NAMES gtest_main
                 PATHS
                 /usr/lib
                 /usr/local/lib
                 /usr/lib/x86_64-linux-gnu
                 ${GTEST_DIR}
                 ${GTEST_DIR}/lib
                 ${GTEST_DIR}/lib/Release)
endif()

find_package_handle_standard_args(
    GTEST DEFAULT_MSG
    GTEST_INCLUDE_DIR
    GTEST_LIBRARY
    GTEST_MAIN_LIBRARY
)

if (GTEST_FOUND)
    set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})
    set(GTEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
    unset(GTEST_DIR)
endif()

mark_as_advanced(GTEST_INCLUDE_DIR GTEST_LIBRARY GTEST_MAIN_LIBRARY)
