bin_PROGRAMS = bluecherry-client

EXTRA_DIST = \
COPYING README.txt translations/README translations/setup-transifex.sh linux/bluecherry-client.desktop \
res/application-resize-full.png \
res/arrow-circle-double.png \
res/audio-stream-available.png \
res/audio-stream-on.png \
res/bluecherry-client.png \
res/bluecherry.icns \
res/bluecherry.ico \
res/brightness.png \
res/cassette.png \
res/collapse-all.png \
res/color.png \
res/contrast.png \
res/control-000-small.png \
res/control-180-small.png \
res/control-double-000-small.png \
res/control-double-180.png \
res/control-double-180-small.png \
res/control-double.png \
res/control-pause.png \
res/control.png \
res/control-stop-180.png \
res/control-stop.png \
res/control-stop-square.png \
res/cross.png \
res/down-arrow.png \
res/exclamation-red.png \
res/exclamation-yellow.png \
res/expand-all.png \
res/icon16.png \
res/icon32.png \
res/icon64.png \
res/layout-12.png \
res/layout-16.png \
res/layout-32.png \
res/layout-4.png \
res/layout-9.png \
res/layout-join.png \
res/layout-join-vertical.png \
res/layout.png \
res/layout-split.png \
res/layout-split-vertical.png \
res/license.txt \
res/loading-circle.gif \
res/loading.gif \
res/logo.png \
res/pencil.png \
res/plus.png \
res/ptz-arrow.png \
res/status-alert.png \
res/status-error.png \
res/status-offline.png \
res/status.png \
res/system-monitor.png \
res/tick.png \
res/webcam.png

CLEANFILES = moc_*.cpp translations/*.qm resources.cpp


pkgbluecherryclientdir = ${datarootdir}/bluecherry-client/translations
dist_pkgbluecherryclient_DATA =  \
translations/bluecherryclient_de_DE.qm \
translations/bluecherryclient_en.qm \
translations/bluecherryclient_es.qm \
translations/bluecherryclient_no.qm \
translations/bluecherryclient_pl.qm \
translations/bluecherryclient_ru.qm \
translations/bluecherryclient_sv_SE.qm

bluecherry_client_CPPFLAGS =  -I$(srcdir)/src
bluecherry_client_CXXFLAGS = $(QT_DEPENDENCIES_CFLAGS) $(FFMPEG_CFLAGS) $(MPV_CFLAGS)
bluecherry_client_LDADD = $(QT_DEPENDENCIES_LIBS) $(FFMPEG_LIBS) $(MPV_LIBS) $(SDL2_LIBS)

bluecherry_client_SOURCES = \
src/audio/AudioPlayer.cpp \
src/camera/DVRCamera.cpp \
src/camera/DVRCameraData.cpp \
src/camera/DVRCameraSettingsReader.cpp \
src/camera/DVRCameraSettingsWriter.cpp \
src/camera/DVRCameraStreamReader.cpp \
src/camera/DVRCameraStreamWriter.cpp \
src/camera/DVRCameraXMLReader.cpp \
 \
src/core/BluecherryApp.cpp \
src/core/CameraPtzControl.cpp \
src/core/EventData.cpp \
src/core/LanguageController.cpp \
src/core/LiveStream.cpp \
src/core/LiveViewManager.cpp \
src/core/LoggableUrl.cpp \
src/core/MJpegStream.cpp \
src/core/PtzPresetsModel.cpp \
src/core/ServerRequestManager.cpp \
src/core/ThreadPause.cpp \
src/core/TransferRateCalculator.cpp \
src/core/UpdateChecker.cpp \
src/core/Version.cpp \
src/core/VaapiHWAccel.cpp \
 \
src/event/CameraEventFilter.cpp \
src/event/EventDownloadManager.cpp \
src/event/EventFilter.cpp \
src/event/EventList.cpp \
src/event/EventParser.cpp \
src/event/EventsCursor.cpp \
src/event/EventsLoader.cpp \
src/event/EventsUpdater.cpp \
src/event/EventVideoDownload.cpp \
src/event/MediaEventFilter.cpp \
src/event/ModelEventsCursor.cpp \
src/event/ThumbnailManager.cpp \
 \
src/rtsp-stream/RtspStream.cpp \
src/rtsp-stream/RtspStreamFrame.cpp \
src/rtsp-stream/RtspStreamFrameFormatter.cpp \
src/rtsp-stream/RtspStreamFrameQueue.cpp \
src/rtsp-stream/RtspStreamThread.cpp \
src/rtsp-stream/RtspStreamWorker.cpp \
 \
src/network/MediaDownloadManager.cpp \
src/network/RemotePortChecker.cpp \
src/network/SocketError.cpp \
 \
src/server/DVRServer.cpp \
src/server/DVRServerConfiguration.cpp \
src/server/DVRServerRepository.cpp \
src/server/DVRServerSettingsReader.cpp \
src/server/DVRServerSettingsWriter.cpp \
 \
src/ui/liveview/LiveViewWindow.cpp \
src/ui/liveview/cameracontainerwidget.cpp \
src/ui/liveview/PtzPresetsWindow.cpp \
\
src/ui/model/DVRServersModel.cpp \
src/ui/model/DVRServersProxyModel.cpp \
src/ui/model/EventsModel.cpp \
src/ui/model/EventsProxyModel.cpp \
src/ui/model/EventSourcesModel.cpp \
src/ui/model/EventTagsModel.cpp \
src/ui/model/SavedLayoutsModel.cpp \
 \
src/ui/AboutDialog.cpp \
src/ui/CrashReportDialog.cpp \
src/ui/DVRServersView.cpp \
src/ui/EventCommentsWidget.cpp \
src/ui/EventsView.cpp \
src/ui/EventsWindow.cpp \
src/ui/EventTagsDelegate.cpp \
src/ui/EventTagsView.cpp \
src/ui/EventTimelineDatePainter.cpp \
src/ui/EventTimelineWidget.cpp \
src/ui/EventTypesFilter.cpp \
src/ui/EventVideoDownloadsWindow.cpp \
src/ui/EventVideoDownloadWidget.cpp \
src/ui/EventVideoPlayer.cpp \
src/ui/EventViewWindow.cpp \
src/ui/ExpandingTextEdit.cpp \
src/ui/MacSplitter.cpp \
src/ui/MacSplitterHandle.cpp \
src/ui/MainWindow.cpp \
src/ui/NumericOffsetWidget.cpp \
src/ui/OptionsDialog.cpp \
src/ui/OptionsGeneralPage.cpp \
src/ui/OptionsServerPage.cpp \
src/ui/RemotePortCheckerWidget.cpp \
src/ui/ServerMenu.cpp \
src/ui/SetupWizard.cpp \
src/ui/StatusBarServerAlert.cpp \
src/ui/SwitchEventsWidget.cpp \
src/ui/TimeRangeScrollBar.cpp \
src/ui/VisibleTimeRange.cpp \
src/ui/WebRtpPortCheckerWidget.cpp \
\
src/ui/StatusBandwidthWidget.cpp \
\
src/utils/DateTimeRange.cpp \
src/utils/DateTimeUtils.cpp \
src/utils/FileUtils.cpp \
src/utils/ImageDecodeTask.cpp \
src/utils/Range.cpp \
src/utils/RangeMap.cpp \
src/utils/StringUtils.cpp \
src/utils/ThreadTask.cpp \
src/utils/ThreadTaskCourier.cpp \
 \
src/video/MediaDownload.cpp \
src/video/VideoHttpBuffer.cpp \
src/video/VideoPlayerBackend.cpp \
src/video/VideoWidget.cpp \
 \
src/video/libmpv/MpvVideoWidget.cpp \
src/video/libmpv/MpvVideoPlayerBackend.cpp \
src/video/libmpv/MpvVideoPlayerFactory.cpp \
src/main.cpp \
\
src/camera/RecordingState.h \
src/server/DVRServerConnectionType.h \
src/video/VideoPlayerFactory.h \
src/ui/SetupWizard_p.h \
src/video/MediaDownload_p.h \
\
src/audio/AudioPlayer.h \
src/camera/DVRCamera.h \
src/camera/DVRCameraData.h \
src/camera/DVRCameraSettingsReader.h \
src/camera/DVRCameraSettingsWriter.h \
src/camera/DVRCameraStreamReader.h \
src/camera/DVRCameraStreamWriter.h \
src/camera/DVRCameraXMLReader.h \
 \
src/core/BluecherryApp.h \
src/core/CameraPtzControl.h \
src/core/EventData.h \
src/core/LanguageController.h \
src/core/LiveStream.h \
src/core/LiveViewManager.h \
src/core/LoggableUrl.h \
src/core/MJpegStream.h \
src/core/PtzPresetsModel.h \
src/core/ServerRequestManager.h \
src/core/ThreadPause.h \
src/core/TransferRateCalculator.h \
src/core/UpdateChecker.h \
src/core/Version.h \
src/core/VaapiHWAccel.h \
 \
src/event/CameraEventFilter.h \
src/event/EventDownloadManager.h \
src/event/EventFilter.h \
src/event/EventList.h \
src/event/EventParser.h \
src/event/EventsCursor.h \
src/event/EventsLoader.h \
src/event/EventsUpdater.h \
src/event/EventVideoDownload.h \
src/event/MediaEventFilter.h \
src/event/ModelEventsCursor.h \
src/event/ThumbnailManager.h \
 \
src/rtsp-stream/RtspStream.h \
src/rtsp-stream/RtspStreamFrame.h \
src/rtsp-stream/RtspStreamFrameFormatter.h \
src/rtsp-stream/RtspStreamFrameQueue.h \
src/rtsp-stream/RtspStreamThread.h \
src/rtsp-stream/RtspStreamWorker.h \
 \
src/network/MediaDownloadManager.h \
src/network/RemotePortChecker.h \
src/network/SocketError.h \
 \
src/server/DVRServer.h \
src/server/DVRServerConfiguration.h \
src/server/DVRServerRepository.h \
src/server/DVRServerSettingsReader.h \
src/server/DVRServerSettingsWriter.h \
 \
src/ui/liveview/LiveViewWindow.h \
src/ui/liveview/cameracontainerwidget.h \
src/ui/liveview/PtzPresetsWindow.h \
\
src/ui/model/DVRServersModel.h \
src/ui/model/DVRServersProxyModel.h \
src/ui/model/EventsModel.h \
src/ui/model/EventsProxyModel.h \
src/ui/model/EventSourcesModel.h \
src/ui/model/EventTagsModel.h \
src/ui/model/SavedLayoutsModel.h \
 \
src/ui/AboutDialog.h \
src/ui/CrashReportDialog.h \
src/ui/DVRServersView.h \
src/ui/EventCommentsWidget.h \
src/ui/EventsView.h \
src/ui/EventsWindow.h \
src/ui/EventTagsDelegate.h \
src/ui/EventTagsView.h \
src/ui/EventTimelineDatePainter.h \
src/ui/EventTimelineWidget.h \
src/ui/EventTypesFilter.h \
src/ui/EventVideoDownloadsWindow.h \
src/ui/EventVideoDownloadWidget.h \
src/ui/EventVideoPlayer.h \
src/ui/EventViewWindow.h \
src/ui/ExpandingTextEdit.h \
src/ui/MacSplitter.h \
src/ui/MacSplitterHandle.h \
src/ui/MainWindow.h \
src/ui/NumericOffsetWidget.h \
src/ui/OptionsDialog.h \
src/ui/OptionsGeneralPage.h \
src/ui/OptionsServerPage.h \
src/ui/RemotePortCheckerWidget.h \
src/ui/ServerMenu.h \
src/ui/SetupWizard.h \
src/ui/StatusBarServerAlert.h \
src/ui/SwitchEventsWidget.h \
src/ui/TimeRangeScrollBar.h \
src/ui/VisibleTimeRange.h \
src/ui/WebRtpPortCheckerWidget.h \
\
src/ui/StatusBandwidthWidget.h \
\
src/utils/DateTimeRange.h \
src/utils/DateTimeUtils.h \
src/utils/FileUtils.h \
src/utils/ImageDecodeTask.h \
src/utils/Range.h \
src/utils/RangeMap.h \
src/utils/StringUtils.h \
src/utils/ThreadTask.h \
src/utils/ThreadTaskCourier.h \
 \
src/video/MediaDownload.h \
src/video/VideoHttpBuffer.h \
src/video/VideoPlayerBackend.h \
src/video/VideoWidget.h \
 \
src/video/libmpv/MpvVideoWidget.h \
src/video/libmpv/MpvVideoPlayerBackend.h \
src/video/libmpv/MpvVideoPlayerFactory.h \
\
translations/bluecherryclient_de_DE.ts \
translations/bluecherryclient_en.ts \
translations/bluecherryclient_es.ts \
translations/bluecherryclient_no.ts \
translations/bluecherryclient_pl.ts \
translations/bluecherryclient_ru.ts \
translations/bluecherryclient_sv_SE.ts \
res/windows.rc \
res/resources.qrc

nodist_bluecherry_client_SOURCES  = \
moc_SetupWizard_p.cpp \
moc_CrashReportDialog.cpp \
moc_EventsView.cpp \
moc_StatusBarServerAlert.cpp \
moc_MacSplitter.cpp \
moc_OptionsGeneralPage.cpp \
moc_AboutDialog.cpp \
moc_EventTimelineWidget.cpp \
moc_cameracontainerwidget.cpp \
moc_LiveViewWindow.cpp \
moc_PtzPresetsWindow.cpp \
moc_VisibleTimeRange.cpp \
moc_EventVideoDownloadWidget.cpp \
moc_OptionsDialog.cpp \
moc_EventTagsDelegate.cpp \
moc_MainWindow.cpp \
moc_MacSplitterHandle.cpp \
moc_EventCommentsWidget.cpp \
moc_StatusBandwidthWidget.cpp \
moc_OptionsServerPage.cpp \
moc_TimeRangeScrollBar.cpp \
moc_EventsWindow.cpp \
moc_ExpandingTextEdit.cpp \
moc_DVRServersView.cpp \
moc_EventTypesFilter.cpp \
moc_DVRServersProxyModel.cpp \
moc_EventSourcesModel.cpp \
moc_EventsProxyModel.cpp \
moc_EventTagsModel.cpp \
moc_EventsModel.cpp \
moc_DVRServersModel.cpp \
moc_SavedLayoutsModel.cpp \
moc_EventViewWindow.cpp \
moc_SwitchEventsWidget.cpp \
moc_EventVideoDownloadsWindow.cpp \
moc_SetupWizard.cpp \
moc_ServerMenu.cpp \
moc_RemotePortCheckerWidget.cpp \
moc_NumericOffsetWidget.cpp \
moc_WebRtpPortCheckerWidget.cpp \
moc_EventVideoPlayer.cpp \
moc_EventTagsView.cpp \
moc_DVRServer.cpp \
moc_DVRServerConfiguration.cpp \
moc_DVRServerRepository.cpp \
moc_DVRCameraData.cpp \
moc_DVRCamera.cpp \
moc_AudioPlayer.cpp \
moc_RemotePortChecker.cpp \
moc_MediaDownloadManager.cpp \
moc_RtspStreamThread.cpp \
moc_RtspStreamWorker.cpp \
moc_RtspStream.cpp \
moc_EventsCursor.cpp \
moc_ModelEventsCursor.cpp \
moc_EventVideoDownload.cpp \
moc_EventsUpdater.cpp \
moc_ThumbnailManager.cpp \
moc_EventsLoader.cpp \
moc_EventDownloadManager.cpp \
moc_ThreadTaskCourier.cpp \
moc_VideoWidget.cpp \
moc_VideoHttpBuffer.cpp \
moc_MediaDownload.cpp \
moc_MpvVideoWidget.cpp \
moc_MpvVideoPlayerBackend.cpp \
moc_VideoPlayerBackend.cpp \
moc_MediaDownload_p.cpp \
moc_TransferRateCalculator.cpp \
moc_LiveViewManager.cpp \
moc_PtzPresetsModel.cpp \
moc_BluecherryApp.cpp \
moc_UpdateChecker.cpp \
moc_ServerRequestManager.cpp \
moc_CameraPtzControl.cpp \
moc_MJpegStream.cpp \
moc_LiveStream.cpp \
resources.cpp

resources.cpp: res/resources.qrc
	$(RCC) -name bluecherry $< > $@

moc_%.cpp: src/ui/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/ui/liveview/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/ui/model/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/server/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/camera/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/audio/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/network/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/rtsp-stream/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/event/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/utils/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/video/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/video/libmpv/%.h
	$(MOC) $< -o $@

moc_%.cpp: src/core/%.h
	$(MOC) $< -o $@

translations/%.qm: translations/%.ts
	$(LRELEASE) $< -qm $@

