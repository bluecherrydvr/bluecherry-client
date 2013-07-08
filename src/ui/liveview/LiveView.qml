/*
 * Copyright 2010-2013 Bluecherry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

import Bluecherry 1.0
import Qt 4.7

Rectangle {
    id: viewArea
    color: "black"

    LiveViewLayout {
        id: viewLayout
        objectName: "viewLayout"
        anchors.fill: parent
        serverRepository: mainServerRepository

        item: Qt.createComponent("LiveFeed.qml")

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
