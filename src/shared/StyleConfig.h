#ifndef STYLECONFIG_H
#define STYLECONFIG_H

#include <QString>
#include <QColor>

namespace StyleConfig {
    inline const QString PATRIOTIC_RED = "#E53935";
    inline const QString PATRIOTIC_RED_DARK = "#B71C1C";
    inline const QString PATRIOTIC_RED_LIGHT = "#FFEBEE";
    inline const QString PATRIOTIC_RED_TINT = "#FFF4F2";
    inline const QString GOLD_ACCENT = "#D4A017";
    
    inline const QString SUCCESS_GREEN = "#388E3C";
    inline const QString INFO_BLUE = "#1976D2";
    inline const QString WARNING_ORANGE = "#F57C00";

    // 现代 UI 扩展色 (Lao Wang's Professional Palette)
    inline const QString PRIMARY_INDIGO = "#6366F1";
    inline const QString ACCENT_EMERALD = "#10B981";
    inline const QString RANK_GOLD = "#F59E0B";
    inline const QString RANK_SILVER = "#94A3B8";
    inline const QString RANK_BRONZE = "#B45309";
    inline const QString DARK_NAVY = "#0F172A";
    inline const QString SLATE_TEXT = "#475569";

    inline const QString TEXT_PRIMARY = "#1A1A1A";
    inline const QString TEXT_SECONDARY = "#6B7280";
    inline const QString TEXT_LIGHT = "#9CA3AF";
    inline const QString TEXT_MUTED = "#64748B";      // 替代 #94A3B8，保证白底 4.5:1 对比度
    inline const QString TEXT_PLACEHOLDER = "#64748B"; // placeholder 专用，保证可读性
    inline const QString TEXT_DISABLED = "#9E9E9E";

    inline const QString BG_APP = "#F5F7FA";
    inline const QString BG_CARD = "#FFFFFF";
    inline const QString BORDER_LIGHT = "#E5E7EB";
    inline const QString SEPARATOR = "#F1F5F9";

    inline const QString SIDEBAR_GRADIENT =
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E53935, stop:0.5 #D32F2F, stop:1 #B71C1C)";
    inline const QString SIDEBAR_GRADIENT_SOFT =
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #EF5350, stop:0.6 #E53935, stop:1 #C62828)";

    // 字号规范
    inline constexpr int FONT_SIZE_XS = 14;  // 最小字号，无障碍基线
    inline constexpr int FONT_SIZE_SM = 14;
    inline constexpr int FONT_SIZE_MD = 15;
    inline constexpr int FONT_SIZE_LG = 16;
    inline constexpr int FONT_SIZE_XL = 18;
    
    inline constexpr int RADIUS_XL = 24;
    inline constexpr int RADIUS_L = 16;
    inline constexpr int RADIUS_M = 12;
    inline constexpr int RADIUS_S = 8;
    
    inline const QString CARD_BASE_STYLE = 
        "background-color: #FFFFFF;"
        "border: 1px solid #E5E7EB;"
        "border-radius: 16px;";
        
    inline const QString CARD_HOVER_STYLE = 
        "background-color: #FAFAFA;"
        "border-color: #D1D5DB;";

    inline const QString ICON_BG_STYLE = 
        "border-radius: 12px;"
        "padding: 8px;";
}

#endif
