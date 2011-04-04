#!/bin/bash

# Assuming that you've built all of the gstreamer dependencies as expected,
# this script will copy them to their local location, and modify them as necessary for
# the bundled distribution. Use /opt/local/ for macports, and /usr/local/ for homebrew.

# Suggested usage is to use a local installation of homebrew, modify it to
# always build with ENV.m32, and use that as the base.

if [ -z $1 ]; then
	echo "Usage: ./import.sh [path-to-libs-prefix]"
	exit 1;
fi

BASE=$1

rm -r lib include plugins
mkdir lib
mkdir plugins
mkdir include

# gstreamer, gst-plugins-base
cp ${BASE}/lib/{libgstreamer,libgstbase,libgstapp,libgstvideo,libgstinterfaces}-0.10.0.dylib lib/
cp -RL ${BASE}/include/gstreamer-0.10/ include/

# more gstreamer and gst-plugins-base, as required by our plugins
cp ${BASE}/lib/libgst{riff,audio,tag,pbutils}-0.10.0.dylib lib/

# gstreamer plugins (gst-plugins-base, gst-plugins-good, gst-ffmpeg)
cp ${BASE}/lib/gstreamer-0.10/libgst{coreelements,app,decodebin2,matroska,osxaudio,ffmpegcolorspace,ffmpeg,typefindfunctions}.so plugins/

# glib
cp ${BASE}/lib/{libglib,libgobject,libgthread,libgmodule,libgio}-2.0.0.dylib lib/
cp -RL ${BASE}/include/glib-2.0/ include/
cp ${BASE}/lib/glib-2.0/include/glibconfig.h include/

# other libraries
cp ${BASE}/lib/{libiconv.2.dylib,libintl.8.dylib,liborc-0.4.0.dylib,libz.1.dylib,libbz2.1.0.dylib} lib/

chmod u+w lib/* plugins/*

# Replace library paths
../../mac/replacepath.py --old ${BASE}'*' --new @loader_path/ --dir lib -R
../../mac/replacepath.py --old ${BASE}'*' --new @loader_path/ --dir plugins -R
