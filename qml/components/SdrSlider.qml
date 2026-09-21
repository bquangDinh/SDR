// qml/StyledSlider.qml
import QtQuick
import QtQuick.Controls.Basic

Slider {
    id: control

    // theme inputs, so the parent can restyle it
    property color accentColor: "#4D5B67"
    property color trackColor: "#99A1A8"
    property color handleColor: "#4D5B67"
    property int trackHeight: 6
    property int handleSize: 18

    implicitWidth: 200
    implicitHeight: handleSize + 8

    // groove plus the filled part to the left of the handle
    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: control.trackHeight
        radius: height / 2
        color: control.trackColor

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: control.enabled ? control.accentColor : Qt.darker(control.trackColor, 1.2)
        }
    }

    // the knob
    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.handleSize
        height: control.handleSize
        radius: width / 2
        color: control.pressed ? Qt.darker(control.handleColor, 1.15) : control.handleColor
        border.width: 2
        border.color: control.accentColor

        scale: control.pressed ? 1.15 : (control.hovered ? 1.08 : 1.0)
        Behavior on scale { NumberAnimation { duration: 100 } }
    }
}