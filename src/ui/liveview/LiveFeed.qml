import Bluecherry 1.0
import Qt 4.7

LiveFeedBase {
    id: feedItem
    width: 200
    height: 150
    z: LiveViewLayout.isDragItem ? 10 : (LiveViewLayout.isDropTarget ? 1 : 0)

    LiveViewLayout.sizeHint: stream.frameSize
    LiveViewLayout.sizePadding: Qt.size(0, header.height)
    LiveViewLayout.fixedAspectRatio: false

    streamItem: stream

    Image {
        id: header
        anchors.top: parent.top
        anchors.left: stream.left
        anchors.right: stream.right
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
            visible: stream.connected

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
                visible: feedItem.hasPtz && !stream.paused
            }

            Text {
                id: fpsText
                color: "#bebebe"
                height: parent.height
                verticalAlignment: Text.AlignVCenter

                Timer {
                    running: fpsText.visible && !stream.paused
                    interval: 500
                    repeat: true
                    triggeredOnStart: true

                    onTriggered: parent.text = stream.fps + "<span style='color:#8e8e8e'>fps</span>"
                }

                states: State {
                    name: "paused"
                    when: stream.paused

                    PropertyChanges {
                        target: fpsText
                        color: "#ffdf6e"
                        text: "Paused"
                    }
                }

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

    LiveStream {
        id: stream

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        onErrorTextChanged: feedItem.setStatusText(errorText)
    }

    onPtzChanged: {
        if (ptz == null)
            feedItem.customCursor = LiveFeedBase.DefaultCursor
    }

    MouseArea {
        anchors.fill: stream

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

    Rectangle {
        id: statusOverlay

        color: "#BE000000"
        anchors.fill: stream
        opacity: 0
        clip: true

        states: State {
            name: "active"
            when: feedItem.statusText

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
            anchors.centerIn: parent
            font.pointSize: 14
            horizontalAlignment: Qt.AlignHCenter

            color: "white"

            text: feedItem.statusText
        }
    }
}
