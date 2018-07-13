if (${SPICA_BUILD_TESTS})
    message(STATUS "[spica] Building unit tests.")

    find_package(GTest REQUIRED)

    enable_testing()
    include_directories(${SPICA_ROOT_DIR}/sources)
    link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

    configure_file(${CMAKE_CURRENT_LIST_DIR}/test_params.h.in
                 ${CMAKE_CURRENT_LIST_DIR}/test_params.h @ONLY)

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

    if (LINUX)
        target_link_libraries(${TEST_NAME} ${CMAKE_FS_LIBS} ${CMAKE_DL_LIBS})
    endif()

    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
        set_property(TARGET ${TEST_NAME} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
    endif()

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose --gtest_shuffle DEPENDS ${TEST_NAME})

    include_directories(${CMAKE_CURRENT_LIST_DIR})
    include_directories(${GTEST_INCLUDE_DIRS})
endif()
