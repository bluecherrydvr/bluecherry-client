/*
 * Copyright 2010-2019 Bluecherry, LLC
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
import QtQuick 2.14

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
