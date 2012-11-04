# The autoconf version must be at least 2.64 to correctly detect
# endianess of Mac OS X universal binary.
AC_PREREQ(2.64)
AC_INIT([snzip], [0.9.0])

AM_INIT_AUTOMAKE

# These are flags passed to automake (though they look like gcc flags!)
AC_PROG_CC
AC_GNU_SOURCE
AC_C_BIGENDIAN

if test "$GCC"; then
   CFLAGS="$CFLAGS -Wall"
fi

AC_CHECK_HEADERS([unistd.h])

AC_SYS_LARGEFILE
AC_CHECK_FUNCS(posix_fadvise futimens futimes)
AC_CHECK_MEMBERS([struct stat.st_mtimensec, struct stat.st_mtim.tv_nsec, struct stat.st_mtimespec.tv_nsec], [], [], [[
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
]])

# unlocked stdio functions
AC_CHECK_FUNCS(getc_unlocked putc_unlocked fread_unlocked fwrite_unlocked ferror_unlocked feof_unlocked)

AC_CHECK_SIZEOF(size_t)

# introduce the optional configure parameter for a non-standard install prefix of snappy
AC_ARG_WITH([snappy],
    [AS_HELP_STRING([--with-snappy=prefix],
        [try this for a non-standard install prefix of the snappy library])],
    [
        CFLAGS="$CFLAGS -I$with_snappy/include"
        LDFLAGS="$LDFLAGS -L$with_snappy/lib"
    ],
    [])

AC_CHECK_LIB([snappy], [snappy_compress], [],
[AC_MSG_ERROR([No snappy library is not found.])])

AC_ARG_ENABLE([sse4.2],
    [AS_HELP_STRING([--enable-sse4.2=yes/runtime-detection/compile-time-detection/no], [use crc32 (SSE4.2) instruction @<:@compile-time-detection@:>@])],
    [
        case "$enableval" in
           yes) ;;
           runtime-detection) ;;
           compile-time-detection) ;;
           no) ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-sse4.2) ;;
        esac
        with_sse4_2=$enableval
    ],[with_sse4_2=compile-time-detection])

if test x$with_sse4_2 != xno; then
   saved_CFLAGS="$CFLAGS"
   if test "$GCC" ; then
      CFLAGS="$CFLAGS -msse4.2"
   fi
   AC_MSG_CHECKING([crc32c (SSE4.2) compiler support])
   AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM([#include <nmmintrin.h>], [_mm_crc32_u32(0, 0);])],
      [AC_MSG_RESULT(yes); have_crc32=yes],
      [AC_MSG_RESULT(no);  have_crc32=no; CFLAGS="$saved_CFLAGS"
       if test x$with_sse4_2 != xcompile-time-detection; then
          AC_MSG_ERROR(bad value ${enableval} for --enable-sse4.2)
       fi
      ])
fi

case "$with_sse4_2" in
   yes)
      USE_CRC32_SSE4_2=1
      ;;
   runtime-detection)
      USE_CRC32_SSE4_2=1
      USE_CRC32_GENERIC=1
      AC_DEFINE([RUNTIME_CPUDETECTION], 1, [enable runtime-cpudetection])
      ;;
   compile-time-detection)
     AC_MSG_CHECKING([crc32c (SSE4.2) hardware support])
     AC_RUN_IFELSE(
        [AC_LANG_PROGRAM([
#include <nmmintrin.h>
#include <signal.h>
static void sigill(int signo)
(
  _exit(1);
)
], [signal(SIGILL, sigill); _mm_crc32_u32(0, 0); return 0;])],
        [AC_MSG_RESULT(yes); USE_CRC32_SSE4_2=1],
        [AC_MSG_RESULT(no);  USE_CRC32_GENERIC=1])
      ;;
   no)
      USE_CRC32_GENERIC=1
      ;;
esac

if test "$USE_CRC32_SSE4_2"; then
   AC_DEFINE([USE_CRC32_SSE4_2], 1, [use sse4.2])
fi

AM_CONDITIONAL(USE_CRC32_SSE4_2, test "$USE_CRC32_SSE4_2")
AM_CONDITIONAL(USE_CRC32_GENERIC, test "$USE_CRC32_GENERIC")

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
