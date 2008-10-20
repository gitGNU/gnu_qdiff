# Process this file with autoconf to produce a configure script.
AC_INIT(streplace)
AM_INIT_AUTOMAKE(qdiff,0.9.0)
AC_CONFIG_SRCDIR(qdiff.cc)
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h float.h limits.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_C_BIGENDIAN
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([asprintf ftruncate memmove memset munmap strchr strdup strerror strrchr strtol strtoul])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
 
echo "========================================================================"
echo "Now type"
echo "    make"
echo or
echo '    make CXXFLAGS="-Wall -W -g"'
echo '    make CXXFLAGS="-Wall -W -Woverloaded-virtual -Wsynth -Wsign-promo -g"'
