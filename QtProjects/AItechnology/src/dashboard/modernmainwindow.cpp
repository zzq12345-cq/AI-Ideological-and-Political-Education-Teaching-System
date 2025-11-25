#include "modernmainwindow.h"
#include "../auth/login/simpleloginwindow.h"
#include "../ui/aipreparationwidget.h"
#include "../questionbank/QuestionRepository.h"
#include "../questionbank/questionbankwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QRegularExpression>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QLineEdit>
#include <QProgressBar>
#include <QDateTime>
#include <QTimer>
#include <QComboBox>
#include <QShortcut>
#include <QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QLegend>
#include <QBarLegendMarker>
#include <QPieLegendMarker>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QStyle>
#include <QIcon>
#include <QSize>
#include <QGraphicsDropShadowEffect>
#include <QtMath>
#include <QToolTip>
#include <QCursor>
#include <QSharedPointer>
#include <QDialog>
#include <QEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QVariantAnimation>
#include <QPointer>
#include <functional>

// 思政课堂色彩体系
const QString PATRIOTIC_RED = "#e53935";          // 主思政红（温暖庄重）
const QString PATRIOTIC_RED_LIGHT = "#ffebee";    // 亮思政红（柔和背景）
const QString PATRIOTIC_RED_TINT = "#ffd6d0";     // 柔和高光
const QString PATRIOTIC_RED_ACCENT = "#ff6f60";   // 渐变强调
const QString PATRIOTIC_RED_GLOW = "#ffc7bf";     // 细腻晕染层
const QString PATRIOTIC_RED_DARK = "#c62828";     // 深思政红（重点强调）
const QString PATRIOTIC_RED_SOFT_LAYER = "#fff4f2"; // 轻盈底色
const QString PATRIOTIC_RED_DEEP_TONE = "#b71c1c";  // 深沉描边
const QString PATRIOTIC_RED_GRADIENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #fbe1dd, stop:0.45 #fff3f2, stop:1 #ffffff)";
const QString PATRIOTIC_RED_DEEP_GRADIENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ff7d6d, stop:0.55 #e53935, stop:1 #b71c1c)";
const QString PATRIOTIC_RED_RIBBON = "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #fff1ed, stop:0.45 #ffe4df, stop:1 #ffd6d0)";

const QString WISDOM_BLUE = "#1976d2";            // 智慧蓝（理性思考）
const QString GROWTH_GREEN = "#388e3c";           // 成长绿（积极向上）
const QString CULTURE_GOLD = "#f57c00";           // 文化金（传统文化）
const QString ACADEMIC_PURPLE = "#7b1fa2";        // 学术紫（深度思考）

// 背景与结构色
const QString BACKGROUND_LIGHT = "#fafafa";       // 主背景
const QString WINDOW_BACKGROUND_GRADIENT = "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fff8f6, stop:0.6 #faf7f7, stop:1 #f5f5f5)";
const QString CARD_WHITE = "#ffffff";             // 卡片背景
const QString LIGHT_GRAY = "#f5f5f5";             // 淡灰背景
const QString SEPARATOR = "#e8eaf6";              // 分隔线
const QString ULTRA_LIGHT_GRAY = "#f7f8fa";

// 现代卡片样式 - 简洁版
const QString CARD_GRADIENT = "#ffffff";
const QString CARD_HOVER_GRADIENT = "#fafafa";
const QString CARD_PRESSED_GRADIENT = "#f5f5f5";
const QString CARD_BORDER_COLOR = "#f0f0f0";
const QString CARD_BORDER_HIGHLIGHT = "#e0e0e0";
const QString CARD_BORDER_ACTIVE = "#d0d0d0";
const int CARD_CORNER_RADIUS = 16;
const int CARD_PADDING_PX = 24;

// 现代化按钮样式系统 - 简化版，移除不支持的CSS动画
const QString BUTTON_PRIMARY_STYLE =
    R"(QPushButton {
        background: %1;
        color: white;
        border: none;
        border-radius: 8px;
        padding: 12px 24px;
        font-size: 14px;
        font-weight: 600;
    }
    QPushButton:hover {
        background: %2;
    }
    QPushButton:pressed {
        background: %3;
    })";

const QString BUTTON_SECONDARY_STYLE =
    R"(QPushButton {
        background: white;
        color: %1;
        border: 2px solid %1;
        border-radius: 8px;
        padding: 10px 22px;
        font-size: 14px;
        font-weight: 600;
    }
    QPushButton:hover {
        background: %1;
        color: white;
    }
    QPushButton:pressed {
        background: %2;
    })";

// 按钮渐变
const QString PRIMARY_BUTTON_GRADIENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ff7466, stop:0.55 #e53935, stop:1 #c62828)";
const QString PRIMARY_BUTTON_HOVER_GRADIENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ff8a7e, stop:0.5 #ed4d44, stop:1 #b71c1c)";
const QString PRIMARY_BUTTON_PRESSED_GRADIENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #e53935, stop:0.7 #c62828, stop:1 #b71c1c)";
const QString SOFT_BUTTON_GRADIENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #fff7f5, stop:1 #ffe8e4)";
const QString SOFT_BUTTON_HOVER_GRADIENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffece7, stop:1 #ffd7d0)";
const QString SOFT_BUTTON_PRESSED_GRADIENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffd1c9, stop:1 #ffc2b8)";

// 文字层次
const QString PRIMARY_TEXT = "#212121";           // 主文本
const QString SECONDARY_TEXT = "#757575";         // 次文本
const QString LIGHT_TEXT = "#9e9e9e";             // 淡文本

const QString SIDEBAR_GRADIENT = "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e53935, stop:0.65 #c62828, stop:1 #1976d2)";

// 侧栏按钮样式常量
const QString SIDEBAR_BTN_NORMAL =
    R"(QPushButton { background-color: transparent; color: %1; border: none; border-left: 4px solid transparent; padding: 10px 12px 10px 20px; font-size: 14px; text-align: left; border-radius: 8px; }
       QPushButton:hover { background-color: %2; })";
const QString SIDEBAR_BTN_ACTIVE =
    R"(QPushButton { background-color: %1; color: %2; border: none; border-left: 4px solid %2; padding: 10px 12px 10px 20px; font-size: 14px; font-weight: bold; text-align: left; border-radius: 8px; }
       QPushButton:hover { background-color: rgba(239, 83, 80, 0.22); })";

struct TrendValue {
    int current = 0;
    int previous = 0;
};

// 学情分析数据结构
struct LearningMetrics {
    TrendValue participation;   // 课堂参与
    TrendValue homework;        // 作业完成
    TrendValue quiz;            // 测验成绩
    TrendValue knowledge;       // 知识掌握
    int mastery;                // 掌握 (%)
    int partial;                // 基本掌握 (%)
    int needsWork;              // 需巩固 (%)
};

QMap<QString, LearningMetrics> createSampleData() {
    QMap<QString, LearningMetrics> data;

    LearningMetrics metrics7d = {
        {92, 89}, // participation
        {88, 84}, // homework
        {79, 82}, // quiz
        {81, 78}, // knowledge
        65, 28, 7
    };

    LearningMetrics metrics30d = {
        {89, 87},
        {85, 83},
        {82, 79},
        {79, 76},
        68, 25, 7
    };

    LearningMetrics metricsSemester = {
        {90, 88},
        {86, 84},
        {85, 82},
        {83, 80},
        70, 24, 6
    };

    data["近7天"] = metrics7d;
    data["近30天"] = metrics30d;
    data["本学期"] = metricsSemester;
    return data;
}

namespace {

QString buildCardStyle(const QString &selector)
{
    return QString(
        "%1 {"
        "  background: %2;"
        "  border: 1px solid %3;"
        "  border-radius: %4px;"
        "  padding: %5px;"
        "  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);"
        "}"
        "%1[cardState=\"hover\"], %1:hover {"
        "  border-color: %6;"
        "  background: %7;"
        "  transform: translateY(-2px);"
        "  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.1);"
        "}"
        "%1[cardState=\"active\"] {"
        "  border-color: %8;"
        "  background: %9;"
        "  transform: translateY(-1px);"
        "  box-shadow: 0 6px 16px rgba(0, 0, 0, 0.12);"
        "}"
        "%1:pressed {"
        "  background: %10;"
        "  transform: translateY(0px);"
        "  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);"
        "  transition-duration: 0.1s;"
        "}"
    ).arg(selector)
     .arg(CARD_GRADIENT)
     .arg(CARD_BORDER_COLOR)
     .arg(CARD_CORNER_RADIUS)
     .arg(CARD_PADDING_PX)
     .arg(CARD_BORDER_HIGHLIGHT)
     .arg(CARD_HOVER_GRADIENT)
     .arg(CARD_BORDER_ACTIVE)
     .arg(CARD_HOVER_GRADIENT)  // 使用hover渐变作为active状态
     .arg(CARD_PRESSED_GRADIENT);
}

void applyCardShadow(QWidget *widget, qreal blurRadius = 24.0, qreal yOffset = 8.0)
{
    if (!widget) {
        return;
    }

    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(0, yOffset);
    shadow->setColor(QColor(15, 23, 42, 35));
    widget->setGraphicsEffect(shadow);
}

class ChartClickFilter : public QObject
{
public:
    explicit ChartClickFilter(std::function<void()> handler, QObject *parent = nullptr)
        : QObject(parent)
        , onClick(std::move(handler))
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (!event || event->type() != QEvent::MouseButtonRelease) {
            return QObject::eventFilter(watched, event);
        }
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent && mouseEvent->button() == Qt::LeftButton && onClick) {
            onClick();
        }
        return QObject::eventFilter(watched, event);
    }

private:
    std::function<void()> onClick;
};

class CardHoverAnimator : public QObject
{
public:
    explicit CardHoverAnimator(QPushButton *target, QObject *parent = nullptr)
        : QObject(parent)
        , button(target)
    {
        if (!button) {
            return;
        }

        button->setAttribute(Qt::WA_Hover, true);
        button->setMouseTracking(true);
        button->installEventFilter(this);
        button->setCursor(Qt::PointingHandCursor);
        button->setProperty("cardState", "base");

        shadowEffect = qobject_cast<QGraphicsDropShadowEffect *>(button->graphicsEffect());
        if (!shadowEffect) {
            shadowEffect = new QGraphicsDropShadowEffect(button);
            shadowEffect->setBlurRadius(18);
            shadowEffect->setOffset(0, 6);
            shadowEffect->setColor(QColor(15, 23, 42, 35));
            button->setGraphicsEffect(shadowEffect);
        }

        baseBlur = shadowEffect->blurRadius();
        baseYOffset = shadowEffect->yOffset();
        baseShadowColor = shadowEffect->color();

        liftAnimation = new QPropertyAnimation(button, "geometry", this);
        liftAnimation->setDuration(180);
        liftAnimation->setEasingCurve(QEasingCurve::OutCubic);

        blurAnimation = new QPropertyAnimation(shadowEffect, "blurRadius", this);
        blurAnimation->setDuration(200);
        blurAnimation->setEasingCurve(QEasingCurve::OutCubic);

        yOffsetAnimation = new QVariantAnimation(this);
        yOffsetAnimation->setDuration(200);
        yOffsetAnimation->setEasingCurve(QEasingCurve::OutCubic);
        connect(yOffsetAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            if (shadowEffect) {
                shadowEffect->setOffset(0, value.toReal());
            }
        });

        shadowColorAnimation = new QVariantAnimation(this);
        shadowColorAnimation->setDuration(200);
        shadowColorAnimation->setEasingCurve(QEasingCurve::OutCubic);
        connect(shadowColorAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            if (shadowEffect) {
                shadowEffect->setColor(value.value<QColor>());
            }
        });
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != button || !event) {
            return QObject::eventFilter(watched, event);
        }

        switch (event->type()) {
        case QEvent::Enter:
            hovered = true;
            basePos = button->pos();
            animateToState();
            updateVisualState();
            break;
        case QEvent::Leave:
            hovered = false;
            pressed = false;
            animateToState();
            updateVisualState();
            break;
        case QEvent::Move:
            if (!hovered) {
                basePos = button->pos();
            }
            break;
        case QEvent::MouseButtonPress:
            pressed = true;
            animateToState();
            updateVisualState();
            break;
        case QEvent::MouseButtonRelease:
            pressed = false;
            animateToState();
            updateVisualState();
            break;
        default:
            break;
        }

        return QObject::eventFilter(watched, event);
    }

private:
    void animateToState()
    {
        if (!button || !shadowEffect) {
            return;
        }

        QRect currentGeo = button->geometry();
        QRect targetGeo = currentGeo;
        targetGeo.moveTop(basePos.y());
        if (hovered) {
            targetGeo.moveTop(basePos.y() - hoverLift);
        }
        if (pressed) {
            targetGeo.moveTop(basePos.y() + pressDrop);
        }

        liftAnimation->stop();
        liftAnimation->setStartValue(currentGeo);
        liftAnimation->setEndValue(targetGeo);
        liftAnimation->start();

        blurAnimation->stop();
        blurAnimation->setStartValue(shadowEffect->blurRadius());
        qreal targetBlur = hovered ? baseBlur + 10 : baseBlur;
        if (pressed) {
            targetBlur = qMax(baseBlur - 2.0, targetBlur - 4.0);
        }
        blurAnimation->setEndValue(targetBlur);
        blurAnimation->start();

        yOffsetAnimation->stop();
        yOffsetAnimation->setStartValue(shadowEffect->yOffset());
        qreal yTarget = hovered ? baseYOffset - 4 : baseYOffset;
        if (pressed) {
            yTarget = baseYOffset - 1;
        }
        yOffsetAnimation->setEndValue(yTarget);
        yOffsetAnimation->start();

        shadowColorAnimation->stop();
        shadowColorAnimation->setStartValue(shadowEffect->color());
        QColor colorTarget = hovered ? QColor(229, 57, 53, 80) : baseShadowColor;
        if (pressed) {
            colorTarget = QColor(229, 57, 53, 95);
        }
        shadowColorAnimation->setEndValue(colorTarget);
        shadowColorAnimation->start();
    }

    void updateVisualState()
    {
        const QString state = pressed ? "pressed" : (hovered ? "hover" : "base");
        button->setProperty("cardState", state);
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }

    QPointer<QPushButton> button;
    QGraphicsDropShadowEffect *shadowEffect = nullptr;
    QPropertyAnimation *liftAnimation = nullptr;
    QPropertyAnimation *blurAnimation = nullptr;
    QVariantAnimation *yOffsetAnimation = nullptr;
    QVariantAnimation *shadowColorAnimation = nullptr;
    QPoint basePos;
    qreal baseBlur = 18.0;
    qreal baseYOffset = 6.0;
    QColor baseShadowColor = QColor(15, 23, 42, 35);
    const int hoverLift = 8;
    const int pressDrop = 2;
    bool hovered = false;
    bool pressed = false;
};

// 简单的卡片悬停事件过滤器 - 仅用于设置cardState属性
class SimpleCardHoverFilter : public QObject
{
public:
    explicit SimpleCardHoverFilter(QPushButton *target, QObject *parent = nullptr)
        : QObject(parent)
        , button(target)
    {
        if (button) {
            button->setProperty("cardState", "base");
        }
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != button || !event) {
            return QObject::eventFilter(watched, event);
        }

        switch (event->type()) {
        case QEvent::Enter:
            button->setProperty("cardState", "hover");
            button->style()->unpolish(button);
            button->style()->polish(button);
            button->update();
            break;
        case QEvent::Leave:
            button->setProperty("cardState", "base");
            button->style()->unpolish(button);
            button->style()->polish(button);
            button->update();
            break;
        case QEvent::MouseButtonPress:
            button->setProperty("cardState", "pressed");
            button->style()->unpolish(button);
            button->style()->polish(button);
            button->update();
            break;
        case QEvent::MouseButtonRelease:
            button->setProperty("cardState", "hover");
            button->style()->unpolish(button);
            button->style()->polish(button);
            button->update();
            break;
        default:
            break;
        }

        return QObject::eventFilter(watched, event);
    }

private:
    QPointer<QPushButton> button;
};

class ButtonHoverAnimator : public QObject
{
public:
    explicit ButtonHoverAnimator(QPushButton *target, QObject *parent = nullptr, int delta = 2)
        : QObject(parent)
        , button(target)
        , scaleDelta(delta)
    {
        if (!button) {
            return;
        }

        button->setAttribute(Qt::WA_Hover, true);
        button->setMouseTracking(true);
        button->installEventFilter(this);
        button->setProperty("actionState", "base");

        geometryAnimation = new QPropertyAnimation(button, "geometry", this);
        geometryAnimation->setDuration(160);
        geometryAnimation->setEasingCurve(QEasingCurve::OutCubic);

        shadowEffect = qobject_cast<QGraphicsDropShadowEffect *>(button->graphicsEffect());
        if (!shadowEffect) {
            shadowEffect = new QGraphicsDropShadowEffect(button);
            shadowEffect->setBlurRadius(16);
            shadowEffect->setOffset(0, 4);
            shadowEffect->setColor(QColor(229, 57, 53, 40));
            button->setGraphicsEffect(shadowEffect);
        }

        baseShadowBlur = shadowEffect->blurRadius();
        baseShadowOffset = shadowEffect->yOffset();

        shadowBlurAnimation = new QPropertyAnimation(shadowEffect, "blurRadius", this);
        shadowBlurAnimation->setDuration(160);
        shadowBlurAnimation->setEasingCurve(QEasingCurve::OutCubic);

        shadowYOffsetAnimation = new QVariantAnimation(this);
        shadowYOffsetAnimation->setDuration(160);
        shadowYOffsetAnimation->setEasingCurve(QEasingCurve::OutCubic);
        connect(shadowYOffsetAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            if (shadowEffect) {
                shadowEffect->setOffset(0, value.toReal());
            }
        });
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != button || !event) {
            return QObject::eventFilter(watched, event);
        }

        switch (event->type()) {
        case QEvent::Enter:
            hovered = true;
            syncBaseGeometry();
            animateState();
            updateActionState();
            break;
        case QEvent::Leave:
            hovered = false;
            pressed = false;
            animateState();
            updateActionState();
            break;
        case QEvent::Move:
        case QEvent::Resize:
            if (!hovered) {
                baseGeometry = button->geometry();
            }
            break;
        case QEvent::MouseButtonPress:
            pressed = true;
            animateState();
            updateActionState();
            break;
        case QEvent::MouseButtonRelease:
            pressed = false;
            animateState();
            updateActionState();
            break;
        default:
            break;
        }

        return QObject::eventFilter(watched, event);
    }

private:
    void syncBaseGeometry()
    {
        if (!baseGeometry.isValid()) {
            baseGeometry = button->geometry();
        }
    }

    void animateState()
    {
        if (!button) {
            return;
        }

        QRect current = button->geometry();
        QRect target = baseGeometry;
        if (hovered) {
            target = target.adjusted(-scaleDelta, -scaleDelta, scaleDelta, scaleDelta);
        }
        if (pressed) {
            target = target.adjusted(pressInset, pressInset, -pressInset, -pressInset);
        }

        geometryAnimation->stop();
        geometryAnimation->setStartValue(current);
        geometryAnimation->setEndValue(target);
        geometryAnimation->start();

        if (!shadowEffect) {
            return;
        }

        shadowBlurAnimation->stop();
        shadowBlurAnimation->setStartValue(shadowEffect->blurRadius());
        qreal blurTarget = hovered ? baseShadowBlur + 4 : baseShadowBlur;
        if (pressed) {
            blurTarget = qMax(baseShadowBlur - 1.0, blurTarget - 2.0);
        }
        shadowBlurAnimation->setEndValue(blurTarget);
        shadowBlurAnimation->start();

        shadowYOffsetAnimation->stop();
        shadowYOffsetAnimation->setStartValue(shadowEffect->yOffset());
        qreal yTarget = hovered ? baseShadowOffset - 1.5 : baseShadowOffset;
        if (pressed) {
            yTarget = baseShadowOffset - 0.5;
        }
        shadowYOffsetAnimation->setEndValue(yTarget);
        shadowYOffsetAnimation->start();
    }

    void updateActionState()
    {
        QString state = "base";
        if (pressed) {
            state = "pressed";
        } else if (hovered) {
            state = "hover";
        }
        button->setProperty("actionState", state);
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }

    QPointer<QPushButton> button;
    QPropertyAnimation *geometryAnimation = nullptr;
    QGraphicsDropShadowEffect *shadowEffect = nullptr;
    QPropertyAnimation *shadowBlurAnimation = nullptr;
    QVariantAnimation *shadowYOffsetAnimation = nullptr;
    QRect baseGeometry;
    qreal baseShadowBlur = 16.0;
    qreal baseShadowOffset = 4.0;
    const int scaleDelta;
    bool hovered = false;
    bool pressed = false;
    const int pressInset = 1;
};

class FrameHoverAnimator : public QObject
{
public:
    explicit FrameHoverAnimator(QWidget *target, QObject *parent = nullptr, int lift = 6)
        : QObject(parent)
        , card(target)
        , hoverLift(lift)
    {
        if (!card) {
            return;
        }

        card->setAttribute(Qt::WA_Hover, true);
        card->setMouseTracking(true);
        card->installEventFilter(this);
        card->setProperty("cardState", "base");

        shadowEffect = qobject_cast<QGraphicsDropShadowEffect *>(card->graphicsEffect());
        if (!shadowEffect) {
            shadowEffect = new QGraphicsDropShadowEffect(card);
            shadowEffect->setBlurRadius(20);
            shadowEffect->setOffset(0, 8);
            shadowEffect->setColor(QColor(15, 23, 42, 30));
            card->setGraphicsEffect(shadowEffect);
        }

        baseBlur = shadowEffect->blurRadius();
        baseYOffset = shadowEffect->yOffset();
        baseShadowColor = shadowEffect->color();

        liftAnimation = new QPropertyAnimation(card, "pos", this);
        liftAnimation->setDuration(200);
        liftAnimation->setEasingCurve(QEasingCurve::OutCubic);

        blurAnimation = new QPropertyAnimation(shadowEffect, "blurRadius", this);
        blurAnimation->setDuration(220);
        blurAnimation->setEasingCurve(QEasingCurve::OutCubic);

        yOffsetAnimation = new QVariantAnimation(this);
        yOffsetAnimation->setDuration(220);
        yOffsetAnimation->setEasingCurve(QEasingCurve::OutCubic);
        connect(yOffsetAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            if (shadowEffect) {
                shadowEffect->setOffset(0, value.toReal());
            }
        });

        shadowColorAnimation = new QVariantAnimation(this);
        shadowColorAnimation->setDuration(220);
        shadowColorAnimation->setEasingCurve(QEasingCurve::OutCubic);
        connect(shadowColorAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            if (shadowEffect) {
                shadowEffect->setColor(value.value<QColor>());
            }
        });
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != card || !event) {
            return QObject::eventFilter(watched, event);
        }

        switch (event->type()) {
        case QEvent::Enter:
            hovered = true;
            basePos = card->pos();
            animateState();
            updateVisualState();
            break;
        case QEvent::Leave:
            hovered = false;
            animateState();
            updateVisualState();
            break;
        case QEvent::Move:
            if (!hovered) {
                basePos = card->pos();
            }
            break;
        default:
            break;
        }

        return QObject::eventFilter(watched, event);
    }

private:
    void animateState()
    {
        if (!card || !shadowEffect) {
            return;
        }

        QPoint currentPos = card->pos();
        QPoint targetPos = basePos;
        if (hovered) {
            targetPos -= QPoint(0, hoverLift);
        }

        liftAnimation->stop();
        liftAnimation->setStartValue(currentPos);
        liftAnimation->setEndValue(targetPos);
        liftAnimation->start();

        blurAnimation->stop();
        blurAnimation->setStartValue(shadowEffect->blurRadius());
        blurAnimation->setEndValue(hovered ? baseBlur + 6 : baseBlur);
        blurAnimation->start();

        yOffsetAnimation->stop();
        yOffsetAnimation->setStartValue(shadowEffect->yOffset());
        yOffsetAnimation->setEndValue(hovered ? baseYOffset - 4 : baseYOffset);
        yOffsetAnimation->start();

        shadowColorAnimation->stop();
        shadowColorAnimation->setStartValue(shadowEffect->color());
        QColor colorTarget = hovered ? QColor(229, 57, 53, 65) : baseShadowColor;
        shadowColorAnimation->setEndValue(colorTarget);
        shadowColorAnimation->start();
    }

    void updateVisualState()
    {
        const QString state = hovered ? "hover" : "base";
        card->setProperty("cardState", state);
        card->style()->unpolish(card);
        card->style()->polish(card);
        card->update();
    }

    QPointer<QWidget> card;
    QGraphicsDropShadowEffect *shadowEffect = nullptr;
    QPropertyAnimation *liftAnimation = nullptr;
    QPropertyAnimation *blurAnimation = nullptr;
    QVariantAnimation *yOffsetAnimation = nullptr;
    QVariantAnimation *shadowColorAnimation = nullptr;
    QPoint basePos;
    qreal baseBlur = 20.0;
    qreal baseYOffset = 8.0;
    QColor baseShadowColor = QColor(15, 23, 42, 30);
    const int hoverLift;
    bool hovered = false;
};
}

ModernMainWindow::ModernMainWindow(const QString &userRole, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , currentUserRole(userRole)
    , currentUsername(username)
{
    qDebug() << "=== ModernMainWindow 构造函数开始 ===";
    qDebug() << "用户角色:" << userRole << "用户名:" << username;

    setWindowTitle("思政智慧课堂 - 教师中心");
    setMinimumSize(1400, 900);
    resize(1600, 1000);

    // 初始化试题库数据仓库
    questionRepository = new QuestionRepository(this);
    questionRepository->loadQuestions("data/questions.json");

    initUI();
    setupMenuBar();
    setupStatusBar();
    setupCentralWidget();
    setupStyles();
    applyPatrioticRedTheme();

    // 创建默认页面
    createDashboard();
    contentStack->setCurrentWidget(dashboardWidget);

    qDebug() << "=== ModernMainWindow 构造函数完成 ===";
}

ModernMainWindow::~ModernMainWindow()
{
}

void ModernMainWindow::initUI()
{
    // 设置窗口基本属性
    setStyleSheet("QMainWindow { background-color: " + BACKGROUND_LIGHT + "; }");
}

void ModernMainWindow::setupMenuBar()
{
    QMenuBar* mainMenuBar = this->menuBar();
    mainMenuBar->setStyleSheet("QMenuBar { background-color: " + CARD_WHITE + "; border-bottom: 1px solid " + SEPARATOR + "; }");

    // 文件菜单
    QMenu *fileMenu = mainMenuBar->addMenu("文件(&F)");
    QAction *newAction = fileMenu->addAction("新建(&N)");
    newAction->setShortcut(QKeySequence::New);

    QAction *openAction = fileMenu->addAction("打开(&O)");
    openAction->setShortcut(QKeySequence::Open);

    fileMenu->addSeparator();
    logoutAction = fileMenu->addAction("注销(&L)");
    logoutAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(logoutAction, &QAction::triggered, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "注销",
            "确定要注销当前账户吗？",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            SimpleLoginWindow *loginWindow = new SimpleLoginWindow();
            loginWindow->show();
            this->close();
        }
    });

    // 工具菜单
    QMenu *toolsMenu = mainMenuBar->addMenu("工具(&T)");
    settingsAction = toolsMenu->addAction("设置(&S)");
    connect(settingsAction, &QAction::triggered, this, &ModernMainWindow::onSettingsClicked);

    // 帮助菜单
    QMenu *helpMenu = mainMenuBar->addMenu("帮助(&H)");
    helpAction = helpMenu->addAction("帮助文档(&H)");
    connect(helpAction, &QAction::triggered, this, &ModernMainWindow::onHelpClicked);

    helpMenu->addSeparator();
    aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, [](){ QMessageBox::about(nullptr, "关于", "思政智慧课堂 - 教师中心"); });
}

void ModernMainWindow::setupStatusBar()
{
    QStatusBar* mainStatusBar = this->statusBar();
    mainStatusBar->setStyleSheet("QStatusBar { background-color: " + CARD_WHITE + "; color: " + PRIMARY_TEXT + "; border-top: 1px solid " + SEPARATOR + "; }");
    mainStatusBar->showMessage("就绪");

    // 添加永久状态信息
    QLabel *statusLabel = new QLabel(QString("当前用户: %1 (%2)").arg(currentUsername).arg(currentUserRole));
    statusLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 12px;");
    mainStatusBar->addPermanentWidget(statusLabel);

    QLabel *timeLabel = new QLabel(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    timeLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 12px;");
    mainStatusBar->addPermanentWidget(timeLabel);

    // 定时更新时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [timeLabel]() {
        timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    });
    timer->start(1000);
}

void ModernMainWindow::setupCentralWidget()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建主内容区域
    contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 创建侧边栏 - 使用白色背景
    sidebar = new QFrame();
    sidebar->setMinimumWidth(240);  // 设置最小宽度
    sidebar->setMaximumWidth(300);  // 设置最大宽度
    sidebar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sidebar->setStyleSheet("QFrame { background: " + CARD_WHITE + "; border-right: 1px solid " + SEPARATOR + "; }");

    sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(24, 24, 24, 24); // 调整边距与用户信息对齐
    sidebarLayout->setSpacing(20);

    // 创建侧边栏顶部用户资料
    createSidebarProfile();

    // 创建导航菜单
    teacherCenterBtn = new QPushButton("教师中心");
    contentAnalysisBtn = new QPushButton("智能内容分析");
    aiPreparationBtn = new QPushButton("AI智能备课");
    resourceManagementBtn = new QPushButton("试题库");
    learningAnalysisBtn = new QPushButton("学情与教评");
  
    // 底部按钮
    settingsBtn = new QPushButton("系统设置");
    helpBtn = new QPushButton("帮助中心");

    // 确保所有按钮都可见
    teacherCenterBtn->setVisible(true);
    contentAnalysisBtn->setVisible(true);
    aiPreparationBtn->setVisible(true);
    resourceManagementBtn->setVisible(true);
    learningAnalysisBtn->setVisible(true);
    settingsBtn->setVisible(true);
    helpBtn->setVisible(true);

    applySidebarIcons();

    // 设置侧边栏按钮样式 - 使用统一样式常量
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    settingsBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    helpBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));

    // 连接信号
    connect(teacherCenterBtn, &QPushButton::clicked, this, [=]() { qDebug() << "教师中心按钮被点击"; onTeacherCenterClicked(); });
    connect(contentAnalysisBtn, &QPushButton::clicked, this, [=]() { qDebug() << "智能内容分析按钮被点击"; onContentAnalysisClicked(); });
    connect(aiPreparationBtn, &QPushButton::clicked, this, [=]() { qDebug() << "AI智能备课按钮被点击"; onAIPreparationClicked(); });
    connect(resourceManagementBtn, &QPushButton::clicked, this, [=]() { qDebug() << "试题库按钮被点击"; onResourceManagementClicked(); });
    connect(learningAnalysisBtn, &QPushButton::clicked, this, [=]() { qDebug() << "学情与教评按钮被点击"; onLearningAnalysisClicked(); });
    connect(settingsBtn, &QPushButton::clicked, this, [=]() { qDebug() << "系统设置按钮被点击"; onSettingsClicked(); });
    connect(helpBtn, &QPushButton::clicked, this, [=]() { qDebug() << "帮助中心按钮被点击"; onHelpClicked(); });

    // 调试按钮状态
    qDebug() << "=== 按钮状态检查 ===";
    qDebug() << "试题库按钮 - 启用:" << resourceManagementBtn->isEnabled() << "可见:" << resourceManagementBtn->isVisible() << "文本:" << resourceManagementBtn->text();
    qDebug() << "AI智能备课按钮 - 启用:" << aiPreparationBtn->isEnabled() << "可见:" << aiPreparationBtn->isVisible() << "文本:" << aiPreparationBtn->text();
    qDebug() << "按钮尺寸 - 试题库:" << resourceManagementBtn->size() << "AI智能备课:" << aiPreparationBtn->size();
    qDebug() << "按钮位置 - 试题库:" << resourceManagementBtn->pos() << "AI智能备课:" << aiPreparationBtn->pos();
    qDebug() << "按钮父控件 - 试题库:" << resourceManagementBtn->parentWidget() << "AI智能备课:" << aiPreparationBtn->parentWidget();
    qDebug() << "侧边栏控件:" << sidebar << "侧边栏可见性:" << sidebar->isVisible();

    // 添加按钮到侧边栏
    sidebarLayout->addWidget(teacherCenterBtn);
    sidebarLayout->addWidget(contentAnalysisBtn);
    sidebarLayout->addWidget(aiPreparationBtn);
    sidebarLayout->addWidget(resourceManagementBtn);
    sidebarLayout->addWidget(learningAnalysisBtn);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(settingsBtn);
    sidebarLayout->addWidget(helpBtn);

    // 创建内容堆栈窗口
    contentStack = new QStackedWidget();
    contentStack->setStyleSheet("background-color: " + BACKGROUND_LIGHT + ";");

    dashboardWidget = new QWidget();
    contentStack->addWidget(dashboardWidget);

    // 创建 AI 智能备课页面
    aiPreparationWidget = new AIPreparationWidget();
    contentStack->addWidget(aiPreparationWidget);

    // 创建试题库页面
    questionBankWindow = new QuestionBankWindow(this);
    contentStack->addWidget(questionBankWindow);

    // 添加到主布局
    contentLayout->addWidget(sidebar);
    contentLayout->addWidget(contentStack);

    mainLayout->addLayout(contentLayout);
}

void ModernMainWindow::applySidebarIcons()
{
    auto setIcon = [this](QPushButton *button, const QString &themeName, QStyle::StandardPixmap fallback) {
        if (!button) {
            return;
        }
        button->setIcon(loadSidebarIcon(themeName, fallback));
        button->setIconSize(QSize(20, 20));
    };

    setIcon(teacherCenterBtn, "user-identity", QStyle::SP_ComputerIcon);
    setIcon(contentAnalysisBtn, "view-statistics", QStyle::SP_FileDialogContentsView);
    setIcon(aiPreparationBtn, "system-run", QStyle::SP_MediaPlay);
    setIcon(resourceManagementBtn, "folder", QStyle::SP_DirIcon);
    setIcon(learningAnalysisBtn, "view-list-details", QStyle::SP_FileDialogDetailedView);
    setIcon(settingsBtn, "settings-configure", QStyle::SP_FileDialogDetailedView);
    setIcon(helpBtn, "help-browser", QStyle::SP_MessageBoxQuestion);
}

QIcon ModernMainWindow::loadSidebarIcon(const QString &themeName, QStyle::StandardPixmap fallback) const
{
    QIcon icon = QIcon::fromTheme(themeName);
    if (icon.isNull()) {
        icon = style()->standardIcon(fallback);
    }
    return icon;
}

void ModernMainWindow::createSidebarProfile()
{
    // 创建扁平化的用户资料区域 - 与导航背景融合
    QFrame *profileWidget = new QFrame();
    profileWidget->setObjectName("sidebarProfile");
    profileWidget->setStyleSheet(
        "QFrame#sidebarProfile {"
        "  background: transparent;"  // 使用透明背景，与侧栏融合
        "  border: none;"  // 移除边框
        "}"
    );

    QVBoxLayout *profileLayout = new QVBoxLayout(profileWidget);
    profileLayout->setContentsMargins(24, 16, 24, 16);  // 与导航菜单对齐
    profileLayout->setSpacing(12);

    // 创建头像容器 (水平布局)
    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->setContentsMargins(0, 0, 0, 0);
    avatarLayout->setSpacing(14);

    // 头像占位符 - 扁平化设计，去掉白色边框
    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(40, 40); // 调整尺寸，更符合扁平设计
    avatarLabel->setStyleSheet(QString(
        "QLabel {"
        "  background-color: %1;"
        "  border-radius: 20px;"  // 调整为完全圆形
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  border: none;"  // 移除边框
        "}"
    ).arg(PATRIOTIC_RED));
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setText("王");

    // 用户信息
    QVBoxLayout *userInfoLayout = new QVBoxLayout();
    userInfoLayout->setContentsMargins(0, 0, 0, 0);
    userInfoLayout->setSpacing(4);

    QLabel *nameLabel = new QLabel("王老师");
    nameLabel->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 15px; font-weight: bold;"); // 调整字体大小

    QLabel *roleLabel = new QLabel("思政教研组");
    roleLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 13px;"); // 使用标准次文本颜色，适配白色背景

    userInfoLayout->addWidget(nameLabel);
    userInfoLayout->addWidget(roleLabel);

    avatarLayout->addWidget(avatarLabel);
    avatarLayout->addLayout(userInfoLayout);
    avatarLayout->addStretch();

    profileLayout->addLayout(avatarLayout);

    // 在线状态指示器 - 扁平化设计
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setSpacing(6); // 减小间距

    QFrame *statusDot = new QFrame();
    statusDot->setFixedSize(8, 8); // 缩小圆点尺寸
    statusDot->setStyleSheet("QFrame { background-color: " + GROWTH_GREEN + "; border-radius: 4px; }");

    QLabel *statusLabel = new QLabel("在线");
    statusLabel->setStyleSheet("color: " + GROWTH_GREEN + "; font-size: 12px; font-weight: 600;"); // 恢复绿色文本

    QLabel *statusHint = new QLabel("实时连接");
    statusHint->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 12px;"); // 使用标准次文本颜色

    statusLayout->addWidget(statusDot);
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(statusHint);
    statusLayout->addStretch();

    profileLayout->addLayout(statusLayout);
    sidebarLayout->addWidget(profileWidget);
}

void ModernMainWindow::createHeaderWidget()
{
    headerWidget = new QFrame();
    headerWidget->setFixedHeight(64); // py-3 = 12px * 2 + line-height ≈ 64px
    headerWidget->setStyleSheet("QFrame { background: #ffffff; border: none; border-bottom: 1px solid rgba(15, 23, 42, 0.08); }");

    auto *headerShadow = new QGraphicsDropShadowEffect(headerWidget);
    headerShadow->setBlurRadius(28);
    headerShadow->setOffset(0, 4);
    headerShadow->setColor(QColor(15, 23, 42, 20));
    headerWidget->setGraphicsEffect(headerShadow);

    headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(32, 14, 32, 14); // 扩展左右留白
    headerLayout->setSpacing(20);

    // 左侧标题
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(14);

    QLabel *starIcon = new QLabel("⭐");
    starIcon->setStyleSheet("color: " + CULTURE_GOLD + "; font-size: 24px;");

    titleLabel = new QLabel("思政智慧课堂");
    titleLabel->setStyleSheet("color: " + PATRIOTIC_RED_DEEP_TONE + "; font-size: 19px; font-weight: 700;");

    titleLayout->addWidget(starIcon);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // 搜索框
    QFrame *searchWrapper = new QFrame();
    searchWrapper->setObjectName("SearchWrapper");
    searchWrapper->setFixedHeight(44);
    searchWrapper->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    searchWrapper->setStyleSheet(
        "#SearchWrapper {"
        "  background-color: #ffffff;"
        "  border: 1px solid " + CARD_BORDER_COLOR + ";"
        "  border-radius: 24px;"
        "}"
    );

    auto *searchShadow = new QGraphicsDropShadowEffect(searchWrapper);
    searchShadow->setBlurRadius(20);
    searchShadow->setOffset(0, 3);
    searchShadow->setColor(QColor(15, 23, 42, 25));
    searchWrapper->setGraphicsEffect(searchShadow);

    QHBoxLayout *searchLayout = new QHBoxLayout(searchWrapper);
    searchLayout->setContentsMargins(20, 0, 20, 0);
    searchLayout->setSpacing(12);

    QLabel *searchIcon = new QLabel("🔍");
    searchIcon->setFixedSize(22, 22);
    searchIcon->setAlignment(Qt::AlignCenter);
    searchIcon->setStyleSheet("QLabel { color: " + SECONDARY_TEXT + "; font-size: 18px; }");

    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("搜索资源、学生...");
    searchInput->setFixedHeight(44);
    searchInput->setStyleSheet(
        "QLineEdit {"
        "  background: #ffffff;"
        "  border: none;"
        "  font-size: 15px;"
        "  color: " + PRIMARY_TEXT + ";"
        "}"
        "QLineEdit::placeholder { color: " + LIGHT_TEXT + "; }"
        "QLineEdit:focus { border: none; }"
    );

    searchLayout->addWidget(searchIcon);
    searchLayout->addWidget(searchInput);

    // 通知按钮 - 使用自定义图片
    notificationBtn = new QPushButton();
    notificationBtn->setFixedSize(40, 40);

    // 加载自定义通知图标
    QPixmap notificationIcon("/Users/zhouzhiqi/QtProjects/AItechnology/images/通知.png");
    if (!notificationIcon.isNull()) {
        // 图片加载成功，设置按钮图标
        notificationBtn->setIcon(notificationIcon);
        notificationBtn->setIconSize(QSize(24, 24));
    } else {
        // 如果图片加载失败，使用备用emoji图标
        notificationBtn->setText("🔔");
    }
    notificationBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1;"
        "  color: %2;"
        "  border: 1px solid rgba(255, 255, 255, 0.3);"
        "  border-radius: 12px;"
        "}"
        "QPushButton[actionState=\"hover\"] {"
        "  background: %3;"
        "  color: %4;"
        "  border: 1px solid rgba(255, 255, 255, 0.5);"
        "}"
        "QPushButton[actionState=\"pressed\"] {"
        "  background: %5;"
        "  color: %6;"
        "  border: 1px solid rgba(255, 255, 255, 0.7);"
        "}"
    ).arg(CARD_WHITE,                    // 正常状态 - 白色背景
          PRIMARY_TEXT,                   // 正常状态 - 深色文字
          ULTRA_LIGHT_GRAY,              // 悬停状态 - 浅灰背景
          PRIMARY_TEXT,                   // 悬停状态 - 深色文字
          LIGHT_GRAY,                     // 按下状态 - 灰色背景
          PRIMARY_TEXT));                 // 按下状态 - 深色文字

    // 头部头像
    headerProfileBtn = new QPushButton();
    headerProfileBtn->setFixedSize(40, 40);
    headerProfileBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 20px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton[actionState=\"hover\"] {"
        "  background: %2;"
        "}"
        "QPushButton[actionState=\"pressed\"] {"
        "  background: %3;"
        "}"
    ).arg(PATRIOTIC_RED,
          PATRIOTIC_RED_DARK,
          PATRIOTIC_RED_DEEP_TONE));
    headerProfileBtn->setText("王");
    // 移除通知按钮的ButtonHoverAnimator，避免红色光晕效果
    new ButtonHoverAnimator(headerProfileBtn, headerProfileBtn, 2);

    headerLayout->addWidget(searchWrapper);
    headerLayout->addSpacing(12);
    headerLayout->addWidget(notificationBtn);
    headerLayout->addWidget(headerProfileBtn);

    // 搜索框快捷键
    auto slashShortcut = new QShortcut(QKeySequence("/"), this);
    connect(slashShortcut, &QShortcut::activated, this, [this](){ this->searchInput->setFocus(); this->searchInput->selectAll(); });

    // Ctrl+K 快捷键
    auto ctrlKShortcut = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(ctrlKShortcut, &QShortcut::activated, this, [this](){ this->searchInput->setFocus(); this->searchInput->selectAll(); });
}


void ModernMainWindow::createCoreFeatures()
{
    coreFeaturesFrame = new QFrame();
    coreFeaturesLayout = new QGridLayout(coreFeaturesFrame);
    coreFeaturesLayout->setHorizontalSpacing(36);
    coreFeaturesLayout->setVerticalSpacing(28);

    // 四个核心功能卡片
    psychologyCard = new QPushButton();
    editDocumentCard = new QPushButton();
    slideshowCard = new QPushButton();
    folderOpenCard = new QPushButton();

    QString cardStyle = QString(
        "QPushButton {"
        "  background: %1;"
        "  border: 1px solid %2;"
        "  border-radius: %3px;"
        "  padding: %4px;"
        "  text-align: left;"
        "}"
        "QPushButton[cardState=\"hover\"] {"
        "  border: 1px solid %5;"
        "  background: %6;"
        "}"
        "QPushButton[cardState=\"pressed\"] {"
        "  border: 1px solid %5;"
        "  background: %7;"
        "}"
    ).arg(CARD_GRADIENT)
     .arg(CARD_BORDER_COLOR)
     .arg(CARD_CORNER_RADIUS)
     .arg(CARD_PADDING_PX)
     .arg(CARD_BORDER_HIGHLIGHT)
     .arg(CARD_HOVER_GRADIENT)
     .arg(PATRIOTIC_RED_GRADIENT);

    QStringList icons = {"💡", "📝", "📊", "📁"};
    QStringList titles = {"智能内容分析", "AI智能备课", "互动教学工具", "试题库"};
    QStringList descriptions = {
        "深挖思政元素，把握正确导向",
        "按章节自动生成PPT，一键生成试卷",
        "创新互动形式，激活红色课堂",
        "汇聚权威材料，构筑精神高地"
    };

    QList<QPushButton*> cards = {psychologyCard, editDocumentCard, slideshowCard, folderOpenCard};
    QStringList accentColors = {PATRIOTIC_RED, WISDOM_BLUE, CULTURE_GOLD, ACADEMIC_PURPLE};

    for (int i = 0; i < 4; ++i) {
        QVBoxLayout *cardLayout = new QVBoxLayout(cards[i]);
        cardLayout->setSpacing(8);  // 减小间距，避免灰色背景条
        cardLayout->setContentsMargins(16, 16, 16, 16);  // 统一边距

        QLabel *iconLabel = new QLabel(icons[i]);
        iconLabel->setStyleSheet("color: " + accentColors[qMin(i, accentColors.size() - 1)] + "; font-size: 24px; font-weight: bold; background: transparent;");
        iconLabel->setAlignment(Qt::AlignCenter);

        QLabel *titleLabel = new QLabel(titles[i]);
        titleLabel->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 16px; font-weight: bold; background: transparent; border: none;");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setMinimumHeight(20);  // 确保标题区域一致

        QLabel *descLabel = new QLabel(descriptions[i]);
        descLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 14px; background: transparent;");
        descLabel->setWordWrap(true);
        descLabel->setAlignment(Qt::AlignCenter);
        descLabel->setMinimumHeight(40);  // 确保描述区域一致

        cardLayout->addWidget(iconLabel);
        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(descLabel);
        cardLayout->addStretch();

        cards[i]->setStyleSheet(cardStyle + " QLabel { background: transparent; border: none; }");
        cards[i]->setMinimumHeight(140);
        cards[i]->setFixedHeight(140);  // 确保所有卡片高度完全一致
        applyCardShadow(cards[i], 18.0, 6.0);
    }

    coreFeaturesLayout->addWidget(psychologyCard, 0, 0);
    coreFeaturesLayout->addWidget(editDocumentCard, 0, 1);
    coreFeaturesLayout->addWidget(slideshowCard, 0, 2);
    coreFeaturesLayout->addWidget(folderOpenCard, 0, 3);

    // 连接核心功能卡片的点击事件
    connect(folderOpenCard, &QPushButton::clicked, this, [=]() {
        qDebug() << "试题库卡片被点击";
        onResourceManagementClicked();
    });

    connect(editDocumentCard, &QPushButton::clicked, this, [=]() {
        qDebug() << "AI智能备课卡片被点击";
        onAIPreparationClicked();
    });

    connect(psychologyCard, &QPushButton::clicked, this, [=]() {
        qDebug() << "智能内容分析卡片被点击";
        onContentAnalysisClicked();
    });

    connect(slideshowCard, &QPushButton::clicked, this, [=]() {
        qDebug() << "互动教学工具卡片被点击";
        // 暂时使用已有的方法或添加新方法
        onLearningAnalysisClicked();
    });

    // 添加悬停支持和工具提示
    QStringList tooltips = {
        "智能分析教学内容中的思政元素，确保价值导向正确",
        "AI智能生成教学PPT和试卷，提高备课效率",
        "丰富的课堂互动工具，打造活跃的思政课堂",
        "精选权威教学资源，构建高质量题库"
    };

    for (int i = 0; i < cards.size(); ++i) {
        cards[i]->setAttribute(Qt::WA_Hover, true);
        cards[i]->setToolTip(tooltips[i]);
        cards[i]->setCursor(Qt::PointingHandCursor);

        // 添加简单的hover事件处理器来设置cardState属性
        cards[i]->installEventFilter(new SimpleCardHoverFilter(cards[i]));
    }
}

void ModernMainWindow::createRecentCourses()
{
    // 1️⃣ 单卡片容器 - 圆角12px + 阴影 + 白色背景
    recentCoursesFrame = new QFrame();
    recentCoursesFrame->setMinimumWidth(460);  // 最小宽度460px，填满网格左列
    recentCoursesFrame->setFixedHeight(140);   // 增加高度给上下更多空间
    recentCoursesFrame->setStyleSheet(
        "QFrame {"
        "  background-color: #FFFFFF;"
        "  border-radius: 12px;"
        "  border: none;"
        "}"
    );
    applyCardShadow(recentCoursesFrame, 10.0, 0.0);  // (blurRadius, yOffset)

    // 2️⃣ 主布局容器 - 水平排列
    QHBoxLayout *mainLayout = new QHBoxLayout(recentCoursesFrame);
    mainLayout->setContentsMargins(20, 24, 20, 24);  // 增加上下内边距到24px
    mainLayout->setSpacing(16);
    mainLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 3️⃣ 左侧内容区域（垂直排列的3行信息）
    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(4);  // 行距4px

    // 标题行
    QLabel *titleLabel = new QLabel("近期课程");
    titleLabel->setStyleSheet(
        "color: #4A4A4A;"
        "font-size: 16px;"
        "font-weight: 600;"
        "margin-bottom: 8px;"  // 标题与内容间距8px
    );

    // 时间行
    QLabel *timeLabel = new QLabel("今日, 10:00 AM");
    timeLabel->setStyleSheet(
        "color: #8B8B8B;"
        "font-size: 14px;"
        "font-weight: 400;"
    );

    // 课程名行（核心强调）
    QLabel *courseTitleLabel = new QLabel("当代思潮与青年担当");
    courseTitleLabel->setStyleSheet(
        "color: #B81919;"
        "font-size: 16px;"
        "font-weight: 700;"
    );

    // 班级行
    QLabel *classLabel = new QLabel("高二（2）班");
    classLabel->setStyleSheet(
        "color: #8B8B8B;"
        "font-size: 14px;"
        "font-weight: 400;"
    );

    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(timeLabel);
    contentLayout->addWidget(courseTitleLabel);
    contentLayout->addWidget(classLabel);
    contentLayout->addStretch();  // 填充剩余空间

    // 4️⃣ 右侧按钮 - 固定尺寸 + 渐变
    enterClassBtn = new QPushButton("进入课堂");
    enterClassBtn->setFixedSize(120, 36);  // 高度36px
    enterClassBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                             stop:0 #BE2A2A, stop:1 #D94C4C);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                             stop:0 #E35A5A, stop:1 #E66B6B);"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                             stop:0 #A81F1F, stop:1 #C93A3A);"
        "}"
    );
    enterClassBtn->setCursor(Qt::PointingHandCursor);

    // 5️⃣ 组装主布局
    mainLayout->addLayout(contentLayout, 1);  // 内容区域拉伸
    mainLayout->addWidget(enterClassBtn, 0, Qt::AlignRight | Qt::AlignVCenter);

    // 信号连接
    connect(enterClassBtn, &QPushButton::clicked,
            this, &ModernMainWindow::onEnterClassClicked);

    // 设置SizePolicy以避免被高卡片撑出空白
    recentCoursesFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

// 创建指标项组件 - 紧凑的单行信息
QWidget* ModernMainWindow::createMetricItem(const QString& name,
                                            const QString& value,
                                            const QString& color,
                                            const QString& tooltip,
                                            const QString& changeText,
                                            int trendDirection)
{
    // 容器：单行、高度56px、圆角10、轻底色
    QWidget *row = new QWidget();
    row->setObjectName("metricItem");
    row->setFixedHeight(56);
    row->setAutoFillBackground(false);  // 禁止自动填充背景
    row->setAttribute(Qt::WA_NoSystemBackground, true);  // 禁用系统背景
    row->setStyleSheet(QString(
        "QWidget#metricItem {"
        "  background-color: %1;"
        "  border-radius: 10px;"
        "  padding: 0 12px;"
        "}"
        "QWidget#metricItem:hover {"
        "  background-color: rgba(25, 118, 210, 0.08);"
        "}"
    ).arg(PATRIOTIC_RED_LIGHT));

    row->setToolTip(tooltip);

    // 水平布局
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(12, 0, 12, 0);
    rowLayout->setSpacing(8);

    // 左侧：彩色圆点 + 名称
    QHBoxLayout *leftLayout = new QHBoxLayout();
    leftLayout->setSpacing(8);

    // 彩色圆点
    QLabel *dotLabel = new QLabel();
    dotLabel->setFixedSize(10, 10);
    dotLabel->setStyleSheet(QString("background-color: %1; border-radius: 5px;").arg(color));

    // 名称 - 降一阶与中灰
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 13px;");
    nameLabel->setToolTip(tooltip);

    leftLayout->addWidget(dotLabel);
    leftLayout->addWidget(nameLabel);
    leftLayout->addStretch();

    // 右侧：数值 - 等宽字体、右对齐、深色
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setObjectName("valueLabel");
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    valueLabel->setAutoFillBackground(false);  // 确保数值标签也无背景

    // 使用系统默认字体避免崩溃
    QFont valueFont = QFont();
    valueFont.setPointSize(20);
    valueFont.setBold(true);
    valueLabel->setFont(valueFont);
    valueLabel->setStyleSheet("color: " + PRIMARY_TEXT + ";");

    QLabel *trendArrowLabel = new QLabel();
    trendArrowLabel->setObjectName("trendArrowLabel");
    trendArrowLabel->setFixedSize(22, 22);
    trendArrowLabel->setAlignment(Qt::AlignCenter);
    trendArrowLabel->setStyleSheet(QString(
        "QLabel#trendArrowLabel {"
        "  border-radius: 11px;"
        "  background-color: rgba(117, 117, 117, 0.15);"
        "  color: %1;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "}"
    ).arg(SECONDARY_TEXT));

    QLabel *trendLabel = new QLabel();
    trendLabel->setObjectName("trendLabel");
    QString trendColor = SECONDARY_TEXT;
    if (trendDirection > 0) {
        trendColor = GROWTH_GREEN;
        trendArrowLabel->setText("↑");
        trendArrowLabel->setStyleSheet(QString(
            "QLabel#trendArrowLabel {"
            "  border-radius: 11px;"
            "  background-color: rgba(56, 142, 60, 0.15);"
            "  color: %1;"
            "  font-size: 12px;"
            "  font-weight: bold;"
        "}"
        ).arg(GROWTH_GREEN));
    } else if (trendDirection < 0) {
        trendColor = PATRIOTIC_RED;
        trendArrowLabel->setText("↓");
        trendArrowLabel->setStyleSheet(QString(
            "QLabel#trendArrowLabel {"
            "  border-radius: 11px;"
            "  background-color: rgba(229, 57, 53, 0.15);"
            "  color: %1;"
            "  font-size: 12px;"
            "  font-weight: bold;"
        "}"
        ).arg(PATRIOTIC_RED));
    } else {
        trendArrowLabel->setText("→");
    }

    QString trendText = changeText.isEmpty() ? QStringLiteral("持平") : changeText;
    trendLabel->setText(trendText);
    trendLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    trendLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600;").arg(trendColor));

    QVBoxLayout *valueLayout = new QVBoxLayout();
    valueLayout->setContentsMargins(0, 8, 0, 8);
    valueLayout->setSpacing(0);
    valueLayout->addWidget(valueLabel);
    QHBoxLayout *trendLayout = new QHBoxLayout();
    trendLayout->setContentsMargins(0, 0, 0, 0);
    trendLayout->setSpacing(6);
    trendLayout->addStretch();
    trendLayout->addWidget(trendArrowLabel);
    trendLayout->addWidget(trendLabel);
    valueLayout->addLayout(trendLayout);

    // 添加到行布局
    rowLayout->addLayout(leftLayout, 1);
    rowLayout->addLayout(valueLayout);

    return row;
}

void ModernMainWindow::createLearningAnalytics()
{
    learningAnalyticsFrame = new QFrame();
    // 移除旧的卡片样式和阴影，使用紧凑卡片样式
    // learningAnalyticsFrame->setStyleSheet(buildCardStyle("QFrame"));
    // applyCardShadow(learningAnalyticsFrame, 26.0, 10.0);
    // new FrameHoverAnimator(learningAnalyticsFrame, learningAnalyticsFrame, 6);

    QVBoxLayout *analyticsLayout = new QVBoxLayout(learningAnalyticsFrame);
    analyticsLayout->setSpacing(24);

    const QString defaultRange = "近7天";
    const QString baseScope = "高二(2)班 · 48名学生";
    const int sampleSize = 48;
    QMap<QString, LearningMetrics> dataByRange = createSampleData();
    LearningMetrics currentMetrics = dataByRange.value(defaultRange);
    auto currentRangeLabel = QSharedPointer<QString>::create(defaultRange);

    auto computeOverallScore = [](const LearningMetrics &metrics) {
        qreal sum = metrics.participation.current + metrics.homework.current + metrics.quiz.current + metrics.knowledge.current;
        return qRound(sum / 4.0);
    };

    auto formatFormulaText = [](const LearningMetrics &metrics, int overallScore) {
        return QString("计算：(课堂参与 %1% + 作业完成 %2% + 测验成绩 %3% + 知识掌握 %4%) ÷ 4 = %5%")
            .arg(metrics.participation.current)
            .arg(metrics.homework.current)
            .arg(metrics.quiz.current)
            .arg(metrics.knowledge.current)
            .arg(overallScore);
    };

    auto buildCompletionTooltip = [baseScope, formatFormulaText](const LearningMetrics &metrics, const QString &range, int overallScore) {
        QString timestamp = QDateTime::currentDateTime().toString("M月d日 hh:mm");
        return QString("综合完成度 · %1\n%2\n样本范围：%3\n更新时间：%4\n数据来源：课堂互动记录、作业提交、阶段测验、知识图谱评测")
            .arg(range)
            .arg(formatFormulaText(metrics, overallScore))
            .arg(baseScope)
            .arg(timestamp);
    };

    auto metricValueByIndex = [](const LearningMetrics &metrics, int index) -> TrendValue {
        switch (index) {
            case 0: return metrics.participation;
            case 1: return metrics.homework;
            case 2: return metrics.quiz;
            default: return metrics.knowledge;
        }
    };

    auto buildTrendArrowStyle = [](const QString &color) {
        QString background = "rgba(117, 117, 117, 0.15)";
        if (color == GROWTH_GREEN) {
            background = "rgba(56, 142, 60, 0.15)";
        } else if (color == PATRIOTIC_RED) {
            background = "rgba(229, 57, 53, 0.15)";
        }
        return QString(
            "QLabel#trendArrowLabel {"
            "  border-radius: 11px;"
            "  background-color: %1;"
            ""
            "  font-size: 12px;"
            "  font-weight: bold;"
            "}"
        ).arg(background, color);
    };

    
    // 紧凑卡片式学情分析区域
    QFrame *analyticsCard = new QFrame();
    analyticsCard->setObjectName("analyticsCompactCard");
    analyticsCard->setStyleSheet(
        "QFrame#analyticsCompactCard {"
        "  background: #FFFFFF;"
        "  border: 1px solid #E8EAF6;"
        "  border-radius: 8px;"
        "  padding: 0px;"
        "}"
    );

    QVBoxLayout *cardLayout = new QVBoxLayout(analyticsCard);
    cardLayout->setContentsMargins(20, 16, 20, 16);  // 左右20px内边距
    cardLayout->setSpacing(16);

    // 标题行：学情分析 + 时间范围选择器
    QHBoxLayout *cardTitleLayout = new QHBoxLayout();
    cardTitleLayout->setSpacing(12);

    QLabel *cardTitle = new QLabel("学情分析");
    cardTitle->setStyleSheet("color: #212121; font-size: 16px; font-weight: 700;");
    cardTitle->setAlignment(Qt::AlignLeft);

    QComboBox *cardTimeRangeCombo = new QComboBox();
    cardTimeRangeCombo->addItems({"近7天", "近30天", "本学期"});
    cardTimeRangeCombo->setCurrentText(defaultRange);
    cardTimeRangeCombo->setStyleSheet(QString(
        "QComboBox {"
        "  background: #F8F9FA;"
        "  border: 1px solid #E8EAF6;"
        "  border-radius: 6px;"
        "  padding: 4px 8px;"
        "  font-size: 13px;"
        "  min-width: 80px;"
        "}"
        "QComboBox::drop-down { border: none; width: 0px; height: 0px; }"
        "QComboBox::down-arrow { image: none; }"
    ));

    cardTitleLayout->addWidget(cardTitle);
    cardTitleLayout->addStretch();
    cardTitleLayout->addWidget(cardTimeRangeCombo);

    cardLayout->addLayout(cardTitleLayout);

    // 内容行：左侧环形图 + 右侧指标列表
    QHBoxLayout *contentRow = new QHBoxLayout();
    contentRow->setSpacing(20);
    contentRow->setAlignment(Qt::AlignCenter);

    // 左侧：紧凑环形图（缩小到120x120px）
    QWidget *compactDonutContainer = new QWidget();
    compactDonutContainer->setFixedSize(120, 120);

    QStackedLayout *compactStackedLayout = new QStackedLayout(compactDonutContainer);
    compactStackedLayout->setStackingMode(QStackedLayout::StackAll);
    compactStackedLayout->setContentsMargins(0, 0, 0, 0);

    // 创建紧凑环形图
    int overallScore = computeOverallScore(currentMetrics);
    QPieSeries *compactDonutSeries = new QPieSeries();
    compactDonutSeries->append("已完成", overallScore);
    compactDonutSeries->append("未完成", qMax(0, 100 - overallScore));
    compactDonutSeries->setHoleSize(0.70);
    compactDonutSeries->setPieSize(0.90);
    compactDonutSeries->setPieStartAngle(270);

    QPieSlice *compactCompletedSlice = compactDonutSeries->slices().at(0);
    QPieSlice *compactRemainingSlice = compactDonutSeries->slices().at(1);
    compactCompletedSlice->setColor(QColor("#D32F2F"));
    compactCompletedSlice->setBorderColor(Qt::transparent);
    compactRemainingSlice->setColor(QColor("#E0E0E0"));
    compactRemainingSlice->setBorderColor(Qt::transparent);

    QChart *compactDonutChart = new QChart();
    compactDonutChart->addSeries(compactDonutSeries);
    compactDonutChart->setBackgroundBrush(Qt::NoBrush);
    compactDonutChart->setBackgroundRoundness(0);
    compactDonutChart->legend()->hide();
    compactDonutChart->setTitle("");

    QChartView *compactDonutChartView = new QChartView(compactDonutChart);
    compactDonutChartView->setRenderHint(QPainter::Antialiasing);
    compactDonutChartView->setFixedSize(120, 120);
    compactDonutChartView->setStyleSheet("QChartView { border: none; background: transparent; }");
    compactDonutChartView->setAutoFillBackground(false);

    // 中心文字
    QWidget *compactCenterTextContainer = new QWidget();
    compactCenterTextContainer->setFixedSize(120, 120);
    compactCenterTextContainer->setAttribute(Qt::WA_TranslucentBackground, true);
    compactCenterTextContainer->setAutoFillBackground(false);
    QVBoxLayout *compactCenterTextLayout = new QVBoxLayout(compactCenterTextContainer);
    compactCenterTextLayout->setContentsMargins(0, 0, 0, 0);
    compactCenterTextLayout->setAlignment(Qt::AlignCenter);
    compactCenterTextLayout->setSpacing(2);

    QLabel *compactPercentLabel = new QLabel(QString::number(overallScore) + "%");
    compactPercentLabel->setAlignment(Qt::AlignCenter);
    QFont compactPercentFont = QFont();
    compactPercentFont.setPointSize(18);
    compactPercentFont.setBold(true);
    compactPercentLabel->setFont(compactPercentFont);
    compactPercentLabel->setStyleSheet("color: #212121;");

    QLabel *compactTitleLabel = new QLabel("完成度");
    compactTitleLabel->setStyleSheet("color: #757575; font-size: 11px;");
    compactTitleLabel->setAlignment(Qt::AlignCenter);

    compactCenterTextLayout->addWidget(compactPercentLabel);
    compactCenterTextLayout->addWidget(compactTitleLabel);

    compactStackedLayout->addWidget(compactDonutChartView);
    compactStackedLayout->addWidget(compactCenterTextContainer);
    compactCenterTextContainer->raise();

    // 右侧：垂直指标列表
    QVBoxLayout *metricsListLayout = new QVBoxLayout();
    metricsListLayout->setSpacing(12);
    metricsListLayout->setContentsMargins(0, 4, 0, 4);  // 上下留白

    struct MetricMeta {
        QString name;
        QString color;
        QString source;
    };

    QVector<MetricMeta> metricMeta = {
        {"课堂参与", "#D32F2F", "课堂签到 + 互动行为日志"},
        {"作业完成", "#1976D2", "作业提交与批改记录"},
        {"测验成绩", "#388E3C", "随堂测验与阶段测试"},
        {"知识掌握", "#F57C00", "知识点掌握度测评"}
    };

    const int metricWeightPercent = metricMeta.isEmpty() ? 0 : qRound(100.0 / metricMeta.size());

    auto buildChangeDescription = [](int diff) {
        if (diff == 0) {
            return QStringLiteral("与上次持平");
        }
        QString sign = diff > 0 ? "+" : "-";
        return QString("较上次 %1%").arg(sign + QString::number(qAbs(diff)));
    };

    auto formatMetricTooltip = [baseScope, metricWeightPercent](const MetricMeta &meta, const TrendValue &value, const QString &range) {
        int diff = value.current - value.previous;
        QString diffText = diff == 0
            ? QStringLiteral("变化：持平")
            : QString("变化：%1%").arg((diff > 0 ? "+" : "-") + QString::number(qAbs(diff)));
        int contribution = qRound(value.current * metricWeightPercent / 100.0);
        return QString("%1 · %2\n数据口径：%3\n来源：%4\n当前：%5% · 上次：%6%\n%7 · 权重：%8% · 对完成度贡献 ≈ %9%")
            .arg(meta.name)
            .arg(range)
            .arg(baseScope)
            .arg(meta.source)
            .arg(value.current)
            .arg(value.previous)
            .arg(diffText)
            .arg(metricWeightPercent)
            .arg(contribution);
    };

    QList<QLabel*> statValueLabels;
    QList<QLabel*> statTrendLabels;
    QList<QLabel*> statTrendArrowLabels;
    QList<QWidget*> statMetricRows;

    // 创建紧凑指标项（每行一个）
    for (int i = 0; i < metricMeta.size(); ++i) {
        TrendValue value = metricValueByIndex(currentMetrics, i);
        int diff = value.current - value.previous;
        int direction = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
        QString changeDescription = buildChangeDescription(diff);

        // 创建紧凑指标行：左侧圆点 + 中间名称数值 + 右侧趋势
        QWidget *metricRow = new QWidget();
        metricRow->setObjectName("compactMetricRow");
        metricRow->setStyleSheet("QWidget#compactMetricRow { background: transparent; }");
        metricRow->setFixedHeight(32);

        QHBoxLayout *metricRowLayout = new QHBoxLayout(metricRow);
        metricRowLayout->setContentsMargins(8, 6, 8, 6);
        metricRowLayout->setSpacing(12);
        metricRowLayout->setAlignment(Qt::AlignLeft);

        // 左侧彩色圆点
        QLabel *colorDot = new QLabel();
        colorDot->setFixedSize(8, 8);
        colorDot->setStyleSheet(QString(
            "QLabel {"
            "  background: %1;"
            "  border-radius: 4px;"
            "}"
        ).arg(metricMeta[i].color));

        // 中间：名称和数值
        QVBoxLayout *nameValueLayout = new QVBoxLayout();
        nameValueLayout->setSpacing(2);
        nameValueLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *nameLabel = new QLabel(metricMeta[i].name);
        nameLabel->setStyleSheet("color: #424242; font-size: 13px; font-weight: 600;");
        nameLabel->setAlignment(Qt::AlignLeft);

        QLabel *valueLabel = new QLabel(QString::number(value.current) + "%");
        valueLabel->setObjectName("valueLabel");
        valueLabel->setStyleSheet("color: #212121; font-size: 14px; font-weight: 700;");
        valueLabel->setAlignment(Qt::AlignLeft);

        nameValueLayout->addWidget(nameLabel);
        nameValueLayout->addWidget(valueLabel);

        // 右侧趋势箭头（简化版）
        QLabel *trendLabel = new QLabel();
        trendLabel->setObjectName("trendLabel");
        trendLabel->setFixedSize(24, 24);
        trendLabel->setAlignment(Qt::AlignCenter);
        trendLabel->setStyleSheet(QString(
            "QLabel#trendLabel {"
            "  color: %1;"
            "  font-size: 12px;"
            "  font-weight: bold;"
            "  border-radius: 12px;"
            "  background: %2;"
            "}"
        ).arg(
            direction == 1 ? "#388E3C" : (direction == -1 ? "#D32F2F" : "#757575"),
            direction == 1 ? "rgba(56, 142, 60, 0.15)" : (direction == -1 ? "rgba(211, 47, 47, 0.15)" : "rgba(117, 117, 117, 0.15)")
        ));
        trendLabel->setText(direction == 1 ? "↑" : (direction == -1 ? "↓" : "→"));

        QString tooltip = formatMetricTooltip(metricMeta[i], value, defaultRange);
        colorDot->setToolTip(tooltip);
        nameLabel->setToolTip(tooltip);
        valueLabel->setToolTip(tooltip);
        trendLabel->setToolTip(tooltip);

        metricRowLayout->addWidget(colorDot, 0, Qt::AlignCenter);
        metricRowLayout->addLayout(nameValueLayout, 1);
        metricRowLayout->addWidget(trendLabel, 0, Qt::AlignCenter);

        statValueLabels.append(valueLabel);
        statTrendLabels.append(trendLabel);
        statTrendArrowLabels.append(trendLabel);
        statMetricRows.append(metricRow);

        metricsListLayout->addWidget(metricRow);
    }

    // 组装内容行
    contentRow->addWidget(compactDonutContainer, 0, Qt::AlignCenter);
    contentRow->addLayout(metricsListLayout, 1);

    cardLayout->addLayout(contentRow);

    // 将卡片添加到主布局
    analyticsLayout->addWidget(analyticsCard);

    QFrame *completionInfoFrame = new QFrame();
    completionInfoFrame->setObjectName("completionInfoFrame");
    completionInfoFrame->setStyleSheet(QString(
        "QFrame#completionInfoFrame {"
        "  background-color: rgba(239, 83, 80, 0.06);"
        "  border: 1px dashed %1;"
        "  border-radius: 12px;"
        "}"
    ).arg(PATRIOTIC_RED));

    QVBoxLayout *completionInfoLayout = new QVBoxLayout(completionInfoFrame);
    completionInfoLayout->setContentsMargins(16, 12, 16, 12);
    completionInfoLayout->setSpacing(4);

    QLabel *completionScopeLabel = new QLabel();
    completionScopeLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 12px;");
    QLabel *completionFormulaLabel = new QLabel();
    completionFormulaLabel->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 13px; font-weight: 500;");
    completionFormulaLabel->setWordWrap(true);
    QLabel *completionSourceLabel = new QLabel("数据来源：课堂互动记录 · 作业提交 · 随堂测验 · 知识图谱评测");
    completionSourceLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 12px;");
    completionSourceLabel->setWordWrap(true);

    completionInfoLayout->addWidget(completionScopeLabel);
    completionInfoLayout->addWidget(completionFormulaLabel);
    completionInfoLayout->addWidget(completionSourceLabel);

    analyticsLayout->addWidget(completionInfoFrame);

    // 完成度拆解模块已移除

    // refreshBreakdownDetails 函数已随完成度拆解模块移除

    auto refreshCompletionInfo = [completionScopeLabel,
                                  completionFormulaLabel,
                                  completionInfoFrame,
                                  compactPercentLabel,
                                  compactTitleLabel,
                                  compactDonutChartView,
                                  compactCompletedSlice,
                                  compactRemainingSlice,
                                  baseScope,
                                  formatFormulaText,
                                  buildCompletionTooltip,
                                  computeOverallScore](const LearningMetrics &metrics, const QString &range) {
        int score = computeOverallScore(metrics);
        int remaining = qMax(0, 100 - score);
        completionScopeLabel->setText(QString("统计范围：%1 · %2").arg(range, baseScope));
        completionFormulaLabel->setText(formatFormulaText(metrics, score));
        QString tooltip = buildCompletionTooltip(metrics, range, score);
        completionInfoFrame->setToolTip(tooltip);
        compactPercentLabel->setText(QString::number(score) + "%");
        compactPercentLabel->setToolTip(tooltip);
        compactTitleLabel->setToolTip(tooltip);
        compactDonutChartView->setToolTip(tooltip);
        compactCompletedSlice->setValue(score);
        // compactCompletedSlice->setToolTip(tooltip); // QPieSlice没有setToolTip方法
        compactRemainingSlice->setValue(remaining);
        // compactRemainingSlice->setToolTip(QString("未完成 %1% · 待跟进任务 = 计划 - 已完成").arg(remaining)); // QPieSlice没有setToolTip方法
        // refreshBreakdownDetails 调用已随完成度拆解模块移除
    };

    refreshCompletionInfo(currentMetrics, defaultRange);

    // 图表区域（独立卡片，右侧展示）
    chartsContainer = new QFrame();
    chartsContainer->setObjectName("analyticsChartsCard");
    chartsContainer->setStyleSheet(buildCardStyle("QFrame#analyticsChartsCard"));
    chartsContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    chartsContainer->setMaximumWidth(360);
    applyCardShadow(chartsContainer, 22.0, 8.0);
    new FrameHoverAnimator(chartsContainer, chartsContainer, 4);
    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsContainer);
    chartsLayout->setContentsMargins(22, 22, 22, 22);
    chartsLayout->setSpacing(16);

    // 设置learningAnalyticsFrame的SizePolicy
    learningAnalyticsFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    const int compactChartHeight = 220;

    // 图表1：柱状图 - 课堂参与度/测验正确率/专注度对比
    QWidget *barChartContainer = new QWidget();
    barChartContainer->setObjectName("barChart");
    barChartContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QVBoxLayout *barLayout = new QVBoxLayout(barChartContainer);
    barLayout->setContentsMargins(16, 16, 16, 16);
    barLayout->setSpacing(8);

    QLabel *barTitle = new QLabel("三维度评分对比");
    barTitle->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 16px; font-weight: bold;");

    QChartView *barChartView = new QChartView();
    barChartView->setRenderHint(QPainter::Antialiasing);
    barChartView->setObjectName("barChartView");
    barChartView->setStyleSheet("QChartView { border: none; background: transparent; }");
    barChartView->setAutoFillBackground(false);  // 禁止自动填充背景
    barChartView->setToolTip("悬停柱体查看当前/上次/目标的具体数值");
    barChartView->setMinimumHeight(compactChartHeight);
    barChartView->setMaximumHeight(compactChartHeight + 30);
    barChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 创建柱状图数据
    QBarSet *set0 = new QBarSet("课堂参与");
    QBarSet *set1 = new QBarSet("作业完成");
    QBarSet *set2 = new QBarSet("测验成绩");
    int participationTarget = qMin(100, currentMetrics.participation.current + 3);
    int homeworkTarget = qMin(100, currentMetrics.homework.current + 2);
    int quizTarget = qMin(100, currentMetrics.quiz.current + 4);
    *set0 << currentMetrics.participation.current << currentMetrics.participation.previous << participationTarget;
    *set1 << currentMetrics.homework.current << currentMetrics.homework.previous << homeworkTarget;
    *set2 << currentMetrics.quiz.current << currentMetrics.quiz.previous << quizTarget;

    set0->setColor(QColor(PATRIOTIC_RED));
    set1->setColor(QColor(WISDOM_BLUE));
    set2->setColor(QColor(GROWTH_GREEN));

    auto formatBarTooltip = [baseScope](const QString &label, int current, int previous, int target, const QString &range) {
        int diff = current - previous;
        QString diffText = diff == 0
            ? QStringLiteral("Δ0%（持平）")
            : QString("Δ%1%").arg((diff > 0 ? "+" : "-") + QString::number(qAbs(diff)));
        return QString("%1 · %2\n当前：%3% · 上次：%4% · %5\n目标值：%6%\n数据口径：%7")
            .arg(label)
            .arg(range)
            .arg(current)
            .arg(previous)
            .arg(diffText)
            .arg(target)
            .arg(baseScope);
    };
    // set0->setToolTip(formatBarTooltip("课堂参与", currentMetrics.participation.current, currentMetrics.participation.previous, participationTarget, defaultRange)); // QBarSet没有setToolTip方法
    // set1->setToolTip(formatBarTooltip("作业完成", currentMetrics.homework.current, currentMetrics.homework.previous, homeworkTarget, defaultRange)); // QBarSet没有setToolTip方法
    // set2->setToolTip(formatBarTooltip("测验成绩", currentMetrics.quiz.current, currentMetrics.quiz.previous, quizTarget, defaultRange)); // QBarSet没有setToolTip方法

    QStringList comparisonBuckets = {"当前值", "上次值", "目标值"};

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(set0);
    barSeries->append(set1);
    barSeries->append(set2);

    QChart *barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setBackgroundBrush(Qt::NoBrush);
    barChart->setBackgroundRoundness(0);  // 去圆角裁剪
    barChart->setTitle("");
    barChart->setAnimationOptions(QChart::SeriesAnimations);

    barChart->createDefaultAxes();
    barChart->axisY()->setRange(0, 100);
    QFont axisFont("PingFang SC", 10);
    barChart->axisX()->setLabelsFont(axisFont);
    barChart->axisY()->setLabelsFont(axisFont);
    // 轴标签用深灰
    barChart->axisX()->setLabelsColor(QColor(PRIMARY_TEXT));
    barChart->axisY()->setLabelsColor(QColor(PRIMARY_TEXT));

    barChartView->setChart(barChart);

    barLayout->addWidget(barTitle);
    barLayout->addWidget(barChartView);

    // 图表2：饼图 - 知识点掌握分布
    QWidget *pieChartContainer = new QWidget();
    pieChartContainer->setObjectName("pieChart");
    pieChartContainer->setAutoFillBackground(false);  // 禁止自动填充背景
    pieChartContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QVBoxLayout *pieLayout = new QVBoxLayout(pieChartContainer);
    pieLayout->setContentsMargins(16, 16, 16, 16);
    pieLayout->setSpacing(8);

    QLabel *pieTitle = new QLabel("知识点掌握分布");
    pieTitle->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 16px; font-weight: bold;");

    QChartView *pieChartView = new QChartView();
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->setObjectName("pieChartView");
    pieChartView->setStyleSheet("QChartView { border: none; background: transparent; }");
    pieChartView->setAutoFillBackground(false);  // 禁止自动填充背景
    pieChartView->setToolTip("悬停或点击扇区查看掌握水平和人数换算");
    pieChartView->setMinimumHeight(compactChartHeight);
    pieChartView->setMaximumHeight(compactChartHeight + 30);
    pieChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->append("掌握", 65);
    pieSeries->append("基本掌握", 28);
    pieSeries->append("需巩固", 7);

    QPieSlice *slice0 = pieSeries->slices().at(0);
    QPieSlice *slice1 = pieSeries->slices().at(1);
    QPieSlice *slice2 = pieSeries->slices().at(2);
    slice0->setColor(QColor(GROWTH_GREEN));
    slice1->setColor(QColor(WISDOM_BLUE));
    slice2->setColor(QColor(ACADEMIC_PURPLE));

    auto formatSliceTooltip = [sampleSize, baseScope](const QString &label, int percent, const QString &range) {
        int students = qRound((percent / 100.0) * sampleSize);
        return QString("%1 · %2\n占比：%3%（约%4人）\n数据口径：%5")
            .arg(label)
            .arg(range)
            .arg(percent)
            .arg(students)
            .arg(baseScope);
    };
    // slice0->setToolTip(formatSliceTooltip("掌握", static_cast<int>(slice0->value()), defaultRange)); // QPieSlice没有setToolTip方法
    // slice1->setToolTip(formatSliceTooltip("基本掌握", static_cast<int>(slice1->value()), defaultRange)); // QPieSlice没有setToolTip方法
    // slice2->setToolTip(formatSliceTooltip("需巩固", static_cast<int>(slice2->value()), defaultRange)); // QPieSlice没有setToolTip方法

    slice0->setLabelVisible(true);
    slice1->setLabelVisible(true);
    slice2->setLabelVisible(true);

    QChart *pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setBackgroundBrush(Qt::NoBrush);
    pieChart->setBackgroundRoundness(0);  // 去圆角裁剪
    pieChart->setTitle("");
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
    pieChart->legend()->show();
    pieChart->legend()->setColor(QColor(PRIMARY_TEXT));

    pieChartView->setChart(pieChart);

    pieLayout->addWidget(pieTitle);
    pieLayout->addWidget(pieChartView);

    auto openChartDialog = [this](const QString &title, const std::function<QChart *()> &chartBuilder) {
        if (!chartBuilder) {
            return;
        }
        QChart *chart = chartBuilder();
        if (!chart) {
            return;
        }
        QDialog dialog(this);
        dialog.setWindowTitle(title);
        dialog.setModal(true);
        dialog.resize(860, 560);
        QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
        QChartView *dialogChartView = new QChartView(chart, &dialog);
        dialogChartView->setRenderHint(QPainter::Antialiasing);
        dialogChartView->setStyleSheet("QChartView { border: none; background: transparent; }");
        dialogLayout->addWidget(dialogChartView);
        dialog.exec();
    };

    std::function<QChart *()> barChartBuilder = [barSeries]() -> QChart* {
        if (!barSeries) {
            return nullptr;
        }
        QBarSeries *seriesCopy = new QBarSeries();
        for (QBarSet *set : barSeries->barSets()) {
            if (!set) {
                continue;
            }
            QBarSet *copy = new QBarSet(set->label());
            for (int valueIndex = 0; valueIndex < set->count(); ++valueIndex) {
                *copy << set->at(valueIndex);
            }
            copy->setColor(set->brush().color());
            copy->setLabelColor(set->labelColor());
            seriesCopy->append(copy);
        }
        QChart *chartCopy = new QChart();
        chartCopy->addSeries(seriesCopy);
        chartCopy->setTitle("三维度评分对比");
        chartCopy->setBackgroundBrush(Qt::NoBrush);
        chartCopy->setAnimationOptions(QChart::SeriesAnimations);
        chartCopy->createDefaultAxes();
        chartCopy->axisY()->setRange(0, 100);
        QFont axisFont("PingFang SC", 12);
        chartCopy->axisX()->setLabelsFont(axisFont);
        chartCopy->axisY()->setLabelsFont(axisFont);
        chartCopy->axisX()->setLabelsColor(QColor(PRIMARY_TEXT));
        chartCopy->axisY()->setLabelsColor(QColor(PRIMARY_TEXT));
        chartCopy->legend()->setAlignment(Qt::AlignBottom);
        return chartCopy;
    };

    std::function<QChart *()> pieChartBuilder = [pieSeries]() -> QChart* {
        if (!pieSeries) {
            return nullptr;
        }
        QPieSeries *seriesCopy = new QPieSeries();
        for (QPieSlice *slice : pieSeries->slices()) {
            if (!slice) {
                continue;
            }
            QPieSlice *copy = new QPieSlice(slice->label(), slice->value());
            copy->setColor(slice->brush().color());
            copy->setLabelVisible(true);
            seriesCopy->append(copy);
        }
        QChart *chartCopy = new QChart();
        chartCopy->addSeries(seriesCopy);
        chartCopy->setTitle("知识点掌握分布");
        chartCopy->setBackgroundBrush(Qt::NoBrush);
        chartCopy->setAnimationOptions(QChart::SeriesAnimations);
        chartCopy->legend()->setAlignment(Qt::AlignBottom);
        return chartCopy;
    };

    const auto installChartMagnifier =
        [openChartDialog](QChartView *view, const QString &title, const std::function<QChart *()> &builder) {
            if (!view || !builder) {
                return;
            }
            QString tooltip = view->toolTip();
            if (!tooltip.contains(QStringLiteral("点击可放大查看"))) {
                tooltip = tooltip.isEmpty()
                    ? QStringLiteral("点击可放大查看")
                    : tooltip + QStringLiteral("\n点击可放大查看");
                view->setToolTip(tooltip);
            }
            view->setCursor(Qt::PointingHandCursor);
            view->viewport()->installEventFilter(new ChartClickFilter([title, builder, openChartDialog]() {
                openChartDialog(title, builder);
            }, view));
        };

    installChartMagnifier(barChartView, QStringLiteral("三维度评分对比"), barChartBuilder);
    installChartMagnifier(pieChartView, QStringLiteral("知识点掌握分布"), pieChartBuilder);

    // 图表容器布局
    // 移除三维评分对比和知识点掌握分布图表
    // chartsLayout->addWidget(barChartContainer);
    // chartsLayout->addWidget(pieChartContainer);

    // 降级提示
    QLabel *fallbackNote = new QLabel("未启用 Qt Charts，已降级为基础视图");
    fallbackNote->setStyleSheet("color: " + LIGHT_TEXT + "; font-size: 12px; font-style: italic;");
    fallbackNote->setVisible(false);
    fallbackNote->setAlignment(Qt::AlignCenter);
    chartsLayout->addWidget(fallbackNote);
    chartsLayout->addStretch();

    // 连接时间范围选择器的信号
    connect(cardTimeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this,
         cardTimeRangeCombo,
         dataByRange,
         defaultRange,
         currentMetrics,
         metricMeta,
         statValueLabels,
         statTrendLabels,
         statTrendArrowLabels,
         statMetricRows,
         buildChangeDescription,
         formatMetricTooltip,
         metricValueByIndex,
         barSeries,
         pieSeries,
         formatBarTooltip,
         formatSliceTooltip,
         refreshCompletionInfo,
         buildTrendArrowStyle,
         computeOverallScore,
         currentRangeLabel](int) {
            QString range = timeRangeCombo->currentText();
            *currentRangeLabel = range;
            LearningMetrics metrics = dataByRange.value(range, dataByRange.value(defaultRange, currentMetrics));
            refreshCompletionInfo(metrics, range);

            for (int i = 0; i < metricMeta.size(); ++i) {
                TrendValue value = metricValueByIndex(metrics, i);
                int diff = value.current - value.previous;
                int direction = diff > 0 ? 1 : (diff < 0 ? -1 : 0);

                if (i < statValueLabels.size() && statValueLabels[i]) {
                    statValueLabels[i]->setText(QString::number(value.current) + "%");
                }
                if (i < statTrendLabels.size() && statTrendLabels[i]) {
                    QString changeDescription = buildChangeDescription(diff);
                    QString arrow = direction > 0 ? "↑" : (direction < 0 ? "↓" : "→");
                    QString trendColor = direction > 0 ? GROWTH_GREEN : (direction < 0 ? PATRIOTIC_RED : SECONDARY_TEXT);
                    statTrendLabels[i]->setText(changeDescription);
                    statTrendLabels[i]->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600;").arg(trendColor));
                    if (i < statTrendArrowLabels.size() && statTrendArrowLabels[i]) {
                        statTrendArrowLabels[i]->setText(arrow);
                        statTrendArrowLabels[i]->setStyleSheet(buildTrendArrowStyle(trendColor));
                    }
                }
                if (i < statMetricRows.size() && statMetricRows[i]) {
                    statMetricRows[i]->setToolTip(formatMetricTooltip(metricMeta[i], value, range));
                }
            }

            QList<QBarSet*> sets = barSeries->barSets();
            auto updateSet = [&](QBarSet *set, const TrendValue &value, const QString &label, int offset) {
                if (!set) {
                    return;
                }
                set->remove(0, set->count());
                int target = qMin(100, value.current + offset);
                *set << value.current << value.previous << target;
                // set->setToolTip(formatBarTooltip(label, value.current, value.previous, target, range)); // QBarSet没有setToolTip方法
            };

            if (sets.size() >= 3) {
                updateSet(sets[0], metrics.participation, "课堂参与", 3);
                updateSet(sets[1], metrics.homework, "作业完成", 2);
                updateSet(sets[2], metrics.quiz, "测验成绩", 4);
            }

            QList<QPieSlice*> slices = pieSeries->slices();
            if (slices.size() >= 3) {
                slices[0]->setValue(metrics.mastery);
                // slices[0]->setToolTip(formatSliceTooltip("掌握", metrics.mastery, range)); // QPieSlice没有setToolTip方法
                slices[1]->setValue(metrics.partial);
                // slices[1]->setToolTip(formatSliceTooltip("基本掌握", metrics.partial, range)); // QPieSlice没有setToolTip方法
                slices[2]->setValue(metrics.needsWork);
                // slices[2]->setToolTip(formatSliceTooltip("需巩固", metrics.needsWork, range)); // QPieSlice没有setToolTip方法
            }

            this->statusBar()->showMessage(
                QString("学情分析 · %1 · 综合完成度 %2%").arg(range).arg(computeOverallScore(metrics))
            );
        });

    // 图表交互
    connect(barSeries, &QBarSeries::clicked, this, [this](int index, QBarSet *set) {
        this->statusBar()->showMessage("柱状图点击：可查看班级/学生下钻（示例）");
    });

    connect(barSeries, &QBarSeries::hovered, this,
        [barChartView, comparisonBuckets, currentRangeLabel, baseScope](bool status, int index, QBarSet *set) {
            if (!status || !set) {
                return;
            }
            QString dimension = (index >= 0 && index < comparisonBuckets.size())
                ? comparisonBuckets[index]
                : QString("维度%1").arg(index + 1);
            QString bucketHint;
            if (dimension == "当前值") {
                bucketHint = QStringLiteral("当前表现");
            } else if (dimension == "上次值") {
                bucketHint = QStringLiteral("上次表现");
            } else if (dimension == "目标值") {
                bucketHint = QStringLiteral("教师设定目标");
            } else {
                bucketHint = dimension;
            }
            QString text = QString("%1 · %2（%3）\n数值：%4%\n数据口径：%5 · %6")
                .arg(set->label())
                .arg(*currentRangeLabel)
                .arg(dimension)
                .arg(QString::number(set->at(index), 'f', 0))
                .arg(baseScope)
                .arg(bucketHint);
            QToolTip::showText(QCursor::pos(), text, barChartView);
        });

    connect(pieSeries, &QPieSeries::clicked, this, [this](QPieSlice *slice) {
        this->statusBar()->showMessage("饼图点击：可查看知识点详细分析（示例）");
    });

    connect(pieSeries, &QPieSeries::hovered, this,
        [pieChartView, formatSliceTooltip, currentRangeLabel](QPieSlice *slice, bool state) {
            if (!state || !slice) {
                return;
            }
            QString text = formatSliceTooltip(slice->label(), static_cast<int>(slice->value()), *currentRangeLabel);
            QToolTip::showText(QCursor::pos(), text, pieChartView);
        });
}

void ModernMainWindow::createRecentActivities()
{
    recentActivitiesFrame = new QFrame();
    recentActivitiesFrame->setObjectName("recentActivitiesCard");
    recentActivitiesFrame->setAttribute(Qt::WA_StyledBackground, true);
    recentActivitiesFrame->setStyleSheet(QString(
        "QFrame#recentActivitiesCard {"
        "  background: #FFFFFF;"
        "  border-radius: 32px;"
        "  border: 1px solid rgba(0, 0, 0, 0.1);"
        "  padding: 12px;"
        "}"
        "QFrame#recentActivitiesCard:hover {"
        "  border-color: rgba(0, 0, 0, 0.2);"
        "  box-shadow: 0 20px 38px rgba(15, 23, 42, 0.16);"
        "  transform: translateY(-2px);"
        "}"
    ));
    applyCardShadow(recentActivitiesFrame, 32.0, 10.0);
    new FrameHoverAnimator(recentActivitiesFrame, recentActivitiesFrame, 5);

    QVBoxLayout *activitiesLayout = new QVBoxLayout(recentActivitiesFrame);
    activitiesLayout->setSpacing(20);
    activitiesLayout->setContentsMargins(28, 28, 28, 28);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(12);

    QLabel *activitiesTitle = new QLabel("近期活动");
    activitiesTitle->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 16px; font-weight: 600; letter-spacing: 0.4px;");

    QLabel *liveBadge = new QLabel("实时");
    liveBadge->setAlignment(Qt::AlignCenter);
    liveBadge->setStyleSheet(
        "QLabel {"
        "  background: rgba(229, 57, 53, 0.12);"
        "  color: " + PATRIOTIC_RED + ";"
        "  border-radius: 12px;"
        "  padding: 4px 12px;"
        "  font-size: 11px;"
        "  font-weight: 600;"
        "}"
    );

    titleLayout->addWidget(activitiesTitle);
    titleLayout->addWidget(liveBadge);
    titleLayout->addStretch();

    QPushButton *viewAllBtn = new QPushButton("查看全部");
    viewAllBtn->setCursor(Qt::PointingHandCursor);
    viewAllBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background: transparent;"
        "  color: %1;"
        "  border: 1px solid rgba(229, 57, 53, 0.35);"
        "  border-radius: 14px;"
        "  padding: 6px 14px;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(229, 57, 53, 0.08);"
        "}"
        "QPushButton:pressed {"
        "  background: rgba(229, 57, 53, 0.14);"
        "}"
    ).arg(PATRIOTIC_RED));
    connect(viewAllBtn, &QPushButton::clicked, this, [this]() {
        this->statusBar()->showMessage("即将进入活动中心（示例）", 3000);
    });

    titleLayout->addWidget(viewAllBtn);

    QVBoxLayout *headerBlock = new QVBoxLayout();
    headerBlock->setSpacing(6);
    headerBlock->addLayout(titleLayout);

    QLabel *subtitle = new QLabel("课堂动态、资源更新与学生行为将在此处同步");
    subtitle->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 13px; letter-spacing: 0.3px;");
    headerBlock->addWidget(subtitle);

    activitiesLayout->addLayout(headerBlock);

    QFrame *listContainer = new QFrame();
    listContainer->setObjectName("recentActivitiesList");
    listContainer->setStyleSheet(
        "QFrame#recentActivitiesList {"
        "  background: transparent;"
        "  border-radius: 0px;"
        "  border: none;"
        "  padding: 0px;"
        "}"
    );

    QVBoxLayout *listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(4, 8, 4, 20);
    listLayout->setSpacing(16);

    struct ActivityEntry {
        QString title;
        QString time;
        QString meta;
        QString icon;
        QString accentColor;
        QString accentBackground;
        QString badge;
    };

    const QList<ActivityEntry> activityData = {
        {QStringLiteral("《全球化与民族主义》的教案已创建"), QStringLiteral("2小时前"), QStringLiteral("教学资源 · 备课"), QStringLiteral("📄"), PATRIOTIC_RED, PATRIOTIC_RED_SOFT_LAYER, QStringLiteral("备课")},
        {QStringLiteral("新生 \"李明\" 已加入高二(2)班"), QStringLiteral("昨天 · 16:30"), QStringLiteral("班级成员 · 学籍"), QStringLiteral("👤"), GROWTH_GREEN, "rgba(56, 142, 60, 0.15)", QStringLiteral("学籍")},
        {QStringLiteral("已有15名学生提交 \"历史分析论文\" 作业"), QStringLiteral("昨天 · 11:00"), QStringLiteral("课堂作业 · 批阅"), QStringLiteral("📤"), PATRIOTIC_RED_DARK, "rgba(229, 57, 53, 0.12)", QStringLiteral("作业")},
        {QStringLiteral("\"冷战纪录片\" 已添加至资源库"), QStringLiteral("2天前"), QStringLiteral("资源更新 · 视频"), QStringLiteral("📹"), CULTURE_GOLD, "rgba(218, 165, 32, 0.18)", QStringLiteral("资源")}
    };

    for (int i = 0; i < activityData.size(); ++i) {
        const ActivityEntry &entry = activityData[i];

        QString itemObject = QStringLiteral("activityItem_%1").arg(i);
        QFrame *activityItem = new QFrame();
        activityItem->setObjectName(itemObject);
        activityItem->setStyleSheet(QString(
            "QFrame#%1 {"
            "  background: transparent;"
            "  border-radius: 0px;"
            "  border: none;"
            "  padding: 0px;"
            "}"
            "QFrame#%1:hover {"
            "  background: transparent;"
            "  border-radius: 0px;"
            "  border: none;"
            "}"
        ).arg(itemObject));

        QHBoxLayout *activityLayout = new QHBoxLayout(activityItem);
        activityLayout->setSpacing(12);  // 图标与文字之间12px间距
        activityLayout->setContentsMargins(0, 0, 0, 0);  // 移除内边距，避免产生背景区域

        QString iconObject = QStringLiteral("activityIcon_%1").arg(i);
        QFrame *iconWrapper = new QFrame();
        iconWrapper->setObjectName(iconObject);
        iconWrapper->setFixedSize(48, 48);
        iconWrapper->setStyleSheet(QString(
            "QFrame#%1 {"
            "  background: transparent;"
            "  border-radius: 18px;"
            "  border: 1px solid rgba(255, 255, 255, 0.55);"
            "}"
        ).arg(iconObject));

        QVBoxLayout *iconLayout = new QVBoxLayout(iconWrapper);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *iconLabel = new QLabel(entry.icon);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet(QString("color: %1; font-size: 22px;").arg(entry.accentColor));
        iconLayout->addWidget(iconLabel);

        activityLayout->addWidget(iconWrapper);

        QVBoxLayout *contentLayout = new QVBoxLayout();
        contentLayout->setSpacing(4);
        contentLayout->setContentsMargins(0, 0, 0, 0);

        // 标题行 - 14px, #4A4A4A, 600字重
        QLabel *titleLabel = new QLabel(entry.title);
        titleLabel->setWordWrap(true);
        titleLabel->setStyleSheet("color: #4A4A4A; font-size: 14px; font-weight: 600;");

        // 时间行 - 14px, #8B8B8B
        QLabel *timeLabel = new QLabel(entry.time);
        timeLabel->setStyleSheet("color: #8B8B8B; font-size: 14px;");

        contentLayout->addWidget(titleLabel);
        contentLayout->addWidget(timeLabel);

        activityLayout->addLayout(contentLayout, 1);

        // 添加弹性空间，确保文字不被遮挡
        activityLayout->addStretch();

        listLayout->addWidget(activityItem);
    }

    listLayout->addStretch();

    activitiesLayout->addWidget(listContainer);
    // 移除底部留白 - 减少近期活动信息下的空白空间

    recentActivitiesFrame->setMaximumWidth(420);
    recentActivitiesFrame->setMaximumHeight(900);  // 增大最大高度，保证内容显示完整
    recentActivitiesFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}

void ModernMainWindow::createDashboard()
{
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardWidget);
    dashboardLayout->setContentsMargins(24, 24, 24, 24);
    dashboardLayout->setSpacing(20);

    // 创建顶部工具栏
    createHeaderWidget();
    dashboardLayout->addWidget(headerWidget);

    // 创建滚动区域
    dashboardScrollArea = new QScrollArea();
    dashboardScrollArea->setWidgetResizable(true);
    dashboardScrollArea->setStyleSheet("QScrollArea { border: none; background-color: " + BACKGROUND_LIGHT + "; }");

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(56, 44, 56, 64);
    scrollLayout->setSpacing(36);

    // 欢迎标题
    QHBoxLayout *welcomeLayout = new QHBoxLayout();
    welcomeLayout->setSpacing(16);

    welcomeLabel = new QLabel("欢迎回来，王老师！");
    welcomeLabel->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 32px; font-weight: bold;");

    subtitleLabel = new QLabel("这是您的课堂活动与教学工具概览。");
    subtitleLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 16px;");

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(6);
    titleLayout->addWidget(welcomeLabel);
    titleLayout->addWidget(subtitleLabel);

    welcomeLayout->addLayout(titleLayout);
    welcomeLayout->addStretch();

    scrollLayout->addLayout(welcomeLayout);

    
    // 核心功能标题
    QLabel *coreTitle = new QLabel("核心功能");
    coreTitle->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 22px; font-weight: bold;");
    scrollLayout->addWidget(coreTitle);

    // 核心功能卡片与标题之间的紧凑间距
    scrollLayout->addSpacing(-6);  // 使用负间距，让标题和按钮极紧密

    // 核心功能卡片
    createCoreFeatures();
    scrollLayout->addWidget(coreFeaturesFrame);

    // 按顺序创建组件
    createRecentCourses();         // 左列上侧卡片
    createLearningAnalytics();     // 左列下侧卡片
    createRecentActivities();      // 右列侧栏卡片

    // 创建两列网格：左列堆叠两个卡片，右列一个侧栏
    QFrame *dashboardGridFrame = new QFrame();
    QGridLayout *grid = new QGridLayout(dashboardGridFrame);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(36);
    grid->setVerticalSpacing(36);
    grid->setColumnStretch(0, 2);   // 左列：占2份宽度（近期课程+学情分析垂直堆叠）
    grid->setColumnStretch(1, 1);   // 右列：占1份宽度（近期活动侧栏）

    // 左列：垂直堆叠容器
    QFrame *leftStackFrame = new QFrame();
    QVBoxLayout *leftStack = new QVBoxLayout(leftStackFrame);
    leftStack->setContentsMargins(0, 0, 0, 0);
    leftStack->setSpacing(36);
    leftStack->addWidget(recentCoursesFrame);
    leftStack->addWidget(learningAnalyticsFrame);

    // 右列：近期活动卡片 + 图表
    QVBoxLayout *rightStack = new QVBoxLayout();
    rightStack->setContentsMargins(0, 0, 0, 0);
    rightStack->setSpacing(32);
    rightStack->setAlignment(Qt::AlignTop);

    if (recentActivitiesFrame) {
        rightStack->addWidget(recentActivitiesFrame, 0);  // stretch factor = 0，防止过长扩展
    }

    // 移除图表容器（三维评分对比和知识点掌握分布）
    // if (chartsContainer) {
    //     rightStack->addWidget(chartsContainer, 0);  // stretch factor = 0
    // }

    // 放入网格
    grid->addWidget(leftStackFrame, 0, 0, Qt::AlignTop | Qt::AlignLeft);

    // 为右侧布局创建一个widget容器
    QWidget *rightWidget = new QWidget();
    rightWidget->setLayout(rightStack);
    grid->addWidget(rightWidget, 0, 1, Qt::AlignTop | Qt::AlignLeft);

    // 配置网格行拉伸以支持高度分布
    grid->setRowStretch(0, 1);     // 允许行垂直拉伸

    // 设置容器大小策略以支持拉伸
    leftStackFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 添加到滚动布局
    scrollLayout->addWidget(dashboardGridFrame);

    // 设置滚动布局间距
    scrollLayout->setSpacing(36);

    // 不在底部重复显示近期活动

    scrollLayout->addStretch();

    dashboardScrollArea->setWidget(scrollContent);
    dashboardLayout->addWidget(dashboardScrollArea);
}

void ModernMainWindow::setupStyles()
{
    // 应用整体样式
    this->setStyleSheet(R"(
        QMainWindow {
            background: )" + WINDOW_BACKGROUND_GRADIENT + R"(;
            font-family: "PingFang SC", -apple-system, sans-serif;
        }
        QMenuBar {
            background-color: )" + CARD_WHITE + R"(;
            color: )" + PRIMARY_TEXT + R"(;
            font-size: 14px;
            border-bottom: 1px solid )" + SEPARATOR + R"(;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 16px;
        }
        QMenuBar::item:selected {
            background-color: rgba(0, 0, 0, 0.05);
        }
        QStatusBar {
            background-color: )" + CARD_WHITE + R"(;
            color: )" + SECONDARY_TEXT + R"(;
            font-size: 12px;
            border-top: 1px solid )" + SEPARATOR + R"(;
        }
        QScrollArea {
            background-color: )" + BACKGROUND_LIGHT + R"(;
            border: none;
        }
        QScrollBar:vertical {
            background-color: #F0F0F0;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: )" + SECONDARY_TEXT + R"(;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: )" + PRIMARY_TEXT + R"(;
        }
        QScrollBar:horizontal {
            background-color: #F0F0F0;
            height: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal {
            background-color: )" + SECONDARY_TEXT + R"(;
            border-radius: 4px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: )" + PRIMARY_TEXT + R"(;
        }
    )");
}

void ModernMainWindow::applyPatrioticRedTheme()
{
    // 最终决定：QtTheme与现有系统不兼容，保持原样
    qDebug() << "保持现有样式，不应用QtTheme以确保系统稳定";

    // 确保主题一致性
    this->update();
}

// 槽函数实现
void ModernMainWindow::onTeacherCenterClicked()
{
    // 重置所有按钮样式
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
      teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    contentStack->setCurrentWidget(dashboardWidget);
    this->statusBar()->showMessage("教师中心");
}

void ModernMainWindow::onContentAnalysisClicked()
{
    onTeacherCenterClicked();
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    this->statusBar()->showMessage("智能内容分析");
}

void ModernMainWindow::onAIPreparationClicked()
{
    qDebug() << "AI智能备课按钮被点击";

    // 重置所有按钮样式
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
      teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));

    // 切换到AI智能备课页面
    if (aiPreparationWidget) {
        qDebug() << "切换到AI智能备课页面";
        contentStack->setCurrentWidget(aiPreparationWidget);
        this->statusBar()->showMessage("AI智能备课");
    } else {
        qDebug() << "错误：aiPreparationWidget为空";
    }
}

void ModernMainWindow::onResourceManagementClicked()
{
    qDebug() << "试题库按钮被点击";

    // 重置所有按钮样式
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
      teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));

    // 切换到试题库页面
    if (questionBankWindow) {
        qDebug() << "切换到试题库页面";
        contentStack->setCurrentWidget(questionBankWindow);
        this->statusBar()->showMessage("试题库");
    } else {
        qDebug() << "错误：questionBankWindow为空";
    }
}

void ModernMainWindow::onLearningAnalysisClicked()
{
    onTeacherCenterClicked();
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    this->statusBar()->showMessage("学情与教评");
}


void ModernMainWindow::onSettingsClicked()
{
    QMessageBox::information(this, "系统设置", "系统设置功能正在开发中...");
}

void ModernMainWindow::onHelpClicked()
{
    QMessageBox::information(this, "帮助中心", "帮助中心功能正在开发中...");
}

void ModernMainWindow::onQuickPreparationClicked()
{
    QMessageBox::information(this, "快速备课", "快速备课功能正在开发中...");
}

void ModernMainWindow::onStartClassClicked()
{
    QMessageBox::information(this, "开始授课", "开始授课功能正在开发中...");
}

void ModernMainWindow::onEnterClassClicked()
{
    QMessageBox::information(this, "进入课堂", "进入课堂功能正在开发中...");
}
