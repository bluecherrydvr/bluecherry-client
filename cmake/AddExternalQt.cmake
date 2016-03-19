ExternalProject_Add( qt
    PREFIX qt
    EXCLUDE_FROM_ALL 1
    URL "http://download.qt.io/official_releases/qt/4.8/4.8.7/qt-everywhere-opensource-src-4.8.7.tar.gz"
    URL_HASH MD5=d990ee66bf7ab0c785589776f35ba6ad

    CONFIGURE_COMMAND ./configure 
	-prefix ${CMAKE_BINARY_DIR}/qt/install/usr 
	-prefix-install
	-confirm-license -opensource 
	
	-no-qt3support 
        -no-xmlpatterns
	-openssl 
	-opengl desktop 
	-webkit 
	-gtkstyle 
	-nomake demos
	-nomake examples
	-nomake docs
	-no-multimedia
	-no-audio-backend
	-no-phonon 
	-no-phonon-backend 
	-no-svg 
	-script 
	-no-scripttools 
	-declarative 
	-no-declarative-debug
	-rpath -release -arch x86
    
    BUILD_IN_SOURCE 1
    BUILD_COMMAND make -j ${CPU_CORES_COUNT}
    # -prefix $HOME/dev/usr
    INSTALL_COMMAND make install 
)

#set( LIBAVCODEC_INCLUDE_DIRS  "${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include" )
#set( LIBAVFORMAT_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include" )
#set( LIBAVUTIL_INCLUDE_DIRS   "${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include" )
#set( LIBSWSCALE_INCLUDE_DIRS  "${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include" )

#set( LIBAVCODEC_LIBRARIES  
#	"${CMAKE_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry/client/libavcodec${CMAKE_SHARED_LIBRARY_SUFFIX}" )
#set( LIBAVFORMAT_LIBRARIES 
#	"${CMAKE_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry/client/libavformat${CMAKE_SHARED_LIBRARY_SUFFIX}" )
#set( LIBAVUTIL_LIBRARIES   
#	"${CMAKE_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry/client/libavutil${CMAKE_SHARED_LIBRARY_SUFFIX}" )
#set( LIBSWSCALE_LIBRARIES  
#	"${CMAKE_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry/client/libswscale${CMAKE_SHARED_LIBRARY_SUFFIX}" )
