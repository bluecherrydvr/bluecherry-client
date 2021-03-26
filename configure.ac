AC_PREREQ([2.68])
AC_INIT([Bluecherry client], [2.2.8dev], [https://github.com/bluecherrydvr/bluecherry-client/issues], [bluecherry-client], [https://bluecherrydvr.com])
AM_INIT_AUTOMAKE([-Wall  foreign subdir-objects])
AC_CONFIG_SRCDIR([src/main.cpp])

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_SYS_LARGEFILE

# Platform-specific environment settings
AS_CASE([$host],
	[*-apple-darwin*], [LDFLAGS="$LDFLAGS -headerpad_max_install_names"],
        [*-w64-mingw*], [
			AC_CHECK_LIB([ws2_32], [main], [LIBS="$LIBS -lws2_32"], [])
			],
			[]
)

AC_ARG_WITH([qt],
  [AS_HELP_STRING([--with-qt=qt4|qt5 qt5 is default],
  [Select Qt version, 4 or 5])],
  [qt_version=$withval],
  [qt_version=qt5])

AS_IF([test "x$qt_version" = xqt5],
	[
	QT_MODULES="Qt5Core Qt5Gui Qt5Sql Qt5Network Qt5Widgets Qt5Concurrent"
	QT_UIC_NAMES="uic-qt5 uic5 uic"
	QT_MOC_NAMES="moc-qt5 moc5 moc"
	QT_RCC_NAMES="rcc-qt5 rcc5 rcc"
	CFLAGS="-fPIC $CFLAGS"
	CXXFLAGS="-fPIC $CXXFLAGS"
	CPPFLAGS="-DQT_DISABLE_DEPRECATED_BEFORE=0 $CPPFLAGS"
	],
  [test "x$qt_version" = xqt4],
  	[
	QT_MODULES="QtCore QtGui QtSql QtNetwork QtDeclarative QtOpenGL"
	QT_UIC_NAMES="uic-qt4 uic4 uic"
	QT_MOC_NAMES="moc-qt4 moc4 moc"
	QT_RCC_NAMES="rcc-qt4 rcc4 rcc"
	],
  [AC_MSG_ERROR(["Invalid Qt version"])])

PKG_CHECK_MODULES([QT_DEPENDENCIES], [$QT_MODULES])
AS_CASE([$host],
	[*-w64-mingw*], [QT_DEPENDENCIES_LIBS="$QT_DEPENDENCIES_LIBS -mwindows"]
	[]
)
AC_SUBST([QT_DEPENDENCIES_CFLAGS])
AC_SUBST([QT_DEPENDENCIES_LIBS])
AC_CHECK_PROGS([UIC], [$QT_UIC_NAMES], [notfound])
AC_CHECK_PROGS([MOC], [$QT_MOC_NAMES], [notfound])
AC_CHECK_PROGS([RCC], [$QT_RCC_NAMES], [notfound])
#fail if not found ...
AS_IF([test "x$UIC" = xnotfound],[AC_MSG_ERROR(["uic program not found. Please install Qt development package"])])
AS_IF([test "x$MOC" = xnotfound],[AC_MSG_ERROR(["moc program not found. Please install Qt development package"])])

AC_CHECK_LIB([pthread], [pthread_create])

PKG_CHECK_MODULES(FFMPEG, libavutil libavformat libavcodec libswscale, HAVE_FFMPEG=yes, AC_MSG_ERROR(["FFMpeg libraries not found"]))

PKG_CHECK_MODULES(SDL2, sdl2, HAVE_LIBSDL2=yes, AC_MSG_ERROR(["libSDL2 not found"]))
PKG_CHECK_MODULES(MPV, mpv, HAVE_MPV=yes, AC_MSG_ERROR(["libmpv not found"]))

VERSION=$PACKAGE_VERSION
AC_SUBST([VERSION])
AC_CONFIG_FILES([Makefile src/bluecherry-config.h])
AC_OUTPUT

