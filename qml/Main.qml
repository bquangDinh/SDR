import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtGraphs

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

    SdrBackend {
        id: sdr
        // onErrorOccurred: (msg) => statusText.text = msg
        // onSpectrum: (bins) => console.log("QML got spectrum, bins:", bins.length)
    }

    Connections {
        target: sdr
        function onSpectrum(bins) {
            spectrumSeries.clear()

            const n = bins.length
            const fMin = graph.axisX.min
            const fMax = graph.axisX.max

            for (let i = 0; i < n; ++i) {
                const freq = fMin + (fMax - fMin) * i / (n - 1)
                spectrumSeries.append(freq, bins[i])
            }
        }

        function onPower(power) {
            power_meter.value = power
        }
    }

    FrameAnimation {
        id: fps
        running: true
    }

    Timer {
        interval: 500          // refresh the label twice a second
        repeat: true
        running: true
        onTriggered: fps_label.text = Math.round(1 / fps.smoothFrameTime) + " FPS"
    }

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

                RowLayout {
                    spacing: 20
                    anchors.fill: parent

                    Label {
                        text: "Settings"
                        font.bold: true
                        font.pixelSize: 16
                        color: "#EBECEE"

                        Layout.leftMargin: 16
                    }

                    Label {
                        text: "View"
                        font.bold: true
                        font.pixelSize: 16
                        color: "#EBECEE"
                    }

                    Label {
                        text: "Tools"
                        font.bold: true
                        font.pixelSize: 16
                        color: "#EBECEE"
                    }

                    Label {
                        text: "Help"
                        font.bold: true
                        font.pixelSize: 16
                        color: "#EBECEE"
                    }

                    Rectangle {
                        color: "#EBECEE"
                        Layout.preferredWidth: 2
                        Layout.maximumWidth: 2
                        Layout.preferredHeight: 20
                        Layout.maximumHeight: 20
                    }

                    SdrPlayBtn {
                        playing: sdr.running
                        onClicked: sdr.running ? sdr.stop() : sdr.start()
                    }

                    SdrFrequencyBox {
                        id: box
                        font.family: figtreeFont.name
                        font.pixelSize: 24
                        font.bold: true
                        color: "#EBECEE"
                        secondaryColor: "#4D5B67"
                        value: sdr.centerFreq
                        onEdited: (newValue) => sdr.setCenterFreq(newValue)
                    }

                    Button {
                        text: "Up"
                        onClicked: sdr.setCenterFreq(sdr.centerFreq + 1000000)
                    }

                    Button {
                        text: "Down"
                        onClicked: sdr.setCenterFreq(sdr.centerFreq - 1000000)
                    }

                    Label {
                        id: fps_label
                        text: "-- FPS"
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    // Button {
                    //     text: sdr.running ? "Stop" : "Start"
                    //     onClicked: sdr.running ? sdr.stop() : sdr.start()
                    // }
                }
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
                                        id: sourceSelect
                                        Layout.fillWidth: true
                                        model: sdr.sourceOptions
                                        textRole: "text"
                                        valueRole: "value"
                                        onActivated: sdr.setSource(currentValue)
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

                                    RowLayout {
                                        Layout.fillWidth: true

                                        Select {
                                            id: deviceSelect
                                            Layout.fillWidth: true
                                            model: sdr.deviceList
                                            textRole: "text"
                                            valueRole: "value"
                                            onActivated: sdr.setDevice(currentValue)
                                        }

                                        // Button {
                                        //     text: "Refresh"
                                        //     onClicked: sdr.refreshDevices()
                                        // }
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
                                        id: sampleRateSelect
                                        Layout.fillWidth: true
                                        model: sdr.sampleRateOptions
                                        textRole: "text"
                                        valueRole: "value"
                                        onActivated: sdr.setSampleRate(currentValue)
                                        currentIndex: 3 // 2_4 MSPS
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
                                        id: samplingModeSelect
                                        Layout.fillWidth: true
                                        model: sdr.sampleModeOptions
                                        textRole: "text"
                                        valueRole: "value"
                                        onActivated: sdr.setSamplingMode(currentValue);
                                        currentIndex: 0
                                    }
                                }


                                GridLayout {
                                    columns: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    SdrCheckBox {
                                        text: "RTL AGC"
                                        checked: sdr.rtlagc
                                        onToggled: sdr.setRtlAGC(checked)
                                    }

                                    SdrCheckBox {
                                        text: "Offset Tuning"
                                        checked: sdr.offsetTuning
                                        onToggled: sdr.setOffsetTuning(checked)
                                    }

                                    SdrCheckBox {
                                        id: tunerAGC
                                        text: "Tuner AGC"
                                        checked: sdr.tunerAGC
                                        onToggled: sdr.setTunerAGC(checked)
                                    }

                                    SdrCheckBox {
                                        text: "Bias-Tee"
                                        checked: sdr.biasTee
                                        onToggled: sdr.setBiasTee(checked)
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
                                            text: !tunerAGC.checked ? gainSlider.value.toFixed(0) + " dB" : "Disabled"
                                            color: "#010C15"
                                            font.family: figtreeFont.name
                                            font.pixelSize: 14
                                            font.italic: tunerAGC.checked
                                        }
                                    }

                                    SdrSlider {
                                        id: gainSlider
                                        opacity: !tunerAGC.checked ? 1 : 0.5
                                        Layout.fillWidth: true
                                        from: 0
                                        to: 50
                                        stepSize: 1
                                        enabled: !tunerAGC.checked
                                        value: 0
                                        onMoved: sdr.setRfGain(value)
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

                                        text: sdr.correctionPPM
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: correctionPPM.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: correctionPPM.activeFocus ? 2 : 1
                                            border.color: correctionPPM.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                        onEditingFinished: {
                                            const correctionPPM = parseInt(text)

                                            if (!isNaN(correctionPPM)) {
                                                sdr.setCorrectionPPM(correctionPPM)
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    SdrCheckBox {
                                        id: shiftCheckbox
                                        text: "Shift"
                                        checked: sdr.shift
                                        onToggled: sdr.setShift(checked)
                                    }

                                    Item {
                                        Layout.fillWidth: true
                                    }

                                    TextField {
                                        id: shiftAmount
                                        Layout.preferredWidth: 80
                                        Layout.fillWidth: false
                                        visible: shiftCheckbox.checked
                                        text: sdr.shiftAmount
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: shiftAmount.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: shiftAmount.activeFocus ? 2 : 1
                                            border.color: shiftAmount.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                        onEditingFinished: {
                                            const shiftAmount = parseInt(text)

                                            if (!isNaN(shiftAmount)) {
                                                sdr.setShiftAmount(shiftAmount)
                                            }
                                        }
                                    }
                                }

                                GridLayout {
                                    columns: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    SdrCheckBox {
                                        text: "CIQ"
                                        checked: sdr.correctIQ
                                        onToggled: sdr.setCorrectIQ(checked)
                                    }

                                    SdrCheckBox {
                                        text: "Invert Spectrum"
                                        checked: sdr.invertSpectrum
                                        onToggled: sdr.setInvertSpectrum(checked)
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
                                        id: demodSelect
                                        Layout.fillWidth: true
                                        model: sdr.demodModeOptions
                                        textRole: "text"
                                        valueRole: "value"
                                        onActivated: sdr.setDemodMode(currentValue)
                                        currentIndex: 0
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

                                        text: sdr.bandwidth
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: radioBandwidth.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: radioBandwidth.activeFocus ? 2 : 1
                                            border.color: radioBandwidth.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                        onEditingFinished: {
                                            const bandwidth = parseInt(text)

                                            if (!isNaN(bandwidth)) {
                                                sdr.setBandwidth(bandwidth)
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8
                                    Layout.bottomMargin: 2

                                    SdrCheckBox {
                                        id: squelch
                                        text: "Squelch"
                                        checked: sdr.squelch
                                        onToggled: sdr.setSquelch(checked)
                                    }

                                    Item { Layout.fillWidth: true }

                                    TextField {
                                        id: squelchAmount
                                        Layout.preferredWidth: 80
                                        Layout.fillWidth: false
                                        visible: squelch.checked

                                        text: sdr.squelchAmount
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: squelchAmount.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: squelchAmount.activeFocus ? 2 : 1
                                            border.color: squelchAmount.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                        onEditingFinished: {
                                            const squelchAmount = parseInt(text)

                                            if (!isNaN(squelchAmount)) {
                                                sdr.setSquelchAmount(squelchAmount)
                                            }
                                        }
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

                                        text: sdr.cwShiftAmount
                                        color: "#EBECEE"

                                        background: Rectangle {
                                            radius: 4
                                            color: cwShiftAmount.hovered ? Qt.darker("#99A1A8", 1.5) : "#99A1A8"
                                            border.width: cwShiftAmount.activeFocus ? 2 : 1
                                            border.color: cwShiftAmount.activeFocus ? "#011526" : "#99A1A8"
                                        }

                                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                                        onEditingFinished: {
                                            const cwShiftAmount = parseInt(text)

                                            if (!isNaN(cwShiftAmount)) {
                                                sdr.setCWShiftAmount(cwShiftAmount)
                                            }
                                        }
                                    }
                                }

                                GridLayout {
                                    columns: 2
                                    Layout.leftMargin: 8
                                    Layout.rightMargin: 8

                                    SdrCheckBox {
                                        text: "Binaural"
                                        checked: sdr.binaural
                                        onToggled: sdr.setBinaural(checked)
                                    }

                                    SdrCheckBox {
                                        text: "Anti-Fading"
                                        checked: sdr.antiFading
                                        onToggled: sdr.setAntiFading(checked)
                                    }

                                    SdrCheckBox {
                                        text: "Lock Carrier"
                                        checked: sdr.lockCarrier
                                        onToggled: sdr.setLockCarrier(checked)
                                    }

                                    SdrCheckBox {
                                        text: "Sticky Lock"
                                        checked: sdr.stickyLock
                                        onToggled: sdr.setStickyLock(checked)
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

                        // FFT Spectrum
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 300
                            Layout.minimumHeight: 300
                            Layout.maximumHeight: 300
                            color: "#CCD0D4"
                            radius: 8

                            ColumnLayout {
                                anchors.fill: parent

                                GraphsView {
                                    id: graph
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true

                                    theme: GraphsTheme {
                                        grid.mainColor: "#606060"
                                        grid.subColor: "#303030"
                                        grid.mainWidth: 1
                                        grid.subWidth: 1
                                    }

                                    axisX: ValueAxis {
                                        id: xAxis
                                        min: sdr.minFreqScaled + (sdr.shift ? (sdr.shiftAmount / sdr.freqScale) : 0)
                                        max: sdr.maxFreqScaled + (sdr.shift ? (sdr.shiftAmount / sdr.freqScale) : 0)
                                        titleText: "Frequency (%1)".arg(sdr.freqScaleLabel)
                                        tickInterval: 0.1
                                        subTickCount: 4
                                        gridVisible: true
                                        subGridVisible: true
                                    }

                                    axisY: ValueAxis {
                                        id: yAxis
                                        min: -150
                                        max: 0
                                        titleText: "dBFS"
                                        tickInterval: 10
                                        subTickCount: 1
                                        subGridVisible: true
                                    }

                                    LineSeries {
                                        id: spectrumSeries
                                        name: "Spectrum"
                                        color: "#4fc3f7"
                                        width: 0.5
                                        capStyle: Qt.RoundCap
                                    }
                                }
                            }
                        }

                        // FFT Wide Waterfall
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            SdrPowerMeter {
                                id: power_meter
                                Layout.fillHeight: true
                                Layout.preferredWidth: implicitWidth
                                from: -150
                                to: 0
                                font.family: figtreeFont.name
                            }

                            Rectangle {
                                id: waterfall_panel
                                color: "#CCD0D4"
                                radius: 8
                                clip: true
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                property real sampleRate: sdr.sampleRate            // must match the SDR
                                property real centerHz: sdr.centerFreq            // hardware center frequency
                                property real channelWidthHz: sdr.bandwidth          // your ±8 kHz channel filter
                                property real offsetHz: 0                    // selected channel, relative to center
                                readonly property real hzPerPixel: sampleRate / width

                                // pixel x → offset in Hz. Left edge = −fs/2, right edge = +fs/2 (matches your fftshift)
                                function setFromX(px) {
                                    const limit = sampleRate / 2 - channelWidthHz;          // keep the channel inside the band
                                    let hz = (px / width - 0.5) * sampleRate;
                                    hz = Math.round(hz / 500) * 500;                        // snap to 500 Hz steps
                                    offsetHz = Math.max(-limit, Math.min(limit, hz));
                                }

                                onOffsetHzChanged: sdr.setOffsetFreq(offsetHz)

                                Waterfall {
                                    id: waterfall
                                    anchors.fill: parent
                                    source: sdr
                                    history: 300
                                    minDb: -150
                                    maxDb: 0
                                    direction: "vertical"
                                }

                                // channel passband: translucent band as wide as the filter
                                Rectangle {
                                    id: band
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    width: Math.max(3, waterfall_panel.channelWidthHz / waterfall_panel.hzPerPixel)
                                    x: (waterfall_panel.offsetHz / waterfall_panel.sampleRate + 0.5) * waterfall_panel.width - width / 2
                                    color: "#4000ff00"                        // ARGB: about 25% opaque green
                                }

                                // center line of the selected channel
                                Rectangle {
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    width: 2
                                    x: band.x + band.width / 2 - 1
                                    color: "#00ff00"
                                }

                                // frequency label next to the line
                                Text {
                                    x: Math.min(band.x + band.width + 4, waterfall_panel.width - width - 4)
                                    y: 6
                                    color: "white"
                                    style: Text.Outline
                                    styleColor: "black"
                                    text: ((waterfall_panel.centerHz + waterfall_panel.offsetHz) / 1e6).toFixed(4) + " MHz"
                                }

                                // click to jump, drag to slide
                                TapHandler {
                                    onTapped: (eventPoint, button) => waterfall_panel.setFromX(eventPoint.position.x)
                                }

                                DragHandler {
                                    target: null                              // don't move any item; just read the position
                                    xAxis.enabled: true
                                    yAxis.enabled: false
                                    onCentroidChanged: if (active) waterfall_panel.setFromX(centroid.position.x)
                                }
                            }
                        }

                        // FFT Narrow Waterfall
                        Rectangle {
                            id: narrow_panel
                            Layout.fillWidth: true
                            Layout.preferredHeight: 100
                            Layout.minimumHeight: 100
                            Layout.maximumHeight: 100
                            color: "#CCD0D4"
                            radius: 8

                            Waterfall {
                                id: waterfall_narrow
                                anchors.fill: parent
                                source: sdr
                                history: 300
                                minDb: -150
                                maxDb: 0
                                useNarrow: true
                                direction: "horizontal"
                            }
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
