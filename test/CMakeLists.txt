# Enable GTest
enable_testing()

option(SPICA_BUILD_TEST "SPICA_BUILD_TEST" ON)
set(GTEST_ROOT $ENV{GTEST_ROOT} CACHE STRING "GTEST_ROOT")

if (${SPICA_BUILD_TEST} STREQUAL "ON")
  # Link directory
  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  if (NOT "${GTEST_ROOT}" STREQUAL "")
    set(GTEST_LIB)
    if (WIN32)
      set(GTEST_LIB "gtest.lib")
      set(GTEST_MAIN_LIB "gtest_main.lib")
    else()
      set(GTEST_LIB "libgtest.a")
      set(GTEST_MAIN_LIB "libgtest_main.a")
    endif()
    set(GTEST_LIBRARY ${GTEST_ROOT}/lib/${GTEST_LIB})
    set(GTEST_MAIN_LIBRARY ${GTEST_ROOT}/lib/${GTEST_MAIN_LIB})
    set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/include)
  else()
    set(GTEST_LIBRARY $ENV{GTEST_LIBRARY})
    set(GTEST_MAIN_LIBRARY $ENV{GTEST_MAIN_LIBRARY})
    set(GTEST_INCLUDE_DIRS $ENV{GTEST_INCLUDE_DIRS})
  endif()

  set(TEST_NAME spica_tests)
  set(SOURCE_FILES
      all_tests.cc
      test_macros.h
      test_stack.cc
      test_geometry.cc
      test_vector.cc
      test_image.cc
      test_color.cc
      test_camera.cc
      test_ray.cc
      test_sampler.cc
      test_trimesh.cc
      test_kdtree.cc
      test_scene.cc
      test_random.cc
      test_renderer_helper.cc
      test_yaml.cc)
    # test_renderer.cc)

  add_executable(${TEST_NAME} ${SOURCE_FILES})
  target_link_libraries(${TEST_NAME} ${GTEST_LIBRARY})
  target_link_libraries(${TEST_NAME} ${GTEST_MAIN_LIBRARY})
  target_link_libraries(${TEST_NAME} "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")

  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set_property(TARGET ${TEST_NAME} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
    configure_file(${SPICA_ROOT_DIRECTORY}/template/project.vcxproj.user.in "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.vcxproj.user" @ONLY)
  endif()

  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose DEPENDS ${TEST_NAME})

  include_directories(${CMAKE_CURRENT_LIST_DIR})
  include_directories(${GTEST_INCLUDE_DIRS})
endif()
