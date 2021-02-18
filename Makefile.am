bin_PROGRAMS = bluecherry-client

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
src/ui/liveview/LiveFeedItem.cpp \
src/ui/liveview/LiveStreamItem.cpp \
src/ui/liveview/LiveViewArea.cpp \
src/ui/liveview/LiveViewGradients.cpp \
src/ui/liveview/LiveViewLayout.cpp \
src/ui/liveview/LiveViewWindow.cpp \
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
src/main.cpp

nodist_bluecherry_client_SOURCES  = \
moc_SetupWizard_p.cpp \
moc_CrashReportDialog.cpp \
moc_EventsView.cpp \
moc_StatusBarServerAlert.cpp \
moc_MacSplitter.cpp \
moc_OptionsGeneralPage.cpp \
moc_AboutDialog.cpp \
moc_EventTimelineWidget.cpp \
moc_LiveViewArea.cpp \
moc_LiveStreamItem.cpp \
moc_LiveFeedItem.cpp \
moc_LiveViewLayout.cpp \
moc_PtzPresetsWindow.cpp \
moc_LiveViewWindow.cpp \
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
qml_resources.cpp \
resources.cpp

resources.cpp: res/resources.qrc
	$(RCC) -g cpp -name bluecherry $< > $@

qml_resources.cpp: src/ui/qml.qrc
	$(RCC) -g cpp -name bluecherry_qrc $< > $@

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


