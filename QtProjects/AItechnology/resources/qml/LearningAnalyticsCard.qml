import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes

Rectangle {
    id: root
    width: 820
    height: 320  // 减少整体高度，从360改为320
    color: "#F3F4F6"  // 与主背景色一致，彻底解决黑色尖角问题
    Layout.alignment: Qt.AlignHCenter
    clip: true  // 添加裁切，确保圆角效果干净

    readonly property real ringWidth: 16
    readonly property real progress: 0.85

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 0  // 去除边距，让白色卡片填满整个区域
        radius: 12
        color: "#FFFFFF"
        antialiasing: true
        border.width: 0
        clip: true  // 确保内容也被裁切，跟随圆角

        // 去除阴影效果，确保没有任何黑色背景
        // Rectangle {
        //     anchors.fill: parent
        //     anchors.margins: 16
        //     radius: 12
        //     color: "#000000"
        //     opacity: 0.08
        //     z: -1
        // }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20  // 大幅减少内边距，从28改为20
            spacing: 12  // 减少标题与内容区域的间距，从20改为12

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "学情分析"
                    color: "#1F2937"
                    font.pixelSize: 18
                    font.bold: true
                    Layout.fillWidth: true
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 72

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
                                property bool tooltipVisible: false

                                Shape {
                                    id: donut
                                    anchors.fill: parent
                                    antialiasing: true

                                    ShapePath {
                                        strokeWidth: ringWidth
                                        strokeColor: "#E5E7EB"
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
                                        strokeColor: "#D32F2F"  // 改为深红色，与按钮颜色一致
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

                                // 悬停显示加权计算公式提示
                                Rectangle {
                                    id: tooltipBubble
                                    anchors.left: parent.left
                                    anchors.leftMargin: -20
                                    anchors.bottom: parent.top
                                    anchors.bottomMargin: 12
                                    color: "#2C2C2C"
                                    radius: 6
                                    z: 100
                                    opacity: donutFrame.tooltipVisible ? 1 : 0
                                    visible: opacity > 0
                                    implicitWidth: tooltipText.implicitWidth + 24
                                    implicitHeight: tooltipText.implicitHeight + 16
                                    Behavior on opacity {
                                        NumberAnimation { duration: 200 }
                                    }

                                    Text {
                                        id: tooltipText
                                        anchors.centerIn: parent
                                        text: "计算公式：\n(课堂参与 92% + 专注度 88% + 测验 79% + 提问转化分 81%) ÷ 4 = 85%"
                                        color: "#FFFFFF"
                                        font.pixelSize: 13
                                        lineHeight: 1.2
                                        wrapMode: Text.Wrap
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                }

                                Shape {
                                    id: tooltipArrow
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.top: tooltipBubble.bottom
                                    width: 16
                                    height: 8
                                    visible: tooltipBubble.visible
                                    opacity: tooltipBubble.opacity
                                    z: tooltipBubble.z
                                    antialiasing: true

                                    ShapePath {
                                        strokeWidth: 0
                                        fillColor: "#2C2C2C"
                                        startX: 0
                                        startY: 0
                                        PathLine { x: tooltipArrow.width / 2; y: tooltipArrow.height }
                                        PathLine { x: tooltipArrow.width; y: 0 }
                                        PathLine { x: 0; y: 0 }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onEntered: donutFrame.tooltipVisible = true
                                    onExited: donutFrame.tooltipVisible = false
                                }
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: parent.width
                                text: "整体学习进度"
                                color: "#6B7280"
                                font.pixelSize: 14
                                font.weight: Font.Normal
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }

                    GridLayout {
                        id: metricsLayout
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignVCenter
                        columns: 2
                        rowSpacing: 15
                        columnSpacing: 15

                        property var metrics: [
                            { color: "#4285F4", label: "课堂参与度", value: "92%" },
                            { color: "#34A853", label: "专注度", value: "88%" },
                            { color: "#FBBC05", label: "测验正确率", value: "79%" },
                            { color: "#EA4335", label: "提问转化分", value: "81%" }
                        ]

                        Repeater {
                            model: metricsLayout.metrics
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                height: 70
                                color: "#F9F9F9"
                                radius: 8
                                antialiasing: true
                                clip: true  // 确保数据卡片内容也被裁切

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 12
                                        height: 12
                                        radius: 6
                                        color: modelData.color
                                        border.width: 0
                                        Layout.alignment: Qt.AlignVCenter
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4

                                        Text {
                                            text: modelData.label
                                            color: "#6B7280"
                                            font.pixelSize: 12
                                            font.weight: Font.Medium
                                            Layout.fillWidth: true
                                        }

                                        Text {
                                            text: modelData.value
                                            color: "#111827"
                                            font.pixelSize: 18
                                            font.weight: Font.DemiBold
                                            Layout.fillWidth: true
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
