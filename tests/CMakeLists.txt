if (${SPICA_BUILD_TESTS})
  message(STATUS "[spica] Building unit tests.")

  enable_testing()
  include_directories(${SPICA_ROOT_DIR}/sources)
  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  configure_file(${CMAKE_CURRENT_LIST_DIR}/test_params.h.in
                 ${CMAKE_CURRENT_LIST_DIR}/test_params.h @ONLY)

  if(WIN32)
    set(GTEST_ROOT "GTEST_ROOT" CACHE PATH "GTEST_ROOT")
    if (NOT EXISTS "${GTEST_ROOT}/include/gtest/gtest.h")
      message(FATAL_ERROR "[ERROR] Google Test not found!!")
    endif()
  else()
    set(GTEST_ROOT "/usr/local")
  endif()

  set(GTEST_INCLUDE_DIRS "${GTEST_ROOT}/include")
  set(GTEST_LIBRARY_DIRS "${GTEST_ROOT}/lib")
  if (WIN32)
    set(GTEST_LIBRARIES
        "$<$<CONFIG:Release>:${GTEST_LIBRARY_DIRS}/${LIB_PREFIX}gtest${LIB_SUFFIX}>"
        "$<$<CONFIG:Release>:${GTEST_LIBRARY_DIRS}/${LIB_PREFIX}gtest_main${LIB_SUFFIX}>"
        "$<$<CONFIG:Debug>:${GTEST_LIBRARY_DIRS}/Debug/${LIB_PREFIX}gtest${LIB_SUFFIX}>"
        "$<$<CONFIG:Debug>:${GTEST_LIBRARY_DIRS}/Debug/${LIB_PREFIX}gtest_main${LIB_SUFFIX}>")
  else()
    set(GTEST_LIBRARIES
        "${GTEST_LIBRARY_DIRS}/${LIB_PREFIX}gtest.a"
        "${GTEST_LIBRARY_DIRS}/${LIB_PREFIX}gtest_main.a")
  endif()

  set(TEST_NAME spica_tests)
  set(SOURCE_FILES
      all_tests.cc
      test_params.h
#      test_stack.cc
      test_geometry.cc
      test_point3d.cc
      test_normal3d.cc
      test_vector2d.cc
      test_vector3d.cc
      test_matrix4x4.cc
      test_transform.cc
      test_quaternion.cc
      test_image.cc
      test_color.cc
#      test_camera.cc
#      test_light.cc
      test_ray.cc
#      test_sampler.cc
#      test_trimesh.cc
#      test_kdtree.cc
      # test_scene.cc)
#      test_random.cc
#      test_renderer_helper.cc
#      test_path.cc
)

  add_definitions(-DGTEST_LANG_CXX11)
  add_executable(${TEST_NAME} ${SOURCE_FILES})
  add_dependencies(${TEST_NAME} ${SPICA_LIBCORE})
  target_link_libraries(${TEST_NAME} ${GTEST_LIBRARIES})
  target_link_libraries(${TEST_NAME} ${SPICA_LIBCORE})

  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set_property(TARGET ${TEST_NAME} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
  endif()

  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose --gtest_shuffle DEPENDS ${TEST_NAME})

  include_directories(${CMAKE_CURRENT_LIST_DIR})
  include_directories(${GTEST_INCLUDE_DIRS})
endif()
