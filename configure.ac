dnl Process this file with autoconf to produce a configure script.
AC_INIT(KevEdit, 0.5.1, bitman@users.sourceforge.net, kevedit)
AC_CONFIG_SRCDIR([src/kevedit/kevedit.c])
AM_CONFIG_HEADER([src/config.h])


dnl Detect host and target environments (target is needed by AM_PATH_SDL)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Use automake
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl Determine available displays
AC_MSG_NOTICE(Searching for available displays)

dnl Check for SDL
AC_ARG_WITH(sdl,
	[  --without-sdl           SDL support (default with)])

if test "x$with_sdl" != "xno"
then
	AM_PATH_SDL2(2.0.0, [AC_MSG_NOTICE(Using SDL display and sound)
			     sdl=true],
			    [AC_MSG_NOTICE(SDL not available for display)])

dnl This must be done before most checks because it can affect what headers
dnl and libraries are available
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

if test x$sdl = xtrue; then
	AC_DEFINE(SDL, [1], [Define to 1 if SDL is available])
	display_objects="$display_objects display_sdl.o"
	synth_objects="$synth_objects sdl_synth.o"
fi
#AM_CONDITIONAL(SDL, [test x$sdl = xtrue])
fi

dnl Check for windows (for glob's sake)
AC_MSG_CHECKING(for Windows)
case "$host" in
	*-*-*cygwin*)
		AC_DEFINE(WINDOWS32, [1], [Define to 1 if host is Windows])
		AC_MSG_RESULT(yes: cygwin)
		;;
	*-*-*mingw32*)
		AC_DEFINE(WINDOWS32, [1], [Define to 1 if host is Windows])
		AC_MSG_RESULT(yes: MingW)
		;;
	*)
		AC_MSG_RESULT(no)
		;;
esac

AC_MSG_CHECKING(for DOS)
case "$host" in
	*msdos*)
		AC_DEFINE(DOS, [1], [Define to 1 if host is DOS])
		display_objects="$display_objects display_dos.o"
		synth_objects="$synth_objects pcspeaker.o"
		AC_MSG_RESULT(yes)
		;;
	*)
		AC_MSG_RESULT(no)
		;;
esac


AC_ARG_WITH(dosbox, AC_HELP_STRING([--without-dosbox],[include DOSBox emulation support]),dosbox=$withval,dosbox=maybe)
if test "$dosbox" == "maybe"; then
	if test -n "$WINDOWS32" || test -n "$DOS"; then
		AC_MSG_NOTICE(DOS/Windows: Not enabling DOSBox support)
	else
		AC_MSG_NOTICE(Non-DOS/Windows: Enabling DOSBox support)
		dosbox=yes
	fi
fi

if test "$dosbox" == "yes"; then
	AC_CHECK_PROG(MKISOFS, mkisofs, mkisofs)
	if test -z "$MKISOFS"; then
		AC_MSG_ERROR(mkisofs is required to build DOSBox support)
	fi
	AC_SUBST(MKISOFS)
	AC_DEFINE(DOSBOX, [1], [Define to 1 if DOSBox is enabled])
fi
AM_CONDITIONAL(WITH_DOSBOX, [test x$dosbox != x])

AC_SUBST([display_objects])
AC_SUBST([synth_objects])

dnl Check for windows resource compiler.
AC_CHECK_PROGS(WINDRES, windres)
AC_SUBST(WINDRES)
AM_CONDITIONAL(HAVE_WINDRES, [test x$WINDRES != x])

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_DIRENT

dnl Checks for library functions.
AC_TYPE_SIGNAL
AC_CHECK_FUNCS(getcwd strdup strstr mkdtemp)

dnl Check for math library
AC_CHECK_LIB(m, floor)

dnl Check for availability of long file names
AC_SYS_LONG_FILE_NAMES

dnl Check for glob and add it to CPPFLAGS
AC_PROG_GNU_GLOB
CPPFLAGS="$CPPFLAGS $GLOBINC"

dnl Add some useful include paths
AC_SUBST(INCLUDE_PATHS) INCLUDE_PATHS='-I$(top_srcdir)/src -I$(top_builddir)'
CPPFLAGS="$CPPFLAGS $INCLUDE_PATHS"

AC_CONFIG_FILES([docs/Makefile
		 dosbox/Makefile
                 glob/Makefile
                 inst/kevedit.spec
                 src/libzzt2/Makefile
                 src/structures/Makefile
                 src/display/Makefile
                 src/synth/Makefile
                 src/themes/Makefile
                 src/themes/rusty/Makefile
                 src/texteditor/Makefile
                 src/help/Makefile
                 src/dialogs/Makefile
                 src/zlaunch/Makefile
                 src/kevedit/Makefile
                 src/Makefile
                 Makefile
                 buildinfo.h])

AC_OUTPUT
