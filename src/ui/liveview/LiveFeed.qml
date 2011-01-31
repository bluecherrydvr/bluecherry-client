import Bluecherry 1.0
import Qt 4.7

Item {
    width: 200
    height: 150

    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.max(20, headerText.paintedHeight)

        gradient: Gradient {
            GradientStop { position: 0; color: "#424242"; }
            GradientStop { position: 0.4; color: "#292929"; }
            GradientStop { position: 0.49; color: "#1c1c1c"; }
            GradientStop { position: 1; color: "#0f0f0f"; }
        }

        Text {
            id: headerText
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.bottomMargin: 1
            color: "#ffffff"
            verticalAlignment: Text.AlignVCenter
            text: "Test Feed"
        }
    }

    MJpegFeed {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
