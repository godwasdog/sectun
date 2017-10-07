#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT(sectun, 0.1, [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11 dist-bzip2 tar-ustar foreign subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_INSTALL
LT_INIT

# define macros in config.h
#AC_DEFINE(DEBUG_SERVER, 0, [log server debug message])
#AC_DEFINE(DEBUG_CLIENT, 0, [log client debug message])
#AC_DEFINE(DEBUG_TRANSPORT,    0, [log transport debug message])
#AC_DEFINE(DEBUG_UDP,    0, [log udp debug message])
#AC_DEFINE(DEBUG_AUTH,    0, [log udp debug message])
#AC_DEFINE(DEBUG_ENCRYPT, 0, [log encrypt debug message])
#AC_DEFINE(DEBUG_HEARTBEAT, 0, [log encrypt debug message])
#AC_DEFINE(DEBUG_KCP,    0, [log kcp debug message])
#AC_DEFINE(DEBUG_TUN,    0, [log tun debug message])
#AC_DEFINE(DEBUG_MAIN,    0, [log tun debug message])

# important configure
AC_DEFINE(UTIL_DUP_STRING_MAX_SIZE,    4096, [static size of dup string])
AC_DEFINE(ARGS_CONFIG_FILE_SIZE,       2048, [config file max size])
AC_DEFINE(DATA_PADDING_SIZE,             32, [data padding size, used for encryption])
AC_DEFINE(DATA_BUFFER_SIZE,            2048, [data read/write buffer size])
AC_DEFINE(ENCRYPT_KEY_SIZE,              32, [data read/write buffer size])
AC_DEFINE(ENCRYPT_VECTOR_SIZE,           16, [data read/write buffer size])
AC_DEFINE(TRANSPORT_CHAIN_MAX,           10, [data read/write buffer size])
AC_DEFINE(TRANSPORT_DELIMITER,          "+", [data read/write buffer size])

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h stdlib.h string.h sys/socket.h unistd.h])

# from OpenVPN configure.ac
case "$host" in
    *-*-linux*)
        AC_DEFINE([TARGET_LINUX], [1], [Are we running on Linux?])
        AC_CHECK_HEADER([linux/if_tun.h],[],[AC_MSG_ERROR([linux/if_tun.h not found.])],[])
        ;;
    *)
        AC_DEFINE([TARGET_UNKNOWN], [1], [Unknown platform ?])
esac

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
# To fix rpl_malloc undefined error in mips cross-compile enviroment.
AC_CHECK_FUNCS([malloc realloc])
AC_CHECK_FUNCS([inet_ntoa memset select socket strchr strdup strrchr])

AC_ARG_ENABLE([debug],
    [  --enable-debug          build with additional debugging code],
    [CFLAGS="$CFLAGS -g -DDEBUG"])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

# Add a option to force static link the target.
AC_ARG_ENABLE([static],
    [  --enable-static         build with static linking],
    [LDFLAGS="$LDFLAGS -static"])

AC_ARG_ENABLE([profile],
    [  --enable-profile        build with profile],
    [CFLAGS="$CFLAGS -pg"])

AC_ARG_ENABLE([analyze],
    [  --enable-analyze        build with analyze],
    [CC="clang --analyze"])

AM_CONDITIONAL(STATIC, test x"$static" = x"true")


AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
