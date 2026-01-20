#include "modernmainwindow.h"
#include "../auth/login/simpleloginwindow.h"
#include "../ui/aipreparationwidget.h"
#include "../questionbank/QuestionRepository.h"
#include "../questionbank/questionbankwindow.h"
#include "../services/DifyService.h"
#include "../services/PPTXGenerator.h"
#include "../ui/AIChatDialog.h"
#include "../ui/ChatWidget.h"
#include "../ui/ChatHistoryWidget.h"
#include "../ui/HotspotTrackingWidget.h"
#include "../services/HotspotService.h"
#include "../hotspot/RealNewsProvider.h"
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QRegularExpression>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
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
#include <QQmlError>
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
#include <QScrollBar>
#include <QSharedPointer>
#include <QDialog>
#include <QFileDialog>
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

    // 初始化 Dify AI 服务
    m_difyService = new DifyService(this);
    
    // 初始化 PPTX 生成器
    m_pptxGenerator = new PPTXGenerator(this);

    // 初始化 PPT 模拟生成定时器
    m_pptSimulationTimer = new QTimer(this);
    m_pptSimulationTimer->setSingleShot(false);
    m_pptSimulationStep = 0;
    m_pptQuestionStep = 0;  // 0=未开始问答
    m_pendingPPTPath = "";
    connect(m_pptSimulationTimer, &QTimer::timeout, this, &ModernMainWindow::onPPTSimulationStep);

    // 初始化打字效果定时器
    m_pptTypingTimer = new QTimer(this);
    m_pptTypingTimer->setSingleShot(false);
    m_pptTypingIndex = 0;
    connect(m_pptTypingTimer, &QTimer::timeout, this, &ModernMainWindow::onPPTTypingStep);

    // 初始化流式更新节流定时器（每100ms最多更新一次UI）
    m_streamUpdateTimer = new QTimer(this);
    m_streamUpdateTimer->setSingleShot(true);
    m_streamUpdatePending = false;
    connect(m_streamUpdateTimer, &QTimer::timeout, this, [this]() {
        if (m_streamUpdatePending && m_bubbleChatWidget) {
            m_bubbleChatWidget->updateLastAIMessage(m_currentAIResponse);
            m_streamUpdatePending = false;
        }
    });

    // API Key 获取优先级：环境变量 > 本地配置文件
    QString apiKey = qgetenv("DIFY_API_KEY");

    if (apiKey.isEmpty()) {
        // 尝试从本地配置文件读取（此文件不提交到 Git）
        // macOS: .app/Contents/MacOS/ -> 需要往上 4 级到项目根目录
        QString configPath = QCoreApplication::applicationDirPath() + "/../../../../.env.local";
        QFile envFile(configPath);
        if (envFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while (!envFile.atEnd()) {
                QString line = QString::fromUtf8(envFile.readLine()).trimmed();
                if (line.startsWith("DIFY_API_KEY=")) {
                    apiKey = line.mid(13);  // 跳过 "DIFY_API_KEY="
                    qDebug() << "[Info] Dify API Key loaded from .env.local";
                    break;
                }
            }
            envFile.close();
        }
    } else {
        qDebug() << "[Info] Dify API Key loaded from environment variable.";
    }

    const bool hasApiKey = !apiKey.isEmpty();
    if (hasApiKey) {
        m_difyService->setApiKey(apiKey);
    } else {
        qDebug() << "[Warning] No API Key found. AI features will be disabled.";
        qDebug() << "[Info] Create .env.local file with: DIFY_API_KEY=your-key";
    }

    // 不再使用独立的 AI 对话框，直接在主页面显示
    // m_chatDialog = new AIChatDialog(m_difyService, this);

    // 连接 Dify 服务的信号到主窗口的处理函数
    connect(m_difyService, &DifyService::streamChunkReceived, this, &ModernMainWindow::onAIStreamChunk);
    connect(m_difyService, &DifyService::thinkingChunkReceived, this, &ModernMainWindow::onAIThinkingChunk);
    connect(m_difyService, &DifyService::messageReceived, this, &ModernMainWindow::onAIResponseReceived);
    connect(m_difyService, &DifyService::errorOccurred, this, &ModernMainWindow::onAIError);
    connect(m_difyService, &DifyService::requestStarted, this, &ModernMainWindow::onAIRequestStarted);
    connect(m_difyService, &DifyService::requestFinished, this, &ModernMainWindow::onAIRequestFinished);

    initUI();
    setupMenuBar();
    setupStatusBar();
    setupCentralWidget();
    setupStyles();
    applyPatrioticRedTheme();

    // 创建默认页面
    createDashboard();
    contentStack->setCurrentWidget(dashboardWidget);

    if (!hasApiKey) {
        QTimer::singleShot(0, this, [this]() {
            if (statusBar()) {
                statusBar()->showMessage("未设置 DIFY_API_KEY：AI 功能暂不可用（可正常使用其他页面）", 8000);
            }
        });
    }

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
    newsTrackingBtn = new QPushButton("时政新闻");
    aiPreparationBtn = new QPushButton("AI智能备课");
    resourceManagementBtn = new QPushButton("试题库");
    learningAnalysisBtn = new QPushButton("学情与教评");
  
    // 底部按钮
    settingsBtn = new QPushButton("系统设置");
    helpBtn = new QPushButton("帮助中心");

    // 确保所有按钮都可见
    teacherCenterBtn->setVisible(true);
    newsTrackingBtn->setVisible(true);
    aiPreparationBtn->setVisible(true);
    resourceManagementBtn->setVisible(true);
    learningAnalysisBtn->setVisible(true);
    settingsBtn->setVisible(true);
    helpBtn->setVisible(true);

    applySidebarIcons();

    // 设置侧边栏按钮样式 - 使用统一样式常量
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    newsTrackingBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    settingsBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    helpBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));

    // 连接信号
    connect(teacherCenterBtn, &QPushButton::clicked, this, [=]() { qDebug() << "教师中心按钮被点击"; onTeacherCenterClicked(); });
    connect(newsTrackingBtn, &QPushButton::clicked, this, [=]() { qDebug() << "时政新闻按钮被点击"; onNewsTrackingClicked(); });
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
    sidebarLayout->addWidget(newsTrackingBtn);
    sidebarLayout->addWidget(aiPreparationBtn);
    sidebarLayout->addWidget(resourceManagementBtn);
    sidebarLayout->addWidget(learningAnalysisBtn);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(settingsBtn);
    sidebarLayout->addWidget(helpBtn);

    // 创建侧边栏堆栈（用于在导航和历史记录之间切换）
    m_sidebarStack = new QStackedWidget();
    m_sidebarStack->setMinimumWidth(240);
    m_sidebarStack->setMaximumWidth(300);
    m_sidebarStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_sidebarStack->addWidget(sidebar);  // 页面0：导航侧边栏
    
    // 创建历史记录侧边栏（将在createAIChatWidget中配置信号）
    m_chatHistoryWidget = new ChatHistoryWidget();
    m_sidebarStack->addWidget(m_chatHistoryWidget);  // 页面1：历史记录侧边栏
    
    // 确保初始显示导航侧边栏
    m_sidebarStack->setCurrentIndex(0);

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

    // 创建时政新闻页面
    m_hotspotService = new HotspotService(this);
    RealNewsProvider *newsProvider = new RealNewsProvider(this);
    // 从环境变量读取天行数据 API Key
    QString tianxingKey = qEnvironmentVariable("TIANXING_API_KEY");
    if (!tianxingKey.isEmpty()) {
        newsProvider->setTianXingApiKey(tianxingKey);
        qDebug() << "[ModernMainWindow] 天行数据 API Key 已配置";
    } else {
        qWarning() << "[ModernMainWindow] TIANXING_API_KEY 未设置，将使用 RSS 源（可能是旧数据）";
    }
    m_hotspotService->setNewsProvider(newsProvider);

    m_hotspotWidget = new HotspotTrackingWidget(this);
    m_hotspotWidget->setHotspotService(m_hotspotService);
    m_hotspotWidget->setDifyService(m_difyService);
    contentStack->addWidget(m_hotspotWidget);

    // 连接试题库返回信号
    connect(questionBankWindow, &QuestionBankWindow::backRequested, this, [this]() {
        // 返回首页（教师中心）
        if (contentStack && dashboardWidget) {
            contentStack->setCurrentWidget(dashboardWidget);
        }
        if (m_sidebarStack) {
            m_sidebarStack->setCurrentIndex(0);
        }
        if (teacherCenterBtn) {
            teacherCenterBtn->setChecked(true);
        }
    });

    // 添加到主布局
    contentLayout->addWidget(m_sidebarStack);  // 使用侧边栏堆栈
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
    setIcon(newsTrackingBtn, "view-statistics", QStyle::SP_FileDialogContentsView);
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


// 旧版核心功能卡片 - 已废弃
void ModernMainWindow::createCoreFeatures() {}

// 旧版近期课程 - 已废弃
void ModernMainWindow::createRecentCourses() {}

// 旧版学情分析 - 已废弃
void ModernMainWindow::createLearningAnalytics() {}

// 旧版近期活动 - 已废弃
void ModernMainWindow::createRecentActivities() {}

// 创建指标项组件 - 紧凑的单行信息
void ModernMainWindow::createDashboard()
{
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardWidget);
    dashboardLayout->setContentsMargins(0, 0, 0, 0);
    dashboardLayout->setSpacing(0);

    // 创建顶部工具栏
    createHeaderWidget();
    dashboardLayout->addWidget(headerWidget);

    // ========== 主内容区域 ==========
    QWidget *contentArea = new QWidget();

    // ========== 欢迎面板（默认显示）==========
    m_welcomePanel = new QWidget();
    m_welcomePanel->setObjectName("welcomePanel");
    m_isConversationStarted = false;

    QVBoxLayout *welcomeLayout = new QVBoxLayout(m_welcomePanel);
    welcomeLayout->setContentsMargins(40, 60, 40, 40);
    welcomeLayout->setSpacing(30);
    welcomeLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // 顶部图标
    QLabel *iconLabel = new QLabel();
    iconLabel->setFixedSize(64, 64);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(R"(
        QLabel {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fecaca, stop:1 #fca5a5);
            border-radius: 16px;
            font-size: 32px;
        }
    )");
    iconLabel->setText("🎓");
    
    // 标题
    QLabel *titleLabel = new QLabel("思政智慧课堂助手");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 28px;
            font-weight: bold;
            color: #1a1a1a;
            background: transparent;
        }
    )");

    // 副标题
    QLabel *subtitleLabel = new QLabel("协助教师备课、学情分析及教学资源管理");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 15px;
            color: #6b7280;
            background: transparent;
        }
    )");

    // 功能卡片容器
    QWidget *cardsContainer = new QWidget();
    QGridLayout *cardsLayout = new QGridLayout(cardsContainer);
    cardsLayout->setContentsMargins(0, 20, 0, 0);
    cardsLayout->setSpacing(16);

    // 创建四个功能卡片
    // 创建功能卡片 - 带有颜色图标背景和悬停效果
    auto createFeatureCard = [this](const QString &icon, const QString &title, const QString &desc, const QString &iconBgColor) -> QPushButton* {
        QPushButton *card = new QPushButton();
        card->setFixedSize(320, 100); // 保持较大的按钮尺寸
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(QString(R"(
            QPushButton {
                background-color: #ffffff;
                border: 1px solid #e5e7eb;
                border-radius: 16px;
                text-align: left;
            }
            QPushButton:hover {
                background-color: #fafafa;
                border-color: #d1d5db;
            }
            QPushButton:pressed {
                background-color: #f5f5f5;
                border-color: #c0c0c0;
            }
        )"));

        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(16, 12, 16, 12);
        cardLayout->setSpacing(14);

        // 图标容器 - 带彩色背景
        QLabel *iconLbl = new QLabel(icon);
        iconLbl->setFixedSize(44, 44);
        iconLbl->setAlignment(Qt::AlignCenter);
        iconLbl->setStyleSheet(QString(R"(
            QLabel {
                background-color: %1;
                border-radius: 10px;
                font-size: 22px;
            }
        )").arg(iconBgColor));

        // 文字区域
        QWidget *textArea = new QWidget();
        textArea->setStyleSheet("background: transparent;");
        QVBoxLayout *textLayout = new QVBoxLayout(textArea);
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(4);

        QLabel *titleLbl = new QLabel(title);
        titleLbl->setStyleSheet("font-size: 15px; font-weight: 600; color: #1f2937; background: transparent;");
        
        QLabel *descLbl = new QLabel(desc);
        descLbl->setStyleSheet("font-size: 12px; color: #9ca3af; background: transparent;");

        textLayout->addWidget(titleLbl);
        textLayout->addWidget(descLbl);

        cardLayout->addWidget(iconLbl);
        cardLayout->addWidget(textArea, 1);

        // 添加阴影效果
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(12);
        shadow->setColor(QColor(0, 0, 0, 25));
        shadow->setOffset(0, 2);
        card->setGraphicsEffect(shadow);

        return card;
    };

    // 四个功能卡片，使用不同的柔和背景色
    QPushButton *card1 = createFeatureCard("📰", "时政新闻", "实时追踪时政热点，把握教学方向", "#fef3c7");  // 淡黄
    QPushButton *card2 = createFeatureCard("📝", "AI智能备课", "一键生成PPT", "#fce7f3");  // 淡粉
    QPushButton *card3 = createFeatureCard("📚", "试题库", "海量思政习题，智能组卷测评", "#dbeafe");  // 淡蓝
    QPushButton *card4 = createFeatureCard("📈", "数据分析报告", "可视化展示教学成果与趋势", "#d1fae5");  // 淡绿

    // 连接卡片点击事件
    connect(card1, &QPushButton::clicked, this, &ModernMainWindow::onNewsTrackingClicked);
    connect(card2, &QPushButton::clicked, this, &ModernMainWindow::onAIPreparationClicked);
    connect(card3, &QPushButton::clicked, this, &ModernMainWindow::onResourceManagementClicked);
    connect(card4, &QPushButton::clicked, this, &ModernMainWindow::onLearningAnalysisClicked);

    cardsLayout->addWidget(card1, 0, 0);
    cardsLayout->addWidget(card2, 0, 1);
    cardsLayout->addWidget(card3, 1, 0);
    cardsLayout->addWidget(card4, 1, 1);

    welcomeLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    welcomeLayout->addWidget(titleLabel);
    welcomeLayout->addWidget(subtitleLabel);
    welcomeLayout->addWidget(cardsContainer, 0, Qt::AlignHCenter);
    welcomeLayout->addStretch();

    // 设置背景透明，使按钮缝隙显示为主页面背景色
    m_welcomePanel->setStyleSheet("QWidget#welcomePanel { background-color: transparent; }");

    // ========== AI 对话组件 ==========
    createAIChatWidget();

    // 主布局
    QVBoxLayout *contentAreaLayout = new QVBoxLayout(contentArea);
    contentAreaLayout->setContentsMargins(0, 0, 0, 0);
    contentAreaLayout->setSpacing(0);

    // 使用 QStackedWidget 来切换欢迎面板和聊天消息区域
    m_mainStack = new QStackedWidget();
    m_mainStack->setObjectName("mainContentStack");
    m_mainStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 页面0：欢迎面板（不含输入框）
    m_mainStack->addWidget(m_welcomePanel);
    
    // 页面1：聊天容器（侧边栏 + 聊天组件）
    if (m_chatContainer) {
        m_chatContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_mainStack->addWidget(m_chatContainer);
        
        // 监听消息发送，开始对话后切换到聊天页面
        connect(m_bubbleChatWidget, &ChatWidget::messageSent, this, [this](const QString &message) {
            Q_UNUSED(message);
            if (!m_isConversationStarted) {
                m_isConversationStarted = true;
            }
            // 每次发送消息都确保显示聊天页面
            m_mainStack->setCurrentWidget(m_chatContainer);
            // 隐藏欢迎页面输入框
            if (m_welcomeInputWidget) {
                m_welcomeInputWidget->hide();
            }
        });
    }
    
    // 默认显示欢迎面板
    m_mainStack->setCurrentWidget(m_welcomePanel);
    
    contentAreaLayout->addWidget(m_mainStack, 1);
    
    // ========== 底部独立输入框（欢迎页面时显示）==========
    m_welcomeInputWidget = new QWidget();
    m_welcomeInputWidget->setObjectName("welcomeInputWidget");
    m_welcomeInputWidget->setFixedHeight(100);
    m_welcomeInputWidget->setStyleSheet("QWidget#welcomeInputWidget { background-color: #f5f7fa; }");
    
    QVBoxLayout *welcomeInputLayout = new QVBoxLayout(m_welcomeInputWidget);
    welcomeInputLayout->setContentsMargins(40, 10, 40, 20);
    welcomeInputLayout->setSpacing(8);
    
    // 输入框容器
    QFrame *inputContainer = new QFrame();
    inputContainer->setObjectName("welcomeInputContainer");
    inputContainer->setFixedHeight(56);
    inputContainer->setStyleSheet(R"(
        QFrame#welcomeInputContainer {
            background-color: #ffffff;
            border-radius: 28px;
            border: 1px solid #e5e7eb;
        }
    )");
    
    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 20));
    shadow->setOffset(0, 4);
    inputContainer->setGraphicsEffect(shadow);
    
    QHBoxLayout *inputLayout = new QHBoxLayout(inputContainer);
    inputLayout->setContentsMargins(12, 8, 12, 8);
    inputLayout->setSpacing(12);
    
    // 加号按钮
    QPushButton *plusBtn = new QPushButton("+");
    plusBtn->setFixedSize(32, 32);
    plusBtn->setCursor(Qt::PointingHandCursor);
    plusBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #9ca3af;
            color: #ffffff;
            border-radius: 16px;
            border: none;
            font-size: 20px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #6b7280; }
    )");
    
    // 输入框
    QLineEdit *welcomeInput = new QLineEdit();
    welcomeInput->setPlaceholderText("向AI助手发送信息...");
    welcomeInput->setFixedHeight(40);
    welcomeInput->setStyleSheet(R"(
        QLineEdit {
            background-color: transparent;
            border: none;
            padding: 0 10px;
            font-size: 15px;
            color: #1a1a1a;
        }
        QLineEdit::placeholder { color: #9ca3af; }
    )");
    
    // 发送按钮
    QPushButton *sendBtn = new QPushButton("↑");
    sendBtn->setFixedSize(32, 32);
    sendBtn->setCursor(Qt::PointingHandCursor);
    sendBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #dc2626;
            color: #ffffff;
            border: none;
            border-radius: 16px;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #b91c1c; }
    )");
    
    inputLayout->addWidget(plusBtn);
    inputLayout->addWidget(welcomeInput, 1);
    inputLayout->addWidget(sendBtn);
    
    // 提示文字
    QLabel *tipLabel = new QLabel("AI可能产生错误信息，请核实重要内容。");
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setStyleSheet("color: #9ca3af; font-size: 12px;");
    
    welcomeInputLayout->addWidget(inputContainer);
    welcomeInputLayout->addWidget(tipLabel);
    
    contentAreaLayout->addWidget(m_welcomeInputWidget);
    
    // 连接欢迎页面输入框的发送功能
    auto sendFromWelcome = [this, welcomeInput]() {
        QString text = welcomeInput->text().trimmed();
        if (text.isEmpty()) return;
        
        // 切换到聊天页面
        m_isConversationStarted = true;
        m_mainStack->setCurrentWidget(m_chatContainer);
        swapToHistorySidebar();  // 切换到历史记录侧边栏
        m_welcomeInputWidget->hide();
        
        // 在聊天组件中设置文本并发送
        if (m_bubbleChatWidget) {
            m_bubbleChatWidget->setInputText(text);
            // 触发发送（模拟回车）
            QTimer::singleShot(50, [this]() {
                if (m_bubbleChatWidget) {
                    // 手动触发发送
                    QString msg = m_bubbleChatWidget->inputText();
                    if (!msg.isEmpty()) {
                        m_bubbleChatWidget->clearInput();
                        emit m_bubbleChatWidget->messageSent(msg);
                    }
                }
            });
        }
        
        welcomeInput->clear();
    };
    
    connect(sendBtn, &QPushButton::clicked, this, sendFromWelcome);
    connect(welcomeInput, &QLineEdit::returnPressed, this, sendFromWelcome);
    
    // 当切换到聊天页面时，隐藏欢迎输入框
    connect(m_mainStack, &QStackedWidget::currentChanged, this, [this](int index) {
        // index 0 = 欢迎页面, index 1 = 聊天页面
        if (m_welcomeInputWidget) {
            m_welcomeInputWidget->setVisible(index == 0);
        }
    });

    dashboardLayout->addWidget(contentArea, 1);
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
    newsTrackingBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    contentStack->setCurrentWidget(dashboardWidget);
    
    // 回到欢迎页面，显示欢迎面板和输入框
    if (m_mainStack && m_welcomePanel) {
        m_mainStack->setCurrentWidget(m_welcomePanel);
    }
    if (m_welcomeInputWidget) {
        m_welcomeInputWidget->show();
    }
    // 重置对话状态（可选：如果你想保留对话历史，可以注释掉下面这行）
    // m_isConversationStarted = false;
    
    this->statusBar()->showMessage("教师中心");
}

void ModernMainWindow::onAIPreparationClicked()
{
    qDebug() << "AI智能备课按钮被点击";

    // 重置所有按钮样式
    newsTrackingBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));

    // 首先切换 contentStack 到 dashboardWidget（m_mainStack 在其中）
    if (contentStack && dashboardWidget) {
        contentStack->setCurrentWidget(dashboardWidget);
    }

    // 然后切换到对话页面
    qDebug() << "切换到AI对话页面";
    if (m_mainStack && m_chatContainer) {
        m_mainStack->setCurrentWidget(m_chatContainer);
        swapToHistorySidebar();  // 切换到历史记录侧边栏
    }
    this->statusBar()->showMessage("AI智能备课");
}

void ModernMainWindow::onResourceManagementClicked()
{
    qDebug() << "试题库按钮被点击";

    // 重置所有按钮样式
    newsTrackingBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
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

void ModernMainWindow::onNewsTrackingClicked()
{
    qDebug() << "切换到时政新闻页面";

    // 重置所有侧边栏按钮样式
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    newsTrackingBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    // 切换到时政新闻页面
    if (contentStack && m_hotspotWidget) {
        contentStack->setCurrentWidget(m_hotspotWidget);
        // 首次进入时刷新数据
        m_hotspotWidget->refresh();
    }

    // 确保显示导航侧边栏
    if (m_sidebarStack) {
        m_sidebarStack->setCurrentIndex(0);
    }

    this->statusBar()->showMessage("时政新闻追踪");
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

// ==================== 新版 UI 组件实现 ====================

void ModernMainWindow::createWelcomeCard()
{
    // 1. 创建行容器 (Row Widget)
    welcomeCard = new QFrame();
    welcomeCard->setObjectName("welcomeRow");
    welcomeCard->setStyleSheet("background: transparent;");
    
    QHBoxLayout *rowLayout = new QHBoxLayout(welcomeCard);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(16);
    rowLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    // 2. 左侧头像 (红色圆形)
    QLabel *avatar = new QLabel("✨");
    avatar->setFixedSize(40, 40);
    avatar->setStyleSheet(
        "background: #e53935;"
        "border-radius: 20px;"
        "color: white;"
        "font-size: 20px;"
        "qproperty-alignment: AlignCenter;"
    );
    rowLayout->addWidget(avatar, 0, Qt::AlignTop);
    
    // 3. 右侧气泡 (白色圆角)
    QFrame *bubble = new QFrame();
    bubble->setObjectName("welcomeBubble");
    bubble->setStyleSheet(
        "QFrame#welcomeBubble {"
        "   background-color: white;"
        "   border-radius: 10px;"
        "   border: 1px solid #e0e0e0;"
        "}"
    );
    
    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(24, 24, 24, 24);
    bubbleLayout->setSpacing(16);
    
    // 欢迎语
    QLabel *title = new QLabel("欢迎回来，" + currentUsername + "！");
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: " + PRIMARY_TEXT + ";");
    
    QLabel *desc = new QLabel("我是您的AI教学助手，很高兴为您服务。您可以直接向我提问，或者点击下方的快捷功能按钮，快速开始您的教学工作。\n今天您有什么教学计划？比如：");
    desc->setWordWrap(true);
    desc->setStyleSheet("font-size: 15px; color: " + SECONDARY_TEXT + "; line-height: 1.6;");
    
    bubbleLayout->addWidget(title);
    bubbleLayout->addWidget(desc);
    
    // 智能建议列表
    QStringList suggestions = {
        "开始智能备课",
        "查看近期课程安排",
        "分析高二(2)班的学情",
        "查找关于“当代思潮”的教学资源"
    };
    
    for (const QString &text : suggestions) {
        QPushButton *btn = new QPushButton("• " + text);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton {"
            "   text-align: left;"
            "   border: none;"
            "   background: transparent;"
            "   color: #e53935;"
            "   font-size: 15px;"
            "   font-weight: 500;"
            "   padding: 4px 0;"
            "}"
            "QPushButton:hover {"
            "   text-decoration: underline;"
            "}"
        );
        bubbleLayout->addWidget(btn);
        
        connect(btn, &QPushButton::clicked, this, [this, text]() {
            QString query = text;
            if (query.startsWith("• ")) query = query.mid(2);

            qDebug() << "[ModernMainWindow] Smart suggestion clicked:" << query;

            if (!m_bubbleChatWidget) {
                qDebug() << "[ModernMainWindow] Error: m_bubbleChatWidget is null!";
                return;
            }

            if (!m_bubbleChatWidget->inputText().isEmpty()) {
                qDebug() << "[ModernMainWindow] Warning: Input not empty, current text:" << m_bubbleChatWidget->inputText();
            }

            m_bubbleChatWidget->setInputText(query);
            m_bubbleChatWidget->focusInput();

            qDebug() << "[ModernMainWindow] Smart suggestion processed successfully";
        });
    }
    
    rowLayout->addWidget(bubble);
    rowLayout->addStretch(); // 确保气泡靠左，不占满全宽
}

void ModernMainWindow::createQuickAccessCard()
{
    // 1. 创建行容器
    quickAccessCard = new QFrame();
    quickAccessCard->setObjectName("quickAccessRow");
    quickAccessCard->setStyleSheet("background: transparent;");
    
    QHBoxLayout *rowLayout = new QHBoxLayout(quickAccessCard);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(16);
    rowLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    // 2. 左侧头像 (红色圆形)
    QLabel *avatar = new QLabel("✨");
    avatar->setFixedSize(40, 40);
    avatar->setStyleSheet(
        "background: #e53935;"
        "border-radius: 20px;"
        "color: white;"
        "font-size: 20px;"
        "qproperty-alignment: AlignCenter;"
    );
    rowLayout->addWidget(avatar, 0, Qt::AlignTop);
    
    // 3. 右侧气泡
    QFrame *bubble = new QFrame();
    bubble->setObjectName("quickAccessBubble");
    bubble->setStyleSheet(
        "QFrame#quickAccessBubble {"
        "   background-color: white;"
        "   border-radius: 10px;"
        "   border: 1px solid #e0e0e0;"
        "}"
    );
    
    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(24, 24, 24, 24);
    bubbleLayout->setSpacing(20);
    
    // 标题 (移入气泡内)
    QLabel *title = new QLabel("这里是您的核心功能快捷入口：");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: " + PRIMARY_TEXT + ";");
    bubbleLayout->addWidget(title);
    
    // 按钮网格
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(16);
    
    struct QuickAction {
        QString title;
        QString icon;
        QString color;
    };
    
    QList<QuickAction> actions = {
        {"智能内容分析", "🔍", "#f5f5f5"},
        {"AI智能备课", "📝", "#f5f5f5"},
        {"互动教学工具", "▶️", "#f5f5f5"},
        {"资源库管理", "📂", "#f5f5f5"}
    };
    
    int row = 0;
    int col = 0;
    
    for (const auto &action : actions) {
        QPushButton *card = new QPushButton();
        card->setFixedSize(220, 70); // 稍微调小一点以适应气泡
        card->setCursor(Qt::PointingHandCursor);
        
        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(16, 0, 16, 0);
        cardLayout->setSpacing(12);
        
        QLabel *icon = new QLabel(action.icon);
        icon->setStyleSheet("font-size: 22px; background: transparent;");
        
        QLabel *text = new QLabel(action.title);
        text->setStyleSheet("font-size: 15px; font-weight: 600; color: " + PRIMARY_TEXT + "; background: transparent;");
        
        cardLayout->addWidget(icon);
        cardLayout->addWidget(text);
        cardLayout->addStretch();
        
        card->setStyleSheet(
            "QPushButton {"
            "   background-color: " + action.color + ";"
            "   border-radius: 10px;"
            "   border: none;"
            "}"
            "QPushButton:hover {"
            "   background-color: #eeeeee;"
            "}"
        );
        
        gridLayout->addWidget(card, row, col);
        
        col++;
        if (col > 1) {
            col = 0;
            row++;
        }
    }
    
    bubbleLayout->addLayout(gridLayout);
    
    rowLayout->addWidget(bubble);
    rowLayout->addStretch();
}

// ==================== AI 对话功能实现 ====================

void ModernMainWindow::createAIChatWidget()
{
    // 创建聊天容器（仅聊天组件，侧边栏在主布局中切换）
    m_chatContainer = new QWidget();
    QHBoxLayout *containerLayout = new QHBoxLayout(m_chatContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    
    // 连接历史记录侧边栏信号（m_chatHistoryWidget 已在 setupCentralWidget 中创建）
    connect(m_chatHistoryWidget, &ChatHistoryWidget::newChatRequested, this, [this]() {
        // 步骤 1: 如果当前有对话，先刷新历史列表（Dify 云端已自动保存）
        if (m_isConversationStarted && m_difyService) {
            // 请求刷新对话列表，让刚才的对话出现在历史记录中
            m_difyService->fetchConversations();
            qDebug() << "[ModernMainWindow] 新建对话 - 刷新历史记录列表";
        }

        // 步骤 2: 清空聊天并重置 Dify 会话
        if (m_bubbleChatWidget) {
            m_bubbleChatWidget->clearMessages();
            m_bubbleChatWidget->addMessage("老师您好！我是智慧课堂助手，请问有什么可以帮你？", false);
        }
        if (m_difyService) {
            m_difyService->clearConversation();
        }

        // 步骤 3: 清除选中状态
        if (m_chatHistoryWidget) {
            m_chatHistoryWidget->clearSelection();
        }

        // 步骤 4: 重置对话开始标志
        m_isConversationStarted = false;
    });
    
    connect(m_chatHistoryWidget, &ChatHistoryWidget::backRequested, this, [this]() {
        // 返回欢迎页面并恢复导航侧边栏
        if (m_mainStack && m_welcomePanel) {
            m_mainStack->setCurrentWidget(m_welcomePanel);
            if (m_welcomeInputWidget) m_welcomeInputWidget->show();
        }
        swapToNavSidebar();
        m_isConversationStarted = false;
    });
    
    connect(m_chatHistoryWidget, &ChatHistoryWidget::historyItemSelected, this, [this](const QString &id) {
        // 加载选中对话的消息历史
        if (m_difyService) {
            m_difyService->fetchMessages(id);
            // 设置当前会话ID以便继续对话
            // (需要在收到消息后处理)
        }
    });
    
    // 连接对话列表接收信号
    connect(m_difyService, &DifyService::conversationsReceived, this, [this](const QJsonArray &conversations) {
        if (!m_chatHistoryWidget) return;
        
        m_chatHistoryWidget->clearHistory();
        
        for (const QJsonValue &val : conversations) {
            QJsonObject conv = val.toObject();
            QString id = conv["id"].toString();
            QString name = conv["name"].toString();
            if (name.isEmpty()) {
                // 如果没有名称，使用对话ID的前几个字符
                name = QString("对话 %1").arg(id.left(8));
            }
            
            // 获取更新时间并格式化
            qint64 updatedAt = conv["updated_at"].toVariant().toLongLong();
            QString timeStr;
            if (updatedAt > 0) {
                QDateTime dt = QDateTime::fromSecsSinceEpoch(updatedAt);
                timeStr = dt.toString("M月d日 HH:mm");
            } else {
                timeStr = "未知时间";
            }
            
            m_chatHistoryWidget->addHistoryItem(id, name, timeStr);
        }
        
        qDebug() << "[ModernMainWindow] Loaded" << conversations.size() << "conversations";
    });
    
    // 连接消息历史接收信号
    connect(m_difyService, &DifyService::messagesReceived, this, [this](const QJsonArray &messages, const QString &conversationId) {
        if (!m_bubbleChatWidget) return;
        
        m_bubbleChatWidget->clearMessages();
        
        // 消息是按时间倒序的，需要反转
        QList<QJsonObject> msgList;
        for (const QJsonValue &val : messages) {
            msgList.prepend(val.toObject());
        }
        
        for (const QJsonObject &msg : msgList) {
            QString query = msg["query"].toString();
            QString answer = msg["answer"].toString();
            
            if (!query.isEmpty()) {
                m_bubbleChatWidget->addMessage(query, true);  // 用户消息
            }
            if (!answer.isEmpty()) {
                m_bubbleChatWidget->addMessage(answer, false);  // AI 消息
            }
        }
        
        qDebug() << "[ModernMainWindow] Loaded" << messages.size() << "messages for conversation:" << conversationId;
    });
    
    // 加载真实对话历史（如果有）
    if (m_difyService) {
        m_difyService->fetchConversations();
        m_difyService->fetchAppInfo();  // 获取动态开场白
    }
    
    // 创建气泡样式聊天组件
    m_bubbleChatWidget = new ChatWidget();
    m_bubbleChatWidget->setPlaceholderText("向AI助手发送信息...");
    containerLayout->addWidget(m_bubbleChatWidget, 1);
    
    // 连接动态开场白信号
    connect(m_difyService, &DifyService::appInfoReceived, this, [this](const QString &name, const QString &introduction) {
        if (m_bubbleChatWidget && !introduction.isEmpty()) {
            m_bubbleChatWidget->clearMessages();
            m_bubbleChatWidget->addMessage(introduction, false);
            qDebug() << "[ModernMainWindow] Loaded dynamic introduction from Dify:" << name;
        }
    });
    
    // 显示开场白
    QString openingMessage = "老师您好！我是智慧课堂助手，请问有什么可以帮你？";
    m_bubbleChatWidget->addMessage(openingMessage, false);

    // 连接消息发送信号到 Dify 服务
    connect(m_bubbleChatWidget, &ChatWidget::messageSent, this, [this](const QString &message) {
        if (message.trimmed().isEmpty()) return;

        // 首次发送消息时，切换到聊天界面并切换侧边栏
        if (m_mainStack && m_mainStack->currentWidget() != m_chatContainer) {
            m_mainStack->setCurrentWidget(m_chatContainer);
            swapToHistorySidebar();  // 切换到历史记录侧边栏
            m_isConversationStarted = true;
        }

        // 显示用户消息
        m_bubbleChatWidget->addMessage(message, true);

        // 如果正在 PPT 问答阶段或打字中，继续问答流程
        if (m_pptQuestionStep > 0 && m_pptQuestionStep <= 5) {
            // 如果还在打字中，忽略用户输入
            if (m_pptTypingTimer->isActive()) {
                return;
            }
            // 如果已经进入生成阶段，忽略
            if (m_pptQuestionStep == 5) {
                return;
            }
            handlePPTConversation(message);
            return;
        }

        // 检测是否是 PPT 生成请求
        if (isPPTGenerationRequest(message)) {
            // 开始问答流程
            m_pptQuestionStep = 1;
            m_pptUserAnswers.clear();
            handlePPTConversation(message);
            return;
        }

        // 清空累积响应
        m_currentAIResponse.clear();

        // 直接发送到 Dify，使用 Dify 中配置的提示词
        if (m_difyService) {
            m_difyService->sendMessage(message);
        }
    });
}

void ModernMainWindow::swapToHistorySidebar()
{
    if (m_sidebarStack) {
        m_sidebarStack->setCurrentIndex(1);  // 历史记录侧边栏
    }
}

void ModernMainWindow::swapToNavSidebar()
{
    if (m_sidebarStack) {
        m_sidebarStack->setCurrentIndex(0);  // 导航侧边栏
    }
}

void ModernMainWindow::appendChatMessage(const QString &sender, const QString &message, bool isUser)
{
    // 直接在主页面的聊天组件中显示消息
    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->addMessage(message, isUser);
    }
}

void ModernMainWindow::onSendChatMessage()
{
    if (!m_chatInput) {
        return;
    }

    QString message = m_chatInput->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    // 显示用户消息
    appendChatMessage("您", message, true);

    // 清空输入框
    m_chatInput->clear();

    // 清空累积响应
    m_currentAIResponse.clear();

    // 发送到 Dify（不添加额外前缀，让 AI 自由使用 Markdown 格式回复）
    if (m_difyService) {
        m_difyService->sendMessage(message);
    }
}

void ModernMainWindow::onAIStreamChunk(const QString &chunk)
{
    if (!m_bubbleChatWidget) {
        qDebug() << "[ModernMainWindow] Error: m_bubbleChatWidget is null!";
        return;
    }

    // 累积响应
    m_currentAIResponse += chunk;

    // 如果是第一个 chunk，先添加一个空的 AI 消息
    if (m_currentAIResponse.length() == chunk.length()) {
        qDebug() << "[ModernMainWindow] Adding first AI message placeholder";
        m_bubbleChatWidget->addMessage("", false); // 添加空的 AI 消息占位
        // 第一个 chunk 立即更新
        m_bubbleChatWidget->updateLastAIMessage(m_currentAIResponse);
    } else {
        // 使用节流机制：标记有待更新，如果定时器没在运行则启动
        m_streamUpdatePending = true;
        if (!m_streamUpdateTimer->isActive()) {
            m_streamUpdateTimer->start(80);  // 80ms 节流间隔
        }
    }
}

void ModernMainWindow::onAIThinkingChunk(const QString &thought)
{
    qDebug() << "[ModernMainWindow] Thinking chunk received:" << thought.left(50) + "...";

    if (!m_bubbleChatWidget) {
        qDebug() << "[ModernMainWindow] Error: m_bubbleChatWidget is null!";
        return;
    }

    // 更新思考过程
    m_bubbleChatWidget->updateLastAIThinking(thought);
}

void ModernMainWindow::onAIResponseReceived(const QString &response)
{
    qDebug() << "[ModernMainWindow] AI Response received, length:" << response.length();
    qDebug() << "[ModernMainWindow] Current accumulated response length:" << m_currentAIResponse.length();

    // 如果没有累积的响应，直接显示
    if (m_currentAIResponse.isEmpty()) {
        qDebug() << "[ModernMainWindow] No accumulated response, adding new message";
        appendChatMessage("AI 助手", response, false);
    } else {
        // 更新最后的 AI 消息为完整响应
        if (m_bubbleChatWidget) {
            qDebug() << "[ModernMainWindow] Updating final AI message";
            m_bubbleChatWidget->updateLastAIMessage(response);
        } else {
            qDebug() << "[ModernMainWindow] Error: m_bubbleChatWidget is null!";
        }
    }
    m_currentAIResponse.clear();
}

void ModernMainWindow::onAIError(const QString &error)
{
    // 添加调试输出
    qDebug() << "[ModernMainWindow] AI Error occurred:" << error;

    // 直接在主页面聊天组件中显示错误消息
    QString errorMessage = QString("⚠️ 错误：%1").arg(error);
    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->addMessage(errorMessage, false);
    } else {
        // 聊天组件尚未初始化，仅输出日志
        qWarning() << "[ModernMainWindow] ChatWidget not ready, error not displayed:" << error;
    }
}

void ModernMainWindow::onAIRequestStarted()
{
    // 添加调试输出
    qDebug() << "[ModernMainWindow] AI Request started";

    // 通过 ChatWidget 的公共方法来控制状态
    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->setInputEnabled(false);
        qDebug() << "[ModernMainWindow] Input disabled";
        // 注意：暂时无法设置发送按钮文本，因为 ChatWidget 没有提供这个方法
    } else {
        qDebug() << "[ModernMainWindow] m_bubbleChatWidget is null!";
    }
}

void ModernMainWindow::onAIRequestFinished()
{
    // 通过 ChatWidget 的公共方法来控制状态
    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->setInputEnabled(true);
        m_bubbleChatWidget->focusInput();
    }
    
    // 检测是否是 PPT 响应（包含 JSON 格式的 PPT 大纲）
    if (m_pptxGenerator && !m_currentAIResponse.isEmpty()) {
        QJsonObject pptJson = PPTXGenerator::parseJsonFromResponse(m_currentAIResponse);
        
        // 检查是否是 PPT 类型
        if (!pptJson.isEmpty() && 
            (pptJson.contains("slides") || pptJson["type"].toString() == "ppt")) {
            
            qDebug() << "[ModernMainWindow] Detected PPT JSON response, offering download";
            
            // 弹出保存对话框
            QString defaultName = pptJson["title"].toString();
            if (defaultName.isEmpty()) defaultName = "思政课PPT";
            defaultName += ".pptx";
            
            QString filePath = QFileDialog::getSaveFileName(
                this, 
                "保存 PPT 文件", 
                QDir::homePath() + "/Desktop/" + defaultName,
                "PowerPoint 文件 (*.pptx)"
            );
            
            if (!filePath.isEmpty()) {
                if (m_pptxGenerator->generateFromJson(pptJson, filePath)) {
                    QMessageBox::information(this, "成功", 
                        QString("PPT 已生成！\n\n文件位置：%1").arg(filePath));
                } else {
                    QMessageBox::warning(this, "生成失败", 
                        QString("PPT 生成失败：%1").arg(m_pptxGenerator->lastError()));
                }
            }
        }
    }
    
    // 刷新对话列表以显示新创建的对话
    if (m_difyService) {
        m_difyService->fetchConversations();
    }
}

// ==================== PPT 模拟生成功能 ====================

bool ModernMainWindow::isPPTGenerationRequest(const QString &message)
{
    // 检测消息中是否包含 PPT 生成相关关键词
    QString lowerMsg = message.toLower();
    bool hasPPTKeyword = lowerMsg.contains("ppt") ||
                         lowerMsg.contains("幻灯片") ||
                         lowerMsg.contains("演示文稿") ||
                         lowerMsg.contains("课件");
    bool hasGenerateKeyword = lowerMsg.contains("生成") ||
                              lowerMsg.contains("制作") ||
                              lowerMsg.contains("做一个") ||
                              lowerMsg.contains("创建") ||
                              lowerMsg.contains("帮我");

    return hasPPTKeyword && hasGenerateKeyword;
}

void ModernMainWindow::handlePPTConversation(const QString &message)
{
    if (!m_bubbleChatWidget) return;

    // 记录用户回答（除了第一次触发）
    if (m_pptQuestionStep > 1) {
        m_pptUserAnswers.append(message);
    }

    // 模拟 AI 思考延迟
    QTimer::singleShot(600, this, [this]() {
        if (!m_bubbleChatWidget) return;

        QString response;
        switch (m_pptQuestionStep) {
            case 1: {
                // 第一个问题：确认主题
                response = "好的，我来帮您制作PPT！\n\n"
                           "为了更好地满足您的教学需求，请问：\n\n"
                           "**1. 这个PPT主要面向哪个年级的学生？**\n"
                           "（例如：七年级、八年级、九年级）";
                m_pptQuestionStep = 2;
                break;
            }
            case 2: {
                // 第二个问题：课时长度
                response = "明白了！\n\n"
                           "**2. 您计划这节课的时长是多少？**\n"
                           "（例如：一课时40分钟、两课时等）";
                m_pptQuestionStep = 3;
                break;
            }
            case 3: {
                // 第三个问题：内容侧重
                response = "好的！\n\n"
                           "**3. 您希望PPT的内容侧重于哪个方面？**\n"
                           "- A. 历史故事与人物事迹\n"
                           "- B. 理论知识与概念讲解\n"
                           "- C. 实践活动与课堂互动\n"
                           "- D. 综合呈现";
                m_pptQuestionStep = 4;
                break;
            }
            case 4: {
                // 问答结束，开始生成
                response = "非常感谢您的回答！我已经了解您的需求：\n\n"
                           "📌 **目标年级**：" + (m_pptUserAnswers.size() > 0 ? m_pptUserAnswers[0] : "初中") + "\n"
                           "📌 **课时安排**：" + (m_pptUserAnswers.size() > 1 ? m_pptUserAnswers[1] : "一课时") + "\n"
                           "📌 **内容侧重**：" + (m_pptUserAnswers.size() > 2 ? m_pptUserAnswers[2] : "综合呈现") + "\n\n"
                           "正在为您生成PPT，请稍候...";
                m_pptQuestionStep = 5;  // 标记为生成阶段，防止再次进入问答
                break;
            }
        }

        // 使用打字效果显示回复
        typeMessageWithEffect(response);
    });
}

void ModernMainWindow::typeMessageWithEffect(const QString &text)
{
    if (!m_bubbleChatWidget) return;

    // 停止之前的打字效果
    m_pptTypingTimer->stop();

    // 设置待打字文本
    m_pptTypingText = text;
    m_pptTypingIndex = 0;

    // 添加空的 AI 消息占位
    m_bubbleChatWidget->addMessage("", false);

    // 开始打字效果（每 30ms 输出一个字符）
    m_pptTypingTimer->start(30);
}

void ModernMainWindow::onPPTTypingStep()
{
    if (!m_bubbleChatWidget || m_pptTypingIndex >= m_pptTypingText.length()) {
        m_pptTypingTimer->stop();

        // 如果是问答结束阶段，延迟后开始生成
        if (m_pptQuestionStep == 5) {
            QTimer::singleShot(800, this, [this]() {
                startPPTSimulation("");
            });
        }
        return;
    }

    // 每次输出多个字符，加快速度
    int charsPerStep = 2;
    int endIndex = qMin(m_pptTypingIndex + charsPerStep, m_pptTypingText.length());
    QString currentText = m_pptTypingText.left(endIndex);

    m_bubbleChatWidget->updateLastAIMessage(currentText);
    m_pptTypingIndex = endIndex;
}

void ModernMainWindow::startPPTSimulation(const QString &userMessage)
{
    Q_UNUSED(userMessage);

    // 设置预制 PPT 路径（从 App Bundle 的 Resources 目录读取）
    QString appPath = QCoreApplication::applicationDirPath();
    // macOS: appPath 是 .app/Contents/MacOS/，需要回到上级找 Resources
    m_pendingPPTPath = appPath + "/../Resources/ppt/爱国主义精神传承.pptx";

    // 检查文件是否存在
    if (!QFile::exists(m_pendingPPTPath)) {
        qDebug() << "[PPT] Resource not found at:" << m_pendingPPTPath;
        if (m_bubbleChatWidget) {
            m_bubbleChatWidget->addMessage("抱歉，PPT 资源文件未找到，请稍后再试。", false);
        }
        m_pptQuestionStep = 0;  // 重置状态
        return;
    }

    // 重置步骤计数
    m_pptSimulationStep = 0;

    // 不创建新气泡，直接在上一条消息基础上更新
    // 开始模拟思考（初始每 800ms 一步，后面会逐渐变慢）
    m_pptSimulationTimer->setInterval(800);
    m_pptSimulationTimer->start();
}

void ModernMainWindow::onPPTSimulationStep()
{
    if (!m_bubbleChatWidget) {
        m_pptSimulationTimer->stop();
        return;
    }

    // 构建需求确认的前缀（保持之前的回答内容）
    QString prefix = "非常感谢您的回答！我已经了解您的需求：\n\n"
                     "📌 **目标年级**：" + (m_pptUserAnswers.size() > 0 ? m_pptUserAnswers[0] : "初中") + "\n"
                     "📌 **课时安排**：" + (m_pptUserAnswers.size() > 1 ? m_pptUserAnswers[1] : "一课时") + "\n"
                     "📌 **内容侧重**：" + (m_pptUserAnswers.size() > 2 ? m_pptUserAnswers[2] : "综合呈现") + "\n\n"
                     "---\n\n";

    // 定义思考过程的各个阶段
    QStringList thinkingSteps = {
        "🤔 正在理解您的需求...",
        "🤔 正在理解您的需求...\n\n📚 分析教学目标和核心知识点...",
        "🤔 正在理解您的需求...\n\n📚 分析教学目标和核心知识点...\n\n🎨 设计课件结构和视觉风格...",
        "🤔 正在理解您的需求...\n\n📚 分析教学目标和核心知识点...\n\n🎨 设计课件结构和视觉风格...\n\n✍️ 生成内容大纲...",
        "🤔 正在理解您的需求...\n\n📚 分析教学目标和核心知识点...\n\n🎨 设计课件结构和视觉风格...\n\n✍️ 生成内容大纲...\n\n🖼️ 排版幻灯片页面...",
        "🤔 正在理解您的需求...\n\n📚 分析教学目标和核心知识点...\n\n🎨 设计课件结构和视觉风格...\n\n✍️ 生成内容大纲...\n\n🖼️ 排版幻灯片页面...\n\n✅ PPT 生成完成！"
    };

    if (m_pptSimulationStep < thinkingSteps.size()) {
        // 更新思考进度（在前缀基础上追加）
        m_bubbleChatWidget->updateLastAIMessage(prefix + thinkingSteps[m_pptSimulationStep]);
        m_pptSimulationStep++;

        // 最后两步放慢速度，更真实
        if (m_pptSimulationStep >= 4) {
            m_pptSimulationTimer->setInterval(1500);  // 最后阶段 1.5 秒
        } else if (m_pptSimulationStep >= 3) {
            m_pptSimulationTimer->setInterval(1200);  // 中后期 1.2 秒
        }
    } else {
        // 思考完成，停止定时器
        m_pptSimulationTimer->stop();

        // 显示最终结果和下载提示
        QString finalMessage = prefix +
                               "🤔 正在理解您的需求...\n\n"
                               "📚 分析教学目标和核心知识点...\n\n"
                               "🎨 设计课件结构和视觉风格...\n\n"
                               "✍️ 生成内容大纲...\n\n"
                               "🖼️ 排版幻灯片页面...\n\n"
                               "✅ **PPT 生成完成！**\n\n"
                               "---\n\n"
                               "📎 **爱国主义精神传承.pptx**\n\n"
                               "课件已生成，包含以下内容：\n"
                               "- 爱国主义精神的历史渊源\n"
                               "- 新时代爱国主义的内涵\n"
                               "- 青少年爱国主义教育实践\n\n"
                               "请点击下方按钮保存到本地：";

        m_bubbleChatWidget->updateLastAIMessage(finalMessage);

        // 延迟一点显示保存对话框，让用户看到完成消息
        QTimer::singleShot(500, this, [this, prefix]() {
            QString savePath = QFileDialog::getSaveFileName(
                this,
                "保存 PPT 文件",
                QDir::homePath() + "/Desktop/爱国主义精神传承.pptx",
                "PowerPoint 文件 (*.pptx)"
            );

            if (!savePath.isEmpty()) {
                // 复制预制的 PPT 到用户选择的位置
                if (QFile::exists(savePath)) {
                    QFile::remove(savePath);
                }

                if (QFile::copy(m_pendingPPTPath, savePath)) {
                    // 更新消息显示保存成功
                    QString successMessage = prefix +
                                           "🤔 正在理解您的需求...\n\n"
                                           "📚 分析教学目标和核心知识点...\n\n"
                                           "🎨 设计课件结构和视觉风格...\n\n"
                                           "✍️ 生成内容大纲...\n\n"
                                           "🖼️ 排版幻灯片页面...\n\n"
                                           "✅ **PPT 生成完成！**\n\n"
                                           "---\n\n"
                                           "📎 **爱国主义精神传承.pptx**\n\n"
                                           "✅ 文件已保存到：\n`" + savePath + "`\n\n"
                                           "您可以使用 PowerPoint 或 WPS 打开编辑。";
                    m_bubbleChatWidget->updateLastAIMessage(successMessage);

                    // 添加到历史记录
                    if (m_chatHistoryWidget) {
                        QString timeStr = QDateTime::currentDateTime().toString("MM-dd HH:mm");
                        m_chatHistoryWidget->insertHistoryItem(0,
                            "ppt_" + QString::number(QDateTime::currentDateTime().toSecsSinceEpoch()),
                            "PPT生成：爱国主义精神传承", timeStr);
                    }
                } else {
                    QMessageBox::warning(this, "生成失败", "文件保存失败，请检查权限或磁盘空间。");
                }
            }
            // 重置问答状态，允许下次继续生成
            m_pptQuestionStep = 0;
            m_pptUserAnswers.clear();
        });
    }
}
