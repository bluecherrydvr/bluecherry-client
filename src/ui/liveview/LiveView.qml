import Bluecherry 1.0
import Qt 4.7

Rectangle {
    id: viewArea
    color: "black"

    Grid {
        spacing: 25
        rows: 4
        anchors.fill: parent

        Repeater {
            model: 16

            MJpegFeed {
                width: 200
                height: 150
            }
        }
    }
}
