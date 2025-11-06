import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: badge

    property string text: ""
    property string type: "default"  // default, accent, success, warning, error
    property bool isDark: Theme.isDark
    property int fontSize: Theme.fontSizeSmall

    width: label.implicitWidth + paddingLeft + paddingRight
    height: 28

    readonly property int paddingLeft: 12
    readonly property int paddingRight: 12

    radius: height / 2

    color: {
        switch(type) {
            case "accent":
                return isDark ? Qt.darker(Theme.accent, 1.5) : Theme.accentLight
            case "success":
                return isDark ? Qt.darker(Theme.success, 1.5) : Theme.successLight
            case "warning":
                return isDark ? Qt.darker(Theme.warning, 1.5) : Theme.warningLight
            case "error":
                return isDark ? Qt.darker(Theme.error, 1.5) : Theme.errorLight
            case "easy":
                return isDark ? Qt.darker(Theme.difficultyEasy, 1.5) : Qt.lighter(Theme.difficultyEasy, 1.5)
            case "medium":
                return isDark ? Qt.darker(Theme.difficultyMedium, 1.5) : Qt.lighter(Theme.difficultyMedium, 1.5)
            case "hard":
                return isDark ? Qt.darker(Theme.difficultyHard, 1.5) : Qt.lighter(Theme.difficultyHard, 1.5)
            default:
                return isDark ? "#374151" : "#F3F4F6"
        }
    }

    Text {
        id: label
        anchors.centerIn: parent
        text: badge.text
        color: {
            switch(type) {
                case "accent":
                    return isDark ? "#7DD3FC" : Theme.accent
                case "success":
                    return isDark ? "#6EE7B7" : "#059669"
                case "warning":
                    return isDark ? "#FBBF24" : "#D97706"
                case "error":
                    return isDark ? "#F87171" : "#DC2626"
                case "easy":
                    return isDark ? "#34D399" : Theme.difficultyEasy
                case "medium":
                    return isDark ? "#FBBF24" : Theme.difficultyMedium
                case "hard":
                    return isDark ? "#F87171" : Theme.difficultyHard
                default:
                    return isDark ? "#D1D5DB" : "#6B7280"
            }
        }
        font.pixelSize: badge.fontSize
        font.weight: Font.Medium
    }
}
