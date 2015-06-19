option(BUILD_VIEWER "BUILD_VIEWER" ON)

if (${BUILD_VIEWER} STREQUAL "ON")

  if (WIN32)
    # set(Your_Qt5_DIR "Your_Qt5_DIR" CACHE STRING "Your_Qt5_DIR")
    # set(Your_freeglut_DIR "Your_freeglut_DIR" CACHE STRING "Your_freeglut_DIR")
    set(Your_Qt5_DIR "C:/Qt/5.4/msvc2013_64_opengl/")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${Your_Qt5_DIR})
  endif()

  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5OpenGL REQUIRED)
  find_package(OpenGL REQUIRED)

  if (WIN32)
    set(Your_freeglut_DIR "C:/CLibrary/glut/" CACHE STRING "Your_OpenGL_DIR")
    set(GLUT_INCLUDE_DIR "${Your_freeglut_DIR}/include/")
    set(GLUT_LIBRARY_DIR "${Your_freeglut_DIR}/lib/x64/")
    set(GLUT_LIBRARY freeglut.lib)
    link_directories(${GLUT_LIBRARY_DIR})
    include_directories(${GLUT_INCLUDE_DIR})
  else()
    find_package(X11 REQUIRED)
    set(GLUT_LIBRARY libglut.a ${X11_LIBRARIES} ${X11_Xxf86vm_LIB} ${X11_Xi_LIB})
  endif()

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})


  add_executable(scene_viewer main.cc
                 scene_viewer.cc
                 scene_viewer.h
                 qgl_render_widget.cc
                 qgl_render_widget.h
                 blinn_phong.vs
                 blinn_phong.fs)

  configure_file(${SPICA_ROOT_DIRECTORY}/template/project.vcxproj.user.in "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scene_viewer.vcxproj.user" @ONLY)

  qt5_use_modules(scene_viewer Widgets OpenGL)

  target_link_libraries(scene_viewer "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")
  target_link_libraries(scene_viewer ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

  set(QT_LIB_LIST Qt5Widgets Qt5Core Qt5Gui Qt5OpenGL)
  if (WIN32)
    set(QT_LIB_LIST ${QT_LIB_LIST} icuin53 icuuc53 icudt53)
  endif()

  if (MSVC)
    foreach (qtlib ${QT_LIB_LIST})
      add_custom_command(TARGET scene_viewer POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy
                         ARGS ${CMAKE_PREFIX_PATH}bin/${qtlib}${BUILD_TYPE_SUFFIX}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endforeach(qtlib)
  endif()

endif()
