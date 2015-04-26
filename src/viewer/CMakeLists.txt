option(BUILD_VIEWER "BUILD_VIEWER" ON)

if (${BUILD_VIEWER} STREQUAL "ON")

  if (MSVC)
    set(CMAKE_PREFIX_PATH "C:/Qt/5.4/msvc2013_64_opengl/" CACHE STRING "CMAKE_PREFIX_PATH")
  endif()

  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5OpenGL REQUIRED)
  find_package(OpenGL REQUIRED)

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  add_executable(scene_viewer main.cc
                 scene_viewer.cc
                 scene_viewer.h
                 qgl_render_widget.cc
                 qgl_render_widget.h)

  qt5_use_modules(scene_viewer Widgets OpenGL)

  target_link_libraries(scene_viewer "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")
  target_link_libraries(scene_viewer ${QT_LIBRARIES} ${OPENGL_LIBRARIES})

  set(QT_LIB_LIST Qt5Widgets Qt5Core Qt5Gui)

  if (MSVC)
    foreach (qtlib ${QT_LIB_LIST})
      add_custom_command(TARGET scene_viewer POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy
                         ARGS ${CMAKE_PREFIX_PATH}bin/${qtlib}${BUILD_TYPE_SUFFIX}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endforeach(qtlib)
  endif()

endif()
