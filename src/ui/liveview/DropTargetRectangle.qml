import Bluecherry 1.0
import Qt 4.7

Item {
    id: dropTargetItem
    z: 9

    property variant dragItem
    property variant dropTarget

    Rectangle {
        id: dropTargetRect

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

        states: State {
            name: "active"
            when: dropTargetItem.dropTarget != null

            PropertyChanges {
                target: dropTargetRect
                visible: true
                x: parent.dropTarget.x
                y: parent.dropTarget.y
                width: parent.dropTarget.width
                height: parent.dropTarget.height
            }
        }

/*
        transitions: [
            Transition {
                to: "active"
                reversible: false

                SmoothedAnimation {
                    target: dropTargetRect
                    property: "x"
                    from: dropTargetItem.dragItem.x
                    to: dropTargetItem.dropTarget.x
                    velocity: 1600
                }

                SmoothedAnimation {
                    target: dropTargetRect
                    property: "y"
                    from: dropTargetItem.dragItem.y
                    to: dropTargetItem.dropTarget.y
                    velocity: 1600
                }

                SmoothedAnimation {
                    target: dropTargetRect

                    property: "width"
                    from: dropTargetItem.dragItem.width
                    to: dropTargetItem.dropTarget.width
                    velocity: 1600
                }

                SmoothedAnimation {
                    target: dropTargetRect
                    property: "height"
                    from: dropTargetItem.dragItem.height
                    to: dropTargetItem.dropTarget.height
                    velocity: 1600
                }

            }
        ]
*/
    }
}
