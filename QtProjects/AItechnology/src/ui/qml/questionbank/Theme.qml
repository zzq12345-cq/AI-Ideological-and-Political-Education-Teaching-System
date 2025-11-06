pragma Singleton

import QtQuick 2.15

QtObject {
    // 主色调 - 爱国红
    readonly property color primary: "#D9001B"
    readonly property color primaryLight: "#FCE4E7"
    readonly property color primaryDark: "#B0001A"

    // 次要色 - 深灰
    readonly property color secondary: "#333333"
    readonly property color secondaryLight: "#F5F5F5"

    // 强调色 - 蓝色
    readonly property color accent: "#4A90E2"
    readonly property color accentLight: "#E8F4FD"
    readonly property color accentDark: "#357ABD"

    // 背景色
    readonly property color backgroundLight: "#F7F8FA"
    readonly property color backgroundDark: "#101622"
    readonly property color surfaceLight: "#FFFFFF"
    readonly property color surfaceDark: "#1E293B"

    // 文字色
    readonly property color textPrimary: "#1F2937"
    readonly property color textSecondary: "#6B7280"
    readonly property color textOnPrimary: "#FFFFFF"
    readonly property color textOnDark: "#E2E8F0"

    // 边框色
    readonly property color borderLight: "#E5E7EB"
    readonly property color borderDark: "#334155"

    // 状态色
    readonly property color success: "#10B981"
    readonly property color successLight: "#D1FAE5"
    readonly property color warning: "#F59E0B"
    readonly property color warningLight: "#FEF3C7"
    readonly property color error: "#EF4444"
    readonly property color errorLight: "#FEE2E2"

    // 难度色
    readonly property color difficultyEasy: "#10B981"      // 简单 - 绿色
    readonly property color difficultyMedium: "#F59E0B"    // 中等 - 橙色
    readonly property color difficultyHard: "#EF4444"      // 困难 - 红色

    // 圆角半径
    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 8
    readonly property int radiusLarge: 12
    readonly property int radiusExtraLarge: 16

    // 间距
    readonly property int spacingMicro: 4
    readonly property int spacingSmall: 8
    readonly property int spacingMedium: 16
    readonly property int spacingLarge: 24
    readonly property int spacingExtraLarge: 32

    // 字体大小
    readonly property int fontSizeMicro: 10
    readonly property int fontSizeSmall: 12
    readonly property int fontSizeMedium: 14
    readonly property int fontSizeLarge: 16
    readonly property int fontSizeExtraLarge: 20
    readonly property int fontSizeTitle: 24
    readonly property int fontSizeDisplay: 32

    // 阴影
    readonly property int shadowElevationLow: 2
    readonly property int shadowElevationMedium: 4
    readonly property int shadowElevationHigh: 8

    // 当前主题
    property bool isDark: false

    function getColor(colorName, isDarkMode) {
        if (isDarkMode) {
            switch(colorName) {
                case "background": return backgroundDark
                case "surface": return surfaceDark
                case "text": return textOnDark
                case "border": return borderDark
                default: return surfaceDark
            }
        } else {
            switch(colorName) {
                case "background": return backgroundLight
                case "surface": return surfaceLight
                case "text": return textPrimary
                case "border": return borderLight
                default: return surfaceLight
            }
        }
    }

    function getDifficultyColor(difficulty) {
        switch(difficulty) {
            case "easy": return difficultyEasy
            case "medium": return difficultyMedium
            case "hard": return difficultyHard
            default: return accent
        }
    }
}
