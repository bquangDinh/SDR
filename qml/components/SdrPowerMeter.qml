import QtQuick

// Vertical level meter with dB scale marks.
// Fills bottom-to-top from `from` to `to`; major ticks + labels every `majorStep`.
Item {
    id: root

    // Range / value
    property real from: -100
    property real to: 0
    property real value: from

    // Scale
    property real majorStep: 10        // labelled tick every 10 dB
    property int minorPerMajor: 1      // unlabelled ticks between majors (0 = none)
    property string suffix: ""         // e.g. " dB" to show units on every label

    // Geometry
    property int barWidth: 10
    property int majorTickLength: 6
    property int minorTickLength: 3
    property int labelGap: 4

    // Colors
    property color trackColor: "#99A1A8"
    property color fillColor: "#F25A38"
    property color tickColor: "#4D5B67"
    property color labelColor: Qt.black

    property alias font: metrics.font

    readonly property real span: to - from
    readonly property real level: span === 0 ? 0 : Math.max(0, Math.min(1, (value - from) / span))

    // Half a label height of padding top and bottom, so the end labels aren't cut off
    readonly property real pad: Math.ceil(metrics.height / 2)

    implicitWidth: barWidth + 1 + majorTickLength + labelGap + Math.ceil(metrics.width) + 2
    implicitHeight: 200

    // Value -> y position on the scale
    function yFor(v) {
        return pad + (height - 2 * pad) * (1 - (v - from) / span)
    }

    // Measures the widest label so the component reports a correct width
    TextMetrics {
        id: metrics
        text: Math.min(root.from, root.to).toFixed(0) + root.suffix
        font.pixelSize: 11
    }

    // Track + fill
    Rectangle {
        id: track
        x: 0
        y: root.pad
        width: root.barWidth
        height: root.height - 2 * root.pad
        radius: 2
        color: root.trackColor

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: parent.height * root.level
            radius: parent.radius
            color: root.fillColor

            Behavior on height { NumberAnimation { duration: 80 } }
        }
    }

    // Minor ticks
    Repeater {
        model: root.minorPerMajor > 0
               ? Math.floor(root.span / (root.majorStep / (root.minorPerMajor + 1)) + 1e-9) + 1
               : 0

        Rectangle {
            readonly property real step: root.majorStep / (root.minorPerMajor + 1)

            visible: index % (root.minorPerMajor + 1) !== 0   // skip positions where a major tick sits
            x: root.barWidth + 1
            y: Math.round(root.yFor(root.to - index * step))
            width: root.minorTickLength
            height: 1
            color: root.tickColor
        }
    }

    // Major ticks + labels
    Repeater {
        model: Math.floor(root.span / root.majorStep + 1e-9) + 1

        Item {
            readonly property real v: root.to - index * root.majorStep

            y: Math.round(root.yFor(v))

            Rectangle {
                x: root.barWidth + 1
                width: root.majorTickLength
                height: 1
                color: root.tickColor
            }

            Text {
                x: root.barWidth + 1 + root.majorTickLength + root.labelGap
                y: -height / 2
                text: v.toFixed(0) + root.suffix
                font: root.font
                color: root.labelColor
            }
        }
    }
}
