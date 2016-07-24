ExternalProject_Add(mplayer
    PREFIX mplayer
    URL "http://www.mplayerhq.hu/MPlayer/releases/MPlayer-1.3.0.tar.gz"

    CONFIGURE_COMMAND ./configure 
        --cc=${CMAKE_C_COMPILER}
        --prefix=/usr 
	--confdir=/etc/bluecherry 
	--datadir=/usr/share/bluecherry-player 
	--libdir=/usr/lib/bluecherry/client
	--disable-mencoder 
	--disable-gui 
	--enable-menu 
	--disable-arts 
	--disable-tv 
	--disable-mp3lame 
	--extra-cflags=-I${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include 
       	--enable-runtime-cpudetection
       	--enable-debug
	--disable-avx2

    BUILD_IN_SOURCE 1
    BUILD_COMMAND make -j ${CPU_CORES_COUNT}
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR}/ffmpeg/install install 
)
#ExternalProject_Get_Property(EXT_BOOST INSTALL_DIR)
#set(BOOST_ROOT "${CMAKE_BINARY_DIR}/InstalledExternals")
