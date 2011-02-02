import Bluecherry 1.0
import Qt 4.7

Rectangle {
    id: viewArea
    color: "black"

    LiveViewLayout {
        id: viewLayout
        objectName: "viewLayout"
        anchors.fill: parent
    }
}
