import Bluecherry 1.0
import Qt 4.7

Rectangle {
    id: viewArea
    color: "black"

    LiveViewLayout {
        id: viewLayout
        objectName: "viewLayout"
        anchors.fill: parent

        item: Qt.createComponent("LiveFeed.qml")

        rows: 4
        columns: 4

        DropTargetRectangle {
            dropTarget: viewLayout.dropTarget
            dragItem: viewLayout.dragItem
        }
    }
}
