PROJECT(agl-audio-plugin C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8)
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

SET(PROJECT_NAME "AGL Audio Policy Plugin")
SET(PROJECT_PRETTY_NAME "AGL Audio PulseAudio Policy Plugin")
SET(PROJECT_DESCRIPTION "AGL PulseAudio plugin, forked from module-murphy-ivi")
SET(PROJECT_VERSION "0.1")
SET(PROJECT_URL "https://github.com/Tarnyko/sample-policy-plugin")

INCLUDE(FindPkgConfig)
INCLUDE(GNUInstallDirs)

###########################################################################

link_libraries(-Wl,--as-needed -Wl,--gc-sections)

add_compile_options(-Wall -Wextra -Wconversion)
add_compile_options(-Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable)
add_compile_options(-Wno-parentheses) # this syntax is heavily used
add_compile_options(-Wno-sign-compare -Wno-sign-conversion)
add_compile_options(-Werror=maybe-uninitialized)
add_compile_options(-Werror=implicit-function-declaration)
add_compile_options(-ffunction-sections -fdata-sections)

###########################################################################

PKG_CHECK_MODULES(pulseaudio-module-devel REQUIRED pulseaudio-module-devel)

ADD_DEFINITIONS(${pulseaudio-module-devel_CFLAGS})
SET(include_dirs ${INCLUDE_DIRS} ${pulseaudio-module-devel_INCLUDE_DIRS})
SET(link_libraries ${LINK_LIBRARIES} ${pulseaudio-module-devel_LIBRARIES})
STRING(REGEX REPLACE ";" " " link_flags "${pulseaudio-module-devel_LDFLAGS}" "")

SET(plugin_install_dir ${CMAKE_INSTALL_LIBDIR}/pulse-6.0/modules)

############################################################

ADD_LIBRARY(agl-audio-plugin MODULE module.c audiomgr.c classify.c discover.c loopback.c node.c router.c socketif.c switch.c tracker.c utils.c zone.c)
INCLUDE_DIRECTORIES(${include_dirs})
TARGET_LINK_LIBRARIES(agl-audio-plugin ${link_libraries})
SET_TARGET_PROPERTIES(agl-audio-plugin PROPERTIES PREFIX ""
                                           LINK_FLAGS "${link_flags} -Wl,-rpath=${plugin_install_dir}")

INSTALL(TARGETS agl-audio-plugin
        LIBRARY DESTINATION ${plugin_install_dir})
