file(GLOB HDR_FILES "*.h" "*.hpp")
file(GLOB SRC_FILES "*.cc")
set(SPICA_API_HEADER "../spica.h")
add_spica_corelib(${SPICA_LIBCORE} ${SPICA_API_HEADER} ${HDR_FILES} ${SRC_FILES})
