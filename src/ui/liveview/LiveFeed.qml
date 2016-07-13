/*
 * Copyright 2010-2013 Bluecherry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

import Bluecherry 1.0
import Qt 4.7

LiveFeedBase {
    id: feedItem
    width: 200
    height: 150
    z: LiveViewLayout.isDragItem ? 10 : (LiveViewLayout.isDropTarget ? 1 : 0)
    serverRepository: mainServerRepository

    LiveViewLayout.sizePadding: Qt.size(0, header.height)
    LiveViewLayout.fixedAspectRatio: false

    streamItem: videoArea

    states: State {
        name: "ready"
        when: stream !== null

        PropertyChanges {
            target: feedItem
            /* Qt.size is necessary to convert from QSize to QSizeF */
            LiveViewLayout.sizeHint: Qt.size(stream.streamSize.width, stream.streamSize.height)
        }

        PropertyChanges {
            target: statusText
            text: statusOverlayMessage(stream.state)
        }

        PropertyChanges {
            target: headerItems
            visible: stream.connected
        }
    }

    Image {
        id: header
        anchors.top: parent.top
        anchors.left: videoArea.left
        anchors.right: videoArea.right
        height: Math.max(20, headerText.paintedHeight)
        clip: true

        source: "image://liveviewgradients/header" + (feedItem.activeFocus ? "/focused" : "")
        sourceSize: Qt.size(1, height)
        fillMode: Image.TileHorizontally

        MouseArea {
            anchors.fill: parent
            drag.target: feedItem
            drag.axis: Drag.XandYAxis

            onPositionChanged: {
                if (drag.active) {
                    if (!feedItem.LiveViewLayout.isDragItem)
                        feedItem.parent.startDrag(feedItem);
                    else
                        feedItem.parent.updateDrag();
                }
            }

            onPressed: feedItem.focus = true
            onReleased: if (drag.active) feedItem.parent.drop()
        }

        Text {
            id: headerText
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 1
            color: "white"
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            height: parent.height
            text: feedItem.cameraName
        }

        Row {
            id: headerItems
            x: {
                var l = headerText.x + headerText.width + 10;
                return l + Math.max(0, (header.width - l) - width - 5);
            }
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 1
            spacing: 10
            visible: false

            /* Disabled due to no support from RTP streams yet */
/*
            Text {
                id: feedRecording
                color: "#8e8e8e"
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                text: "Not Recording"
                visible: !feed.paused

                states: State {
                    name: "recording"
                    when: feedItem.recordingState == LiveFeedBase.MotionActive

                    PropertyChanges {
                        target: feedRecording
                        text: "Recording"
                        color: "#ff6262"
                    }
                }
            }
*/

            HeaderPTZControl {
                id: headerPtzElement
                height: parent.height
                visible: feedItem.hasPtz && parent.visible && !stream.paused
            }

            Image {
                id: audioStreamIcon
                source: "qrc:/icons/audio-stream-available.png" /*stream !== null && stream.audioPlaying ? "qrc:/icons/audio-stream-on.png" : "qrc:/icons/audio-stream-available.png"*/
                height: parent.height
                fillMode: Image.PreserveAspectFit
                visible: parent.visible && stream.audio /*stream !== null && stream.audio*/

                states: [
                    /*State {
                        name: "hasAudio"
                        when: stream && stream.audio

                        PropertyChanges {
                            target: audioStreamIcon
                            source: "qrc:/icons/audio-stream-available.png"
                        }
                    },*/
                    State {
                        name: "AudioIsPlaying"
                        when: stream && stream.audioPlaying

                        PropertyChanges {
                            target: audioStreamIcon
                            source: "qrc:/icons/audio-stream-on.png"
                        }
                    }
                ]
            }

            Text {
                id: fpsText
                color: "#bebebe"
                height: parent.height
                verticalAlignment: Text.AlignVCenter

                Timer {
                    id: fpsTimer
                    interval: 500
                    repeat: true
                    triggeredOnStart: true
                    running: false

                    onTriggered: parent.text = Math.round(stream.receivedFps) + "<span style='color:#8e8e8e'>fps</span>"
                }

                states: [
                    State {
                        name: "paused"
                        when: stream && stream.paused

                        PropertyChanges {
                            target: fpsText
                            color: "#ffdf6e"
                            text: "Paused"
                        }
                    },
                    State {
                        name: "active"
                        when: stream && !stream.paused

                        PropertyChanges {
                            target: fpsTimer
                            running: fpsText.visible
                        }
                    }
                ]

                MouseArea {
                    id: fpsMouseArea
                    anchors.fill: fpsText
                    anchors.leftMargin: -4
                    anchors.rightMargin: -5
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onPressed: feedItem.showFpsMenu(fpsMouseArea);
                }

                Rectangle {
                    anchors.fill: fpsText
                    anchors.leftMargin: -4
                    anchors.rightMargin: -5
                    z: -1

                    color: "#77000000"
                    visible: fpsMouseArea.containsMouse
                }
            }


        }
    }

    LiveStreamDisplay {
        id: videoArea

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    onPtzChanged: {
        if (ptz == null)
            feedItem.customCursor = LiveFeedBase.DefaultCursor
    }

    MouseArea {
        anchors.fill: videoArea

        hoverEnabled: feedItem.ptz != null

        onPressed: {
            feedItem.focus = true
        }

        function moveForPosition(x, y) {
            var xarea = width / 4, yarea = height / 4
            var movement = CameraPtzControl.NoMovement

            if (x < xarea)
                movement |= CameraPtzControl.MoveWest
            else if (x >= 3*xarea)
                movement |= CameraPtzControl.MoveEast
            if (y < yarea)
                movement |= CameraPtzControl.MoveNorth
            else if (y >= 3*yarea)
                movement |= CameraPtzControl.MoveSouth

            return movement;
        }

        onClicked: {
            if (feedItem.ptz == null)
                return;

            var movement = moveForPosition(mouse.x, mouse.y)
            if (movement != CameraPtzControl.NoMovement)
                feedItem.ptz.move(movement)
            else
                mouse.accepted = false
        }

        onDoubleClicked: {
            if (feedItem.ptz == null)
                return;

            if (moveForPosition(mouse.x, mouse.y) != CameraPtzControl.NoMovement)
                mouse.accepted = false
            else
                feedItem.ptz.move(CameraPtzControl.MoveTele)
        }

        onPositionChanged: {
            if (feedItem.ptz == null)
                return;

            var movements = moveForPosition(mouse.x, mouse.y)
            var cursor = LiveFeedBase.DefaultCursor

            if (movements & CameraPtzControl.MoveNorth) {
                if (movements & CameraPtzControl.MoveWest)
                    cursor = LiveFeedBase.MoveCursorNW
                else if (movements & CameraPtzControl.MoveEast)
                    cursor = LiveFeedBase.MoveCursorNE
                else
                    cursor = LiveFeedBase.MoveCursorN
            }
            else if (movements & CameraPtzControl.MoveSouth) {
                if (movements & CameraPtzControl.MoveWest)
                    cursor = LiveFeedBase.MoveCursorSW
                else if (movements & CameraPtzControl.MoveEast)
                    cursor = LiveFeedBase.MoveCursorSE
                else
                    cursor = LiveFeedBase.MoveCursorS
            }
            else if (movements & CameraPtzControl.MoveWest)
                cursor = LiveFeedBase.MoveCursorW
            else if (movements & CameraPtzControl.MoveEast)
                cursor = LiveFeedBase.MoveCursorE

            feedItem.customCursor = cursor
        }

        onExited: feedItem.customCursor = LiveFeedBase.DefaultCursor
    }

    Keys.onPressed: {
        if (ptz == null || event.isAutoRepeat)
            return

        switch (event.key) {
        case Qt.Key_Left:
            ptz.move(CameraPtzControl.MoveWest)
            break
        case Qt.Key_Right:
            ptz.move(CameraPtzControl.MoveEast)
            break
        case Qt.Key_Up:
            ptz.move((event.modifiers & Qt.ShiftModifier) ? CameraPtzControl.MoveTele : CameraPtzControl.MoveNorth)
            break
        case Qt.Key_Down:
            ptz.move((event.modifiers & Qt.ShiftModifier) ? CameraPtzControl.MoveWide : CameraPtzControl.MoveSouth)
            break
        default:
            return
        }

        event.accepted = true
    }

    Rectangle {
        id: focusRect
        anchors.fill: parent
        anchors.bottomMargin: 1
        anchors.rightMargin: 1
        color: "transparent"
        border.color: "#2d9ae6"
        border.width: 1
        radius: 4
        smooth: false
        z: 3

        visible: feedItem.activeFocus && (feedItem.parent.rows > 1 || feedItem.parent.columns > 1)
    }

    function statusOverlayMessage(state) {
        switch (state) {
            case LiveStream.Error: return "<span style='color:#ff0000'>Error<br><font size=10px>"
                                   + stream.errdesc +"</font></span>";
            case LiveStream.StreamOffline: return "<span style='color:#888888'>Offline</span>";
            case LiveStream.NotConnected: return "Disconnected";
            case LiveStream.Connecting: return "Connecting...";
            default: return "";
        }
    }

    Rectangle {
        id: statusOverlay

        color: "#BE000000"
        anchors.fill: videoArea
        opacity: 0
        clip: true

        states: State {
            name: "active"
            when: statusText.text

            PropertyChanges { target: statusOverlay; visible: true; opacity: 1; }
        }

        transitions: Transition {
            to: "active"
            reversible: true

            PropertyAnimation {
                target: statusOverlay
                property: "opacity"
                duration: 500
            }
        }

        Text {
            id: statusText
            anchors.centerIn: parent
            font.pointSize: 14
            horizontalAlignment: Qt.AlignHCenter

            color: "white"
        }
    }
}
