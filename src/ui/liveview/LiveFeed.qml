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

    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: feed.left
        anchors.right: feed.right
        height: Math.max(20, headerText.paintedHeight)

        /* This gradient should be implemented in some other way to improve performance */
        gradient: Gradient {
            GradientStop { position: 0; color: feedItem.activeFocus ? "#626262" : "#4c4c4c"; }
            GradientStop { position: 0.4; color: feedItem.activeFocus ? "#494949" : "#333333"; }
            GradientStop { position: 0.49; color: feedItem.activeFocus ? "#3c3c3c" : "#262626"; }
            GradientStop { position: 1; color: feedItem.activeFocus ? "#2f2f2f" : "#191919"; }
        }

        Text {
            id: headerText
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.bottomMargin: 1
            color: "white"
            verticalAlignment: Text.AlignVCenter
            text: feedItem.cameraName + (feed.paused ? " (paused)" : "")
        }

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

        onClicked: {
            if (feedItem.ptz == null)
                return;

            var xarea = width / 4, yarea = height / 4
            var movement = CameraPtzControl.NoMovement

            if (mouse.x < xarea)
                movement |= CameraPtzControl.MoveWest
            else if (mouse.x >= 3*xarea)
                movement |= CameraPtzControl.MoveEast
            if (mouse.y < yarea)
                movement |= CameraPtzControl.MoveNorth
            else if (mouse.y >= 3*yarea)
                movement |= CameraPtzControl.MoveSouth

            feedItem.ptz.move(movement)
        }

        onDoubleClicked: mouse.accepted = false

        onPositionChanged: {
            if (feedItem.ptz == null)
                return;

            var xarea = width / 4, yarea = height / 4;
            if (mouse.x < xarea) {
                if (mouse.y <= yarea)
                    feedItem.customCursor = LiveFeedBase.MoveCursorNW
                else if (mouse.y >= 3*yarea)
                    feedItem.customCursor = LiveFeedBase.MoveCursorSW
                else
                    feedItem.customCursor = LiveFeedBase.MoveCursorW
            } else if (mouse.x >= 3*xarea) {
                if (mouse.y <= yarea)
                    feedItem.customCursor = LiveFeedBase.MoveCursorNE
                else if (mouse.y >= 3*yarea)
                    feedItem.customCursor = LiveFeedBase.MoveCursorSE
                else
                    feedItem.customCursor = LiveFeedBase.MoveCursorE
            }
            else if (mouse.y <= yarea)
                feedItem.customCursor = LiveFeedBase.MoveCursorN
            else if (mouse.y >= 3*yarea)
                feedItem.customCursor = LiveFeedBase.MoveCursorS
            else
                feedItem.customCursor = LiveFeedBase.DefaultCursor
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
        id: statusOverlay

        color: "#BE000000"
        anchors.fill: feed
        opacity: 0

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
