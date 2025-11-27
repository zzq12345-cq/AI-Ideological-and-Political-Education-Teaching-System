import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Page {
    id: root

    property bool isDark: Theme.isDark
    property bool isFilterExpanded: true

    background: Rectangle {
        color: isDark ? Theme.backgroundDark : Theme.backgroundLight
    }

    property var questionRepository: null  // QuestionRepository对象

    // 筛选条件
    readonly property alias filterCourse: filterPanel.selectedCourse
    readonly property alias filterVersion: filterPanel.selectedVersion
    readonly property alias filterGrade: filterPanel.selectedGrade
    readonly property alias filterChapter: filterPanel.selectedChapter
    readonly property alias filterPaperType: filterPanel.selectedPaperType
    readonly property alias filterQuestionType: filterPanel.selectedQuestionType
    readonly property alias filterDifficulty: filterPanel.selectedDifficulty

    header: ToolBar {
        Material.background: isDark ? Qt.darker(Theme.surfaceDark, 0.5) : Theme.surfaceLight

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24

            // 返回按钮
            Button {
                Layout.alignment: Qt.AlignLeft

                background: Rectangle {
                    color: "transparent"
                    radius: 8
                }

                contentItem: RowLayout {
                    spacing: 8

                    Text {
                        text: "←"
                        font.pixelSize: Theme.fontSizeMedium
                        color: isDark ? "#D1D5DB" : "#6B7280"
                    }

                    Text {
                        Layout.fillWidth: true

                        text: "返回主界面"
                        color: isDark ? "#D1D5DB" : "#6B7280"
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                    }
                }

                onClicked: {
                    // TODO: 返回主界面
                    console.log("返回主界面")
                }
            }

            // 标题
            Text {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                text: "AI 智能试题库"
                color: isDark ? Theme.textOnDark : Theme.textPrimary
                font.pixelSize: Theme.fontSizeDisplay
                font.weight: Font.Black
                horizontalAlignment: Text.AlignHCenter
            }

            // 主题切换和筛选折叠按钮
            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 8

                // 筛选折叠按钮（移动端）
                Button {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    visible: !root.isFilterExpanded

                    background: Rectangle {
                        color: "transparent"
                        radius: 8
                    }

                    contentItem: Text {
                        text: "☰"
                        font.pixelSize: Theme.fontSizeLarge
                        color: isDark ? "#D1D5DB" : "#6B7280"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        drawer.open()
                    }
                }

                // 主题切换按钮
                Button {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    background: Rectangle {
                        color: "transparent"
                        radius: 8
                    }

                    contentItem: Text {
                        text: isDark ? "🌙" : "☀️"
                        font.pixelSize: Theme.fontSizeLarge
                        color: isDark ? "#D1D5DB" : "#6B7280"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        Theme.isDark = !Theme.isDark
                        Material.theme = isDark ? Material.Dark : Material.Light
                    }
                }
            }
        }
    }

    // 移动端侧边抽屉
    Drawer {
        id: drawer
        edge: Qt.LeftEdge
        width: Math.min(parent.width * 0.85, 320)
        height: parent.height

        Material.background: isDark ? Qt.darker(Theme.surfaceDark, 0.5) : Theme.surfaceLight

        FilterPanel {
            anchors.fill: parent
            isDark: root.isDark

            onGenerateClicked: {
                drawer.close()
                applyFilters()
            }
        }
    }

    // 主内容区域
    RowLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 24

        // 左侧筛选面板
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: root.isFilterExpanded ? 320 : 0
            Layout.maximumWidth: root.isFilterExpanded ? 320 : 0
            clip: true

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 200 }
            }

            color: "transparent"
            visible: root.isFilterExpanded

            // 筛选面板
            FilterPanel {
                anchors.fill: parent
                anchors.rightMargin: 24
                isDark: root.isDark

                onGenerateClicked: {
                    applyFilters()
                }
            }
        }

        // 右侧题目显示区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: "transparent"

            QuestionView {
                anchors.fill: parent
                anchors.leftMargin: root.isFilterExpanded ? 0 : 24
                isDark: root.isDark
                question: questionRepository?.currentQuestion || null
                currentIndex: questionRepository?.currentIndex + 1 || 0
                totalCount: questionRepository?.totalCount || 0
                showAnswer: questionRepository?.isShowAnswer || false

                onAnswerChanged: {
                    // 处理答案选择
                }

                onShowAnswerClicked: {
                    if (questionRepository) {
                        questionRepository.setShowAnswer(!questionRepository.isShowAnswer)
                    }
                }

                onExportClicked: {
                    // 处理导出
                    if (questionRepository && questionRepository.filteredQuestions.length > 0) {
                        // TODO: 实现导出功能
                        console.log("导出试卷")
                    }
                }

                onPreviousClicked: {
                    // 上一题
                    if (questionRepository && questionRepository.currentIndex > 0) {
                        questionRepository.previousQuestion()
                    }
                }
            }
        }
    }

    // 筛选应用函数
    function applyFilters() {
        if (!questionRepository) {
            console.warn("QuestionRepository is not set")
            return
        }

        console.log("应用筛选:", {
            course: filterCourse,
            version: filterVersion,
            grade: filterGrade,
            chapter: filterChapter,
            paperType: filterPaperType,
            questionType: filterQuestionType,
            difficulty: filterDifficulty
        })

        // 创建筛选条件
        var criteria = {
            courses: filterCourse !== "不限" ? [filterCourse] : [],
            versions: filterVersion !== "不限" ? [filterVersion] : [],
            grades: filterGrade !== "不限" ? [filterGrade] : [],
            chapters: filterChapter !== "不限" ? [filterChapter] : [],
            paperTypes: filterPaperType !== "不限" ? [filterPaperType] : [],
            questionTypes: filterQuestionType !== "不限" ? [filterQuestionType] : [],
            difficulties: filterDifficulty !== "不限" ? [filterDifficulty] : []
        }

        // 应用筛选并设置第一题
        var filtered = questionRepository.getFilteredQuestions(criteria)
        if (filtered.length > 0) {
            questionRepository.setCurrentIndex(0)
        }
    }

    // 响应式处理
    Component.onCompleted: {
        // 监听屏幕宽度变化
        root.isFilterExpanded = root.width > 800
    }

    onWidthChanged: {
        root.isFilterExpanded = root.width > 800
    }
}
