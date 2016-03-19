ExternalProject_Add(mplayer
    PREFIX mplayer
    URL "http://www.mplayerhq.hu/MPlayer/releases/MPlayer-1.3.0.tar.gz"
    URL_HASH MD5=8786e3c61f7ab64d27c2fb965d68d883

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

    BUILD_IN_SOURCE 1
    BUILD_COMMAND make -j ${CPU_CORES_COUNT}
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR}/ffmpeg/install install 
)
#ExternalProject_Get_Property(EXT_BOOST INSTALL_DIR)
#set(BOOST_ROOT "${CMAKE_BINARY_DIR}/InstalledExternals")
