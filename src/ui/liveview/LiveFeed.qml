import Bluecherry 1.0
import Qt 4.7

LiveFeedBase {
    id: feedItem
    width: 200
    height: 150
    z: activeFocus ? 1 : 0

    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.max(20, headerText.paintedHeight)

        /* This gradient should be implemented in some other way to improve performance */
        gradient: Gradient {
            GradientStop { position: 0; color: feedItem.activeFocus ? "#626262" : "#424242"; }
            GradientStop { position: 0.4; color: feedItem.activeFocus ? "#494949" : "#292929"; }
            GradientStop { position: 0.49; color: feedItem.activeFocus ? "#3c3c3c" : "#1c1c1c"; }
            GradientStop { position: 1; color: feedItem.activeFocus ? "#2f2f2f" : "#0f0f0f"; }
        }

        Text {
            id: headerText
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.bottomMargin: 1
            color: feedItem.LiveViewLayout.isDragItem ? "#ff0000" : "#ffffff"
            verticalAlignment: Text.AlignVCenter
            text: feedItem.cameraName
        }

        MouseArea {
            anchors.fill: parent
            drag.target: feedItem
            drag.axis: Drag.XandYAxis
            drag.minimumX: 0
            drag.minimumY: 0
            drag.maximumX: drag.target ? (viewArea.x + viewArea.width - drag.target.width) : 0
            drag.maximumY: drag.target ? (viewArea.y + viewArea.height - drag.target.height) : 0

            onPositionChanged: {
                if (drag.active) {
                    if (!feedItem.LiveViewLayout.isDragItem)
                        feedItem.parent.startDrag(feedItem);
                    else
                        feedItem.parent.updateDrag();
                }
            }

            onReleased: {
                feedItem.parent.moveItem(feedItem, Qt.point(feedItem.x, feedItem.y));
                feedItem.parent.endDrag();
            }
        }
    }

    MJpegFeed {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        objectName: "mjpegFeed"
    }
}
