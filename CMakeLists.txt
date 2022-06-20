cmake_minimum_required(VERSION 3.21.1)

project(BluecherryClient
    VERSION 2.2.9
#    DESCRIPTION
#    HOMEPAGE_URL
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(PkgConfig
    REQUIRED
)
pkg_check_modules(FFmpeg
    REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
    libswscale
)
pkg_check_modules(MPV
    REQUIRED IMPORTED_TARGET
    mpv
)

find_package(Qt5
    REQUIRED COMPONENTS
    Network
    Widgets
)

find_package(SDL2
    REQUIRED
)

add_executable(bluecherry-client)
target_sources(bluecherry-client
    PRIVATE
    src/audio/AudioPlayer.cpp

    src/camera/DVRCamera.cpp
    src/camera/DVRCameraData.cpp
    src/camera/DVRCameraSettingsReader.cpp
    src/camera/DVRCameraSettingsWriter.cpp
    src/camera/DVRCameraStreamReader.cpp
    src/camera/DVRCameraStreamWriter.cpp
    src/camera/DVRCameraXMLReader.cpp

    src/core/BluecherryApp.cpp
    src/core/CameraPtzControl.cpp
    src/core/EventData.cpp
    src/core/LanguageController.cpp
    src/core/LiveStream.cpp
    src/core/LiveViewManager.cpp
    src/core/LoggableUrl.cpp
    src/core/MJpegStream.cpp
    src/core/PtzPresetsModel.cpp
    src/core/ServerRequestManager.cpp
    src/core/ThreadPause.cpp
    src/core/TransferRateCalculator.cpp
    src/core/UpdateChecker.cpp
    src/core/Version.cpp
    src/core/VaapiHWAccel.cpp

    src/event/CameraEventFilter.cpp
    src/event/EventDownloadManager.cpp
    src/event/EventFilter.cpp
    src/event/EventList.cpp
    src/event/EventParser.cpp
    src/event/EventsCursor.cpp
    src/event/EventsLoader.cpp
    src/event/EventsUpdater.cpp
    src/event/EventVideoDownload.cpp
    src/event/MediaEventFilter.cpp
    src/event/ModelEventsCursor.cpp
    src/event/ThumbnailManager.cpp

    src/main.cpp

    src/network/MediaDownloadManager.cpp
    src/network/RemotePortChecker.cpp
    src/network/SocketError.cpp

    src/rtsp-stream/RtspStream.cpp
    src/rtsp-stream/RtspStreamFrame.cpp
    src/rtsp-stream/RtspStreamFrameFormatter.cpp
    src/rtsp-stream/RtspStreamFrameQueue.cpp
    src/rtsp-stream/RtspStreamThread.cpp
    src/rtsp-stream/RtspStreamWorker.cpp

    src/server/DVRServer.cpp
    src/server/DVRServerConfiguration.cpp
    src/server/DVRServerRepository.cpp
    src/server/DVRServerSettingsReader.cpp
    src/server/DVRServerSettingsWriter.cpp

    src/ui/AboutDialog.cpp
    src/ui/CrashReportDialog.cpp
    src/ui/DVRServersView.cpp
    src/ui/EventCommentsWidget.cpp
    src/ui/EventsView.cpp
    src/ui/EventsWindow.cpp
    src/ui/EventTagsDelegate.cpp
    src/ui/EventTagsView.cpp
    src/ui/EventTimelineDatePainter.cpp
    src/ui/EventTimelineWidget.cpp
    src/ui/EventTypesFilter.cpp
    src/ui/EventVideoDownloadsWindow.cpp
    src/ui/EventVideoDownloadWidget.cpp
    src/ui/EventVideoPlayer.cpp
    src/ui/EventViewWindow.cpp
    src/ui/ExpandingTextEdit.cpp
    src/ui/MacSplitter.cpp
    src/ui/MacSplitterHandle.cpp
    src/ui/MainWindow.cpp
    src/ui/NumericOffsetWidget.cpp
    src/ui/OptionsDialog.cpp
    src/ui/OptionsGeneralPage.cpp
    src/ui/OptionsServerPage.cpp
    src/ui/RemotePortCheckerWidget.cpp
    src/ui/ServerMenu.cpp
    src/ui/SetupWizard.cpp
    src/ui/StatusBandwidthWidget.cpp
    src/ui/StatusBarServerAlert.cpp
    src/ui/SwitchEventsWidget.cpp
    src/ui/TimeRangeScrollBar.cpp
    src/ui/VisibleTimeRange.cpp
    src/ui/WebRtpPortCheckerWidget.cpp
    src/ui/liveview/LiveViewWindow.cpp
    src/ui/liveview/cameracontainerwidget.cpp
    src/ui/liveview/PtzPresetsWindow.cpp
    src/ui/model/DVRServersModel.cpp
    src/ui/model/DVRServersProxyModel.cpp
    src/ui/model/EventsModel.cpp
    src/ui/model/EventsProxyModel.cpp
    src/ui/model/EventSourcesModel.cpp
    src/ui/model/EventTagsModel.cpp
    src/ui/model/SavedLayoutsModel.cpp

    src/utils/DateTimeRange.cpp
    src/utils/DateTimeUtils.cpp
    src/utils/FileUtils.cpp
    src/utils/ImageDecodeTask.cpp
    src/utils/Range.cpp
    src/utils/RangeMap.cpp
    src/utils/StringUtils.cpp
    src/utils/ThreadTask.cpp
    src/utils/ThreadTaskCourier.cpp

    src/video/MediaDownload.cpp
    src/video/VideoHttpBuffer.cpp
    src/video/VideoPlayerBackend.cpp
    src/video/VideoWidget.cpp
    src/video/libmpv/MpvVideoWidget.cpp
    src/video/libmpv/MpvVideoPlayerBackend.cpp
    src/video/libmpv/MpvVideoPlayerFactory.cpp


    src/audio/AudioPlayer.h

    src/camera/DVRCamera.h
    src/camera/DVRCameraData.h
    src/camera/DVRCameraSettingsReader.h
    src/camera/DVRCameraSettingsWriter.h
    src/camera/DVRCameraStreamReader.h
    src/camera/DVRCameraStreamWriter.h
    src/camera/DVRCameraXMLReader.h
    src/camera/RecordingState.h

    src/core/BluecherryApp.h
    src/core/CameraPtzControl.h
    src/core/EventData.h
    src/core/LanguageController.h
    src/core/LiveStream.h
    src/core/LiveViewManager.h
    src/core/LoggableUrl.h
    src/core/MJpegStream.h
    src/core/PtzPresetsModel.h
    src/core/ServerRequestManager.h
    src/core/ThreadPause.h
    src/core/TransferRateCalculator.h
    src/core/UpdateChecker.h
    src/core/Version.h
    src/core/VaapiHWAccel.h

    src/event/CameraEventFilter.h
    src/event/EventDownloadManager.h
    src/event/EventFilter.h
    src/event/EventList.h
    src/event/EventParser.h
    src/event/EventsCursor.h
    src/event/EventsLoader.h
    src/event/EventsUpdater.h
    src/event/EventVideoDownload.h
    src/event/MediaEventFilter.h
    src/event/ModelEventsCursor.h
    src/event/ThumbnailManager.h

    src/network/MediaDownloadManager.h
    src/network/RemotePortChecker.h
    src/network/SocketError.h

    src/rtsp-stream/RtspStream.h
    src/rtsp-stream/RtspStreamFrame.h
    src/rtsp-stream/RtspStreamFrameFormatter.h
    src/rtsp-stream/RtspStreamFrameQueue.h
    src/rtsp-stream/RtspStreamThread.h
    src/rtsp-stream/RtspStreamWorker.h

    src/server/DVRServer.h
    src/server/DVRServerConfiguration.h
    src/server/DVRServerConnectionType.h
    src/server/DVRServerRepository.h
    src/server/DVRServerSettingsReader.h
    src/server/DVRServerSettingsWriter.h

    src/ui/AboutDialog.h
    src/ui/CrashReportDialog.h
    src/ui/DVRServersView.h
    src/ui/EventCommentsWidget.h
    src/ui/EventsView.h
    src/ui/EventsWindow.h
    src/ui/EventTagsDelegate.h
    src/ui/EventTagsView.h
    src/ui/EventTimelineDatePainter.h
    src/ui/EventTimelineWidget.h
    src/ui/EventTypesFilter.h
    src/ui/EventVideoDownloadsWindow.h
    src/ui/EventVideoDownloadWidget.h
    src/ui/EventVideoPlayer.h
    src/ui/EventViewWindow.h
    src/ui/ExpandingTextEdit.h
    src/ui/MacSplitter.h
    src/ui/MacSplitterHandle.h
    src/ui/MainWindow.h
    src/ui/NumericOffsetWidget.h
    src/ui/OptionsDialog.h
    src/ui/OptionsGeneralPage.h
    src/ui/OptionsServerPage.h
    src/ui/RemotePortCheckerWidget.h
    src/ui/ServerMenu.h
    src/ui/SetupWizard.h
    src/ui/SetupWizard_p.h
    src/ui/StatusBandwidthWidget.h
    src/ui/StatusBarServerAlert.h
    src/ui/SwitchEventsWidget.h
    src/ui/TimeRangeScrollBar.h
    src/ui/VisibleTimeRange.h
    src/ui/WebRtpPortCheckerWidget.h
    src/ui/liveview/LiveViewWindow.h
    src/ui/liveview/cameracontainerwidget.h
    src/ui/liveview/PtzPresetsWindow.h
    src/ui/model/DVRServersModel.h
    src/ui/model/DVRServersProxyModel.h
    src/ui/model/EventsModel.h
    src/ui/model/EventsProxyModel.h
    src/ui/model/EventSourcesModel.h
    src/ui/model/EventTagsModel.h
    src/ui/model/SavedLayoutsModel.h

    src/utils/DateTimeRange.h
    src/utils/DateTimeUtils.h
    src/utils/FileUtils.h
    src/utils/ImageDecodeTask.h
    src/utils/Range.h
    src/utils/RangeMap.h
    src/utils/StringUtils.h
    src/utils/ThreadTask.h
    src/utils/ThreadTaskCourier.h

    src/video/MediaDownload.h
    src/video/MediaDownload_p.h
    src/video/VideoHttpBuffer.h
    src/video/VideoPlayerBackend.h
    src/video/VideoPlayerFactory.h
    src/video/VideoWidget.h
    src/video/libmpv/MpvVideoWidget.h
    src/video/libmpv/MpvVideoPlayerBackend.h
    src/video/libmpv/MpvVideoPlayerFactory.h
)
target_compile_definitions(bluecherry-client
    PRIVATE
    PACKAGE_VERSION="${BluecherryClient_VERSION}"
    QT_DISABLE_DEPRECATED_BEFORE=0x000000
)
target_include_directories(bluecherry-client
    PRIVATE
    src/
)
target_link_libraries(bluecherry-client
    PkgConfig::FFmpeg
    PkgConfig::MPV
    Qt::Network
    Qt::Widgets
    SDL2::SDL2
)
