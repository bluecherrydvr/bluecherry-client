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

        Rectangle {
            id: headerPtzElement
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.right
            clip: true
            width: 30 + ptzText.paintedWidth
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
            }

            transitions: [
                Transition {
                    to: "enabled"

                    AnchorAnimation {
                        duration: 400
                        easing.type: Easing.OutQuad
                    }
                },
                Transition {
                    to: ""

                    SequentialAnimation {
                        AnchorAnimation {
                            duration: 400
                            easing.type: Easing.InQuad
                        }
                        PropertyAction {
                            target: headerPtzElement
                            property: "visible"
                        }
                    }
                }
            ]

            gradient: Gradient {
                GradientStop { position: 0; color: "#2d2d2d"; }
                GradientStop { position: 1; color: "#030303"; }
            }

            Behavior on width {
                SmoothedAnimation {
                    velocity: 250
                }
            }

            Text {
                id: ptzText

                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                verticalAlignment: Qt.AlignVCenter
                color: "#75c0ff"
                text: (feedItem.ptz == null || feedItem.ptz.currentPreset < 0) ? "PTZ Enabled" : ("PTZ: " + feedItem.ptz.currentPresetName)
            }

            Image {
                anchors.left: ptzText.right
                anchors.leftMargin: 6
                anchors.verticalCenter: parent.verticalCenter

                source: ":/icons/down-arrow.png"
            }

            MouseArea {
                acceptedButtons: MouseArea.Left | MouseArea.Right
                anchors.fill: parent

                onPressed: feedItem.showPtzMenu(headerPtzElement)
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
