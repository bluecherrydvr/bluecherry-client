import Bluecherry 1.0
import Qt 4.7

Rectangle {
    id: viewArea
    color: "black"

    LiveViewLayout {
        id: viewLayout
        objectName: "viewLayout"
        anchors.fill: parent

        rows: 4
        columns: 4
    }

    MouseArea {
        anchors.fill: viewLayout
        drag.axis: Drag.XandYAxis
        drag.minimumX: 0
        drag.minimumY: 0
        drag.maximumX: drag.target ? (viewArea.x + viewArea.width - drag.target.width) : 0
        drag.maximumY: drag.target ? (viewArea.y + viewArea.height - drag.target.height) : 0

        onPressed: {
            drag.target = viewLayout.childAt(mouseX, mouseY)
            if (drag.target != null)
                drag.target.z = 100
        }

        onReleased: {
            if (drag.target != null)
                drag.target.z = 0
        }
    }
}
