#include "modernmainwindow.h"
#include "../auth/login/simpleloginwindow.h"
#include "../ui/aipreparationwidget.h"
#include "../questionbank/QuestionRepository.h"
#include "../questionbank/questionbankwindow.h"
#include "../services/DifyService.h"
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
    m_difyService->setApiKey("app-ohNvxW8RFmbFVEBBrwMWyOfz");
    
    // 连接 Dify 服务信号
    connect(m_difyService, &DifyService::streamChunkReceived, this, &ModernMainWindow::onAIStreamChunk);
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

    // 创建滚动区域
    dashboardScrollArea = new QScrollArea();
    dashboardScrollArea->setWidgetResizable(true);
    dashboardScrollArea->setStyleSheet("QScrollArea { border: none; background-color: " + BACKGROUND_LIGHT + "; }");

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(40, 32, 40, 32);
    scrollLayout->setSpacing(24);

    // 1. 欢迎卡片 (含智能建议) - 消息气泡风格
    createWelcomeCard();
    scrollLayout->addWidget(welcomeCard);

    // 2. 核心功能快捷入口 - 消息气泡风格
    // 标题已移入 createQuickAccessCard 的气泡中
    createQuickAccessCard();
    scrollLayout->addWidget(quickAccessCard);

    scrollLayout->addStretch();

    dashboardScrollArea->setWidget(scrollContent);
    dashboardLayout->addWidget(dashboardScrollArea);

    // 3. AI 对话栏 (底部固定)
    createAIChatWidget();
    
    // 添加聊天显示区域 (默认隐藏，直到有对话)
    // 注意：m_chatDisplay 在 createAIChatWidget 中初始化
    if (m_chatDisplay) {
        dashboardLayout->addWidget(m_chatDisplay);
        m_chatDisplay->setVisible(false); // 默认隐藏
    }
    
    dashboardLayout->addWidget(m_chatWidget);
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
            m_chatInput->setText(query);
            m_chatInput->setFocus();
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
    // 1. 初始化聊天显示区域
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setObjectName("chatDisplay");
    m_chatDisplay->setStyleSheet(
        "QTextEdit#chatDisplay {"
        "   background-color: " + BACKGROUND_LIGHT + ";"
        "   border: none;"
        "   padding: 20px;"
        "   font-size: 16px;"
        "   line-height: 1.6;"
        "}"
    );
    
    // 2. 初始化底部输入栏容器
    m_chatWidget = new QFrame();
    m_chatWidget->setObjectName("chatWidget");
    m_chatWidget->setStyleSheet("background: transparent;");
    m_chatWidget->setFixedHeight(110); // 增加高度以容纳免责声明
    
    QVBoxLayout *mainLayout = new QVBoxLayout(m_chatWidget);
    mainLayout->setContentsMargins(40, 0, 40, 10); // 左右留白，底部留白
    mainLayout->setSpacing(8);
    
    // 输入框容器 (白色悬浮框)
    QFrame *inputContainer = new QFrame();
    inputContainer->setObjectName("inputContainer");
    inputContainer->setStyleSheet(
        "QFrame#inputContainer {"
        "   background: #ffffff;"
        "   border-radius: 12px;"
        "   border: 1px solid #e0e0e0;"
        "}"
    );
    inputContainer->setFixedHeight(64);
    
    // 添加阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(inputContainer);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 15));
    inputContainer->setGraphicsEffect(shadow);
    
    QHBoxLayout *inputLayout = new QHBoxLayout(inputContainer);
    inputLayout->setContentsMargins(16, 8, 16, 8);
    inputLayout->setSpacing(16);
    
    // 添加按钮 (+)
    QPushButton *addBtn = new QPushButton("+");
    addBtn->setFixedSize(36, 36);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(
        "QPushButton {"
        "   background: #9e9e9e;" // 灰色圆形
        "   border-radius: 18px;"
        "   color: white;"
        "   font-size: 24px;"
        "   border: none;"
        "   padding-bottom: 2px;"
        "}"
        "QPushButton:hover {"
        "   background: #757575;"
        "}"
    );
    inputLayout->addWidget(addBtn);
    
    // 输入框
    m_chatInput = new QLineEdit();
    m_chatInput->setPlaceholderText("向AI助手发送信息...");
    m_chatInput->setFixedHeight(40);
    m_chatInput->setStyleSheet(
        "QLineEdit {"
        "   background: transparent;"
        "   border: none;"
        "   font-size: 15px;"
        "   color: " + PRIMARY_TEXT + ";"
        "}"
    );
    inputLayout->addWidget(m_chatInput);
    
    // 发送按钮 (红色圆形箭头)
    m_sendBtn = new QPushButton("↑");
    m_sendBtn->setFixedSize(36, 36);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton {"
        "   background: #e53935;" // 红色圆形
        "   border-radius: 18px;"
        "   color: white;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   background: #d32f2f;"
        "}"
        "QPushButton:pressed {"
        "   background: #b71c1c;"
        "}"
    );
    inputLayout->addWidget(m_sendBtn);
    
    mainLayout->addWidget(inputContainer);
    
    // 免责声明
    QLabel *disclaimerLabel = new QLabel("AI可能产生错误信息，请核实重要内容。");
    disclaimerLabel->setAlignment(Qt::AlignCenter);
    disclaimerLabel->setStyleSheet("color: #9e9e9e; font-size: 12px;");
    mainLayout->addWidget(disclaimerLabel);
    
    // 连接信号
    connect(m_sendBtn, &QPushButton::clicked, this, &ModernMainWindow::onSendChatMessage);
    connect(m_chatInput, &QLineEdit::returnPressed, this, &ModernMainWindow::onSendChatMessage);
}

void ModernMainWindow::appendChatMessage(const QString &sender, const QString &message, bool isUser)
{
    if (!m_chatDisplay) return;

    if (!m_chatDisplay->isVisible()) {
        m_chatDisplay->setVisible(true);
        // 可以设置一个最大高度，避免占据太多空间
        m_chatDisplay->setMaximumHeight(400);
    }

    QString color = isUser ? WISDOM_BLUE : PATRIOTIC_RED;
    QString html = QString(
        "<div style='margin-bottom: 12px;'>"
        "<span style='color: %1; font-weight: bold;'>%2：</span>"
        "<span style='color: %3;'>%4</span>"
        "</div>"
    ).arg(color, sender, PRIMARY_TEXT, message.toHtmlEscaped().replace("\n", "<br>"));
    
    m_chatDisplay->append(html);
    
    // 滚动到底部
    QScrollBar *scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ModernMainWindow::onSendChatMessage()
{
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
    
    // 发送到 Dify
    m_difyService->sendMessage(message);
}

void ModernMainWindow::onAIStreamChunk(const QString &chunk)
{
    if (!m_chatDisplay) return;

    // 累积响应
    m_currentAIResponse += chunk;
    
    // 更新显示（实时流式效果）
    // 移除之前的 AI 响应行（如果有）并重新添加
    // QString displayHtml = m_chatDisplay->toHtml();
    
    // 简单方案：直接更新最后一条消息
    // 这里为了简单起见，我们在完整响应后再显示
}

void ModernMainWindow::onAIResponseReceived(const QString &response)
{
    // 显示完整的 AI 回复
    appendChatMessage("AI 助手", response, false);
    m_currentAIResponse.clear();
}

void ModernMainWindow::onAIError(const QString &error)
{
    if (!m_chatDisplay) return;

    if (!m_chatDisplay->isVisible()) {
        m_chatDisplay->setVisible(true);
        m_chatDisplay->setMaximumHeight(400);
    }

    QString errorHtml = QString(
        "<div style='margin-bottom: 12px; color: #c62828;'>"
        "<span style='font-weight: bold;'>⚠️ 错误：</span>%1"
        "</div>"
    ).arg(error.toHtmlEscaped());
    
    m_chatDisplay->append(errorHtml);
}

void ModernMainWindow::onAIRequestStarted()
{
    m_sendBtn->setEnabled(false);
    m_sendBtn->setText("发送中...");
    m_chatInput->setEnabled(false);
}

void ModernMainWindow::onAIRequestFinished()
{
    m_sendBtn->setEnabled(true);
    m_sendBtn->setText("发送");
    m_chatInput->setEnabled(true);
    m_chatInput->setFocus();
}
