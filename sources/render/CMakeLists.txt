set(BUILD_TARGET "${SPICA_PREFIX}_render")

file(GLOB HDR_FILES "*.h" "*.hpp")
file(GLOB SRC_FILES "*.cc")

add_spica_corelib(${BUILD_TARGET} ${HDR_FILES} ${SRC_FILES} LINK_LIBRARIES "${SPICA_PREFIX}_core")
