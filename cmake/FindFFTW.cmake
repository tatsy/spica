include(FindPackageHandleStandardArgs)

if (WIN32)
  find_path(FFTW_INCLUDE_DIR
            NAMES fftw3.h
            PATHS
            ${FFTW_ROOT_DIR}/include)

  find_library(FFTW_LIBRARY
               NAMES fftw3.lib
               PATHS
               ${HALIDE_ROOT_DIR}/Release)
else()
  find_path(FFTW_INCLUDE_DIR
            NAMES fftw3.h
            PATHS
            /usr/include
            /usr/local/include
            /opt/local/include)

  find_library(FFTW_LIBRARY
               NAMES libfftw3.a
               PATHS
               /usr/lib
               /usr/local/lib
               /usr/lib/x86_64-linux-gnu)
endif()

find_package_handle_standard_args(
  FFTW DEFAULT_MSG
  FFTW_INCLUDE_DIR
  FFTW_LIBRARY
)

if (FFTW_FOUND)
  set(FFTW_INCLUDE_DIRS ${FFTW_INCLUDE_DIR})
  set(FFTW_LIBRARIES ${FFTW_LIBRARY})
else()
  set(FFTW_ROOT_DIR "HALIDE_ROOT_DIR" CACHE PATH "")
endif()

mark_as_advanced(FFTW_INCLUDE_DIR FFTW_LIBRARY)
