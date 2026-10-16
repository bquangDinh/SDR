import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Shapes

Button {
    id: control

    property color iconColor: "#EBECEE"
    property color backgroundColor: "#F25A38"
    property bool playing: false

    implicitWidth: 30
    implicitHeight: 30

    background: Rectangle {
        radius: 8
        color: control.pressed ? Qt.lighter(control.backgroundColor, 1.5) : control.hovered ? Qt.darker(control.backgroundColor, 1.5) : control.backgroundColor
    }

    contentItem: Item {
        anchors.centerIn: parent

        Shape {
            anchors.centerIn: parent
            width: 14
            height: 14
            visible: !control.playing
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                fillColor: control.iconColor
                strokeWidth: -1
                startX: 0
                startY: 0
                PathLine { x: 14; y: 7 }
                PathLine { x: 0; y: 14 }
                PathLine { x: 0; y: 0 }
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: 12
            height: 12
            radius: 2
            color: control.iconColor
            visible: control.playing
        }
    }
}
