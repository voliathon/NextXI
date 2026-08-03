#ifndef WINDOWER_VERSION_HPP
#define WINDOWER_VERSION_HPP

#define WINDOWER_VERSION_MAJOR 5
#define WINDOWER_VERSION_MINOR 0
#define WINDOWER_VERSION_BUILD 0
#define WINDOWER_VERSION_REV 0

#define WINDOWER_COPYRIGHT_NAME "Windower Dev Team"

#ifndef WINDOWER_BUILD_TAG
#    ifdef WINDOWER_RELEASE_BUILD
#        define WINDOWER_BUILD_TAG ""
#    else
#        define WINDOWER_BUILD_TAG "Development Build"
#    endif
#endif

#ifndef RC_INVOKED
#    define WINDOWER_UTF_8_2(x) u8##x
#    define WINDOWER_UTF_8(x) WINDOWER_UTF_8_2(x)
#else
#    define WINDOWER_UTF_8(x)
#    define __has_include(x) defined(WINDOWER_AUTO_VERSION)
#endif

#if WINDOWER_AUTO_VERSION && __has_include("version.auto.hpp")
#    include "version.auto.hpp"
#endif

#ifndef RC_INVOKED
#    define WINDOWER_COPYRIGHT_SYMBOL u8"\u00A9"
#else
#    define WINDOWER_COPYRIGHT_SYMBOL "\xA9"
#endif
#ifndef WINDOWER_STRINGIFY
#    define WINDOWER_STRINGIFY_2(s) WINDOWER_UTF_8(#s)
#    define WINDOWER_STRINGIFY(s) WINDOWER_STRINGIFY_2(s)
#endif

#define WINDOWER_COPYRIGHT_STRING                                              \
    WINDOWER_UTF_8("Copyright ")                                               \
    WINDOWER_COPYRIGHT_SYMBOL WINDOWER_UTF_8(" ")                              \
    WINDOWER_UTF_8(WINDOWER_COPYRIGHT_NAME)

#define WINDOWER_VERSION_STRING                                                \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_MAJOR) WINDOWER_UTF_8(".")             \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_MINOR) WINDOWER_UTF_8(".")             \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_BUILD) WINDOWER_UTF_8(".")             \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_REV)

#define WINDOWER_VERSION_BUILD_STRING                                          \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_BUILD) WINDOWER_UTF_8(".")             \
    WINDOWER_STRINGIFY(WINDOWER_VERSION_REV)

#define WINDOWER_BUILD_TAG_STRING WINDOWER_UTF_8(WINDOWER_BUILD_TAG)

#endif
