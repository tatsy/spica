set(BUILD_TARGET "${SPICA_PREFIX}_core")

file(GLOB HDR_FILES "*.h")
file(GLOB SRC_FILES "*.cc")

add_spica_corelib(${BUILD_TARGET} ${HDR_FILES} ${SRC_FILES})
