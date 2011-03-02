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

        Rectangle {
            id: dropTargetRect
            z: 9

            color: "transparent"
            radius: 2
            border.color: "#ffa200"
            border.width: 2

            visible: false

            /*
            Behavior on x { SmoothedAnimation { velocity: 1600 } }
            Behavior on y { SmoothedAnimation { velocity: 1600 } }
            Behavior on width { SmoothedAnimation { velocity: 1600 } }
            Behavior on height { SmoothedAnimation { velocity: 1600 } }
            */

            states: [
                State {
                    name: "activeOverItem"
                    when: viewLayout.dropTarget != null

                    PropertyChanges {
                        target: dropTargetRect
                        visible: true
                        x: viewLayout.dropTarget.x
                        y: viewLayout.dropTarget.y
                        width: viewLayout.dropTarget.width
                        height: viewLayout.dropTarget.height
                    }
                }/*,
                State {
                    name: "activeOverEmpty"
                    when: viewLayout.dragItem != null && viewLayout.dropTarget == null

                    PropertyChanges {
                        target: dropTargetRect
                        visible: true
                    }

                    StateChangeScript {
                        script: viewLayout.set(3, 3, dropTargetRect);
                    }
                }*/
            ]

            transitions: [
                Transition {
                    from: "activeOverEmpty"

                    ScriptAction {
                        script: viewLayout.takeItem(dropTargetRect)
                    }
                }
            ]
        }
    }
}
