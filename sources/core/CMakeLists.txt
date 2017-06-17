file(GLOB HDR_FILES "*.h" "*.hpp")
file(GLOB SRC_FILES "*.cc")
add_spica_corelib(${SPICA_LIBCORE} ${HDR_FILES} ${SRC_FILES})
