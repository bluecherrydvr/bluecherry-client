#
# Copyright 2010-2013 Bluecherry
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

QT += core gui network webkit opengl declarative
CONFIG(static):QTPLUGIN += qjpeg qgif

TARGET = BluecherryClient
TEMPLATE = app

unix:!macx {
    error("qMake builds are not supported on Linux anymore, use CMake instead")
}
win32 {
    error("qMake builds are not supported on Windows anymore, use CMake instead")
}

macx:QMAKE_POST_LINK += cp $${_PRO_FILE_PWD_}/mac/Info.plist $${OUT_PWD}/$${TARGET}.app/Contents/;

# __STDC_CONSTANT_MACROS is necessary for libav on Linux
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII __STDC_CONSTANT_MACROS
INCLUDEPATH += src

win32-msvc2008|win32-msvc2010 {
    # Qt defaults to setting this, breakpad defaults to not. Qt doesn't use wchar_t in the
    # public API, so we can use a different setting. Otherwise, it would cause linker errors.
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    QMAKE_CXXFLAGS_RELEASE += -Zi
    # We want REF,ICF, but as of now (MSVC 2010, beta8), doing so results in
    # a binary that expects g_free to be in avformat-54.dll, not glib-2.0-0.dll.
    # Once gstreamer (and thus, glib) is gone, this can be re-enabled.
    QMAKE_LFLAGS_RELEASE += /DEBUG #/OPT:REF,ICF
}

isEmpty(LIBAV_PATH):error(Set LIBAV_PATH to the libav installed prefix)
INCLUDEPATH += "$$LIBAV_PATH/include"
LIBS += -L"$$LIBAV_PATH/lib" -lavformat -lavcodec -lavutil -lswscale
# Is this necessary with shared libav?
macx:LIBS += -framework QuartzCore -framework VideoDecodeAcceleration

macx {
    # Bundled GStreamer
    CONFIG(x86_64):error(GStreamer cannot currently be built for x86_64 on OS X)

    DEFINES += USE_GSTREAMER
    GSTREAMER_PATH = "$$PWD/gstreamer-bin/mac"
    LIBS += -L"$$GSTREAMER_PATH/lib" -lgstreamer-0.10.0 -lgstapp-0.10.0 -lgstvideo-0.10.0 -lgstinterfaces-0.10.0 -lglib-2.0.0 -lgobject-2.0.0
    INCLUDEPATH += "$$GSTREAMER_PATH/include" "/usr/include/libxml2"

    QMAKE_POST_LINK += cd $$PWD; mac/deploy.sh $${OUT_PWD}/$${TARGET}.app
    CONFIG(release, debug|release):QMAKE_POST_LINK += " $$[QT_INSTALL_BINS]/macdeployqt"
    QMAKE_POST_LINK += "; cd - >/dev/null;"
}

!CONFIG(no-breakpad) {
    DEFINES += USE_BREAKPAD
    INCLUDEPATH += "$$PWD/breakpad/src"
    SOURCES += src/utils/Breakpad.cpp

    macx {
        QMAKE_LFLAGS += -F$$PWD/breakpad-bin/mac
        LIBS += -framework Breakpad

        QMAKE_POST_LINK += cd "$$PWD"; breakpad-bin/mac/gather_symbols.sh $${OUT_PWD}/$${TARGET}; cd - >/dev/null;
    }
}

SOURCES += src/main.cpp \
    src/ui/MainWindow.cpp \
    src/ui/OptionsDialog.cpp \
    src/core/DVRServer.cpp \
    src/core/BluecherryApp.cpp \
    src/ui/DVRServersModel.cpp \
    src/ui/DVRServersView.cpp \
    src/ui/OptionsServerPage.cpp \
    src/ui/NumericOffsetWidget.cpp \
    src/core/DVRCamera.cpp \
    src/core/MJpegStream.cpp \
    src/ui/EventsModel.cpp \
    src/ui/EventsWindow.cpp \
    src/ui/EventTimelineWidget.cpp \
    src/utils/ThreadTask.cpp \
    src/utils/ThreadTaskCourier.cpp \
    src/utils/ImageDecodeTask.cpp \
    src/ui/EventViewWindow.cpp \
    src/ui/EventTagsView.cpp \
    src/ui/EventTagsModel.cpp \
    src/ui/EventTagsDelegate.cpp \
    src/ui/EventCommentsWidget.cpp \
    src/ui/ExpandingTextEdit.cpp \
    src/ui/CrashReportDialog.cpp \
    src/ui/EventSourcesModel.cpp \
    src/core/EventData.cpp \
    src/ui/EventTypesFilter.cpp \
    src/ui/EventVideoPlayer.cpp \
    src/video/VideoPlayerBackend_gst.cpp \
    src/video/VideoHttpBuffer.cpp \
    src/ui/ServerConfigWindow.cpp \
    src/ui/AboutDialog.cpp \
    src/ui/SavedLayoutsModel.cpp \
    src/core/ServerRequestManager.cpp \
    src/video/GstSinkWidget.cpp \
    src/ui/OptionsGeneralPage.cpp \
    src/ui/EventsView.cpp \
    src/utils/FileUtils.cpp \
    src/ui/liveview/LiveViewArea.cpp \
    src/ui/liveview/LiveViewLayout.cpp \
    src/ui/liveview/LiveFeedItem.cpp \
    src/ui/liveview/LiveViewWindow.cpp \
    src/core/CameraPtzControl.cpp \
    src/ui/liveview/PtzPresetsWindow.cpp \
    src/core/PtzPresetsModel.cpp \
    src/ui/liveview/LiveViewGradients.cpp \
    src/ui/SetupWizard.cpp \
    src/video/MediaDownload.cpp \
    src/utils/RangeMap.cpp \
    src/ui/MacSplitter.cpp \
    src/core/TransferRateCalculator.cpp \
    src/utils/StringUtils.cpp \
    src/core/LiveViewManager.cpp \
    src/core/LiveStream.cpp \
    src/core/LiveStreamWorker.cpp \
    src/ui/liveview/LiveStreamItem.cpp \
    src/ui/StatusBarServerAlert.cpp \
    src/network/SocketError.cpp \
    src/network/RemotePortChecker.cpp \
    src/ui/RemotePortCheckerWidget.cpp \
    src/ui/WebRtpPortCheckerWidget.cpp \
    src/event/EventsLoader.cpp \
    src/event/EventsCursor.cpp \
    src/event/ModelEventsCursor.cpp \
    src/ui/SwitchEventsWidget.cpp \
    src/network/MediaDownloadManager.cpp \
    src/event/EventDownloadManager.cpp \
    src/event/EventVideoDownload.cpp \
    src/event/EventList.cpp \
    src/event/EventFilter.cpp \
    src/event/MediaEventFilter.cpp \
    src/event/CameraEventFilter.cpp \
    src/ui/EventVideoDownloadWidget.cpp \
    src/ui/EventVideoDownloadsWindow.cpp \
    src/utils/Range.cpp

!macx:SOURCES += src/ui/StatusBandwidthWidget.cpp

HEADERS  += src/ui/MainWindow.h \
    src/ui/OptionsDialog.h \
    src/core/DVRServer.h \
    src/core/BluecherryApp.h \
    src/ui/DVRServersModel.h \
    src/ui/DVRServersView.h \
    src/ui/OptionsServerPage.h \
    src/ui/NumericOffsetWidget.h \
    src/core/DVRCamera.h \
    src/core/MJpegStream.h \
    src/ui/EventsModel.h \
    src/ui/EventsWindow.h \
    src/ui/EventTimelineWidget.h \
    src/utils/ThreadTask.h \
    src/utils/ThreadTaskCourier.h \
    src/utils/ImageDecodeTask.h \
    src/ui/EventViewWindow.h \
    src/ui/EventTagsView.h \
    src/ui/EventTagsModel.h \
    src/ui/EventTagsDelegate.h \
    src/ui/EventCommentsWidget.h \
    src/ui/ExpandingTextEdit.h \
    src/ui/CrashReportDialog.h \
    src/ui/EventSourcesModel.h \
    src/core/EventData.h \
    src/ui/EventTypesFilter.h \
    src/ui/EventVideoPlayer.h \
    src/video/VideoPlayerBackend_gst.h \
    src/video/VideoHttpBuffer.h \
    src/ui/ServerConfigWindow.h \
    src/ui/AboutDialog.h \
    src/ui/SavedLayoutsModel.h \
    src/core/ServerRequestManager.h \
    src/video/GstSinkWidget.h \
    src/ui/OptionsGeneralPage.h \
    src/ui/EventsView.h \
    src/utils/FileUtils.h \
    src/ui/liveview/LiveViewArea.h \
    src/ui/liveview/LiveViewLayout.h \
    src/ui/liveview/LiveFeedItem.h \
    src/ui/liveview/LiveViewWindow.h \
    src/core/CameraPtzControl.h \
    src/ui/liveview/PtzPresetsWindow.h \
    src/core/PtzPresetsModel.h \
    src/ui/liveview/LiveViewGradients.h \
    src/ui/SetupWizard.h \
    src/ui/SetupWizard_p.h \
    src/video/MediaDownload.h \
    src/utils/RangeMap.h \
    src/video/MediaDownload_p.h \
    src/ui/MacSplitter.h \
    src/core/TransferRateCalculator.h \
    src/ui/StatusBandwidthWidget.h \
    src/utils/StringUtils.h \
    src/core/LiveViewManager.h \
    src/core/LiveStream.h \
    src/core/LiveStreamWorker.h \
    src/ui/liveview/LiveStreamItem.h \
    src/ui/StatusBarServerAlert.h \
    src/network/SocketError.h \
    src/network/RemotePortChecker.h \
    src/ui/RemotePortCheckerWidget.h \
    src/ui/WebRtpPortCheckerWidget.h \
    src/event/EventsLoader.h \
    src/event/EventsCursor.h \
    src/event/ModelEventsCursor.h \
    src/ui/SwitchEventsWidget.h \
    src/network/MediaDownloadManager.h \
    src/event/EventDownloadManager.h \
    src/event/EventVideoDownload.h \
    src/event/EventList.h \
    src/event/EventFilter.h \
    src/event/MediaEventFilter.h \
    src/event/CameraEventFilter.h \
    src/ui/EventVideoDownloadWidget.h \
    src/ui/EventVideoDownloadsWindow.h \
    src/utils/Range.h

RESOURCES += \
    res/resources.qrc \
    src/ui/qml.qrc
win32:RC_FILE = res/windows.rc
ICON = res/bluecherry.icns

win32:SOURCES += src/utils/explorerstyle.cpp
win32:HEADERS += src/utils/explorerstyle.h

macx:OBJECTIVE_SOURCES += src/utils/PlatformOSX.mm \
    src/ui/StatusBandwidthWidget_mac.mm
macx:LIBS += -framework CoreServices -framework AppKit

OTHER_FILES += mac/Info.plist "linux/Bluecherry Client.desktop" \
    src/ui/liveview/LiveView.qml \
    src/ui/liveview/LiveFeed.qml \
    src/utils/PlatformOSX.mm \
    src/ui/liveview/HeaderPTZControl.qml \
    src/ui/StatusBandwidthWidget_mac.mm \
    win/installer.nsi \
    README.txt \
    COPYING \
    BUILD-WINDOWS.txt \
    debian/rules \
    debian/copyright \
    debian/control \
    debian/compat \
    debian/changelog
