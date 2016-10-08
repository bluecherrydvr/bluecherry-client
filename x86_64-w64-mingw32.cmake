# Define the environment for cross compiling from Linux to Win32
SET(CMAKE_SYSTEM_NAME    Windows) # Target system name
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_C_COMPILER     "x86_64-w64-mingw32-gcc")
SET(CMAKE_CXX_COMPILER   "x86_64-w64-mingw32-g++")
SET(CMAKE_RC_COMPILER    "x86_64-w64-mingw32-windres")
SET(CMAKE_RANLIB         "x86_64-w64-mingw32-ranlib")

SET(QT_MOC_EXECUTABLE "/lib/qt4/x86_64-w64-mingw32/bin/moc.exe")
SET(QT_UIC_EXECUTABLE "/lib/qt4/x86_64-w64-mingw32/bin/uic.exe")
SET(QT_RCC_EXECUTABLE "/lib/qt4/x86_64-w64-mingw32/bin/rcc.exe")
SET(QT_LUPDATE_EXECUTABLE "/lib/qt4/x86_64-w64-mingw32/bin/lupdate.exe")
SET(QT_LRELEASE_EXECUTABLE "/lib/qt4/x86_64-w64-mingw32/bin/lrelease.exe")

# Configure the behaviour of the find commands 
#SET(CMAKE_FIND_ROOT_PATH "/usr/x86_64-w64-mingw32")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
