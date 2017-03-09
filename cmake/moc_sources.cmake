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

set (bluecherry_client_MOC_SRCS
    src/audio/AudioPlayer.h
    src/camera/DVRCamera.h
    src/camera/DVRCameraData.h

    src/core/BluecherryApp.h
    src/core/CameraPtzControl.h
    src/core/LiveStream.h
    src/core/LiveViewManager.h
    src/core/MJpegStream.h
    src/core/PtzPresetsModel.h
    src/core/ServerRequestManager.h
    src/core/TransferRateCalculator.h
    src/core/UpdateChecker.h

    src/event/EventDownloadManager.h
    src/event/EventsCursor.h
    src/event/EventsLoader.h
    src/event/EventsUpdater.h
    src/event/EventVideoDownload.h
    src/event/ModelEventsCursor.h
    src/event/ThumbnailManager.h

    src/rtsp-stream/RtspStream.h
    src/rtsp-stream/RtspStreamThread.h
    src/rtsp-stream/RtspStreamWorker.h

    src/network/MediaDownloadManager.h
    src/network/RemotePortChecker.h

    src/server/DVRServer.h
    src/server/DVRServerConfiguration.h
    src/server/DVRServerRepository.h

    src/ui/liveview/LiveFeedItem.h
    src/ui/liveview/LiveStreamItem.h
    src/ui/liveview/LiveViewArea.h
    src/ui/liveview/LiveViewLayout.h
    src/ui/liveview/LiveViewWindow.h
    src/ui/liveview/PtzPresetsWindow.h

    src/ui/model/DVRServersModel.h
    src/ui/model/DVRServersProxyModel.h
    src/ui/model/EventsModel.h
    src/ui/model/EventsProxyModel.h
    src/ui/model/EventSourcesModel.h
    src/ui/model/EventTagsModel.h
    src/ui/model/SavedLayoutsModel.h

    src/ui/AboutDialog.h
    src/ui/CrashReportDialog.h
    src/ui/DVRServersView.h
    src/ui/EventCommentsWidget.h
    src/ui/EventsView.h
    src/ui/EventsWindow.h
    src/ui/EventTagsDelegate.h
    src/ui/EventTagsView.h
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
    src/ui/ServerConfigWindow.h
    src/ui/ServerMenu.h
    src/ui/SetupWizard.h
    src/ui/SetupWizard_p.h
    src/ui/StatusBandwidthWidget.h
    src/ui/StatusBarServerAlert.h
    src/ui/SwitchEventsWidget.h
    src/ui/TimeRangeScrollBar.h
    src/ui/VisibleTimeRange.h
    src/ui/WebRtpPortCheckerWidget.h

    src/utils/ThreadTaskCourier.h

    src/video/MediaDownload.h
    src/video/MediaDownload_p.h
    src/video/VideoHttpBuffer.h
    src/video/VideoPlayerBackend.h
    src/video/VideoWidget.h

    src/video/libmpv/MpvVideoPlayerBackend.h
    src/video/libmpv/MpvVideoWidget.h
#    src/video/mplayer/MplayerProcess.h

)

if (WIN32)
    list (APPEND bluecherry_client_MOC_FILES
        src/utils/explorerstyle.h
    )
endif (WIN32)

qt4_wrap_cpp (bluecherry_client_MOC_FILES ${bluecherry_client_MOC_SRCS})

list (APPEND bluecherry_client_SRCS
    ${bluecherry_client_MOC_FILES}
)
