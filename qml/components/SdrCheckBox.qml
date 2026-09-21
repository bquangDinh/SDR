// qml/StyledCheckBox.qml
import QtQuick
import QtQuick.Controls.Basic

CheckBox {
    id: control

    property color backgroundColor: "#4D5B67"
    property color accentColor: "#89b4fa"
    property color borderColor: "#45475a"
    property color textColor: "#010C15"
    property int boxSize: 20

    spacing: 8
    font.pixelSize: 14

    // the box
    indicator: Rectangle {
        implicitWidth: control.boxSize
        implicitHeight: control.boxSize
        x: control.leftPadding
        y: (control.height - height) / 2
        radius: 4
        color: control.hovered ? Qt.darker(control.backgroundColor, 1.5) : control.backgroundColor

        Behavior on color { ColorAnimation { duration: 100 } }

        // checkmark, drawn as a rotated L shape
        Rectangle {
            visible: control.checkState === Qt.Checked
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -1
            width: parent.width * 0.28
            height: parent.height * 0.5
            color: "transparent"
            rotation: 45
            border.width: 0

            Rectangle { width: 2; height: parent.height; anchors.right: parent.right; color: "white"; radius: 1 }
            Rectangle { width: parent.width + 2; height: 2; anchors.bottom: parent.bottom; anchors.right: parent.right; color: "white"; radius: 1 }
        }

        // dash for the partially checked state (tristate only)
        Rectangle {
            visible: control.checkState === Qt.PartiallyChecked
            anchors.centerIn: parent
            width: parent.width * 0.5
            height: 2
            radius: 1
            color: control.accentColor
        }

        // focus ring for keyboard navigation
        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 6
            height: parent.height + 6
            radius: 6
            color: "transparent"
            border.width: 2
            border.color: Qt.alpha(control.accentColor, 0.5)
            visible: control.visualFocus
        }
    }

    // the label, on the right of the box
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.enabled ? control.textColor : Qt.darker(control.textColor, 1.8)
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing   // keeps text clear of the box
        elide: Text.ElideRight
    }
}