import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Rectangle {
    id: filterPanel

    property bool isDark: Theme.isDark
    property alias selectedCourse: courseCombo.currentText
    property alias selectedVersion: versionCombo.currentText
    property alias selectedGrade: gradeCombo.currentText
    property alias selectedChapter: chapterCombo.currentText
    property string selectedPaperType: "不限"
    property string selectedQuestionType: "不限"
    property string selectedDifficulty: "不限"

    signal generateClicked()

    width: 320
    color: isDark ? Qt.darker(Theme.surfaceDark, 0.5) : Theme.surfaceLight
    radius: Theme.radiusLarge

    // 移除DropShadow依赖，使用边框代替

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: Theme.spacingLarge

        // 标题
        Text {
            Layout.fillWidth: true

            text: "试题筛选"
            color: isDark ? Theme.textOnDark : Theme.textPrimary
            font.pixelSize: Theme.fontSizeTitle
            font.weight: Font.Bold
        }

        // 筛选条件
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            contentWidth: availableWidth
            contentHeight: column.implicitHeight

            ColumnLayout {
                id: column
                width: parent.width
                spacing: Theme.spacingMedium

                // 下拉选择组
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacingMedium

                    // 课程范围
                    Text {
                        Layout.fillWidth: true

                        text: "选择课程范围"
                        color: isDark ? Theme.textOnDark : Theme.textSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    ComboBox {
                        id: courseCombo
                        Layout.fillWidth: true

                        model: ["思想道德与法治", "中国近现代史纲要", "马克思主义基本原理", "毛泽东思想和中国特色社会主义理论体系概论"]

                        background: Rectangle {
                            color: isDark ? Theme.backgroundDark : Theme.backgroundLight
                            border.color: filterPanel.isDark ? Theme.borderDark : Theme.borderLight
                            border.width: 1
                            radius: Theme.radiusMedium

                            Behavior on border.color {
                                ColorAnimation { duration: 200 }
                            }
                        }

                        contentItem: Text {
                            leftPadding: 12
                            verticalAlignment: Text.AlignVCenter
                            text: courseCombo.displayText
                            color: isDark ? Theme.textOnDark : Theme.textPrimary
                            font.pixelSize: Theme.fontSizeMedium
                        }

                        indicator: Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: "v"
                            color: isDark ? Theme.textOnDark : Theme.textSecondary
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        popup: Popup {
                            y: courseCombo.height - 1
                            width: courseCombo.width
                            implicitHeight: contentItem.implicitHeight
                            padding: 8

                            contentItem: ListView {
                                clip: true
                                model: courseCombo.popup.visible ? courseCombo.delegateModel : null
                                currentIndex: courseCombo.highlightedIndex
                            }

                            background: Rectangle {
                                color: isDark ? Theme.surfaceDark : Theme.surfaceLight
                                border.color: isDark ? Theme.borderDark : Theme.borderLight
                                border.width: 1
                                radius: Theme.radiusMedium
                            }
                        }
                    }

                    // 教材版本
                    Text {
                        Layout.fillWidth: true

                        text: "选择教材版本"
                        color: isDark ? Theme.textOnDark : Theme.textSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    ComboBox {
                        id: versionCombo
                        Layout.fillWidth: true

                        model: ["人教版", "部编版", "高教版", "马工程版"]

                        background: Rectangle {
                            color: isDark ? Theme.backgroundDark : Theme.backgroundLight
                            border.color: isDark ? Theme.borderDark : Theme.borderLight
                            border.width: 1
                            radius: Theme.radiusMedium
                        }

                        contentItem: Text {
                            leftPadding: 12
                            verticalAlignment: Text.AlignVCenter
                            text: versionCombo.displayText
                            color: isDark ? Theme.textOnDark : Theme.textPrimary
                            font.pixelSize: Theme.fontSizeMedium
                        }

                        indicator: Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: "v"
                            color: isDark ? Theme.textOnDark : Theme.textSecondary
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        popup: Popup {
                            y: versionCombo.height - 1
                            width: versionCombo.width
                            implicitHeight: contentItem.implicitHeight
                            padding: 8

                            contentItem: ListView {
                                clip: true
                                model: versionCombo.popup.visible ? versionCombo.delegateModel : null
                                currentIndex: versionCombo.highlightedIndex
                            }

                            background: Rectangle {
                                color: isDark ? Theme.surfaceDark : Theme.surfaceLight
                                border.color: isDark ? Theme.borderDark : Theme.borderLight
                                border.width: 1
                                radius: Theme.radiusMedium
                            }
                        }
                    }

                    // 年级学期
                    Text {
                        Layout.fillWidth: true

                        text: "选择年级学期"
                        color: isDark ? Theme.textOnDark : Theme.textSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    ComboBox {
                        id: gradeCombo
                        Layout.fillWidth: true

                        model: ["七年级上学期", "七年级下学期", "八年级上学期", "八年级下学期", "九年级上学期", "九年级下学期"]

                        background: Rectangle {
                            color: isDark ? Theme.backgroundDark : Theme.backgroundLight
                            border.color: isDark ? Theme.borderDark : Theme.borderLight
                            border.width: 1
                            radius: Theme.radiusMedium
                        }

                        contentItem: Text {
                            leftPadding: 12
                            verticalAlignment: Text.AlignVCenter
                            text: gradeCombo.displayText
                            color: isDark ? Theme.textOnDark : Theme.textPrimary
                            font.pixelSize: Theme.fontSizeMedium
                        }

                        indicator: Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: "v"
                            color: isDark ? Theme.textOnDark : Theme.textSecondary
                            font.pixelSize: Theme.fontSizeSmall
                        }
                    }

                    // 章节
                    Text {
                        Layout.fillWidth: true

                        text: "选择章节"
                        color: isDark ? Theme.textOnDark : Theme.textSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    ComboBox {
                        id: chapterCombo
                        Layout.fillWidth: true

                        model: ["第一章 绪论", "第二章 世界的物质性", "第三章 实践与认识", "第四章 人类社会及其发展规律"]

                        background: Rectangle {
                            color: isDark ? Theme.backgroundDark : Theme.backgroundLight
                            border.color: isDark ? Theme.borderDark : Theme.borderLight
                            border.width: 1
                            radius: Theme.radiusMedium
                        }

                        contentItem: Text {
                            leftPadding: 12
                            verticalAlignment: Text.AlignVCenter
                            text: chapterCombo.displayText
                            color: isDark ? Theme.textOnDark : Theme.textPrimary
                            font.pixelSize: Theme.fontSizeMedium
                        }

                        indicator: Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: "v"
                            color: isDark ? Theme.textOnDark : Theme.textSecondary
                            font.pixelSize: Theme.fontSizeSmall
                        }
                    }
                }

                // 按钮选择组
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacingMedium

                    // 试卷类型
                    Text {
                        Layout.fillWidth: true

                        text: "试卷类型"
                        color: isDark ? Theme.textOnDark : Theme.textSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        property var types: ["不限", "章节练习", "期中考试", "期末考试", "模拟测试"]

                        onTypesChanged: {
                            // 动态创建按钮
                        }

                        Repeater {
                            model: ["不限", "章节练习", "期中考试", "期末考试", "模拟测试"]
                            delegate: Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36

                                text: modelData

                                background: Rectangle {
                                    color: selectedPaperType === modelData ?
                                              (isDark ? Qt.darker(Theme.primary, 1.2) : Theme.primaryLight) :
                                              (isDark ? "#374151" : "#F3F4F6")
                                    radius: 8

                                    Behavior on color {
                                        ColorAnimation { duration: 200 }
                                    }
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: selectedPaperType === modelData ?
                                              Theme.primary :
                                              (isDark ? "#D1D5DB" : "#6B7280")
                                    font.pixelSize: Theme.fontSizeSmall
                                    font.weight: Font.Medium
                                }

                                onClicked: {
                                    selectedPaperType = modelData
                                }
                            }
                        }
                    }

                    // 题目题型
                    Text {
                        Layout.fillWidth: true

                        text: "题目题型"
                        color: isDark ? Theme.textOnDark : Theme.textSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        Repeater {
                            model: ["不限", "单选题", "多选题", "判断题", "简答题"]
                            delegate: Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36

                                text: modelData

                                background: Rectangle {
                                    color: selectedQuestionType === modelData ?
                                              (isDark ? Qt.darker(Theme.primary, 1.2) : Theme.primaryLight) :
                                              (isDark ? "#374151" : "#F3F4F6")
                                    radius: 8

                                    Behavior on color {
                                        ColorAnimation { duration: 200 }
                                    }
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: selectedQuestionType === modelData ?
                                              Theme.primary :
                                              (isDark ? "#D1D5DB" : "#6B7280")
                                    font.pixelSize: Theme.fontSizeSmall
                                    font.weight: Font.Medium
                                }

                                onClicked: {
                                    selectedQuestionType = modelData
                                }
                            }
                        }
                    }

                    // 题目难度
                    Text {
                        Layout.fillWidth: true

                        text: "题目难度"
                        color: isDark ? Theme.textOnDark : Theme.textSecondary
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        Repeater {
                            model: ["不限", "简单", "中等", "困难"]
                            delegate: Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36

                                text: modelData

                                background: Rectangle {
                                    color: selectedDifficulty === modelData ?
                                              (isDark ? Qt.darker(Theme.primary, 1.2) : Theme.primaryLight) :
                                              (isDark ? "#374151" : "#F3F4F6")
                                    radius: 8

                                    Behavior on color {
                                        ColorAnimation { duration: 200 }
                                    }
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: selectedDifficulty === modelData ?
                                              Theme.primary :
                                              (isDark ? "#D1D5DB" : "#6B7280")
                                    font.pixelSize: Theme.fontSizeSmall
                                    font.weight: Font.Medium
                                }

                                onClicked: {
                                    selectedDifficulty = modelData
                                }
                            }
                        }
                    }
                }
            }
        }

        // 开始生成按钮
        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 48

            text: "开始生成"

            background: Rectangle {
                color: Theme.primary
                radius: Theme.radiusMedium

                Behavior on color {
                    ColorAnimation { duration: 200 }
                }

                SequentialAnimation {
                    running: parent.parent.parent.parent.parent.parent.parent && button.pressed
                    PropertyAnimation {
                        target: button
                        property: "scale"
                        to: 0.95
                        duration: 100
                    }
                    PropertyAnimation {
                        target: button
                        property: "scale"
                        to: 1.0
                        duration: 100
                    }
                }
            }

            contentItem: RowLayout {
                spacing: 8

                Text {
                    Layout.fillWidth: true

                    text: "开始生成"
                    color: Theme.textOnPrimary
                    font.pixelSize: Theme.fontSizeMedium
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    text: "*"
                    font.pixelSize: Theme.fontSizeMedium
                }
            }

            onClicked: {
                generateClicked()
            }
        }
    }

    // 滚动条样式
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        contentItem: Rectangle {
            implicitWidth: 8
            radius: 4
            color: isDark ? "#4B5563" : "#D1D5DB"
        }
    }
}
