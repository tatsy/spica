option(SPICA_BUILD_VIEWER "BUILD_VIEWER" ON)

if (${SPICA_BUILD_VIEWER} STREQUAL "ON")
  set(BUILD_TARGET spica_viewer)

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

  add_executable(${BUILD_TARGET} ${SOURCES} ${HEADERS} ${SHADERS})
  add_dependencies(${BUILD_TARGET} ${SPICA_TARGET_NAME})

  if (MSVC)
    source_group(Sources FILES ${SOURCES} ${HEADERS})
    source_group(Shaders FILES ${SHADERS})
    source_group("Qt Files" FILES ${QT_GEN_FILES})
  endif()

  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set_property(TARGET ${BUILD_TARGET} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
  endif()

  qt5_use_modules(${BUILD_TARGET} Widgets OpenGL)

  target_link_libraries(${BUILD_TARGET} ${SPICA_LIBS})
  target_link_libraries(${BUILD_TARGET} ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

  set(QT_LIB_LIST Qt5Widgets Qt5Core Qt5Gui Qt5OpenGL)
  if (WIN32)
    set(QT_LIB_LIST ${QT_LIB_LIST} icuin54 icuuc54 icudt54)
  endif()

  if (MSVC)
    foreach (qtlib ${QT_LIB_LIST})
      add_custom_command(TARGET ${BUILD_TARGET} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy
                         ARGS ${CMAKE_PREFIX_PATH}/bin/${qtlib}${BUILD_TYPE_SUFFIX}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endforeach(qtlib)
  endif()

endif()
