import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15
import QtQuick.Effects 1.15

Rectangle {
    id: root
    width: 840
    height: 360
    color: "#F5F6FA"

    readonly property real ringWidth: 16
    readonly property real progress: 0.85

    DropShadow {
        anchors.fill: card
        source: card
        horizontalOffset: 0
        verticalOffset: 10
        radius: 24
        samples: 32
        color: "#1A000000"
    }

    Rectangle {
        id: card
        anchors.centerIn: parent
        width: parent.width - 72
        height: parent.height - 48
        radius: 16
        color: "#FFFFFF"
        antialiasing: true
        border.color: "#F0F0F0"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 32

            Item {
                id: chartArea
                Layout.preferredWidth: 240
                Layout.fillHeight: true

                Column {
                    anchors.centerIn: parent
                    spacing: 16

                    Item {
                        id: donutFrame
                        width: 200
                        height: 200

                        Shape {
                            id: donut
                            anchors.fill: parent
                            antialiasing: true

                            ShapePath {
                                strokeWidth: ringWidth
                                strokeColor: "#EEEEEE"
                                capStyle: ShapePath.RoundCap
                                fillColor: "transparent"
                                PathAngleArc {
                                    centerX: donut.width / 2
                                    centerY: donut.height / 2
                                    radiusX: (donut.width - ringWidth) / 2
                                    radiusY: (donut.height - ringWidth) / 2
                                    startAngle: -90
                                    sweepAngle: 360
                                }
                            }

                            ShapePath {
                                strokeWidth: ringWidth
                                strokeColor: "#D32F2F"
                                capStyle: ShapePath.RoundCap
                                fillColor: "transparent"
                                PathAngleArc {
                                    centerX: donut.width / 2
                                    centerY: donut.height / 2
                                    radiusX: (donut.width - ringWidth) / 2
                                    radiusY: (donut.height - ringWidth) / 2
                                    startAngle: -90
                                    sweepAngle: 360 * progress
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "85%"
                            color: "#111827"
                            font.pixelSize: 48
                            font.bold: true
                        }
                    }

                    Text {
                        anchors.horizontalCenter: donutFrame.horizontalCenter
                        text: "整体学习进度"
                        color: "#6B7280"
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }
                }
            }

            GridLayout {
                id: metricsLayout
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter
                columns: 2
                columnSpacing: 28
                rowSpacing: 18

                property var metrics: [
                    { color: "#4285F4", label: "课堂参与度", value: "92%" },
                    { color: "#34A853", label: "专注度", value: "88%" },
                    { color: "#FBBC05", label: "测验正确率", value: "79%" },
                    { color: "#EA4335", label: "提问次数", value: "12" }
                ]

                Repeater {
                    model: metricsLayout.metrics
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Rectangle {
                            width: 12
                            height: 12
                            radius: 6
                            color: modelData.color
                            Layout.alignment: Qt.AlignVCenter
                        }

                        ColumnLayout {
                            spacing: 4
                            Layout.fillWidth: true

                            Text {
                                text: modelData.label
                                color: "#6B7280"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                            }

                            Text {
                                text: modelData.value
                                color: "#1F2937"
                                font.pixelSize: 20
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                }
            }
        }
    }
}
