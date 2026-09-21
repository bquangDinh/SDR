// qml/StyledRadioButton.qml
import QtQuick
import QtQuick.Controls.Basic

RadioButton {
    id: control

    property color accentColor: "#EBECEE"
    property color borderColor: "#45475a"
    property color textColor: "#010C15"
    property color backgroundColor: "#4D5B67"
    property int circleSize: 20

    spacing: 8
    font.pixelSize: 14

    indicator: Rectangle {
        implicitWidth: control.circleSize
        implicitHeight: control.circleSize
        x: control.leftPadding
        y: (control.height - height) / 2
        radius: width / 2                       // full radius makes it a circle
        color: control.backgroundColor
        border.width: 2
        border.color: control.checked || control.hovered ? control.accentColor
                                                         : control.borderColor

        Rectangle {                             // inner dot, shown when selected
            anchors.centerIn: parent
            width: parent.width * 0.5
            height: width
            radius: width / 2
            color: control.accentColor
            visible: control.checked
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.enabled ? control.textColor : Qt.darker(control.textColor, 1.8)
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
        elide: Text.ElideRight
    }
}