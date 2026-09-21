import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import sdr_qt

ApplicationWindow {
    id: window
    width: 1920
    height: 1080
    visible: true
    title: qsTr("SDR.")
    color: "#EBECEE"

    FontLoader {
        id: figtreeFont
        source: "../fonts/Figtree-VariableFont_wght.ttf"
    }

    font.family: figtreeFont.name
    font.pixelSize: 14

    // SdrBackend {
    //     id: sdr
    //     onErrorOccurred: (msg) => statusText.text = msg
    //     onSpectrum: (bins) => console.log("QML got spectrum, bins:", bins.length)
    // }

    // Row {
    //     spacing: 8

    //     Button {
    //         text: sdr.running ? "Stop" : "Start"
    //         onClicked: sdr.running ? sdr.stop() : sdr.start()
    //     }

    //     Text {
    //         text: "chunks: " + sdr.chunkCount
    //     }

    //     Text {
    //         id: statusText;
    //         color: "white"
    //     }
    // }

    ColumnLayout {
        anchors.fill: parent

        // Top Menu Bar
        RowLayout {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 4

            Rectangle {
                Layout.preferredWidth: 260
                Layout.maximumWidth: 260
                Layout.minimumWidth: 260
                Layout.preferredHeight: 44
                Layout.leftMargin: 8
                Layout.topMargin: 8
                color: "#EBECEE"

                ColumnLayout {
                    anchors.fill: parent

                    Label {
                        text: "SDR."
                        font.pixelSize: 32
                        color: "black"
                        font.family: figtreeFont.name
                        Layout.alignment: Qt.AlignCenter
                        font.bold: true
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 8
                Layout.topMargin: 4
                Layout.preferredHeight: 44
                color: "#F25A38"
                radius: 8
            }
        }

        // Main Container
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            // Configuration Panels
            ColumnLayout {
                Layout.fillWidth: false
                Layout.preferredWidth: 270
                Layout.maximumWidth: 270
                Layout.minimumWidth: 270
                Layout.fillHeight: true

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 8
                    Layout.rightMargin: 2
                    Layout.bottomMargin: 8
                    color: "#EBECEE"


                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        // Source Configuration
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 630
                            Layout.minimumHeight: 630
                            color: "#CCD0D4"
                            radius: 8

                            ColumnLayout {
                                anchors.fill: parent

                                Label {
                                    Layout.topMargin: 8
                                    text: "Source"
                                    Layout.alignment: Qt.AlignCenter
                                    color: "#4D5B67"
                                    font.pixelSize: 16
                                }

                                // Source Selection
                                ColumnLayout {
                                    spacing: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    Label {
                                        text: "Source"
                                        color: "#010C15"
                                        font.family: figtreeFont.name
                                        font.pixelSize: 14
                                    }

                                    Select {
                                        Layout.fillWidth: true
                                        model: ["RTL-SDR", "AirSpy"]
                                    }
                                }

                                // Device Selection
                                ColumnLayout {
                                    spacing: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    Label {
                                        text: "Device"
                                        color: "#010C15"
                                        font.family: figtreeFont.name
                                        font.pixelSize: 14
                                    }

                                    Select {
                                        Layout.fillWidth: true
                                        model: ["Generic RTL2832U OEM (0)", "AirSpy"]
                                    }
                                }

                                // Sample Rate
                                ColumnLayout {
                                    spacing: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    Label {
                                        text: "Sample Rate"
                                        color: "#010C15"
                                        font.family: figtreeFont.name
                                        font.pixelSize: 14
                                    }

                                    Select {
                                        Layout.fillWidth: true
                                        model: ["3.2 MSPS", "2.8 MSPS"]
                                    }
                                }

                                // Sampling Mode
                                ColumnLayout {
                                    spacing: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    Label {
                                        text: "Sampling Mode"
                                        color: "#010C15"
                                        font.family: figtreeFont.name
                                        font.pixelSize: 14
                                    }

                                    Select {
                                        Layout.fillWidth: true
                                        model: ["Quadrature Sampling", "2.8 MSPS"]
                                    }
                                }


                                GridLayout {
                                    columns: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    SdrCheckBox {
                                        text: "RTL AGC"
                                    }

                                    SdrCheckBox {
                                        text: "Offset Timing"
                                    }

                                    SdrCheckBox {
                                        text: "Tuner AGC"
                                    }

                                    SdrCheckBox {
                                        text: "Bias-Tee"
                                    }
                                }

                                ColumnLayout {
                                    spacing: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    RowLayout {
                                        Label {
                                            text: "RF Gain"
                                            color: "#010C15"
                                            font.family: figtreeFont.name
                                            font.pixelSize: 14
                                        }

                                        Item { Layout.fillWidth: true }

                                        Label {
                                            text: gainSlider.value.toFixed(0) + " dB"
                                            color: "#010C15"
                                            font.family: figtreeFont.name
                                            font.pixelSize: 14
                                        }
                                    }

                                    SdrSlider {
                                        id: gainSlider
                                        Layout.fillWidth: true
                                        from: 0
                                        to: 50
                                        stepSize: 1
                                        value: 20
                                    }
                                }

                                RowLayout {
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    Label {
                                        text: "Correction (ppm)"
                                        color: "#010C15"
                                        font.family: figtreeFont.name
                                        font.pixelSize: 14
                                    }

                                    Item { Layout.fillWidth: true }

                                    TextField {
                                        id: correctionPPM
                                        Layout.preferredWidth: 80
                                        Layout.fillWidth: false

                                        text: "0"
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: correctionPPM.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: correctionPPM.activeFocus ? 2 : 1
                                            border.color: correctionPPM.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    }
                                }

                                GridLayout {
                                    columns: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    SdrCheckBox {
                                        text: "Shift"
                                    }

                                    SdrCheckBox {
                                        text: "Correct IQ"
                                    }

                                    SdrCheckBox {
                                        Layout.columnSpan: 2
                                        text: "Invert Spectrum"
                                    }
                                }
                            }
                        }

                        // Radio Configuration
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: "#CCD0D4"
                            radius: 8

                            ColumnLayout {
                                anchors.fill: parent

                                Label {
                                    Layout.topMargin: 8
                                    text: "Radio"
                                    Layout.alignment: Qt.AlignCenter
                                    color: "#4D5B67"
                                    font.pixelSize: 16
                                }

                                ColumnLayout {
                                    spacing: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    Label {
                                        text: "Mode"
                                        color: "#010C15"
                                        font.family: figtreeFont.name
                                        font.pixelSize: 14
                                    }

                                    Select {
                                        Layout.fillWidth: true
                                        model: ["NFM", "AM", "LSB", "USB", "WFM", "DSB", "CW", "RAW"]
                                    }
                                }

                                RowLayout {
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    Label {
                                        text: "Bandwidth"
                                        color: "#010C15"
                                        font.family: figtreeFont.name
                                        font.pixelSize: 14
                                    }

                                    Item { Layout.fillWidth: true }

                                    TextField {
                                        id: radioBandwidth
                                        Layout.preferredWidth: 80
                                        Layout.fillWidth: false

                                        text: "0"
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: radioBandwidth.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: radioBandwidth.activeFocus ? 2 : 1
                                            border.color: radioBandwidth.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    }
                                }

                                RowLayout {
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    SdrCheckBox {
                                        id: squelch
                                        text: "Squelch"
                                    }

                                    Item { Layout.fillWidth: true }

                                    TextField {
                                        id: squelchAmount
                                        Layout.preferredWidth: 80
                                        Layout.fillWidth: false
                                        visible: squelch.checked

                                        text: "0"
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: squelchAmount.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: squelchAmount.activeFocus ? 2 : 1
                                            border.color: squelchAmount.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    }
                                }

                                RowLayout {
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    Label {
                                        text: "CW Shift"
                                        color: "#010C15"
                                        font.family: figtreeFont.name
                                        font.pixelSize: 14
                                    }

                                    Item { Layout.fillWidth: true }

                                    TextField {
                                        id: cwShiftAmount
                                        Layout.preferredWidth: 80
                                        Layout.fillWidth: false

                                        text: "0"
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: cwShiftAmount.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: cwShiftAmount.activeFocus ? 2 : 1
                                            border.color: cwShiftAmount.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                    }
                                }

                                GridLayout {
                                    columns: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8

                                    SdrCheckBox {
                                        text: "Binaural"
                                    }

                                    SdrCheckBox {
                                        text: "Anti-Fading"
                                    }

                                    SdrCheckBox {
                                        text: "Lock Carrier"
                                    }

                                    SdrCheckBox {
                                        text: "Sticky Lock"
                                    }
                                }

                                Item {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }
            }

            // Main Content
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 2
                    Layout.rightMargin: 2
                    Layout.bottomMargin: 8
                    color: "#EBECEE"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 300
                            Layout.minimumHeight: 300
                            Layout.maximumHeight: 300
                            color: "#CCD0D4"
                            radius: 8
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: "#CCD0D4"
                            radius: 8
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 100
                            Layout.minimumHeight: 100
                            Layout.maximumHeight: 100
                            color: "#CCD0D4"
                            radius: 8
                        }
                    }
                }
            }

            // Adjustment Panel
            ColumnLayout {
                Layout.fillWidth: false
                Layout.preferredWidth: 84
                Layout.maximumWidth: 84
                Layout.minimumWidth: 84
                Layout.fillHeight: true

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 2
                    Layout.rightMargin: 8
                    Layout.bottomMargin: 8
                    color: "#CCD0D4"
                    radius: 8

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        Label {
                            text: "Adjustment"
                            Layout.alignment: Qt.AlignCenter
                            color: "#4D5B67"
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }
}
