#--------------------------------------------------------------------------
# this file has the following sections:
#
# initialization
# api options
# host magic
# checks for programs
# visibility foo
# typedefs, structures, and compiler characteristics
# basic functions, headers, libs
# vars
# enables
# unit tests
# required libs (pcap, luajit, pcre, dnet, daq, zlib)
# optional libs (pthreads, lzma, ssl / crypto, intel soft cpm)
# outputs
#
# if you add an AC_DEFINE() for a symbol that appears in an exported
# header, you must also add that symbol to CPPFLAGS.  This ensures that
# external modules like those in examples can use pkg-config to get the
# same symbols Snort did.
#--------------------------------------------------------------------------
# initialization
#--------------------------------------------------------------------------

AC_INIT([snort], [3.0.0-a4], [snort-team@sourcefire.com])
AC_PREREQ([2.69])

AC_CONFIG_SRCDIR([src/main.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h src/framework/api_options.h])

AM_INIT_AUTOMAKE(foreign nostdinc)
AM_SILENT_RULES([yes])

AC_PROG_CC([clang gcc])
AC_PROG_CXX([clang++ g++])
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_C_BIGENDIAN
AC_C_INLINE

# g++ 4.7 will pass the c++11 check but is not sufficient
if test "x$CXX" = "xg++" ; then
    if `$CXX -dumpversion | awk '{exit $1<4.8?0:1}'` ; then
       echo
       echo "   ERROR:  g++ >= 4.8 is required"
       echo
       exit 1
    fi
fi

CFLAGS="-std=gnu99 $CFLAGS"

AX_CXX_COMPILE_STDCXX_11

LT_INIT

PKG_PROG_PKG_CONFIG

AX_CODE_COVERAGE

#--------------------------------------------------------------------------
# api options
#--------------------------------------------------------------------------

# build options which affect plugin compatibility must be appended to this
# string to ensure issues are caught when loading:

API_OPTIONS=""

# presently such cases were eliminated.  however, if any arise, add a
# statement like the following after the associated AC_DEFINE() statement:

# AC_DEFINE(([HAVE_OPTION],[1], [option-name is enabled])
# API_OPTIONS="$API_OPTIONS option-name"

# API_OPTIONS is #defined in config.h but that file is not exported so it
# is also #defined in api_options.h.

#--------------------------------------------------------------------------
# host magic :(
#--------------------------------------------------------------------------

AC_CANONICAL_HOST

linux="no"
macos="no"

case "$host" in
  *-openbsd*)
    AC_DEFINE([OPENBSD],[1],[Define if OpenBSD])
    ;;
  *-solaris*)
    AC_DEFINE([SOLARIS],[1],[Define if Solaris])
    ;;
  *-sunos*)
    AC_DEFINE([SUNOS],[1],[Define if SunOS])
    ;;
  *-linux*)
    linux="yes"
    AC_DEFINE([LINUX],[1],[Define if Linux])
    ;;
  *-freebsd*)
    AC_DEFINE([FREEBSD],[1],[Define if FreeBSD])
    ;;
  *-apple*)
    macos="yes"
    AC_DEFINE([MACOS],[1],[Define if MacOS])
esac

#--------------------------------------------------------------------------
# Checks for programs.
#--------------------------------------------------------------------------

AC_CHECK_PROG(have_asciidoc,asciidoc,yes,no)

AM_CONDITIONAL(MAKE_HTML_DOC, [test "x$have_asciidoc" = "xyes"])

AC_CHECK_PROG(have_dblatex,dblatex,yes,no)

AM_CONDITIONAL(MAKE_PDF_DOC, [test "x$have_dblatex" = "xyes" -a "x$have_asciidoc" = "xyes"])

AC_CHECK_PROG(have_w3m,w3m,yes,no)

AM_CONDITIONAL(MAKE_TEXT_DOC, [test "x$have_w3m" = "xyes" -a "x$have_asciidoc" = "xyes"])

AC_MSG_CHECKING(for sparc)
if eval "echo $host_cpu|grep -i sparc >/dev/null"; then
    AC_DEFINE([WORDS_MUSTALIGN],[1],[Define if words must align])
    AC_MSG_RESULT(yes)

    # gcc, sparc and optimization not so good
    if test -n "$GCC"; then
        NO_OPTIMIZE="yes"
    fi
else
    AC_MSG_RESULT(no)
fi

#--------------------------------------------------------------------------
# visibility foo
#--------------------------------------------------------------------------

# modified from gnulib/m4/visibility.m4
AC_DEFUN([CC_VISIBILITY],
[
    AC_REQUIRE([AC_PROG_CC])
    AC_MSG_CHECKING([for visibility support])
    gl_save_CPPFLAGS="$CPPFLAGS"

    AC_CACHE_VAL(gl_cv_cc_visibility, [
        # Add -Werror flag since some compilers, e.g. icc 7.1, don't support it,
        # but only warn about it instead of compilation failing
        CPPFLAGS="$CPPFLAGS -Werror -fvisibility=hidden"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            extern __attribute__((__visibility__("hidden"))) int hiddenvar;
            extern __attribute__((__visibility__("default"))) int exportedvar;
            extern __attribute__((__visibility__("hidden"))) int hiddenfunc (void);
            extern __attribute__((__visibility__("default"))) int exportedfunc (void);]],
            [[]])],
            [gl_cv_cc_visibility="yes"],
            [gl_cv_cc_visibility="no"])
    ])
    AC_MSG_RESULT([$gl_cv_cc_visibility])
    CPPFLAGS="$gl_save_CPPFLAGS"
    if test "x$gl_cv_cc_visibility" = "xyes"; then
        CPPFLAGS="$CPPFLAGS -fvisibility=hidden"
        AC_DEFINE([HAVE_VISIBILITY],[1],
            [Define if the compiler supports visibility declarations.])
    fi
])
CC_VISIBILITY()

#--------------------------------------------------------------------------
# thread-local storage foo
#--------------------------------------------------------------------------
# FIXIT-L should define a macro for try-compile-link-cache-val idiom
AC_DEFUN([CXX_EXTERN_GNU_TLS],
[
    AC_REQUIRE([AC_PROG_CXX])
    AC_MSG_CHECKING([for extern GNU __thread support])
    gl_save_CPPFLAGS="$CPPFLAGS"

    AC_LANG_PUSH(C++)
    AC_CACHE_VAL(gl_cv_cxx_extern_gnu_tls, [
        CPPFLAGS="-std=c++11 -fPIC -shared -Wl,-undefined,dynamic_lookup"
        AC_LINK_IFELSE([AC_LANG_SOURCE([[
            extern __thread int x;
            void foo() { ++x; }]])],
            [gl_cv_cxx_extern_gnu_tls="yes"],
            [gl_cv_cxx_extern_gnu_tls="no"])
    ])
    AC_LANG_POP()

    AC_MSG_RESULT([$gl_cv_cxx_extern_gnu_tls])
    CPPFLAGS="$gl_save_CPPFLAGS"
])
CXX_EXTERN_GNU_TLS()

AC_DEFUN([CXX_THREAD_LOCAL],
[
    AC_REQUIRE([AC_PROG_CXX])
    AC_MSG_CHECKING([for thread_local keyword support])
    gl_save_CPPFLAGS="$CPPFLAGS"

    AC_LANG_PUSH(C++)
    AC_CACHE_VAL(gl_cv_cxx_thread_local, [
        CPPFLAGS="-std=c++11"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            thread_local int x;]],
            [[]])],
            [gl_cv_cxx_thread_local="yes"],
            [gl_cv_cxx_thread_local="no"])
    ])
    AC_LANG_POP()

    AC_MSG_RESULT([$gl_cv_cxx_thread_local])
    CPPFLAGS="$gl_save_CPPFLAGS"
])
CXX_THREAD_LOCAL()

#if test "x$gl_cv_cxx_extern_gnu_tls" = "xno"; then
#    if test "x$gl_cv_cxx_thread_local" = "xyes"; then
#        AC_DEFINE([USE_THREAD_LOCAL],[1],
#            [Define if thread_local keyword should be used])
#    else
#        AC_MSG_ERROR([Compiler does not support thread_local OR extern __thread declarations])
#    fi
#fi

#--------------------------------------------------------------------------
# typedefs, structures, and compiler characteristics
#--------------------------------------------------------------------------

#AC_CHECK_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_CHECK_SIZEOF([char])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long int])
AC_CHECK_SIZEOF([long long int])
AC_CHECK_SIZEOF([unsigned int])
AC_CHECK_SIZEOF([unsigned long int])
AC_CHECK_SIZEOF([unsigned long long int])

# Check for int types
AC_CHECK_TYPES([uint8_t,uint16_t,uint32_t,uint64_t])
AC_CHECK_TYPES([int8_t,int16_t,int32_t,int64_t])

#--------------------------------------------------------------------------
# basic functions, headers, libs
#--------------------------------------------------------------------------

AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD
AC_FUNC_ALLOCA

AC_CHECK_FUNCS([endgrent endpwent ftruncate getcwd gettimeofday inet_ntoa isascii localtime_r memchr memmove memset mkdir select socket strcasecmp strchr strdup strerror strncasecmp strrchr strstr strtol strtoul mallinfo malloc_trim])

AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h libintl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/socket.h sys/time.h syslog.h unistd.h wchar.h])

AC_CHECK_LIB(dl, dlsym, DLLIB="yes", DLLIB="no")

#--------------------------------------------------------------------------
# vars
#--------------------------------------------------------------------------

AC_ARG_VAR(SIGNAL_SNORT_RELOAD, set the SIGNAL_SNORT_RELOAD value)

if test "x$SIGNAL_SNORT_RELOAD" != "x" ; then
    AC_DEFINE_UNQUOTED([SIGNAL_SNORT_RELOAD], [$SIGNAL_SNORT_RELOAD], [Set by user])
fi

AC_ARG_VAR(SIGNAL_SNORT_DUMP_STATS, set the SIGNAL_SNORT_DUMP_STATS value)

if test "x$SIGNAL_SNORT_DUMP_STATS" != "x" ; then
    AC_DEFINE_UNQUOTED([SIGNAL_SNORT_DUMP_STATS], [$SIGNAL_SNORT_DUMP_STATS], [Set by user])
fi

AC_ARG_VAR(SIGNAL_SNORT_ROTATE_STATS, set the SIGNAL_SNORT_ROTATE_STATS value)

if test "x$SIGNAL_SNORT_ROTATE_STATS" != "x" ; then
    AC_DEFINE_UNQUOTED([SIGNAL_SNORT_ROTATE_STATS], [$SIGNAL_SNORT_ROTATE_STATS], [Set by user])
fi

AC_ARG_VAR(SIGNAL_SNORT_READ_ATTR_TBL, set the SIGNAL_SNORT_READ_ATTR_TBL value)

if test "x$SIGNAL_SNORT_READ_ATTR_TBL" != "x" ; then
    AC_DEFINE_UNQUOTED([SIGNAL_SNORT_READ_ATTR_TBL], [$SIGNAL_SNORT_READ_ATTR_TBL], [Set by user])
fi

#--------------------------------------------------------------------------
# enables
#--------------------------------------------------------------------------

AM_CONDITIONAL(LINUX, [test "x$linux" = "xyes"])

AC_ARG_ENABLE(static-ips-actions,
    AC_HELP_STRING([--disable-static-ips-actions],[do not include ips actions in binary ]),
    static_ips_actions="$enableval", static_ips_actions="yes")

AM_CONDITIONAL(STATIC_IPS_ACTIONS, [test "x$static_ips_actions" = "xyes"])

if test "x$static_ips_actions" = "xyes"; then
    AC_DEFINE(STATIC_IPS_ACTIONS, [1], [include internal ips actions in binary])
fi

AC_ARG_ENABLE(static-inspectors,
    AC_HELP_STRING([--disable-static-inspectors],[do not include inspectors in binary ]),
    static_inspectors="$enableval", static_inspectors="yes")

AM_CONDITIONAL(STATIC_INSPECTORS, [test "x$static_inspectors" = "xyes"])

if test "x$static_inspectors" = "xyes"; then
    AC_DEFINE(STATIC_INSPECTORS, [1], [include internal inspectors in binary])
fi

AC_ARG_ENABLE(static-loggers,
    AC_HELP_STRING([--disable-static-loggers],[do not include loggers in binary ]),
    static_loggers="$enableval", static_loggers="yes")

AM_CONDITIONAL(STATIC_LOGGERS, [test "x$static_loggers" = "xyes"])

if test "x$static_loggers" = "xyes"; then
    AC_DEFINE(STATIC_LOGGERS, [1], [include internal loggers in binary])
fi

AC_ARG_ENABLE(static-ips-options,
    AC_HELP_STRING([--disable-static-ips-options],[do not include ips options in binary ]),
    static_ips_options="$enableval", static_ips_options="yes")

AM_CONDITIONAL(STATIC_IPS_OPTIONS, [test "x$static_ips_options" = "xyes"])

if test "x$static_ips_options" = "xyes"; then
    AC_DEFINE(STATIC_IPS_OPTIONS, [1], [include internal ips options in binary])
fi

AC_ARG_ENABLE(static-search-engines,
    AC_HELP_STRING([--disable-static-search-engines],[do not include search engines in binary ]),
    static_search_engines="$enableval", static_search_engines="yes")

AM_CONDITIONAL(STATIC_SEARCH_ENGINES, [test "x$static_search_engines" = "xyes"])

if test "x$static_search_engines" = "xyes"; then
    AC_DEFINE(STATIC_SEARCH_ENGINES, [1], [include internal search engines in binary])
fi

AC_ARG_ENABLE(static-codecs,
    AC_HELP_STRING([--disable-static-codecs],[do not include codecs in binary ]),
    static_codecs="$enableval", static_codecs="yes")

AM_CONDITIONAL(STATIC_CODECS, [test "x$static_codecs" = "xyes"])

if test "x$static_codecs" = "xyes"; then
    AC_DEFINE(STATIC_CODECS, [1], [include internal decoders in binary])
fi

AC_ARG_ENABLE(static-piglets,
  AC_HELP_STRING([--disable-static-piglets],[do not include piglets in binary]),
  static_piglets="$enableval", static_piglets="yes")

AM_CONDITIONAL(STATIC_PIGLETS, [test "x$static_piglets" = "xyes"])

if test "x$static_piglets" = "xyes"; then
    AC_DEFINE(STATIC_PIGLETS, [1], [include internal piglets in binary])
fi

AC_ARG_ENABLE(shell,
    AC_HELP_STRING([--enable-shell],[enable command line shell support]),
    enable_shell="$enableval", enable_shell="no")

if test "x$enable_shell" = "xyes"; then
    AC_DEFINE(BUILD_SHELL, [1], [enable shell support])
fi

AC_ARG_ENABLE(large-pcap,
    AC_HELP_STRING([--enable-large-pcap],[enable support for pcaps larger than 2 GB]),
    enable_large_pcap="$enableval", enable_large_pcap="no")

if test "x$enable_large_pcap" = "xyes"; then
    CPPFLAGS="${CPPFLAGS} -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
fi

AC_ARG_ENABLE(debug-msgs,
    AC_HELP_STRING([--enable-debug-msgs],
        [enable debug printing options (bugreports and developers only)]),
    enable_debug_msgs="$enableval", enable_debug_msgs="no")

AM_CONDITIONAL(DEBUG_MSGS, test "x$enable_debug_msgs" = "xyes")

if test "x$enable_debug_msgs" = "xyes"; then
    CPPFLAGS="$CPPFLAGS -DDEBUG_MSGS"
    AC_DEFINE(DEBUG_MSGS, [1], [enable debug messages])
fi

AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug],[enable debugging options (bugreports and developers only)]),
    enable_debug="$enableval", enable_debug="no")

if test "x$enable_debug" = "xyes"; then
    CPPFLAGS="$CPPFLAGS -g -DDEBUG"
    AC_DEFINE(DEBUG, [1], [enable debug build])
else
    # disable assert()ions
    AC_DEFINE(NDEBUG, [1], [disable assertions])
fi

AC_ARG_ENABLE(gdb,
    AC_HELP_STRING([--enable-gdb],[enable gdb debugging information]),
    enable_gdb="$enableval", enable_gdb="no")

if test "x$enable_gdb" = "xyes"; then
    CFLAGS="$CFLAGS -g -ggdb"
    CXXFLAGS="$CXXFLAGS -g -ggdb"
fi

AC_ARG_ENABLE(gprof-profile,
    AC_HELP_STRING([--enable-gprof-profile],[enable gprof profiling output (developers only)]),
    enable_gprof_profile="$enableval", enable_gprof_profile="no")

# FIXIT-L need to check for g++ not gcc
if test "x$enable_gprof_profile" = "xyes"; then
    if test -n "$GCC"; then
        CFLAGS="$CFLAGS -pg"
        CXXFLAGS="$CXXFLAGS -pg"
    fi
    AC_DEFINE(PROFILE, [1], [enable gprof profiling])
fi

AC_ARG_ENABLE(corefiles,
    AC_HELP_STRING([--disable-corefiles],[prevent Snort from generating core files]),
    enable_corefiles="$enableval", enable_corefiles="yes")

if test "x$enable_corefiles" = "xno"; then
    AC_DEFINE(NOCOREFILE, [1], [do not generate a core file on segfault etc.])
fi

AC_ARG_ENABLE(address-sanitizer,
    AC_HELP_STRING([--enable-address-sanitizer],[enable AddressSanitizer support]),
    [
        ASAN_CCXFLAGS="-fsanitize=address -fno-omit-frame-pointer"
    ])

#--------------------------------------------------------------------------
# C++11 threads
#--------------------------------------------------------------------------

AC_LANG_PUSH([C++])
ax_cxx11_thread_flags="none -pthread"
for flag in $ax_cxx11_thread_flags ; do
    case $flag in
        none)
        AC_MSG_CHECKING([whether C++11 threads work without any flags])
        ;;

        -*)
        AC_MSG_CHECKING([whether C++11 threads work with $flag])
        CXX11_THREAD_CXXFLAGS="$flag"
        ;;
    esac

    save_CXXFLAGS="$CXXFLAGS"
    CXXFLAGS="$CXX11_THREAD_CXXFLAGS $CXXFLAGS"

    AC_RUN_IFELSE([AC_LANG_PROGRAM([#include <iostream>
                   #include <thread>
                   void call_from_thread() { }],
                   [std::thread t1(call_from_thread); t1.join()])],
                   [ax_cxx11_thread_ok=yes],
                   [ax_cxx11_thread_ok=no])

    CXXFLAGS="$save_CXXFLAGS"

    AC_MSG_RESULT([$ax_cxx11_thread_ok])
    if test "x$ax_cxx11_thread_ok" = xyes ; then
        break;
    fi

    CXX11_THREAD_CXXFLAGS=
done
AC_LANG_POP([C++])
if test "x$ax_cxx11_thread_ok" != xyes ; then
    AC_MSG_ERROR([could not find CXXFLAGS that allow C++11 threads to work])
fi
CXXFLAGS="$CXX11_THREAD_CXXFLAGS $CXXFLAGS"

#--------------------------------------------------------------------------
# unit tests
#--------------------------------------------------------------------------

AC_ARG_ENABLE(unit_tests,
    AC_HELP_STRING([--enable-unit-tests],[build unit tests]),
    enable_unit_tests="$enableval", enable_unit_tests="no")

have_cpputest_pc="no"

if test "x$enable_unit_tests" = "xyes"; then
    PKG_CHECK_EXISTS([cpputest], [ have_cpputest_pc="yes" ], [ have_cpputest_pc="no" ])

    if test "x${have_cpputest_pc}" = "xyes" ; then
        CPPUTEST_CPPFLAGS="`${PKG_CONFIG} --cflags cpputest`"
        CPPUTEST_LDFLAGS="`${PKG_CONFIG} --libs cpputest`"
    fi

    AC_DEFINE(UNIT_TEST, [1], [enable unit tests in build])
fi

AM_CONDITIONAL(BUILD_UNIT_TESTS, test "x$enable_unit_tests" = "xyes")
AM_CONDITIONAL(BUILD_CPPUTESTS, test "x$have_cpputest_pc" = "xyes")

#--------------------------------------------------------------------------
# piglet
#--------------------------------------------------------------------------

AC_ARG_ENABLE(piglet,
    AC_HELP_STRING([--enable-piglet],[build piglet test harness]),
    enable_piglet="$enableval", enable_piglet="no")

if test "x$enable_piglet" = "xyes"; then
    AC_DEFINE(PIGLET, [1], [enable piglet test harness in build])
fi

AM_CONDITIONAL(BUILD_PIGLET, test "x$enable_piglet" = "xyes")

#--------------------------------------------------------------------------
# pcap
#--------------------------------------------------------------------------

AC_ARG_WITH(pcap_includes,
    AC_HELP_STRING([--with-pcap-includes=DIR],[libpcap include directory]),
    [with_libpcap_includes="$withval"],[with_libpcap_includes="no"])

if test "x$with_libpcap_includes" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -I${with_libpcap_includes}"
fi

AC_ARG_WITH(pcap_libraries,
    AC_HELP_STRING([--with-pcap-libraries=DIR],[libpcap library directory]),
    [with_libpcap_libraries="$withval"],[with_libpcap_libraries="no"])

if test "x$with_libpcap_libraries" != "xno"; then
    LDFLAGS="${LDFLAGS} -L${with_libpcap_libraries}"
fi

LPCAP=""
AC_CHECK_LIB(pcap, pcap_datalink,, LPCAP="no")

# If both the AC_CHECK_LIB for normal pcap and pfring-enabled pcap fail then exit.
if test "x$LPCAP" = "xno"; then
  if test "x$LPFRING_PCAP" = "xno"; then
    echo
    echo "   ERROR:  Libpcap library/headers (libpcap.a (or .so)/pcap.h)"
    echo "   not found, go get it from http://www.tcpdump.org"
    echo "   or use the --with-pcap-* options, if you have it installed"
    echo "   in unusual place.  Also check if your libpcap depends on another"
    echo "   shared library that may be installed in an unusual place"
    exit 1
  fi
fi

AC_MSG_CHECKING([for pcap_lib_version])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
        [[#include <pcap.h>]],
        [[pcap_lib_version();]]
    )],
    [have_pcap_lib_version="yes"],
    [have_pcap_lib_version="no"]
)
AC_MSG_RESULT($have_pcap_lib_version)

if test "x$have_pcap_lib_version" = "xyes"; then
    AC_DEFINE([HAVE_PCAP_LIB_VERSION],[1],
        [Can output the library version.])
fi

#--------------------------------------------------------------------------
# hwloc
#--------------------------------------------------------------------------

AC_MSG_CHECKING([for hwloc pkg-config presence])
PKG_CHECK_EXISTS([hwloc], [have_hwloc_pkgconfig="yes"], [have_hwloc_pkgconfig="no"])
AC_MSG_RESULT(${have_hwloc_pkgconfig})

if test "${have_hwloc_pkgconfig}" = "yes" ; then
    HWLOC_CFLAGS=`${PKG_CONFIG} --cflags hwloc`
    HWLOC_LDFLAGS=`${PKG_CONFIG} --libs-only-L hwloc`
fi

AC_ARG_WITH(hwloc_includes,
    AC_HELP_STRING([--with-hwloc-includes=DIR],[hwloc include directory]),
    [with_hwloc_includes="$withval"],[with_hwloc_includes="no"])

if test "x$with_hwloc_includes" != "xno"; then
    HWLOC_CFLAGS="-I${with_hwloc_includes}"
fi

AC_ARG_WITH(hwloc_libraries,
    AC_HELP_STRING([--with-hwloc-libraries=DIR],[hwloc library directory]),
    [with_hwloc_libraries="$withval"],[with_hwloc_libraries="no"])

if test "x$with_hwloc_libraries" != "xno"; then
    HWLOC_LDFLAGS="-L${with_hwloc_libraries}"
fi

CPPFLAGS="${HWLOC_CFLAGS} ${CPPFLAGS}"
LDFLAGS="${HWLOC_LDFLAGS} ${LDFLAGS}"

AC_CHECK_HEADERS([hwloc.h], , [AC_MSG_ERROR(hwloc header not found.)])
AC_CHECK_LIB(hwloc, hwloc_set_thread_cpubind, , [AC_MSG_ERROR(Linking against hwloc library failed.)])

#--------------------------------------------------------------------------
# luajit
#--------------------------------------------------------------------------

AC_MSG_CHECKING([for luajit pkg-config presence])
PKG_CHECK_EXISTS([luajit], [ have_luajit_pkgconfig="yes" ], [ have_luajit_pkgconfig="no" ])
AC_MSG_RESULT(${have_luajit_pkgconfig})

if test "${have_luajit_pkgconfig}" = "yes" ; then
    LUAJIT_CFLAGS=`${PKG_CONFIG} --cflags luajit`
    LUAJIT_LDFLAGS=`${PKG_CONFIG} --libs-only-L luajit`
fi

AC_ARG_WITH(luajit_includes,
    AC_HELP_STRING([--with-luajit-includes=DIR],[luajit include directory]),
    [with_luajit_includes="$withval"],[with_luajit_includes="no"])

if test "x$with_luajit_includes" != "xno"; then
    LUAJIT_CFLAGS="-I${with_luajit_includes}"
fi

AC_ARG_WITH(luajit_libraries,
    AC_HELP_STRING([--with-luajit-libraries=DIR],[luajit library directory]),
    [with_luajit_libraries="$withval"],[with_luajit_libraries="no"])

if test "x$with_luajit_libraries" != "xno"; then
    LUAJIT_LDFLAGS="-L${with_luajit_libraries}"
fi

if test "x$macos" != "xno"; then
    LUAJIT_LDFLAGS="${LUAJIT_LDFLAGS} -pagezero_size 10000 -image_base 100000000"
fi

CPPFLAGS="${CPPFLAGS} ${LUAJIT_CFLAGS}"
LDFLAGS="${LDFLAGS} ${LUAJIT_LDFLAGS}"

AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([luajit.h], , [AC_MSG_ERROR("LuaJIT header not found.")])
AC_CHECK_LIB(luajit-5.1, main, , [AC_MSG_ERROR("Linking against LuaJIT library failed.")])
AC_LANG_POP([C++])

#--------------------------------------------------------------------------
# pcre
#--------------------------------------------------------------------------

AC_MSG_CHECKING([for libpcre pkg-config presence])
PKG_CHECK_EXISTS([libpcre], [ have_libpcre_pkgconfig="yes" ], [ have_libpcre_pkgconfig="no" ])
AC_MSG_RESULT(${have_libpcre_pkgconfig})

if test "${have_libpcre_pkgconfig}" = "yes" ; then
    PCRE_CFLAGS=`${PKG_CONFIG} --cflags libpcre`
    PCRE_LDFLAGS=`${PKG_CONFIG} --libs-only-L libpcre`
fi

AC_ARG_WITH(pcre_includes,
    AC_HELP_STRING([--with-pcre-includes=DIR],[libpcre include directory]),
    [with_libpcre_includes="$withval"],[with_libpcre_includes="no"])

if test "x$with_libpcre_includes" != "xno"; then
    PCRE_CFLAGS="-I${with_libpcre_includes}"
elif test "${have_libpcre_pkgconfig}" = "no"; then
    PCRE_CFLAGS="`pcre-config --cflags`"
fi

AC_ARG_WITH(pcre_libraries,
    AC_HELP_STRING([--with-pcre-libraries=DIR],[libpcre library directory]),
    [with_libpcre_libraries="$withval"],[with_libpcre_libraries="no"])

if test "x$with_libpcre_libraries" != "xno"; then
    PCRE_LDFLAGS="-L${with_libpcre_libraries}"
elif test "${have_libpcre_pkgconfig}" = "no"; then
    PCRE_LDFLAGS="`pcre-config --libs`"
fi

CPPFLAGS="${CPPFLAGS} ${PCRE_CFLAGS}"
LDFLAGS="${LDFLAGS} ${PCRE_LDFLAGS}"

# PCRE configuration (required)
# Verify that we have the headers
PCRE_H=""
AC_CHECK_HEADERS(pcre.h,, PCRE_H="no")
if test "x$PCRE_H" = "xno"; then
  echo
  echo "   ERROR:  Libpcre header not found."
  echo "   Get it from http://www.pcre.org"
  exit 1
fi

# Verify that we have the library
PCRE_L=""
pcre_version_six=""
AC_CHECK_LIB(pcre, pcre_compile, ,PCRE_L="no")
if test "x$PCRE_L" = "xno"; then
    echo
    echo "   ERROR:  Libpcre library not found."
    echo "   Get it from http://www.pcre.org"
    echo
    exit 1
else
    AC_MSG_CHECKING(for libpcre version 6.0 or greater)
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pcre.h>]], [[
       #if (PCRE_MAJOR < 6)
       #error "Version failure"
       #else
       int a, b = 0, c = 0, d = 0;
       pcre *tmp = NULL;
       a = pcre_copy_named_substring(tmp, "", &b, c, "", "", d);
       #endif
      ]])],[pcre_version_six="yes"],[pcre_version_six="no"])
fi

if test "x$pcre_version_six" != "xyes"; then
    AC_MSG_RESULT(no)
    echo
    echo "    ERROR:  Libpcre library version >= 6.0 not found."
    echo "    Get it from http://www.pcre.org"
    echo
    exit 1
else
    AC_MSG_RESULT(yes)
fi

#--------------------------------------------------------------------------
# dnet
#--------------------------------------------------------------------------

AC_ARG_WITH(dnet_includes,
    AC_HELP_STRING([--with-dnet-includes=DIR],[libdnet include directory]),
    [with_dnet_includes="$withval"],[with_dnet_includes="no"])

if test "x$with_dnet_includes" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -I${with_dnet_includes}"
else
    CPPFLAGS="${CPPFLAGS} `dnet-config --cflags 2>/dev/null`"
fi

AC_ARG_WITH(dnet_libraries,
    AC_HELP_STRING([--with-dnet-libraries=DIR],[libdnet library directory]),
    [with_dnet_libraries="$withval"],[with_dnet_libraries="no"])

if test "x$with_dnet_libraries" != "xno"; then
    LDFLAGS="${LDFLAGS} -L${with_dnet_libraries}"
else
    LDFLAGS="${LDFLAGS} `dnet-config --libs 2>/dev/null`"
fi

AC_CHECK_HEADERS(dnet.h,,DNET_H="no")
AC_CHECK_HEADERS(dumbnet.h,,DUMBNET_H="no")

if test "x$DNET_H" = "xno" -a "x$DUMBNET_H" = "xno"; then
    echo
    echo "   ERROR:  dnet header not found, go get it from"
    echo "   https://github.com/dugsong/libdnet.git or use the --with-dnet-*"
    echo "   options, if you have it installed in an unusual place"
    exit
fi

AC_CHECK_LIB(dnet, eth_set,,[DNET="no"])
AC_CHECK_LIB(dumbnet, eth_set,,[DUMBNET="no"])

if test "x$DNET" = "xno" -a "x$DUMBNET" = "xno"; then
    echo
    echo "   ERROR:  dnet library not found, go get it from"
    echo "   http://code.google.com/p/libdnet/ or use the --with-dnet-*"
    echo "   options, if you have it installed in an unusual place"
    exit
fi

#--------------------------------------------------------------------------
# daq
#--------------------------------------------------------------------------

AC_ARG_WITH(daq_includes,
    AC_HELP_STRING([--with-daq-includes=DIR],[DAQ include directory]),
    [with_daq_includes="$withval"],[with_daq_includes="no"])

if test "x$with_daq_includes" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -I${with_daq_includes}"
fi

AC_ARG_WITH(daq_libraries,
    AC_HELP_STRING([--with-daq-libraries=DIR],[DAQ library directory]),
    [with_daq_libraries="$withval"],[with_daq_libraries="no"])

if test "x$with_daq_libraries" != "xno"; then
    LDFLAGS="${LDFLAGS} -L${with_daq_libraries}"
fi

AC_ARG_ENABLE(static_daq,
    AC_HELP_STRING([--disable-static-daq],[link static DAQ modules.]),
    enable_static_daq="$enableval", enable_static_daq="yes")

if test "$DLLIB" != "no"; then
    LIBS="${LIBS} -ldl"
else
    AC_CHECK_LIB(c, dlsym, DLLIB="yes", DLLIB="no")
    if test "$DLLIB" = "no"; then
       echo
       echo "   ERROR:  programmatic interface to dynamic link loader"
       echo "   not found.  Cannot build Snort."
       echo
       exit 1
    fi
fi

if test "x$enable_static_daq" = "xyes"; then
    LDAQ=""
    LIBS="${LIBS} `daq-modules-config --static --libs`"
    AC_CHECK_LIB([daq_static], [daq_load_modules],
        [LIBS="-ldaq_static ${LIBS}"], [LDAQ="no"], [ ])

    if test "x$LDAQ" = "xno"; then
        echo
        echo "   ERROR:  daq_static library not found, go get it from"
        echo "   http://www.snort.org/."
        #AC_MSG_ERROR("Fatal!")  # FIXTHIS switch over to this macro
        exit 1                   # instead of raw exits!
    fi
else
    LDAQ=""
    AC_CHECK_LIB([daq], [daq_load_modules],
        [LIBS="${LIBS} -ldaq"], [LDAQ="no"], [ ])

    if test "x$LDAQ" = "xno"; then
        echo
        echo "   ERROR:  daq library not found, go get it from"
        echo "   http://www.snort.org/."
        #AC_MSG_ERROR("Fatal!")
        exit 1
    fi
fi

#--------------------------------------------------------------------------
# zlib
#--------------------------------------------------------------------------

Z_LIB=""
AC_CHECK_HEADERS(zlib.h,, Z_LIB="no")
if test "x$Z_LIB" = "xno"; then
    echo
    echo "   ERROR:  zlib header not found, go get it from"
    echo "   http://www.zlib.net"
    exit
fi

Z_LIB=""
AC_CHECK_LIB(z, inflate,, Z_LIB="no")
if test "x$Z_LIB" = "xno"; then
    echo
    echo "   ERROR:  zlib library not found, go get it from"
    echo "   http://www.zlib.net"
    exit
fi
LIBS="$LIBS -lz"

#--------------------------------------------------------------------------
# lzma (optional)
#--------------------------------------------------------------------------

AC_ARG_WITH(lzma_includes,
    AC_HELP_STRING([--with-lzma-includes=DIR],[liblzma include directory]),
    [with_lzma_includes="$withval"],[with_lzma_includes="no"])

if test "x$with_lzma_includes" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -I${with_lzma_includes}"
fi

AC_ARG_WITH(lzma_libraries,
    AC_HELP_STRING([--with-lzma-libraries=DIR],[liblzma library directory]),
    [with_lzma_libraries="$withval"],[with_lzma_libraries="no"])

if test "x$with_lzma_libraries" != "xno"; then
    LDFLAGS="${LDFLAGS} -L${with_lzma_libraries}"
fi

AC_CHECK_HEADERS(lzma.h, LZMA_HEADERS="yes", LZMA_HEADERS="no")
AC_CHECK_LIB(lzma, lzma_code, LZMA_LIB="yes", LZMA_LIB="no")

if test "x$LZMA_LIB" != "xno"; then
    if test "x$LZMA_HEADERS" != "xno"; then
        AC_DEFINE([HAVE_LZMA],[1],[can build lzma code])
        LIBS="${LIBS} -llzma"
    fi
fi

#--------------------------------------------------------------------------
# ssl / crypto (optional)
#--------------------------------------------------------------------------

AC_ARG_WITH(openssl_includes,
    AC_HELP_STRING([--with-openssl-includes=DIR],[openssl include directory]),
    [with_openssl_includes="$withval"],[with_openssl_includes="no"])

if test "x$with_openssl_includes" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -I${with_openssl_includes}"
fi

AC_ARG_WITH(openssl_libraries,
    AC_HELP_STRING([--with-openssl-libraries=DIR],[openssl library directory]),
    [with_openssl_libraries="$withval"],[with_openssl_libraries="no"])

if test "x$with_openssl_libraries" != "xno"; then
    LDFLAGS="${LDFLAGS} -L${with_openssl_libraries}"
fi

AC_CHECK_LIB([crypto],[SHA256_Init],AC_DEFINE([HAVE_OPENSSL_SHA],[1],openssl SHA available),)
AC_CHECK_LIB([crypto],[MD5_Init],AC_DEFINE([HAVE_OPENSSL_MD5],[1],openssl MD5 available),)

AM_CONDITIONAL([BUILD_SSL_MD5], test "x$ac_cv_lib_crypto_MD5_Init" != "xyes" )
AM_CONDITIONAL([BUILD_SSL_SHA], test "x$ac_cv_lib_crypto_SHA256_Init" != "xyes" )

if test "x$ac_cv_lib_crypto_MD5_Init" = "xyes"; then
    LIBS="${LIBS} -lcrypto"
fi

#--------------------------------------------------------------------------
# intel soft cpm (optional)
#--------------------------------------------------------------------------

enable_intel_soft_cpm="yes"

AC_ARG_WITH(intel_soft_cpm_includes,
    AC_HELP_STRING([--with-intel-soft-cpm-includes=DIR],[Intel Soft CPM include directory]),
    [with_intel_soft_cpm_includes="$withval"],[with_intel_soft_cpm_includes="no"])

if test "x$with_intel_soft_cpm_includes" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -I${with_intel_soft_cpm_includes}"
else
    enable_intel_soft_cpm="no"
fi

AC_ARG_WITH(intel_soft_cpm_libraries,
    AC_HELP_STRING([--with-intel-soft-cpm-libraries=DIR],[Intel Soft CPM library directory]),
    [with_intel_soft_cpm_libraries="$withval"],[with_intel_soft_cpm_libraries="no"])

if test "x$with_intel_soft_cpm_libraries" != "xno"; then
    LDFLAGS="${LDFLAGS} -L${with_intel_soft_cpm_libraries}"
    LIBS="${LIBS} -lpm"
else
    enable_intel_soft_cpm="no"
fi

AM_CONDITIONAL(HAVE_INTEL_SOFT_CPM, test "x$enable_intel_soft_cpm" = "xyes")

if test "x$enable_intel_soft_cpm" = "xyes"; then
    AC_DEFINE(INTEL_SOFT_CPM, [1], [enable intel cpm support in build])
fi

#--------------------------------------------------------------------------
# hyperscan (optional)
#--------------------------------------------------------------------------

AC_MSG_CHECKING([for hyperscan pkg-config presence])
PKG_CHECK_EXISTS([libhs], [ have_hyperscan_pkgconfig="yes" ], [ have_hyperscan_pkgconfig="no" ])
AC_MSG_RESULT(${have_hyperscan_pkgconfig})

if test "${have_hyperscan_pkgconfig}" = "yes" ; then
    HYPERSCAN_CFLAGS=`${PKG_CONFIG} --cflags libhs`
    HYPERSCAN_LDFLAGS=`${PKG_CONFIG} --libs-only-L libhs`
fi

AC_ARG_WITH(hyperscan_includes,
    AC_HELP_STRING([--with-hyperscan-includes=DIR],[libhs include directory]),
    [with_hyperscan_includes="$withval"],[with_hyperscan_includes="no"])

if test "x$with_hyperscan_includes" != "xno"; then
    HYPERSCAN_CFLAGS="-I${with_hyperscan_includes}"
fi

AC_ARG_WITH(hyperscan_libraries,
    AC_HELP_STRING([--with-hyperscan-libraries=DIR],[libhs library directory]),
    [with_hyperscan_libraries="$withval"],[with_hyperscan_libraries="no"])

if test "x$with_hyperscan_libraries" != "xno"; then
    HYPERSCAN_LDFLAGS="-L${with_hyperscan_libraries}"
fi

CPPFLAGS="${CPPFLAGS} ${HYPERSCAN_CFLAGS}"
LDFLAGS="${LDFLAGS} ${HYPERSCAN_LDFLAGS}"

AC_CHECK_HEADERS(hs.h, HS_HEADERS="yes", HS_HEADERS="no")
AC_CHECK_LIB(hs, hs_scan, HS_LIB="yes", HS_LIB="no")

if test "x$HS_LIB" != "xno"; then
    if test "x$HS_HEADERS" != "xno"; then
        AC_DEFINE([HAVE_HYPERSCAN],[1],[can build hyperscan code])
        LIBS="${LIBS} -lhs"
    fi
fi

AM_CONDITIONAL([HAVE_HYPERSCAN], [test "x$HS_HEADERS" = "xyes" -a "x$HS_LIB" = "xyes"])

#--------------------------------------------------------------------------
# outputs
#--------------------------------------------------------------------------

AC_DEFINE_UNQUOTED([API_OPTIONS], ["$API_OPTIONS"], [plugin api related build options])

if test -n "$ASAN_CCXFLAGS" ; then
    CFLAGS="$ASAN_CCXFLAGS $CFLAGS"
    CXXFLAGS="$ASAN_CCXFLAGS $CXXFLAGS"
fi

if test "x${CODE_COVERAGE_ENABLED}" = "xyes" ; then
    CFLAGS=`echo $CFLAGS | ${SED} 's/-O\w//g'`
    CXXFLAGS=`echo $CXXFLAGS | ${SED} 's/-O\w//g'`
fi

# Global defaults for AM_* flags.  Can be overridden on a per-Makefile.am basis.
AM_CPPFLAGS='-I$(top_builddir) -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/src/network_inspectors'
AM_CPPFLAGS="${AM_CPPFLAGS} ${CODE_COVERAGE_CPPFLAGS}"
AM_CFLAGS="${CODE_COVERAGE_CFLAGS}"
AM_CXXFLAGS="${CODE_COVERAGE_CXXFLAGS}"
AM_LDFLAGS="${CODE_COVERAGE_LDFLAGS}"

# Special AM_CPPFLAGS for snort2lua.
snort2lua_CPPFLAGS='-I$(top_builddir) -I$(top_srcdir)/tools/snort2lua'
snort2lua_CPPFLAGS="${snort2lua_CPPFLAGS} ${CODE_COVERAGE_CPPFLAGS}"

AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LDFLAGS)

AC_SUBST(snort2lua_CPPFLAGS)

AC_SUBST(CPPUTEST_CPPFLAGS)
AC_SUBST(CPPUTEST_LDFLAGS)

AC_CONFIG_FILES([ \
snort.pc \
Makefile \
src/Makefile \
src/actions/Makefile \
src/codecs/Makefile \
src/codecs/root/Makefile \
src/codecs/link/Makefile \
src/codecs/ip/Makefile \
src/codecs/misc/Makefile \
src/control/Makefile \
src/decompress/Makefile \
src/detection/Makefile \
src/events/Makefile \
src/file_api/Makefile \
src/filters/Makefile \
src/flow/Makefile \
src/framework/Makefile \
src/hash/Makefile \
src/hash/test/Makefile \
src/helpers/Makefile \
src/lua/Makefile \
src/ips_options/Makefile \
src/ips_options/test/Makefile \
src/latency/Makefile \
src/log/Makefile \
src/loggers/Makefile    \
src/main/Makefile \
src/managers/Makefile \
src/memory/Makefile \
src/mime/Makefile \
src/stream/Makefile \
src/stream/base/Makefile \
src/stream/ip/Makefile \
src/stream/icmp/Makefile \
src/stream/libtcp/Makefile \
src/stream/tcp/Makefile \
src/stream/udp/Makefile \
src/stream/user/Makefile \
src/stream/file/Makefile \
src/stream/tcp/test/Makefile \
src/network_inspectors/Makefile \
src/network_inspectors/arp_spoof/Makefile \
src/network_inspectors/binder/Makefile \
src/network_inspectors/normalize/Makefile \
src/network_inspectors/perf_monitor/Makefile \
src/network_inspectors/port_scan/Makefile \
src/network_inspectors/reputation/Makefile \
src/packet_io/Makefile \
src/parser/Makefile \
src/piglet/Makefile \
src/piglet_plugins/Makefile \
src/service_inspectors/Makefile     \
src/service_inspectors/back_orifice/Makefile \
src/service_inspectors/dce_rpc/Makefile \
src/service_inspectors/dnp3/Makefile \
src/service_inspectors/dns/Makefile \
src/service_inspectors/ftp_telnet/Makefile \
src/service_inspectors/gtp/Makefile \
src/service_inspectors/http_inspect/Makefile \
src/service_inspectors/imap/Makefile \
src/service_inspectors/modbus/Makefile \
src/service_inspectors/nhttp_inspect/Makefile \
src/service_inspectors/pop/Makefile \
src/service_inspectors/rpc_decode/Makefile \
src/service_inspectors/sip/Makefile \
src/service_inspectors/smtp/Makefile \
src/service_inspectors/ssh/Makefile \
src/service_inspectors/ssl/Makefile \
src/service_inspectors/wizard/Makefile \
src/ports/Makefile \
src/protocols/Makefile \
src/search_engines/Makefile \
src/search_engines/test/Makefile \
src/sfip/Makefile \
src/side_channel/Makefile \
src/connectors/Makefile \
src/connectors/file_connector/Makefile \
src/sfrt/Makefile \
src/target_based/Makefile \
src/host_tracker/Makefile \
src/host_tracker/test/Makefile \
src/catch/Makefile \
src/time/Makefile \
src/profiler/Makefile \
src/utils/Makefile \
src/utils/test/Makefile \
lua/Makefile \
doc/Makefile \
daqs/Makefile \
tools/Makefile \
tools/u2boat/Makefile \
tools/u2spewfoo/Makefile \
tools/snort2lua/Makefile \
tools/snort2lua/config_states/Makefile \
tools/snort2lua/data/Makefile \
tools/snort2lua/data/data_types/Makefile \
tools/snort2lua/keyword_states/Makefile \
tools/snort2lua/output_states/Makefile \
tools/snort2lua/preprocessor_states/Makefile \
tools/snort2lua/rule_states/Makefile \
tools/snort2lua/helpers/Makefile \
])

AC_OUTPUT
AC_MSG_RESULT([
    $PACKAGE $VERSION
    ========

    prefix:         ${prefix}
    sysconfdir:     ${sysconfdir}
    libdir:         ${libdir}
    includedir:     ${includedir}

    compiler:       ${CC}
    compiler++:     ${CXX}
    cflags:         ${CFLAGS}
    cxxflags:       ${CXXFLAGS}
    cppflags:       ${CPPFLAGS}
    ldflags:        ${LDFLAGS}
    libs:           ${LIBS}

    am_cppflags:    ${AM_CPPFLAGS}
    am_cflags:      ${AM_CFLAGS}
    am_cxxflags:    ${AM_CXXFLAGS}
    am_ldflags:     ${AM_LDFLAGS}
])
