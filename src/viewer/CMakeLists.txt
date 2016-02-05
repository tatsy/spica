option(SPICA_BUILD_VIEWER "BUILD_VIEWER" ON)

if (${SPICA_BUILD_VIEWER} STREQUAL "ON")

  if (WIN32)
    set(QT5_ROOT "QT5_ROOT" CACHE PATH "QT5_ROOT")
    if (NOT EXISTS "${QT5_ROOT}/lib/cmake")
      message(FATAL_ERROR "[ERROR] Qt5 not found!!")
    endif()
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_ROOT})
  endif()

  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5OpenGL REQUIRED)
  find_package(OpenGL REQUIRED)

  # if (WIN32)
  #   set(Your_freeglut_DIR "C:/CLibrary/glut/" CACHE STRING "Your_OpenGL_DIR")
  #   set(GLUT_INCLUDE_DIR "${Your_freeglut_DIR}/include/")
  #   set(GLUT_LIBRARY_DIR "${Your_freeglut_DIR}/lib/x64/")
  #   set(GLUT_LIBRARY freeglut.lib)
  #   link_directories(${GLUT_LIBRARY_DIR})
  #   include_directories(${GLUT_INCLUDE_DIR})
  # else()
  #   find_package(X11 REQUIRED)
  #   set(GLUT_LIBRARY libglut.a ${X11_LIBRARIES})
  #   if (X11_xf86vmode_FOUND)
  #     set(GLUT_LIBRARY ${GLUT_LIBRARY} ${X11_Xxf86vm_LIB})
  #   endif()
  #   if (X11_Xi_FOUND)
  #     set(GLUT_LIBRARY ${GLUT_LIBRARY} ${X11_Xi_LIB})
  #   endif()
  # endif()

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  set(SOURCES main.cc
              render_thread.cc
              scene_viewer.cc
              scene_viewer_slots.cc
              render_param_widget.cc
              qgl_render_widget.cc)

  set(HEADERS scene_viewer.h
              render_thread.h
              render_param_widget.h
              qgl_render_widget.h)

  set(SHADERS blinn_phong.vs blinn_phong.fs)

  set(QT_GEN_FILES ${CMAKE_CURRENT_BINARY_DIR}/scene_viewer_automoc.cpp)

  add_executable(scene_viewer ${SOURCES} ${HEADERS} ${SHADERS})

  if (MSVC)
    source_group(Sources FILES ${SOURCES} ${HEADERS})
    source_group(Shaders FILES ${SHADERS})
    source_group("Qt Files" FILES ${QT_GEN_FILES})
  endif()


  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set_property(TARGET scene_viewer APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
  endif()


  qt5_use_modules(scene_viewer Widgets OpenGL)

  target_link_libraries(scene_viewer "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")
  target_link_libraries(scene_viewer ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

  set(QT_LIB_LIST Qt5Widgets Qt5Core Qt5Gui Qt5OpenGL)
  if (WIN32)
    set(QT_LIB_LIST ${QT_LIB_LIST} icuin54 icuuc54 icudt54)
  endif()

  if (MSVC)
    foreach (qtlib ${QT_LIB_LIST})
      add_custom_command(TARGET scene_viewer POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy
                         ARGS ${CMAKE_PREFIX_PATH}/bin/${qtlib}${BUILD_TYPE_SUFFIX}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endforeach(qtlib)
  endif()

endif()
