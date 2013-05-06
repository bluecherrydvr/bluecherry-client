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

include_directories (src)

set (bluecherry_client_main_SRCS
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
    src/core/LiveStream.cpp
    src/core/LiveStreamWorker.cpp
    src/core/LiveViewManager.cpp
    src/core/MJpegStream.cpp
    src/core/PtzPresetsModel.cpp
    src/core/ServerRequestManager.cpp
    src/core/TransferRateCalculator.cpp
    src/core/UpdateChecker.cpp
    src/core/Version.cpp

    src/event/CameraEventFilter.cpp
    src/event/EventDownloadManager.cpp
    src/event/EventFilter.cpp
    src/event/EventList.cpp
    src/event/EventParser.cpp
    src/event/EventsCursor.cpp
    src/event/EventsLoader.cpp
    src/event/EventVideoDownload.cpp
    src/event/MediaEventFilter.cpp
    src/event/ModelEventsCursor.cpp

    src/network/MediaDownloadManager.cpp
    src/network/RemotePortChecker.cpp
    src/network/SocketError.cpp

    src/server/DVRServer.cpp
    src/server/DVRServerConfiguration.cpp
    src/server/DVRServerRepository.cpp
    src/server/DVRServerSettingsReader.cpp
    src/server/DVRServerSettingsWriter.cpp

    src/ui/liveview/LiveFeedItem.cpp
    src/ui/liveview/LiveStreamItem.cpp
    src/ui/liveview/LiveViewArea.cpp
    src/ui/liveview/LiveViewGradients.cpp
    src/ui/liveview/LiveViewLayout.cpp
    src/ui/liveview/LiveViewWindow.cpp
    src/ui/liveview/PtzPresetsWindow.cpp

    src/ui/model/DVRServersModel.cpp
    src/ui/model/DVRServersProxyModel.cpp
    src/ui/model/EventsModel.cpp
    src/ui/model/EventSourcesModel.cpp
    src/ui/model/EventTagsModel.cpp
    src/ui/model/SavedLayoutsModel.cpp

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
    src/ui/MainWindow.cpp
    src/ui/NumericOffsetWidget.cpp
    src/ui/OptionsDialog.cpp
    src/ui/OptionsGeneralPage.cpp
    src/ui/OptionsServerPage.cpp
    src/ui/RemotePortCheckerWidget.cpp
    src/ui/ServerConfigWindow.cpp
    src/ui/SetupWizard.cpp
    src/ui/StatusBandwidthWidget.cpp
    src/ui/StatusBarServerAlert.cpp
    src/ui/SwitchEventsWidget.cpp
    src/ui/TimeRangeScrollBar.cpp
    src/ui/VisibleTimeRange.cpp
    src/ui/WebRtpPortCheckerWidget.cpp

    src/utils/DateTimeRange.cpp
    src/utils/DateTimeUtils.cpp
    src/utils/FileUtils.cpp
    src/utils/ImageDecodeTask.cpp
    src/utils/Range.cpp
    src/utils/RangeMap.cpp
    src/utils/StringUtils.cpp
    src/utils/ThreadTask.cpp
    src/utils/ThreadTaskCourier.cpp

    src/video/GstSinkWidget.cpp
    src/video/MediaDownload.cpp
    src/video/VideoHttpBuffer.cpp
    src/video/VideoPlayerBackend_gst.cpp
)

if (WIN32)
    list (APPEND bluecherry_client_main_SRCS
        src/utils/explorerstyle.cpp
    )
endif (WIN32)

list (APPEND bluecherry_client_SRCS
    ${bluecherry_client_main_SRCS}
)
