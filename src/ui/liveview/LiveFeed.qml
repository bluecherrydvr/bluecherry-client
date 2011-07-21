import Bluecherry 1.0
import Qt 4.7

LiveFeedBase {
    id: feedItem
    width: 200
    height: 150
    z: LiveViewLayout.isDragItem ? 10 : (LiveViewLayout.isDropTarget ? 1 : 0)

    LiveViewLayout.sizeHint: feed.frameSize
    LiveViewLayout.sizePadding: Qt.size(0, header.height)
    LiveViewLayout.fixedAspectRatio: false

    Image {
        id: header
        anchors.top: parent.top
        anchors.left: feed.left
        anchors.right: feed.right
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
            anchors.leftMargin: 4
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 1
            color: "white"
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            height: parent.height
            text: feedItem.cameraName
            //elide: Text.ElideRight
        }

        Row {
            id: headerItems
            x: headerText.x + headerText.paintedWidth + 10
            width: parent.width - x
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 1
            spacing: 10

            Text {
                id: feedRecording
                color: "#8e8e8e"
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                text: "No Recording"

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
        }

        HeaderPTZControl {
            id: headerPtzElement
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.right
            clip: true
            visible: false

            states: State {
                name: "enabled"
                when: feedItem.ptz != null

                PropertyChanges {
                    target: headerPtzElement
                    visible: true
                }

                AnchorChanges {
                    target: headerPtzElement
                    anchors.left: undefined
                    anchors.right: parent.right
                }

                PropertyChanges {
                    target: headerItems
                    width: header.width - x - headerPtzElement.width
                }

                AnchorChanges {
                    target: headerText
                    anchors.right: headerPtzElement.left
                }
            }
        }
    }

    MJpegFeed {
        id: feed

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        objectName: "mjpegFeed"

        onErrorTextChanged: feedItem.setStatusText(errorText)
    }

    onPtzChanged: {
        if (ptz == null)
            feedItem.customCursor = LiveFeedBase.DefaultCursor
    }

    MouseArea {
        anchors.fill: feed

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
        anchors.fill: feed
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
