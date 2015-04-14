# Enable GTest
enable_testing()

# Link directory
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Function to add GTest
if (NOT "$ENV{GTEST_ROOT}" STREQUAL "")
  set(GTEST_LIB)
  if (WIN32)
    set(GTEST_LIB "gtest.lib")
    set(GTEST_MAIN_LIB "gtest_main.lib")
  else(WIN32)
    set(GTEST_LIB "libgtest.a")
    set(GTEST_MAIN_LIB "libgtest_main.a")
  endif(WIN32)
  set(GTEST_LIBRARY $ENV{GTEST_ROOT}/lib/${GTEST_LIB})
  set(GTEST_MAIN_LIBRARY $ENV{GTEST_ROOT}/lib/${GTEST_MAIN_LIB})
  set(GTEST_INCLUDE_DIRS $ENV{GTEST_ROOT}/include)
else()
  set(GTEST_LIBRARY $ENV{GTEST_LIBRARY})
  set(GTEST_MAIN_LIBRARY $ENV{GTEST_MAIN_LIBRARY})
  set(GTEST_INCLUDE_DIRS $ENV{GTEST_INCLUDE_DIRS})
endif()

function (add_gtest test_name test_source)
  set(SOURCE_FILES ${test_source})

  add_executable(${test_name} ${SOURCE_FILES})
  target_link_libraries(${test_name} ${GTEST_LIBRARY})
  target_link_libraries(${test_name} ${GTEST_MAIN_LIBRARY})
  target_link_libraries(${test_name} "${LIB_PREFIX}rainy_renderer${LIB_SUFFIX}")

  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction(add_gtest)

add_gtest(test_Vector3 test_Vector3.cc)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS test_Vector3)

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${GTEST_INCLUDE_DIRS})
