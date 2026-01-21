import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: optionItem

    property string optionText: ""
    property string optionLabel: ""  // "A", "B", "C", "D"
    property bool selected: false
    property bool isCorrect: false
    property bool showAnswer: false
    property bool isDark: Theme.isDark
    property string questionType: "single"  // single, multi, truefalse

    signal clicked()

    width: parent.width
    height: 56

    radius: Theme.radiusMedium

    color: {
        if (showAnswer && isCorrect) {
            return isDark ? Qt.darker(Theme.success, 0.7) : Theme.successLight
        }
        return isDark ? Qt.darker(Theme.surfaceDark, 0.2) : Theme.surfaceLight
    }

    border.width: 2
    border.color: {
        if (showAnswer && isCorrect) {
            return isDark ? Theme.success : "#059669"
        }
        if (selected) {
            return Theme.accent
        }
        return isDark ? Theme.borderDark : Theme.borderLight
    }

    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    Behavior on border.color {
        ColorAnimation { duration: 200 }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: optionItem.clicked()
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 16

        // 选项标识
        Rectangle {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignVCenter

            radius: width / 2

            color: {
                if (showAnswer && isCorrect) {
                    return Theme.success
                }
                return selected ? Theme.accent : (isDark ? "#374151" : "#F3F4F6")
            }

            Text {
                anchors.centerIn: parent
                text: optionLabel
                color: {
                    if (showAnswer && isCorrect) {
                        return "#FFFFFF"
                    }
                    return selected ? "#FFFFFF" : (isDark ? "#D1D5DB" : "#6B7280")
                }
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.Medium
            }
        }

        // 选项文本
        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            text: optionText
            color: {
                if (showAnswer && isCorrect) {
                    return isDark ? "#6EE7B7" : "#059669"
                }
                return isDark ? Theme.textOnDark : Theme.textPrimary
            }
            font.pixelSize: Theme.fontSizeMedium
            font.weight: selected ? Font.Medium : Font.Normal
            wrapMode: Text.WordWrap
            lineHeight: 1.4
        }

        // 正确答案指示
        Text {
            visible: showAnswer && isCorrect
            Layout.alignment: Qt.AlignVCenter

            text: "[v]"
            color: Theme.success
            font.pixelSize: Theme.fontSizeLarge
            font.weight: Font.Bold
        }
    }

    // 悬停效果
    states: [
        State {
            name: "hovered"
            when: mouseArea.containsMouse
            PropertyChanges {
                target: optionItem
                color: showAnswer && isCorrect ? color : (isDark ? Qt.lighter(Theme.surfaceDark, 1.1) : "#F9FAFB")
            }
        }
    ]

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }
}
