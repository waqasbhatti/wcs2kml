#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(wcs2kml, 0.1, opensource@google.com)
AC_CONFIG_SRCDIR(README)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(src/config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Turn on library exporting.
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Location of libpng directories as input by user.
png_dir_given="no"

AC_ARG_WITH(png_dir, AC_HELP_STRING([--with-png-dir=PREFIX],
                                    [prefix directory containing libpng]),
            [png_dir_given="yes"])

if test "$png_dir_given" = "yes"; then
  PNG_INCLUDE="-I$with_png_dir/include"
  PNG_LIB="-L$with_png_dir/lib"
  AC_SUBST(PNG_INCLUDE)
  AC_SUBST(PNG_LIB)
  CFLAGS="$CFLAGS -I$with_png_dir/include"
  CPPFLAGS="$CPPFLAGS -I$with_png_dir/include"
  CXXFLAGS="$CXXFLAGS -I$with_png_dir/include"
  LDFLAGS="$LDFLAGS -L$with_png_dir/lib"
fi

# Location of gflags directories as input by user.
gflags_dir_given="no"

AC_ARG_WITH(gflags_dir, AC_HELP_STRING([--with-gflags-dir=PREFIX],
                                       [prefix directory containing gflags]),
            [gflags_dir_given="yes"])

if test "$gflags_dir_given" = "yes"; then
  GFLAGS_INCLUDE="-I$with_gflags_dir/include"
  GFLAGS_LIB="-L$with_gflags_dir/lib"
  AC_SUBST(GFLAGS_INCLUDE)
  AC_SUBST(GFLAGS_LIB)
  CFLAGS="$CFLAGS -I$with_gflags_dir/include"
  CPPFLAGS="$CPPFLAGS -I$with_gflags_dir/include"
  CXXFLAGS="$CXXFLAGS -I$with_gflags_dir/include"
  LDFLAGS="$LDFLAGS -L$with_gflags_dir/lib"
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h \
                  strings.h sys/file.h sys/socket.h sys/time.h unistd.h], [],
                  [AC_MSG_ERROR([cannot find required header])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_CHECK_FUNCS([ftruncate gethostbyname gethostname gettimeofday \
                memset socket strcasecmp strchr strncasecmp strrchr \
                strstr strtol], [],
                [AC_MSG_ERROR([cannot find required function])])

# Checks for math library.
AC_CHECK_LIB([m], [floor], [],
             [AC_MSG_ERROR([cannot find required math function])])
AC_CHECK_LIB([m], [pow], [],
             [AC_MSG_ERROR([cannot find required math function])])
AC_CHECK_LIB([m], [sqrt], [],
             [AC_MSG_ERROR([cannot find required math function])])

# Check for C++ stl.
AC_LANG_CPLUSPLUS
AC_CHECK_LIB([stdc++], [main], [], [AC_MSG_ERROR([cannot find libstdc++])])
AC_CHECK_HEADER([string], [], [AC_MSG_ERROR([cannot find STL string])])
AC_CHECK_HEADER([vector], [], [AC_MSG_ERROR([cannot find STL vector])])

# Check for libpng.
AC_LANG_C
AC_CHECK_HEADER(png.h, [], [AC_MSG_ERROR([cannot find png.h])])
AC_CHECK_LIB([png], [png_set_add_alpha], [],
             [AC_MSG_ERROR([cannot find libpng])])

# Check for gflags
AC_LANG_CPLUSPLUS
AC_CHECK_HEADER(google/gflags.h, [], [AC_MSG_ERROR([cannot find gflags.h])])
AC_CHECK_LIB([gflags], [main], [],
             [AC_MSG_ERROR([cannot find libgflags])])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/google/Makefile
                 src/google/libwcs/Makefile])
AC_OUTPUT
