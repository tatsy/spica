if (${SPICA_BUILD_GUI})
  message(STATUS "[spica] Building GUI.")
  set(BUILD_TARGET spica_viewer)

  set(QT5_ROOT "QT5_ROOT" CACHE PATH "QT5_ROOT")
  if (NOT EXISTS "${QT5_ROOT}/lib/cmake")
    message(FATAL_ERROR "[ERROR] Qt5 not found!!")
  endif()
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_ROOT})

  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5OpenGL REQUIRED)
  find_package(OpenGL REQUIRED)

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  configure_file(shader_location.in.h
                 ${CMAKE_CURRENT_LIST_DIR}/shader_location.h
                 @ONLY)

  set(SOURCES ${SOURCES}
      main.cc
      render_worker.cc
      scene_viewer.cc
      image_viewer.cc
      opengl_viewer.cc)

  set(HEADERS ${HEADERS}
      shader_location.h
      scene_viewer.h
      image_viewer.h
      render_worker.h
      opengl_viewer.h
      report_engine.h)

  set(SHADERS blinn_phong.vs blinn_phong.fs)

  set(QT_GEN_FILES ${CMAKE_CURRENT_BINARY_DIR}/scene_viewer_automoc.cpp)

  add_executable(${BUILD_TARGET} ${SOURCES} ${HEADERS} ${SHADERS})
  add_dependencies(${BUILD_TARGET} ${SPICA_TARGET_NAME})

  if (MSVC)
    source_group("Source Files" FILES ${SOURCES} ${HEADERS})
    source_group("Shader Files" FILES ${SHADERS})
    source_group("Qt Files" FILES ${QT_GEN_FILES})
  endif()

  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set_property(TARGET ${BUILD_TARGET} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
  endif()

  qt5_use_modules(${BUILD_TARGET} Widgets OpenGL)

  target_link_libraries(${BUILD_TARGET} ${SPICA_LIBS})
  target_link_libraries(${BUILD_TARGET} ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
endif()
