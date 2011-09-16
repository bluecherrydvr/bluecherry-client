import Bluecherry 1.0
import Qt 4.7

Rectangle {
    id: headerPtzElement
    width: 12 + ptzText.width
    color: mouseArea.containsMouse ? "#77000000" : "transparent"

    Text {
        id: ptzText

        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        verticalAlignment: Qt.AlignVCenter
        color: (feedItem.ptz == null) ? "#8e8e8e" : "#75c0ff"
        text: {
            if (feedItem.ptz == null || feedItem.ptz.currentPreset < 0)
                return "PTZ"
            else
                return "PTZ: <b>" + feedItem.ptz.currentPresetName + "</b>"
        }
    }

    MouseArea {
        id: mouseArea
        acceptedButtons: MouseArea.Left | MouseArea.Right
        anchors.fill: parent
        hoverEnabled: true

        onPressed: feedItem.showPtzMenu(headerPtzElement)
    }
}
