QT += core gui network webkit opengl declarative
CONFIG(static):QTPLUGIN += qjpeg qgif

TARGET = BluecherryClient
TEMPLATE = app

unix:!macx {
    TARGET = bluecherry-client
    target.path = /usr/bin
    shortcut.path = /usr/share/applications
    shortcut.files = "linux/Bluecherry Client.desktop"
    resources.path = /usr/share/bluecherry-client
    resources.files = "res/bluecherry.png"
    INSTALLS += target shortcut resources
}

macx:QMAKE_POST_LINK += cp $${_PRO_FILE_PWD_}/mac/Info.plist $${OUT_PWD}/$${TARGET}.app/Contents/;

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
INCLUDEPATH += src

win32-msvc2008|win32-msvc2010 {
    # Qt defaults to setting this, breakpad defaults to not. Qt doesn't use wchar_t in the
    # public API, so we can use a different setting. Otherwise, it would cause linker errors.
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    QMAKE_CXXFLAGS_RELEASE += -Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF,ICF
}

unix:!macx {
    # GStreamer
    CONFIG += link_pkgconfig
    PKGCONFIG += gstreamer-0.10 gstreamer-interfaces-0.10 gstreamer-app-0.10 gstreamer-video-0.10
    DEFINES += USE_GSTREAMER
}

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

win32 {
    isEmpty(GSTREAMER_PATH) {
        message(Using bundled GStreamer)
        GSTREAMER_PATH = "$$PWD/gstreamer-bin/win"
    }
    INCLUDEPATH += "$${GSTREAMER_PATH}/include" "$${GSTREAMER_PATH}/include/gstreamer-0.10" "$${GSTREAMER_PATH}/include/glib-2.0" "$${GSTREAMER_PATH}/include/libxml2"
    LIBS += -L"$${GSTREAMER_PATH}/lib" gstreamer-0.10.lib gstinterfaces-0.10.lib gstapp-0.10.lib gstvideo-0.10.lib glib-2.0.lib gobject-2.0.lib
    DEFINES += USE_GSTREAMER
    CONFIG(debug, debug|release):DEFINES += GSTREAMER_PLUGINS=\\\"$$PWD/gstreamer-bin/win/plugins\\\"
    CONFIG(release, debug|release):DEFINES += GSTREAMER_PLUGINS=\\\"plugins\\\"
}

!CONFIG(no-breakpad) {
    DEFINES += USE_BREAKPAD
    INCLUDEPATH += "$$PWD/breakpad/src"
    SOURCES += src/utils/Breakpad.cpp

    unix:QMAKE_CXXFLAGS_RELEASE += -gstabs
    
    unix:!macx {
        BREAKPAD_LIB = "$$PWD/breakpad/src/client/linux/libbreakpad.a"
        BREAKPAD_DUMPSYMS = "$$PWD/breakpad/src/tools/linux/dump_syms/dump_syms"
        LIBS += $$BREAKPAD_LIB

        breakpad-client.target = $$BREAKPAD_LIB
        breakpad-client.commands = $(MAKE) -C "$$PWD/breakpad/src/client/linux/"
        breakpad-dumpsyms.target = $$BREAKPAD_DUMPSYMS
        breakpad-dumpsyms.commands = $(MAKE) -C "$$PWD/breakpad/src/tools/linux/dump_syms/"

        QMAKE_EXTRA_TARGETS += breakpad-client breakpad-dumpsyms
        PRE_TARGETDEPS += $$BREAKPAD_LIB $$BREAKPAD_DUMPSYMS

        QMAKE_POST_LINK = python "$$PWD/breakpad-bin/symbolstore.py" "$$BREAKPAD_DUMPSYMS" $${TARGET}.symbols $(TARGET); $$QMAKE_POST_LINK
    }

    macx {
        QMAKE_LFLAGS += -F$$PWD/breakpad-bin/mac
        LIBS += -framework Breakpad

        QMAKE_POST_LINK += cd "$$PWD"; breakpad-bin/mac/gather_symbols.sh $${OUT_PWD}/$${TARGET}; cd - >/dev/null;
    }

    win32 {
        CONFIG(debug, debug|release):LIBS += -L"$$PWD/breakpad-bin/win/lib-debug"
        CONFIG(release, debug|release):LIBS += -L"$$PWD/breakpad-bin/win/lib-release"
        LIBS += common.lib crash_generation_client.lib exception_handler.lib
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
    src/ui/EventVideoDownload.cpp \
    src/ui/ServerConfigWindow.cpp \
    src/ui/AboutDialog.cpp \
    src/ui/SavedLayoutsModel.cpp \
    src/core/ServerRequestManager.cpp \
    src/video/GstSinkWidget.cpp \
    src/ui/OptionsGeneralPage.cpp \
    src/ui/EventsView.cpp \
    src/utils/FileUtils.cpp \
    src/ui/liveview/LiveViewArea.cpp \
    src/ui/liveview/MJpegFeedItem.cpp \
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
    src/ui/StatusBandwidthWidget.cpp \
    src/utils/StringUtils.cpp

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
    src/video/VideoContainer.h \
    src/ui/EventVideoDownload.h \
    src/ui/ServerConfigWindow.h \
    src/ui/AboutDialog.h \
    src/ui/SavedLayoutsModel.h \
    src/core/ServerRequestManager.h \
    src/video/GstSinkWidget.h \
    src/ui/OptionsGeneralPage.h \
    src/ui/EventsView.h \
    src/utils/FileUtils.h \
    src/ui/liveview/LiveViewArea.h \
    src/ui/liveview/MJpegFeedItem.h \
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
    src/utils/StringUtils.h

RESOURCES += \
    res/resources.qrc \
    src/ui/qml.qrc
win32:RC_FILE = res/windows.rc
ICON = res/bluecherry.icns

win32:SOURCES += src/utils/explorerstyle.cpp
win32:HEADERS += src/utils/explorerstyle.h

macx:OBJECTIVE_SOURCES += src/utils/PlatformOSX.mm
macx:LIBS += -framework CoreServices

OTHER_FILES += mac/Info.plist "linux/Bluecherry Client.desktop" \
    src/ui/liveview/LiveView.qml \
    src/ui/liveview/LiveFeed.qml \
    src/utils/PlatformOSX.mm \
    src/ui/liveview/HeaderPTZControl.qml






