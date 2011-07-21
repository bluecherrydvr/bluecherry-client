import Bluecherry 1.0
import Qt 4.7

Image {
    id: headerPtzElement
    width: 30 + ptzText.paintedWidth

    source: "image://liveviewgradients/ptzHeader"
    sourceSize: Qt.size(1, height)
    fillMode: Image.TileHorizontally

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
