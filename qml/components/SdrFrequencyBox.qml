import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root
    spacing: 5

    // Shared by every digit label (an alias can only point at one item)
    property font font
    property color color: "black"
    property color secondaryColor: "black"

    property real value
    signal edited(real value)

    function digitAt(place) {
        // TODO: check "place" validity
        return Math.floor(value / Math.pow(10, place)) % 10;
    }

    function increaseValueByPlace(place) {
        const digit = root.digitAt(place);

        let newValue = root.value;

        if (digit === 9) {
            newValue -= 9 * Math.pow(10, place);
        } else {
            newValue += Math.pow(10, place)
        }

        root.edited(newValue);
    }

    function decreaseValueByPlace(place) {
        const digit = root.digitAt(place);

        let newValue = root.value;

        if (digit === 0) {
            newValue += 9 * Math.pow(10, place);
        } else {
            newValue -= Math.pow(10, place)
        }

        root.edited(newValue);
    }

    function formatFrequency(hz, decimals = 3, trimZeros = true) {
        if (!isFinite(hz))
            return "—";

        const units = [
            { scale: 1,   suffix: "Hz"  },
            { scale: 1e3, suffix: "kHz" },
            { scale: 1e6, suffix: "MHz" },
            { scale: 1e9, suffix: "GHz" }
        ];

        // pick the largest unit where the value is >= 1
        const abs = Math.abs(hz);
        let i = 0;
        while (i < units.length - 1 && abs >= units[i + 1].scale)
            i++;

        // round, and bump to the next unit if rounding produced 1000 (e.g. 999.9996 MHz -> 1 GHz)
        let scaled = Number((hz / units[i].scale).toFixed(decimals));
        if (Math.abs(scaled) >= 1000 && i < units.length - 1) {
            i++;
            scaled = Number((hz / units[i].scale).toFixed(decimals));
        }

        const text = trimZeros ? String(scaled) : scaled.toFixed(decimals);
        return text + " " + units[i].suffix;
    }

    RowLayout {
        spacing: 0

        Item {
            id: oneBillion
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: oneBillion_Label
                anchors.centerIn: parent
                text: root.digitAt(9)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: oneBillion_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: oneBillion_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: oneBillion_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: oneBillion_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            const digit = root.digitAt(9);

                            let newValue = root.value;

                            // Limit only to 3 billion Hz
                            if (digit === 3) {
                                newValue -= 3 * Math.pow(10, 9);
                            } else {
                                newValue += Math.pow(10, 9)
                            }

                            root.edited(newValue);
                        }
                    }
                }

                Rectangle {
                    id: oneBillion_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: oneBillion_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: oneBillion_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            const digit = root.digitAt(9);

                            let newValue = root.value;

                            // Limit only to 3 billion Hz
                            if (digit === 0) {
                                newValue += 3 * Math.pow(10, 9);
                            } else {
                                newValue -= Math.pow(10, 9)
                            }

                            root.edited(newValue);
                        }
                    }
                }
            }
        }

        Text {
            id: billion_separator
            text: ","
            font: root.font
            color: root.color
            Layout.alignment: Qt.AlignVCenter
        }

        // ---------- Hundred millions (10^8) ----------
        Item {
            id: hundredMillion
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: hundredMillion_Label
                anchors.centerIn: parent
                text: root.digitAt(8)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: hundredMillion_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: hundredMillion_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: hundredMillion_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: hundredMillion_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(8)
                    }
                }

                Rectangle {
                    id: hundredMillion_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: hundredMillion_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: hundredMillion_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(8)
                    }
                }
            }
        }

        // ---------- Ten millions (10^7) ----------
        Item {
            id: tenMillion
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: tenMillion_Label
                anchors.centerIn: parent
                text: root.digitAt(7)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: tenMillion_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: tenMillion_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: tenMillion_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: tenMillion_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(7)
                    }
                }

                Rectangle {
                    id: tenMillion_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: tenMillion_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: tenMillion_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(7)
                    }
                }
            }
        }

        // ---------- Millions (10^6) ----------
        Item {
            id: million
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: million_Label
                anchors.centerIn: parent
                text: root.digitAt(6)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: million_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: million_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: million_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: million_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(6)
                    }
                }

                Rectangle {
                    id: million_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: million_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: million_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(6)
                    }
                }
            }
        }

        // ---------- Separator: millions | thousands ----------
        Text {
            id: million_separator
            text: ","
            font: root.font
            color: root.color
            Layout.alignment: Qt.AlignVCenter
        }

        // ---------- Hundred thousands (10^5) ----------
        Item {
            id: hundredThousand
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: hundredThousand_Label
                anchors.centerIn: parent
                text: root.digitAt(5)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: hundredThousand_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: hundredThousand_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: hundredThousand_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: hundredThousand_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(5)
                    }
                }

                Rectangle {
                    id: hundredThousand_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: hundredThousand_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: hundredThousand_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(5)
                    }
                }
            }
        }

        // ---------- Ten thousands (10^4) ----------
        Item {
            id: tenThousand
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: tenThousand_Label
                anchors.centerIn: parent
                text: root.digitAt(4)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: tenThousand_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: tenThousand_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: tenThousand_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: tenThousand_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(4)
                    }
                }

                Rectangle {
                    id: tenThousand_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: tenThousand_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: tenThousand_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(4)
                    }
                }
            }
        }

        // ---------- Thousands (10^3) ----------
        Item {
            id: thousand
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: thousand_Label
                anchors.centerIn: parent
                text: root.digitAt(3)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: thousand_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: thousand_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: thousand_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: thousand_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(3)
                    }
                }

                Rectangle {
                    id: thousand_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: thousand_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: thousand_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(3)
                    }
                }
            }
        }

        // ---------- Separator: thousands | hundreds ----------
        Text {
            id: thousand_separator
            text: ","
            font: root.font
            color: root.color
            Layout.alignment: Qt.AlignVCenter
        }

        // ---------- Hundreds (10^2) ----------
        Item {
            id: hundred
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: hundred_Label
                anchors.centerIn: parent
                text: root.digitAt(2)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: hundred_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: hundred_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: hundred_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: hundred_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(2)
                    }
                }

                Rectangle {
                    id: hundred_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: hundred_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: hundred_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(2)
                    }
                }
            }
        }

        // ---------- Tens (10^1) ----------
        Item {
            id: ten
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: ten_Label
                anchors.centerIn: parent
                text: root.digitAt(1)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: ten_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: ten_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: ten_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: ten_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(1)
                    }
                }

                Rectangle {
                    id: ten_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: ten_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: ten_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(1)
                    }
                }
            }
        }

        // ---------- Ones (10^0) ----------
        Item {
            id: one
            implicitWidth: 20
            implicitHeight: 40

            Text {
                id: one_Label
                anchors.centerIn: parent
                text: root.digitAt(0)
                font: root.font
                color: root.color
            }

            ColumnLayout {
                id: one_knotsArea
                anchors.fill: parent
                z: 1 //  stacking on top of label

                Rectangle {
                    id: one_up_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: one_up_mouseArea.containsMouse ? "#80011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: one_up_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.increaseValueByPlace(0)
                    }
                }

                Rectangle {
                    id: one_down_btn
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.45
                    color: one_down_mouseArea.containsMouse ? "#4D011526" : "transparent"
                    border.color: "transparent"

                    MouseArea {
                        id: one_down_mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: root.decreaseValueByPlace(0)
                    }
                }
            }
        }

        Text {
            text: "Hz"
            font: root.font
            color: root.color
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            Layout.preferredWidth: 20
            Layout.fillHeight: true
        }

        Item {
            Layout.preferredWidth: 120
            Layout.fillHeight: true

            Text {
                anchors.centerIn: parent
                text: "(%1)".arg(formatFrequency(root.value, 3, true))
                font.family: root.font.family
                font.weight: root.font.weight
                font.pixelSize: root.font.pixelSize * 0.8     // 80% of the digits' size
                color: root.secondaryColor
            }
        }
    }
}
