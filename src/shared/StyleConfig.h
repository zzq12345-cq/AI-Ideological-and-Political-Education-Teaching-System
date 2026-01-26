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
    
    inline const QString TEXT_PRIMARY = "#1A1A1A";
    inline const QString TEXT_SECONDARY = "#6B7280";
    inline const QString TEXT_LIGHT = "#9CA3AF";
    
    inline const QString BG_APP = "#F5F7FA";
    inline const QString BG_CARD = "#FFFFFF";
    inline const QString BORDER_LIGHT = "#E5E7EB";
    inline const QString SEPARATOR = "#F1F5F9";
    
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
