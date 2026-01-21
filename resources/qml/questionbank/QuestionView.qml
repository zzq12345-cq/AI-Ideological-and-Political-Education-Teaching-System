import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: questionView

    property var question: null  // Question对象
    property bool isDark: Theme.isDark
    property int currentIndex: 1
    property int totalCount: 20
    property bool showAnswer: false
    property string selectedAnswer: ""

    signal answerChanged(string answer)
    signal showAnswerClicked()
    signal exportClicked()
    signal previousClicked()

    color: isDark ? Qt.darker(Theme.surfaceDark, 0.5) : Theme.surfaceLight
    radius: Theme.radiusLarge

    // 移除DropShadow依赖

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: Theme.spacingLarge

        // 头部：徽标和进度
        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMedium

            // 左：徽标
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Badge {
                    text: {
                        switch(question?.type) {
                            case "single": return "单选题"
                            case "multi": return "多选题"
                            case "truefalse": return "判断题"
                            case "short": return "简答题"
                            default: return ""
                        }
                    }
                    type: "accent"
                    isDark: questionView.isDark
                }

                Badge {
                    text: {
                        switch(question?.difficulty) {
                            case "easy": return "简单"
                            case "medium": return "中等"
                            case "hard": return "困难"
                            default: return ""
                        }
                    }
                    type: question?.difficulty
                    isDark: questionView.isDark
                }
            }

            // 右：进度
            Text {
                Layout.alignment: Qt.AlignRight

                text: "进度: " + currentIndex + " / " + totalCount
                color: isDark ? Theme.textOnDark : Theme.textSecondary
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.Medium
            }
        }

        // 题目内容区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 200

            color: "transparent"
            radius: Theme.radiusMedium

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingMedium
                spacing: Theme.spacingMedium

                // 题干
                Text {
                    Layout.fillWidth: true

                    text: question?.stem || "请选择筛选条件并点击开始生成"
                    color: isDark ? Theme.textOnDark : Theme.textPrimary
                    font.pixelSize: Theme.fontSizeExtraLarge
                    font.weight: Font.Normal
                    wrapMode: Text.WordWrap
                    lineHeight: 1.6
                }

                // 选项列表
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    model: question?.options || []
                    spacing: 12

                    delegate: OptionItem {
                        width: ListView.view.width
                        optionText: modelData
                        optionLabel: String.fromCharCode(65 + index)
                        questionType: question?.type || "single"
                        selected: {
                            if (question?.type === "single") {
                                return selectedAnswer === String.fromCharCode(65 + index)
                            } else if (question?.type === "multi") {
                                return selectedAnswer.indexOf(String.fromCharCode(65 + index)) !== -1
                            }
                            return false
                        }
                        isCorrect: {
                            if (!question) return false
                            if (question.type === "single") {
                                return question.answer === String.fromCharCode(65 + index)
                            } else if (question.type === "multi") {
                                return question.answer.indexOf(String.fromCharCode(65 + index)) !== -1
                            } else if (question.type === "truefalse") {
                                return question.answer === modelData
                            }
                            return false
                        }
                        showAnswer: questionView.showAnswer
                        isDark: questionView.isDark

                        onClicked: {
                            if (question) {
                                if (question.type === "single") {
                                    selectedAnswer = String.fromCharCode(65 + index)
                                    answerChanged(selectedAnswer)
                                } else if (question.type === "multi") {
                                    var letter = String.fromCharCode(65 + index)
                                    if (selectedAnswer.indexOf(letter) === -1) {
                                        selectedAnswer += letter
                                    } else {
                                        selectedAnswer = selectedAnswer.replace(letter, "")
                                    }
                                    answerChanged(selectedAnswer)
                                } else if (question.type === "truefalse") {
                                    selectedAnswer = modelData
                                    answerChanged(selectedAnswer)
                                }
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        contentItem: Rectangle {
                            implicitWidth: 8
                            radius: 4
                            color: isDark ? "#4B5563" : "#D1D5DB"
                        }
                    }
                }

                // 答案和解析（仅在显示答案时显示）
                Rectangle {
                    Layout.fillWidth: true
                    Layout.maximumHeight: showAnswer ? implicitHeight : 0
                    Layout.minimumHeight: showAnswer ? implicitHeight : 0
                    clip: true

                    color: "transparent"
                    radius: Theme.radiusMedium

                    Behavior on Layout.maximumHeight {
                        NumberAnimation { duration: 200 }
                    }

                    implicitHeight: answerColumn.implicitHeight + 24

                    Column {
                        id: answerColumn
                        width: parent.width - 24
                        anchors.centerIn: parent
                        spacing: 16

                        // 分隔线
                        Rectangle {
                            width: parent.width
                            height: 1
                            color: isDark ? Theme.borderDark : Theme.borderLight
                        }

                        // 正确答案
                        Text {
                            width: parent.width
                            text: "正确答案"
                            color: isDark ? Theme.textOnDark : Theme.textSecondary
                            font.pixelSize: Theme.fontSizeMedium
                            font.weight: Font.Bold
                        }

                        Text {
                            width: parent.width
                            text: question?.answer || ""
                            color: Theme.success
                            font.pixelSize: Theme.fontSizeLarge
                            font.weight: Font.Bold
                        }

                        // 解析
                        Text {
                            width: parent.width
                            text: "解析"
                            color: isDark ? Theme.textOnDark : Theme.textSecondary
                            font.pixelSize: Theme.fontSizeMedium
                            font.weight: Font.Bold
                        }

                        Text {
                            width: parent.width
                            text: question?.explain || ""
                            color: isDark ? Theme.textOnDark : Theme.textSecondary
                            font.pixelSize: Theme.fontSizeMedium
                            wrapMode: Text.WordWrap
                            lineHeight: 1.6
                        }
                    }
                }
            }
        }

        // 底部按钮
        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMedium

            // 上一题
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48

                enabled: currentIndex > 1

                background: Rectangle {
                    color: enabled ?
                              (isDark ? "#374151" : "#E5E7EB") :
                              (isDark ? "#1F2937" : "#F3F4F6")
                    radius: Theme.radiusMedium

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                contentItem: RowLayout {
                    spacing: 8

                    Text {
                        Layout.fillWidth: true

                        text: "上一题"
                        color: enabled ?
                                 (isDark ? "#D1D5DB" : "#6B7280") :
                                 (isDark ? "#6B7280" : "#9CA3AF")
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        text: "←"
                        font.pixelSize: Theme.fontSizeMedium
                        color: parent.color
                    }
                }

                onClicked: previousClicked()
            }

            // 查看答案
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48

                background: Rectangle {
                    color: isDark ? Qt.darker(Theme.accent, 1.2) : Theme.accentLight
                    radius: Theme.radiusMedium

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                contentItem: RowLayout {
                    spacing: 8

                    Text {
                        Layout.fillWidth: true

                        text: showAnswer ? "隐藏答案" : "查看答案"
                        color: isDark ? "#7DD3FC" : Theme.accent
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        text: showAnswer ? "[Show]" : "[Hide]"
                        font.pixelSize: Theme.fontSizeMedium
                    }
                }

                onClicked: showAnswerClicked()
            }

            // 导出试卷
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48

                background: Rectangle {
                    color: Theme.accent
                    radius: Theme.radiusMedium

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                contentItem: RowLayout {
                    spacing: 8

                    Text {
                        Layout.fillWidth: true

                        text: "导出试卷"
                        color: Theme.textOnPrimary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        text: "[DL]"
                        font.pixelSize: Theme.fontSizeMedium
                    }
                }

                onClicked: exportClicked()
            }
        }
    }
}
