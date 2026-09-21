import QtQuick
import QtQuick.Controls.Basic

ComboBox {
    id: control

    // theme inputs, so the parent can restyle it
    property color backgroundColor: "#99A1A8"
    property color borderColor: "#99A1A8"
    property color accentColor: "#4D5B67"
    property color textColor: "#EBECEE"

    implicitWidth: 160
    implicitHeight: 36
    font.pixelSize: 14

    // closed-box text
    contentItem: Text {
        leftPadding: 12
        rightPadding: control.indicator.width + 20
        text: control.displayText
        font: control.font
        color: control.textColor
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    // arrow: drawn chevron that flips when the popup is open
    indicator: Canvas {
        id: arrow
        x: control.width - width - 12
        y: (control.height - height) / 2
        width: 12
        height: 8
        rotation: control.popup.visible ? 180 : 0
        Behavior on rotation { NumberAnimation { duration: 150 } }

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.lineWidth = 2
            ctx.lineCap = "round"
            ctx.lineJoin = "round"
            ctx.strokeStyle = control.textColor
            ctx.beginPath()
            ctx.moveTo(1, 1)
            ctx.lineTo(width / 2, height - 1)
            ctx.lineTo(width - 1, 1)
            ctx.stroke()
        }

        Connections {
            target: control
            function onTextColorChanged() { arrow.requestPaint() }
        }
    }

    // frame
    background: Rectangle {
        radius: 6
        color: control.hovered ? Qt.darker(control.backgroundColor, 1.5) : control.backgroundColor
        border.width: (control.visualFocus || control.popup.visible) ? 2 : 1
        border.color: (control.visualFocus || control.popup.visible)
                      ? control.accentColor : control.borderColor
    }

    // each row in the dropdown
    delegate: ItemDelegate {
        id: item
        width: control.width - 8
        height: 32
        text: control.textAt(index)          // works with or without textRole
        highlighted: control.highlightedIndex === index

        contentItem: Text {
            text: item.text
            font: control.font
            color: item.highlighted ? control.accentColor : control.textColor
            verticalAlignment: Text.AlignVCenter
            leftPadding: 8
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: 4
            color: item.highlighted ? Qt.alpha(control.accentColor, 0.15) : "transparent"
        }
    }

    // the dropdown container
    popup: Popup {
        y: control.height + 4
        width: control.width
        padding: 4
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 240)

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
        background: Rectangle {
            radius: 6
            color: control.backgroundColor
            border.color: control.accentColor
        }
    }
}
