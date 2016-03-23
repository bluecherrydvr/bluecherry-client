ExternalProject_Add(ffmpeg
    PREFIX ffmpeg
    URL "https://github.com/FFmpeg/FFmpeg/releases/download/n3.0/ffmpeg-3.0.tar.gz"
    URL_HASH MD5=123af0b03e4b3819eaf3c1e4ae123d46

    CONFIGURE_COMMAND ./configure
	--cc=${CMAKE_C_COMPILER} 
	--cxx=${CMAKE_CXX_COMPILER} 
        --prefix=/usr
	--libdir=/usr/lib/bluecherry/client
        --enable-pic 
	#--enable-lto
	--enable-rpath
        --disable-stripping 
        --disable-doc
        --disable-everything
	--disable-programs
	--disable-static
        --enable-shared
                
	--enable-opengl
        --enable-xlib 
        --disable-sdl 
                
	--enable-outdev=opengl
	--enable-outdev=xv

        --enable-protocol=file
        --enable-protocol=pipe
        --enable-protocol=http
        --enable-protocol=https
              
        --enable-muxer=matroska
        --enable-muxer=mjpeg
        --enable-muxer=rtp
        --enable-muxer=mp4
        --enable-muxer=image2
        --enable-muxer=rawvideo
              
        --enable-demuxer=matroska
        --enable-demuxer=mjpeg
        --enable-demuxer=rtp
        --enable-demuxer=rtsp
        --enable-demuxer=image2
        --enable-demuxer=rawvideo
               
        --enable-decoder=h264
        --enable-decoder=mpeg4
        --enable-decoder=mjpeg
	--enable-decoder=rawvideo
                
	--enable-parser=h264
        --enable-parser=mpeg4video
        --enable-parser=mjpeg
                
        --enable-encoder=mjpeg
        --enable-encoder=mpeg4
        --enable-encoder=rawvideo
        
	--enable-filter=scale
        --enable-filter=fps

    BUILD_IN_SOURCE 1
    BUILD_COMMAND make -j ${CPU_CORES_COUNT}
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR}/ffmpeg/install install 
)
#ExternalProject_Get_Property(ffmpeg INSTALL_DIR)

set( LIBAVCODEC_INCLUDE_DIRS  "${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include" )
set( LIBAVFORMAT_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include" )
set( LIBAVUTIL_INCLUDE_DIRS   "${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include" )
set( LIBSWSCALE_INCLUDE_DIRS  "${CMAKE_BINARY_DIR}/ffmpeg/install/usr/include" )

set( LIBAVCODEC_LIBRARIES  
	"${CMAKE_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry/client/libavcodec${CMAKE_SHARED_LIBRARY_SUFFIX}" )
set( LIBAVFORMAT_LIBRARIES 
	"${CMAKE_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry/client/libavformat${CMAKE_SHARED_LIBRARY_SUFFIX}" )
set( LIBAVUTIL_LIBRARIES   
	"${CMAKE_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry/client/libavutil${CMAKE_SHARED_LIBRARY_SUFFIX}" )
set( LIBSWSCALE_LIBRARIES  
	"${CMAKE_BINARY_DIR}/ffmpeg/install/usr/lib/bluecherry/client/libswscale${CMAKE_SHARED_LIBRARY_SUFFIX}" )
