import Bluecherry 1.0
import Qt 4.7

Rectangle {
    Grid {
        spacing: 25
        rows: 4
        anchors.fill: parent

        Repeater {
            model: 16

            MJpegStream {
                width: 200
                height: 150

                RotationAnimation on rotation {
                    from: 0
                    to: 360
                    direction: RotationAnimation.Clockwise
                    duration: 1000 * ((index % 4)+1)
                    loops: Animation.Infinite
                }
            }
        }
    }
}
