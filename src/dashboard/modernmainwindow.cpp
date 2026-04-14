#include "modernmainwindow.h"
#include "../shared/StyleConfig.h"
#include "../auth/login/simpleloginwindow.h"
#include "../ui/aipreparationwidget.h"
#include "../ui/pptpreviewpage.h"

#include "../questionbank/questionbankwindow.h"
#include "../services/DifyService.h"
#include "../services/PPTXGenerator.h"
#include "../services/ZhipuPPTAgentService.h"
#include "../ui/AIChatDialog.h"
#include "../ui/ChatWidget.h"
#include "../ui/ChatHistoryWidget.h"
#include "../ui/HotspotTrackingWidget.h"
#include "../services/HotspotService.h"
#include "../hotspot/RealNewsProvider.h"
#include "../settings/UserSettingsDialog.h"
#include "../settings/UserSettingsManager.h"
#include "../analytics/DataAnalyticsWidget.h"
#include "../analytics/ui/AnalyticsNavigationBar.h"
#include "../analytics/ui/PersonalAnalyticsPage.h"
#include "../analytics/ui/ClassAnalyticsPage.h"
#include "../notifications/NotificationService.h"
#include "../notifications/ui/NotificationWidget.h"
#include "../notifications/ui/NotificationBadge.h"
#include "../attendance/ui/AttendanceWidget.h"
#include "../attendance/services/AttendanceService.h"
#include "../ui/LessonPlanEditor.h"
#include "../config/embedded_keys.h"
#include "../config/AppConfig.h"
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
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
#include <QFileInfo>
#include <QStandardPaths>
#include <QEvent>
#include <algorithm>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSvgRenderer>
#include <QPainter>
#include <QVariantAnimation>
#include <QPointer>
#include <functional>

// 思政课堂色彩体系
const QString PATRIOTIC_RED = StyleConfig::PATRIOTIC_RED;
const QString PATRIOTIC_RED_LIGHT = StyleConfig::PATRIOTIC_RED_LIGHT;
const QString PATRIOTIC_RED_DARK = StyleConfig::PATRIOTIC_RED_DARK;
const QString PATRIOTIC_RED_TINT = StyleConfig::PATRIOTIC_RED_TINT;
const QString GOLD_ACCENT = StyleConfig::GOLD_ACCENT;
const QString GROWTH_GREEN = StyleConfig::SUCCESS_GREEN;
const QString WISDOM_BLUE = StyleConfig::INFO_BLUE;

// 背景与结构色
const QString BACKGROUND_LIGHT = StyleConfig::BG_APP;
const QString CARD_WHITE = StyleConfig::BG_CARD;
const QString SEPARATOR = StyleConfig::SEPARATOR;
const QString PRIMARY_TEXT = StyleConfig::TEXT_PRIMARY;
const QString SECONDARY_TEXT = StyleConfig::TEXT_SECONDARY;
const QString LIGHT_TEXT = StyleConfig::TEXT_LIGHT;

const QString WINDOW_BACKGROUND_GRADIENT = "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFF8F6, stop:0.6 #FAF7F7, stop:1 #F5F5F5)";
const QString LIGHT_GRAY = "#F5F5F5";
const QString ULTRA_LIGHT_GRAY = "#F7F8FA";

const QString SIDEBAR_GRADIENT = "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e53935, stop:0.65 #c62828, stop:1 #1976d2)";

// 侧栏按钮样式常量
const QString SIDEBAR_BTN_NORMAL =
    R"(QPushButton { 
        background-color: transparent; 
        color: %1; 
        border: none; 
        border-radius: 12px; 
        padding: 12px 16px; 
        font-size: 14px; 
        text-align: left; 
    }
    QPushButton:hover { 
        background-color: rgba(0, 0, 0, 0.04); 
    })";

const QString SIDEBAR_BTN_ACTIVE =
    R"(QPushButton { 
        background-color: %1; 
        color: %2; 
        border: none; 
        border-radius: 12px; 
        padding: 12px 16px; 
        font-size: 14px; 
        font-weight: 600; 
        text-align: left; 
    }
    QPushButton:hover { 
        background-color: %1; 
    })";

namespace {
const QString CARD_GRADIENT = "#FFFFFF";
const QString CARD_HOVER_GRADIENT = "#FAFAFA";
const QString CARD_PRESSED_GRADIENT = "#F5F5F5";
const QString CARD_BORDER_COLOR = "#F0F0F0";
const QString CARD_BORDER_HIGHLIGHT = "#E0E0E0";
const QString CARD_BORDER_ACTIVE = "#D0D0D0";
const int CARD_CORNER_RADIUS = StyleConfig::RADIUS_L;
const int CARD_PADDING_PX = 24;
const QString PATRIOTIC_RED_DEEP_TONE = StyleConfig::PATRIOTIC_RED_DARK;
const QString CULTURE_GOLD = StyleConfig::GOLD_ACCENT;

QString findBundledDemoPPTPath()
{
    const QString appPath = QCoreApplication::applicationDirPath();
    const QStringList pptCandidatePaths = {
        QDir::cleanPath(appPath + "/../Resources/ppt/爱国主义精神传承.pptx"),
        QDir::cleanPath(appPath + "/ppt/爱国主义精神传承.pptx"),
        QDir::cleanPath(appPath + "/resources/ppt/爱国主义精神传承.pptx"),
        QDir::cleanPath(appPath + "/../resources/ppt/爱国主义精神传承.pptx")
    };

    for (const QString &candidatePath : pptCandidatePaths) {
        if (QFile::exists(candidatePath)) {
            return candidatePath;
        }
    }

    return QString();
}

struct PPTQuestionConfig
{
    QString question;
    QStringList options;
};

QList<PPTQuestionConfig> buildPPTQuestionFlow()
{
    return {
        {
            QStringLiteral("第1问：这次课件更偏向什么授课场景？"),
            {
                QStringLiteral("新授课导入"),
                QStringLiteral("课堂讲授"),
                QStringLiteral("公开课展示"),
                QStringLiteral("复习总结")
            }
        },
        {
            QStringLiteral("第2问：你希望整体表达风格更接近哪一种？"),
            {
                QStringLiteral("严谨知识型"),
                QStringLiteral("启发互动型"),
                QStringLiteral("故事表达型"),
                QStringLiteral("青年化表达")
            }
        },
        {
            QStringLiteral("第3问：这份 PPT 你更想突出哪一类内容？"),
            {
                QStringLiteral("知识梳理"),
                QStringLiteral("案例分析"),
                QStringLiteral("金句提炼"),
                QStringLiteral("课堂讨论")
            }
        },
        {
            QStringLiteral("第4问：最后确认一下，你更偏好哪种呈现节奏？"),
            {
                QStringLiteral("简洁精炼"),
                QStringLiteral("标准教学节奏"),
                QStringLiteral("内容丰富一些"),
                QStringLiteral("更适合展示")
            }
        }
    };
}

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

        // 在构造时记录原始位置，避免动画期间 Enter 事件覆盖导致漂移
        basePos = button->pos();
        basePosInitialized = true;

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
            // 不在 Enter 时覆盖 basePos，避免动画中间态被当作基准
            if (!basePosInitialized) {
                basePos = button->pos();
                basePosInitialized = true;
            }
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
            // 仅在非 hovered 且动画未运行时更新基准位置
            if (!hovered && liftAnimation->state() != QAbstractAnimation::Running) {
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
    bool basePosInitialized = false;
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

        // 在构造时记录原始位置，避免动画期间 Enter 事件覆盖导致漂移
        basePos = card->pos();
        frameBasePosInitialized = true;

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
            // 不在 Enter 时覆盖 basePos，避免动画中间态被当作基准
            if (!frameBasePosInitialized) {
                basePos = card->pos();
                frameBasePosInitialized = true;
            }
            animateState();
            updateVisualState();
            break;
        case QEvent::Leave:
            hovered = false;
            animateState();
            updateVisualState();
            break;
        case QEvent::Move:
            // 仅在非 hovered 且动画未运行时更新基准位置
            if (!hovered && liftAnimation->state() != QAbstractAnimation::Running) {
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
    bool frameBasePosInitialized = false;
    qreal baseBlur = 20.0;
    qreal baseYOffset = 8.0;
    QColor baseShadowColor = QColor(15, 23, 42, 30);
    const int hoverLift;
    bool hovered = false;
};
}

ModernMainWindow::ModernMainWindow(const QString &userRole, const QString &username, const QString &userId, QWidget *parent)
    : QMainWindow(parent)
    , currentUserRole(userRole)
    , currentUsername(username)
    , currentUserId(userId)
{
    qDebug() << "=== ModernMainWindow 构造函数开始 ===";
    qDebug() << "用户角色:" << userRole << "用户名:" << username << "用户ID:" << userId;

    setWindowTitle("思政智慧课堂 - 教师中心");
    setMinimumSize(1400, 900);
    resize(1600, 1000);

    // 初始化 Dify AI 服务
    m_difyService = new DifyService(this);

    // 绑定真实用户 ID 到 Dify 服务
    if (!userId.isEmpty()) {
        m_difyService->setUserId(userId);
    }
    
    // 初始化 PPTX 生成器
    m_pptxGenerator = new PPTXGenerator(this);

    // 初始化真实 PPT Agent 服务
    m_pptAgentService = new ZhipuPPTAgentService(this);
    
    // API Key 获取优先级：环境变量 > 本地配置文件 > 内嵌Key
    m_streamUpdateTimer = new QTimer(this);
    m_streamUpdateTimer->setSingleShot(true);
    m_streamUpdatePending = false;
    connect(m_streamUpdateTimer, &QTimer::timeout, this, [this]() {
        if (m_streamUpdatePending && m_bubbleChatWidget) {
            // 过滤 Markdown 格式符号
            QString displayText = m_currentAIResponse;
            displayText.remove(QRegularExpression("^##\\s*", QRegularExpression::MultilineOption));
            displayText.remove(QRegularExpression("//+\\s*"));
            displayText.remove(QRegularExpression("\\*\\*"));
            m_bubbleChatWidget->updateLastAIMessage(displayText);
            m_streamUpdatePending = false;
        }
    });

    // API Key 获取：环境变量 > 随包 config.env > .env.local
    QString difyApiKey = AppConfig::get("DIFY_API_KEY");
    if (!difyApiKey.isEmpty()) {
        qDebug() << "[Info] Dify API Key loaded.";
    }

    QString zhipuApiKey = AppConfig::get("ZHIPU_API_KEY", EmbeddedKeys::ZHIPU_API_KEY);
    if (!zhipuApiKey.isEmpty()) {
        qDebug() << "[Info] Zhipu API Key loaded.";
    }

    QString zhipuBaseUrl = AppConfig::get("ZHIPU_BASE_URL", EmbeddedKeys::ZHIPU_BASE_URL);
    if (zhipuBaseUrl.isEmpty()) {
        zhipuBaseUrl = QStringLiteral("https://open.bigmodel.cn/api/coding/paas/v4");
    }

    const bool hasDifyApiKey = !difyApiKey.isEmpty();
    if (hasDifyApiKey) {
        m_difyService->setApiKey(difyApiKey);
    } else {
        qDebug() << "[Warning] No Dify API Key found. Chat features will be disabled.";
        qDebug() << "[Info] Create .env.local file with: DIFY_API_KEY=your-key";
    }

    const bool hasZhipuApiKey = !zhipuApiKey.isEmpty();
    if (hasZhipuApiKey) {
        m_pptAgentService->setApiKey(zhipuApiKey);
        if (!zhipuBaseUrl.isEmpty()) {
            m_pptAgentService->setBaseUrl(zhipuBaseUrl);
        }
    } else {
        qDebug() << "[Warning] No Zhipu API Key found. PPT generation will be disabled.";
        qDebug() << "[Info] Create .env.local file with: ZHIPU_API_KEY=your-key";
    }

    auto *analyticsDifyService = new DifyService(this);
    if (hasDifyApiKey) {
        analyticsDifyService->setApiKey(difyApiKey);
    }
    if (!currentUserId.isEmpty()) {
        analyticsDifyService->setUserId(currentUserId + "_analytics");
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

    // ==================== PPT Agent 信号连接 ====================
    connect(m_pptAgentService, &ZhipuPPTAgentService::progressUpdated,
            this, [this](int percent, const QString &stage, const QString &detail) {
        if (m_bubbleChatWidget) {
            QString msg = QString("好的，我来帮您生成关于「%1」的课件...\n\n"
                                  "阶段%2: %3 (%4%)\n\n"
                                  "正在%5...")
                              .arg(m_pptTopic)
                              .arg(stage.contains("大纲") ? "1/3" : (stage.contains("策划") ? "2/3" : "3/3"))
                              .arg(stage)
                              .arg(percent)
                              .arg(detail);
            m_bubbleChatWidget->updateLastAIMessage(msg);
        }
    });

    connect(m_pptAgentService, &ZhipuPPTAgentService::errorOccurred,
            this, [this](const QString &error) {
        // "Operation canceled" 类错误已经在 Agent 内部静默返回，此处不予提示
        if (error.contains("canceled", Qt::CaseInsensitive) || error.contains("被中止")) {
            return;
        }
        if (m_bubbleChatWidget) {
            m_bubbleChatWidget->updateLastAIMessage(
                QString("抱歉，PPT 生成失败：%1\n\n请稍后重试。").arg(error));
        }
    });

    connect(m_pptAgentService, &ZhipuPPTAgentService::allSlidesGenerated,
            this, [this](const QStringList &svgCodes, const QVector<QImage> &previews) {
        handleChatPPTComplete(svgCodes, previews);
    });

    // 初始化通知服务
    m_notificationService = new NotificationService(this);
    // TODO: 从登录状态获取用户ID，暂时用用户名模拟
    m_notificationService->setCurrentUserId(username);
    connect(m_notificationService, &NotificationService::unreadCountChanged,
            this, &ModernMainWindow::onUnreadCountChanged);

    initUI();
    setupMenuBar();
    setupStatusBar();
    loadDeletedHistoryIds();
    loadHistoryEntries();
    setupCentralWidget();
    if (m_dataAnalyticsWidget) {
        m_dataAnalyticsWidget->setDifyService(analyticsDifyService);
        connect(m_dataAnalyticsWidget, &DataAnalyticsWidget::reportGenerationStarted, this, [this, analyticsDifyService]() {
            setActiveHistoryType(AIHistoryType::AnalyticsReport);
            m_pendingHistoryType = AIHistoryType::AnalyticsReport;
            m_pendingHistoryTitle = QStringLiteral("AI智能分析报告");
            m_pendingAnalyticsContent.clear();
            m_pendingCasePrompt.clear();
            m_pendingLessonPlanContent.clear();
            if (analyticsDifyService) {
                analyticsDifyService->clearConversation();
            }
        });
        connect(m_dataAnalyticsWidget, &DataAnalyticsWidget::reportGenerationFinished, this, [this, analyticsDifyService](const QString &content) {
            if (!analyticsDifyService) {
                return;
            }
            const QString conversationId = analyticsDifyService->currentConversationId();
            if (!conversationId.isEmpty()) {
                recordAnalyticsHistory(conversationId, QStringLiteral("AI智能分析报告"), content);
            }
        });
    }

    QTimer::singleShot(0, this, [this]() {
        if (m_notificationService) {
            m_notificationService->fetchNotifications();
        }
    });

    setupStyles();
    applyPatrioticRedTheme();

    // 创建默认页面
    createDashboard();
    contentStack->setCurrentWidget(dashboardWidget);

    if (!hasDifyApiKey) {
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
    attendanceBtn = new QPushButton("考勤管理");
    learningAnalysisBtn = new QPushButton("数据分析");

    // 底部按钮
    settingsBtn = new QPushButton("系统设置");
    helpBtn = new QPushButton("帮助中心");
    logoutBtn = new QPushButton("退出登录");

    // 确保所有按钮都可见
    teacherCenterBtn->setVisible(true);
    newsTrackingBtn->setVisible(true);
    aiPreparationBtn->setVisible(true);
    resourceManagementBtn->setVisible(true);
    attendanceBtn->setVisible(true);
    learningAnalysisBtn->setVisible(true);
    settingsBtn->setVisible(true);
    logoutBtn->setVisible(true);

    applySidebarIcons();

    // 设置侧边栏按钮样式 - 使用统一样式常量
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    newsTrackingBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    attendanceBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    settingsBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    helpBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    logoutBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));

    // 连接信号
    connect(teacherCenterBtn, &QPushButton::clicked, this, [=]() { qDebug() << "教师中心按钮被点击"; onTeacherCenterClicked(); });
    connect(newsTrackingBtn, &QPushButton::clicked, this, [=]() { qDebug() << "时政新闻按钮被点击"; onNewsTrackingClicked(); });
    connect(aiPreparationBtn, &QPushButton::clicked, this, [=]() { qDebug() << "AI智能备课按钮被点击"; onAIPreparationClicked(); });
    connect(resourceManagementBtn, &QPushButton::clicked, this, [=]() { qDebug() << "试题库按钮被点击"; onResourceManagementClicked(); });
    connect(attendanceBtn, &QPushButton::clicked, this, [=]() { qDebug() << "考勤管理按钮被点击"; onAttendanceClicked(); });
    connect(learningAnalysisBtn, &QPushButton::clicked, this, [=]() { qDebug() << "学情与教评按钮被点击"; onLearningAnalysisClicked(); });
    connect(settingsBtn, &QPushButton::clicked, this, [=]() { qDebug() << "系统设置按钮被点击"; onSettingsClicked(); });
    connect(helpBtn, &QPushButton::clicked, this, [=]() { qDebug() << "帮助中心按钮被点击"; onHelpClicked(); });
    connect(logoutBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "退出登录",
            "确定要退出当前账户吗？",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            SimpleLoginWindow *loginWindow = new SimpleLoginWindow();
            loginWindow->show();
            this->close();
            this->deleteLater();
        }
    });

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
    sidebarLayout->addWidget(attendanceBtn);
    sidebarLayout->addWidget(learningAnalysisBtn);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(settingsBtn);
    sidebarLayout->addWidget(logoutBtn);

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

    m_pptPreviewPage = new PPTPreviewPage(this);
    contentStack->addWidget(m_pptPreviewPage);
    connect(m_pptPreviewPage, &PPTPreviewPage::backRequested,
            this, &ModernMainWindow::onPPTPreviewBackRequested);
    connect(m_pptPreviewPage, &PPTPreviewPage::downloadRequested,
            this, &ModernMainWindow::onPPTPreviewDownloadRequested);

    // 创建试题库页面
    questionBankWindow = new QuestionBankWindow(this);
    contentStack->addWidget(questionBankWindow);

    // 创建时政新闻页面
    m_hotspotService = new HotspotService(this);
    RealNewsProvider *newsProvider = new RealNewsProvider(this);
    // API Key 优先级：环境变量 > 随包 config.env > .env.local
    QString tianxingKey = AppConfig::get("TIANXING_API_KEY");
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

    // 创建数据分析报告页面
    m_dataAnalyticsWidget = new DataAnalyticsWidget(this);
    contentStack->addWidget(m_dataAnalyticsWidget);

    // 创建考勤管理页面
    m_attendanceWidget = new AttendanceWidget(this);
    auto *attendanceService = new AttendanceService(this);
    attendanceService->setCurrentUserId("teacher_001");  // TODO: 使用实际登录用户ID
    m_attendanceWidget->setAttendanceService(attendanceService);
    contentStack->addWidget(m_attendanceWidget);

    // 连接时政热点"生成案例"信号 - 自动切换到AI对话页面并发送请求
    connect(m_hotspotWidget, &HotspotTrackingWidget::teachingContentRequested, this, [this](const NewsItem &news) {
        qDebug() << "[ModernMainWindow] 收到生成教学案例请求:" << news.title;

        // 1. 切换到AI对话页面
        if (contentStack && dashboardWidget) {
            contentStack->setCurrentWidget(dashboardWidget);
        }
        if (m_mainStack && m_chatContainer) {
            m_mainStack->setCurrentWidget(m_chatContainer);
            setActiveHistoryType(AIHistoryType::CaseAnalysis);
            swapToHistorySidebar();
        }

        // 2. 更新侧边栏按钮状态
        resetAllSidebarButtons();
        aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

        // 3. 构建教学案例生成提示并直接发送（不显示问候语）
        QString prompt = QString(
            "请根据以下时政新闻，生成一份适合思政课堂使用的教学案例分析。\n\n"
            "【新闻标题】%1\n"
            "【新闻来源】%2\n"
            "【新闻摘要】%3\n\n"
            "请按以下格式输出：\n"
            "## 案例背景\n"
            "简要介绍新闻背景\n\n"
            "## 思政价值\n"
            "分析该新闻蕴含的思政教育价值\n\n"
            "## 讨论话题\n"
            "设计2-3个适合课堂讨论的话题\n\n"
            "## 延伸思考\n"
            "引导学生进行深入思考的问题"
        ).arg(news.title, news.source, news.summary);

        // 4. 在聊天界面显示用户的请求（简化版）
        if (m_bubbleChatWidget) {
            QString userMsg = QString("请根据新闻《%1》生成思政教学案例").arg(news.title);
            m_bubbleChatWidget->addMessage(userMsg, true);
        }

        // 5. 发送到Dify（直接发送，不需要问候语）
        if (m_difyService) {
            m_pendingHistoryType = AIHistoryType::CaseAnalysis;
            m_pendingHistoryTitle = QString("案例分析：%1").arg(news.title);
            m_pendingCasePrompt = QString("请根据新闻《%1》生成思政教学案例").arg(news.title);
            m_pendingAnalyticsContent.clear();
            m_pendingLessonPlanContent.clear();
            m_difyService->clearConversation();
            m_difyService->sendMessage(prompt);
        }

        this->statusBar()->showMessage("AI智能备课 - 正在生成教学案例...");
    });

    // 连接试题库返回信号
    connect(questionBankWindow, &QuestionBankWindow::backRequested, this, [this]() {
        // 返回首页（教师中心）
        if (contentStack && dashboardWidget) {
            contentStack->setCurrentWidget(dashboardWidget);
        }
        // 恢复主侧边栏显示
        if (m_sidebarStack) {
            m_sidebarStack->setVisible(true);
            m_sidebarStack->setCurrentIndex(0);
        }
        // 重置按钮状态
        resetAllSidebarButtons();
        if (teacherCenterBtn) {
            teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
        }
        // 回到欢迎页面
        if (m_mainStack && m_welcomePanel) {
            m_mainStack->setCurrentWidget(m_welcomePanel);
            if (m_welcomeInputWidget) m_welcomeInputWidget->show();
        }
        this->statusBar()->showMessage("教师中心");
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
    setIcon(attendanceBtn, "x-office-calendar", QStyle::SP_FileDialogListView);
    setIcon(learningAnalysisBtn, "view-list-details", QStyle::SP_FileDialogDetailedView);
    settingsBtn->setIcon(QIcon(":/resources/icons/settings.svg"));
    setIcon(helpBtn, "help-browser", QStyle::SP_MessageBoxQuestion);
    logoutBtn->setIcon(QIcon(":/resources/icons/logout.svg"));
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
    m_avatarLabel = new QLabel();
    m_avatarLabel->setFixedSize(40, 40); // 调整尺寸，更符合扁平设计
    m_avatarLabel->setStyleSheet(QString(
        "QLabel {"
        "  background-color: %1;"
        "  border-radius: 20px;"  // 调整为完全圆形
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  border: none;"  // 移除边框
        "}"
    ).arg(PATRIOTIC_RED));
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setText(UserSettingsManager::instance()->avatarInitial());

    // 用户信息
    QVBoxLayout *userInfoLayout = new QVBoxLayout();
    userInfoLayout->setContentsMargins(0, 0, 0, 0);
    userInfoLayout->setSpacing(4);

    m_userNameLabel = new QLabel(UserSettingsManager::instance()->displayName());
    m_userNameLabel->setStyleSheet("color: " + PRIMARY_TEXT + "; font-size: 15px; font-weight: bold;"); // 调整字体大小

    QLabel *roleLabel = new QLabel(UserSettingsManager::instance()->department());
    roleLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 13px;"); // 使用标准次文本颜色，适配白色背景

    userInfoLayout->addWidget(m_userNameLabel);
    userInfoLayout->addWidget(roleLabel);

    avatarLayout->addWidget(m_avatarLabel);
    avatarLayout->addLayout(userInfoLayout);
    avatarLayout->addStretch();


    // 监听用户设置变化，更新头像和名称
    connect(UserSettingsManager::instance(), &UserSettingsManager::settingsChanged, this, [this]() {
        m_avatarLabel->setText(UserSettingsManager::instance()->avatarInitial());
        m_userNameLabel->setText(UserSettingsManager::instance()->displayName());
    });

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

    QLabel *searchIcon = new QLabel();
    searchIcon->setFixedSize(22, 22);
    searchIcon->setAlignment(Qt::AlignCenter);
    // 加载搜索图标 SVG
    QSvgRenderer searchRenderer(QString(":/icons/resources/icons/search.svg"));
    if (searchRenderer.isValid()) {
        QPixmap searchPixmap(18, 18);
        searchPixmap.fill(Qt::transparent);
        QPainter searchPainter(&searchPixmap);
        searchRenderer.render(&searchPainter);
        searchIcon->setPixmap(searchPixmap);
    }
    searchIcon->setStyleSheet("QLabel { background: transparent; }");

    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("搜索资源、学生...");
    searchInput->setFixedHeight(44);
    searchInput->setStyleSheet(QString(
        "QLineEdit {"
        "  background: #FFFFFF;"
        "  border: none;"
        "  font-size: 14px;"
        "  color: %1;"
        "}"
        "QLineEdit::placeholder { color: %2; }"
    ).arg(StyleConfig::TEXT_PRIMARY, StyleConfig::TEXT_LIGHT));

    searchLayout->addWidget(searchIcon);
    searchLayout->addWidget(searchInput);

    // 通知按钮 - 使用自定义图片
    notificationBtn = new QPushButton();
    notificationBtn->setFixedSize(40, 40);

    // 加载自定义通知图标
    QPixmap notificationIcon(":/icons/resources/icons/notification.svg");
    if (!notificationIcon.isNull()) {
        notificationBtn->setIcon(notificationIcon);
        notificationBtn->setIconSize(QSize(24, 24));
    } else {
        QSvgRenderer notifRenderer(QString(":/icons/resources/icons/notification.svg"));
        if (notifRenderer.isValid()) {
            QPixmap notifPixmap(24, 24);
            notifPixmap.fill(Qt::transparent);
            QPainter notifPainter(&notifPixmap);
            notifRenderer.render(&notifPainter);
            notificationBtn->setIcon(notifPixmap);
            notificationBtn->setIconSize(QSize(24, 24));
        }
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

    headerLayout->addWidget(searchWrapper);
    headerLayout->addSpacing(12);
    headerLayout->addWidget(notificationBtn);

    // 通知按钮上添加小红点
    m_notificationBadge = new NotificationBadge(notificationBtn);
    // 按钮固定40x40，小红点18x18，放在右上角
    m_notificationBadge->move(24, -4);
    m_notificationBadge->setCount(0);

    // 创建通知弹窗
    m_notificationWidget = new NotificationWidget(m_notificationService, this);
    m_notificationWidget->hide();

    // 连接通知按钮点击事件
    connect(notificationBtn, &QPushButton::clicked, this, &ModernMainWindow::onNotificationBtnClicked);

    // 搜索框快捷键
    auto slashShortcut = new QShortcut(QKeySequence("/"), this);
    connect(slashShortcut, &QShortcut::activated, this, [this](){ this->searchInput->setFocus(); this->searchInput->selectAll(); });

    // Ctrl+K 快捷键
    auto ctrlKShortcut = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(ctrlKShortcut, &QShortcut::activated, this, [this](){ this->searchInput->setFocus(); this->searchInput->selectAll(); });

    // 全局搜索：按回车时根据当前页面执行搜索
    connect(searchInput, &QLineEdit::returnPressed, this, [this]() {
        QString keyword = searchInput->text().trimmed();
        if (keyword.isEmpty()) return;

        // 全局搜索：暂时只打印日志
        qDebug() << "[ModernMainWindow] 全局搜索，关键词:" << keyword;
        this->statusBar()->showMessage(QString("搜索: %1（当前页面暂不支持搜索）").arg(keyword), 3000);
    });

    // Ctrl+1~6 切换侧边栏页面
    // 按钮和对应点击槽函数的映射
    struct PageMapping {
        QString shortcutKey;
        std::function<void()> action;
    };
    QVector<PageMapping> pageMappings = {
        {"Ctrl+1", [this]() { onTeacherCenterClicked(); }},
        {"Ctrl+2", [this]() { onNewsTrackingClicked(); }},
        {"Ctrl+3", [this]() { onAIPreparationClicked(); }},
        {"Ctrl+4", [this]() { onResourceManagementClicked(); }},
        {"Ctrl+5", [this]() { onAttendanceClicked(); }},
        {"Ctrl+6", [this]() { onLearningAnalysisClicked(); }},
    };
    for (const auto &mapping : pageMappings) {
        auto *shortcut = new QShortcut(QKeySequence(mapping.shortcutKey), this);
        auto action = mapping.action;
        connect(shortcut, &QShortcut::activated, this, action);
    }

    // Ctrl+N 新建对话
    auto *newChatShortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(newChatShortcut, &QShortcut::activated, this, [this]() {
        // 复用 ChatHistoryWidget 的新建对话逻辑
        if (m_chatHistoryWidget) {
            emit m_chatHistoryWidget->newChatRequested();
        }
        // 确保切换到AI对话页面
        onAIPreparationClicked();
        qDebug() << "[ModernMainWindow] 快捷键 Ctrl+N - 新建对话";
    });

    // Escape 清空搜索框并取消焦点
    auto *escShortcut = new QShortcut(QKeySequence("Escape"), this);
    connect(escShortcut, &QShortcut::activated, this, [this]() {
        if (searchInput->hasFocus()) {
            searchInput->clear();
            searchInput->clearFocus();
        }
    });
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
        }
    )");
    // 加载学士帽图标 SVG
    QSvgRenderer gradRenderer(QString(":/icons/resources/icons/graduation.svg"));
    if (gradRenderer.isValid()) {
        QPixmap gradPixmap(32, 32);
        gradPixmap.fill(Qt::transparent);
        QPainter gradPainter(&gradPixmap);
        gradRenderer.render(&gradPainter);
        iconLabel->setPixmap(gradPixmap);
    }
    
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
    auto createFeatureCard = [this](const QString &iconPath, const QString &title, const QString &desc, const QString &iconBgColor) -> QPushButton* {
        QPushButton *card = new QPushButton();
        card->setFixedSize(320, 100);
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(QString(R"(
            QPushButton {
                background-color: %1;
                border: 1px solid %2;
                border-radius: %3px;
                text-align: left;
            }
            QPushButton:hover {
                background-color: #FAFAFA;
                border-color: #D1D5DB;
            }
            QPushButton:pressed {
                background-color: #F5F5F5;
            }
        )").arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT, QString::number(StyleConfig::RADIUS_L)));

        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(16, 12, 16, 12);
        cardLayout->setSpacing(14);

        QLabel *iconLbl = new QLabel();
        iconLbl->setFixedSize(44, 44);
        iconLbl->setAlignment(Qt::AlignCenter);
        
        QSvgRenderer renderer(iconPath);
        if (renderer.isValid()) {
            QPixmap pixmap(24, 24);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            renderer.render(&painter);
            iconLbl->setPixmap(pixmap);
        }

        iconLbl->setStyleSheet(QString(R"(
            QLabel {
                background-color: %1;
                border-radius: 10px;
            }
        )").arg(iconBgColor));

        QWidget *textArea = new QWidget();
        textArea->setStyleSheet("background: transparent;");
        QVBoxLayout *textLayout = new QVBoxLayout(textArea);
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(4);

        QLabel *titleLbl = new QLabel(title);
        titleLbl->setFixedHeight(22);  // 固定高度，保证布局一致
        titleLbl->setStyleSheet(QString("font-size: 15px; font-weight: 600; color: %1; background: transparent;").arg(StyleConfig::TEXT_PRIMARY));
        
        QLabel *descLbl = new QLabel(desc);
        descLbl->setFixedHeight(20);  // 固定高度，避免不同文字长度导致布局不一致
        descLbl->setStyleSheet(QString("font-size: 12px; color: %1; background: transparent;").arg(StyleConfig::TEXT_SECONDARY));

        textLayout->addWidget(titleLbl);
        textLayout->addWidget(descLbl);

        cardLayout->addWidget(iconLbl);
        cardLayout->addWidget(textArea, 1);

        applyCardShadow(card, 12.0, 2.0);
        new CardHoverAnimator(card, card);

        return card;
    };

    // 四个功能卡片，使用不同的柔和背景色
    QPushButton *card1 = createFeatureCard(":/icons/resources/icons/news.svg", "时政新闻", "实时追踪时政热点，把握教学方向", "#fef3c7");  // 淡黄
    QPushButton *card2 = createFeatureCard(":/icons/resources/icons/document.svg", "AI智能备课", "一键生成PPT", "#fce7f3");  // 淡粉
    QPushButton *card3 = createFeatureCard(":/icons/resources/icons/book.svg", "试题库", "海量思政习题，智能组卷测评", "#dbeafe");  // 淡蓝
    QPushButton *card4 = createFeatureCard(":/icons/resources/icons/analytics.svg", "数据分析报告", "可视化展示教学成果与趋势", "#d1fae5");  // 淡绿

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
        setActiveHistoryType(AIHistoryType::Chat);
        swapToHistorySidebar();  // 切换到历史记录侧边栏
        m_welcomeInputWidget->hide();
        
        // 在聊天组件中设置文本并发送
        if (m_bubbleChatWidget) {
            m_bubbleChatWidget->setInputText(text);
            // 触发发送（模拟回车）
            QTimer::singleShot(50, this, [this]() {
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
            font-family: "PingFang SC", "Noto Sans SC", "Microsoft YaHei", "Source Han Sans SC", "Helvetica Neue", Arial, sans-serif;
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
            background: transparent;
            width: 6px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background-color: #D1D5DB;
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #9CA3AF;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            height: 0; background: transparent;
        }
        QScrollBar:horizontal {
            background: transparent;
            height: 6px;
            margin: 0;
        }
        QScrollBar::handle:horizontal {
            background-color: #D1D5DB;
            border-radius: 3px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: #9CA3AF;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            width: 0; background: transparent;
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

void ModernMainWindow::resetAllSidebarButtons()
{
    // 统一重置所有侧边栏按钮为普通状态（带 null 检查防止构造阶段崩溃）
    if (teacherCenterBtn) teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    if (newsTrackingBtn) newsTrackingBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    if (aiPreparationBtn) aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    if (resourceManagementBtn) resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    if (attendanceBtn) attendanceBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    if (learningAnalysisBtn) learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    if (settingsBtn) settingsBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    if (helpBtn) helpBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
    if (logoutBtn) logoutBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(PRIMARY_TEXT, PATRIOTIC_RED_LIGHT));
}

// 槽函数实现
void ModernMainWindow::onTeacherCenterClicked()
{
    resetAllSidebarButtons();
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

    resetAllSidebarButtons();
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    // 首先切换 contentStack 到 dashboardWidget（m_mainStack 在其中）
    if (contentStack && dashboardWidget) {
        contentStack->setCurrentWidget(dashboardWidget);
    }

    // 然后切换到对话页面
    qDebug() << "切换到AI对话页面";
    if (m_mainStack && m_chatContainer) {
        m_mainStack->setCurrentWidget(m_chatContainer);
        const AIHistoryType type = (m_aiTabWidget && m_aiTabWidget->currentIndex() == 1)
            ? AIHistoryType::LessonPlan
            : AIHistoryType::Chat;
        setActiveHistoryType(type);
        swapToHistorySidebar();  // 切换到历史记录侧边栏
    }
    this->statusBar()->showMessage("AI智能备课");
}

void ModernMainWindow::onResourceManagementClicked()
{
    qDebug() << "试题库按钮被点击";

    resetAllSidebarButtons();
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    // 切换到试题库页面
    if (questionBankWindow) {
        qDebug() << "切换到试题库页面";
        contentStack->setCurrentWidget(questionBankWindow);
        // 隐藏主侧边栏（试题库自带出题历史侧边栏）
        if (m_sidebarStack) {
            m_sidebarStack->setVisible(false);
        }
        this->statusBar()->showMessage("试题库");
    } else {
        qDebug() << "错误：questionBankWindow为空";
    }
}

void ModernMainWindow::onAttendanceClicked()
{
    qDebug() << "切换到考勤管理页面";

    resetAllSidebarButtons();
    attendanceBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    // 切换到考勤管理页面
    if (m_attendanceWidget) {
        contentStack->setCurrentWidget(m_attendanceWidget);
        this->statusBar()->showMessage("考勤管理");
    }
}

void ModernMainWindow::onLearningAnalysisClicked()
{
    qDebug() << "切换到数据分析报告页面";

    resetAllSidebarButtons();
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    // 切换到数据分析报告页面
    if (m_dataAnalyticsWidget) {
        contentStack->setCurrentWidget(m_dataAnalyticsWidget);
        setActiveHistoryType(AIHistoryType::AnalyticsReport);
        m_dataAnalyticsWidget->refresh();  // 刷新数据
        refreshHistorySidebar();
        this->statusBar()->showMessage("数据分析报告");
    }
}

void ModernMainWindow::onNewsTrackingClicked()
{
    qDebug() << "切换到时政新闻页面";

    resetAllSidebarButtons();
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
    UserSettingsDialog dialog(this);
    dialog.exec();
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
    QLabel *avatar = new QLabel();
    avatar->setFixedSize(40, 40);
    // 加载 sparkle 图标 SVG
    QSvgRenderer sparkleRenderer(QString(":/icons/resources/icons/sparkle.svg"));
    if (sparkleRenderer.isValid()) {
        QPixmap sparklePixmap(20, 20);
        sparklePixmap.fill(Qt::transparent);
        QPainter sparklePainter(&sparklePixmap);
        sparkleRenderer.render(&sparklePainter);
        avatar->setPixmap(sparklePixmap);
    }
    avatar->setStyleSheet(
        "background: #e53935;"
        "border-radius: 20px;"
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
    QLabel *avatar = new QLabel();
    avatar->setFixedSize(40, 40);
    // 加载 sparkle 图标 SVG
    QSvgRenderer sparkleRenderer(QString(":/icons/resources/icons/sparkle.svg"));
    if (sparkleRenderer.isValid()) {
        QPixmap sparklePixmap(20, 20);
        sparklePixmap.fill(Qt::transparent);
        QPainter sparklePainter(&sparklePixmap);
        sparkleRenderer.render(&sparklePainter);
        avatar->setPixmap(sparklePixmap);
    }
    avatar->setStyleSheet(
        "background: #e53935;"
        "border-radius: 20px;"
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
        QString iconPath;
        QString color;
    };

    QList<QuickAction> actions = {
        {"智能内容分析", ":/icons/resources/icons/search.svg", "#f5f5f5"},
        {"AI智能备课", ":/icons/resources/icons/document.svg", "#f5f5f5"},
        {"互动教学工具", ":/icons/resources/icons/play.svg", "#f5f5f5"},
        {"资源库管理", ":/icons/resources/icons/folder.svg", "#f5f5f5"}
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

        QLabel *icon = new QLabel();
        // 加载 SVG 图标
        QSvgRenderer actionRenderer(action.iconPath);
        if (actionRenderer.isValid()) {
            QPixmap actionPixmap(22, 22);
            actionPixmap.fill(Qt::transparent);
            QPainter actionPainter(&actionPixmap);
            actionRenderer.render(&actionPainter);
            icon->setPixmap(actionPixmap);
        }
        icon->setStyleSheet("background: transparent;");

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
        resetConversationForType(m_activeHistoryType);
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
        handleHistorySelection(id);
    });
    connect(m_chatHistoryWidget, &ChatHistoryWidget::historyDeleteRequested,
            this, &ModernMainWindow::onHistoryDeleteRequested);
    
    // 连接对话列表接收信号
    connect(m_difyService, &DifyService::conversationsReceived, this, [this](const QJsonArray &conversations) {
        for (const QJsonValue &val : conversations) {
            QJsonObject conv = val.toObject();
            QString id = conv["id"].toString();
            if (id.isEmpty() || m_deletedHistoryIds.contains(id)) {
                continue;
            }

            AIHistoryEntry entry = m_historyEntries.value(id);
            entry.conversationId = id;
            if (entry.title.isEmpty()) {
                QString name = conv["name"].toString();
                entry.title = name.isEmpty() ? QString("对话 %1").arg(id.left(8)) : name;
            }
            if (entry.type == AIHistoryType::Chat && entry.title.isEmpty()) {
                entry.title = QString("对话 %1").arg(id.left(8));
            }

            qint64 updatedAt = conv["updated_at"].toVariant().toLongLong();
            if (updatedAt > 0) {
                entry.updatedAt = QDateTime::fromSecsSinceEpoch(updatedAt);
            } else if (!entry.updatedAt.isValid()) {
                entry.updatedAt = QDateTime::currentDateTime();
            }

            if (entry.type != AIHistoryType::LessonPlan &&
                entry.type != AIHistoryType::AnalyticsReport &&
                entry.type != AIHistoryType::CaseAnalysis) {
                // 尝试从标题推断类型（本地记录丢失后的恢复策略）
                const QString title = entry.title;
                if (title.startsWith(QStringLiteral("教案")) ||
                    title.contains(QStringLiteral("# 教案生成任务"))) {
                    entry.type = AIHistoryType::LessonPlan;
                } else {
                    entry.type = AIHistoryType::Chat;
                }
            }

            m_historyEntries.insert(id, entry);
        }

        refreshHistorySidebar();
        qDebug() << "[ModernMainWindow] Loaded" << conversations.size() << "conversations";
    });
    
    connect(m_difyService, &DifyService::conversationCreated, this, [this](const QString &conversationId) {
        AIHistoryEntry entry = m_historyEntries.value(conversationId);
        entry.conversationId = conversationId;
        entry.type = m_pendingHistoryType;
        entry.title = m_pendingHistoryTitle.isEmpty()
            ? historyHeaderTitle(m_pendingHistoryType)
            : m_pendingHistoryTitle;
        entry.updatedAt = QDateTime::currentDateTime();
        entry.lessonContent = m_pendingLessonPlanContent;
        entry.previewText = !m_pendingAnalyticsContent.isEmpty() ? m_pendingAnalyticsContent : m_pendingCasePrompt;
        upsertHistoryEntry(entry);

        if (entry.type == AIHistoryType::LessonPlan && m_lessonPlanEditor) {
            m_lessonPlanEditor->setConversationId(conversationId);
        }
    });

    // 连接消息历史接收信号
    connect(m_difyService, &DifyService::messagesReceived, this, [this](const QJsonArray &messages, const QString &conversationId) {
        const AIHistoryEntry entry = m_historyEntries.value(conversationId);
        if ((entry.type != AIHistoryType::Chat && entry.type != AIHistoryType::CaseAnalysis) || !m_bubbleChatWidget) {
            return;
        }

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

    // ========== 创建AI智能备课标签页 ==========
    m_aiTabWidget = new QTabWidget();
    m_aiTabWidget->setDocumentMode(true);  // 更现代的外观,无边框

    // 样式设置（思政红主题）
    m_aiTabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: none;
            background: #F8F9FA;
        }
        QTabBar::tab {
            background: #FFFFFF;
            color: #666666;
            padding: 14px 28px;
            margin-right: 4px;
            border: 1px solid #E0E0E0;
            border-bottom: none;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            font-size: 14px;
            font-weight: 500;
        }
        QTabBar::tab:selected {
            background: #F8F9FA;
            color: #C62828;
            border-bottom: 3px solid #C62828;
            font-weight: 600;
        }
        QTabBar::tab:hover {
            background: #F5F5F5;
            color: #C62828;
        }
    )");

    // 标签页1: AI对话 - 使用SVG图标
    m_bubbleChatWidget = new ChatWidget();
    m_bubbleChatWidget->setPlaceholderText("向AI助手发送信息...");
    m_aiTabWidget->addTab(m_bubbleChatWidget, QIcon(":/icons/resources/icons/chat-bubble.svg"), "AI对话");

    // 标签页2: 教案编辑器 - 使用SVG图标
    m_lessonPlanEditor = new LessonPlanEditor();
    m_lessonPlanEditor->setDifyService(m_difyService);
    m_aiTabWidget->addTab(m_lessonPlanEditor, QIcon(":/icons/resources/icons/document-edit.svg"), "教案编辑");

    connect(m_aiTabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 1) {
            setActiveHistoryType(AIHistoryType::LessonPlan);
            return;
        }

        if (m_activeHistoryType != AIHistoryType::CaseAnalysis) {
            setActiveHistoryType(AIHistoryType::Chat);
        }
    });

    // 连接教案编辑器的保存信号
    connect(m_lessonPlanEditor, &LessonPlanEditor::saveRequested,
            this, [this](const QString &title, const QString &content) {
        qDebug() << "[ModernMainWindow] 教案已保存：" << title;
        // 可在此处理保存到数据库等逻辑
        const QString conversationId = m_lessonPlanEditor ? m_lessonPlanEditor->conversationId() : QString();
        if (!conversationId.isEmpty()) {
            recordLessonPlanHistory(conversationId, title, content);
        }
    });

    connect(m_lessonPlanEditor, &LessonPlanEditor::aiGenerationStarted, this, [this]() {
        if (!m_difyService || !m_lessonPlanEditor) {
            return;
        }

        setActiveHistoryType(AIHistoryType::LessonPlan);
        if (m_aiTabWidget) {
            m_aiTabWidget->setCurrentWidget(m_lessonPlanEditor);
        }

        const QString title = m_lessonPlanEditor->getCurrentLessonTitle();
        m_pendingHistoryType = AIHistoryType::LessonPlan;
        m_pendingHistoryTitle = title.isEmpty() ? QStringLiteral("教案编辑") : QStringLiteral("教案：") + title;
        m_pendingLessonPlanContent.clear();
        m_pendingAnalyticsContent.clear();
        m_pendingCasePrompt.clear();
        m_difyService->clearConversation();
        m_lessonPlanEditor->setConversationId(QString());
    });

    connect(m_lessonPlanEditor, &LessonPlanEditor::aiGenerationFinished, this, [this]() {
        if (!m_lessonPlanEditor) {
            return;
        }

        const QString conversationId = m_lessonPlanEditor->conversationId();
        if (!conversationId.isEmpty()) {
            recordLessonPlanHistory(
                conversationId,
                m_lessonPlanEditor->getCurrentLessonTitle(),
                m_lessonPlanEditor->getContent());
        }
    });

    // 添加标签页容器到主布局
    containerLayout->addWidget(m_aiTabWidget, 1);

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
    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->addMessage(openingMessage, false);
    }

    // 连接消息发送信号到 Dify 服务
    if (m_bubbleChatWidget) {
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

            if (m_isPptQuestionFlowActive) {
                handlePPTQuestionAnswer(message);
                return;
            }

            // PPT 请求先走伪提问流程，不发给 Dify
            if (isPPTGenerationRequest(message)) {
                startPPTQuestionFlow(message);
                return;
            }

            // 清空累积响应
            m_currentAIResponse.clear();

            // 直接发送到 Dify，使用 Dify 中配置的提示词
            if (m_difyService) {
                m_pendingHistoryType = AIHistoryType::Chat;
                m_pendingHistoryTitle = message.left(24);
                m_pendingLessonPlanContent.clear();
                m_pendingAnalyticsContent.clear();
                m_pendingCasePrompt.clear();
                m_difyService->sendMessage(message);
            }
        });
    }
}

QString ModernMainWindow::historyHeaderTitle(AIHistoryType type) const
{
    switch (type) {
    case AIHistoryType::Chat:
        return QStringLiteral("对话历史");
    case AIHistoryType::LessonPlan:
        return QStringLiteral("教案历史");
    case AIHistoryType::AnalyticsReport:
        return QStringLiteral("分析报告历史");
    case AIHistoryType::CaseAnalysis:
        return QStringLiteral("案例分析历史");
    }

    return QStringLiteral("历史记录");
}

QString ModernMainWindow::historyNewButtonText(AIHistoryType type) const
{
    switch (type) {
    case AIHistoryType::Chat:
        return QStringLiteral("➕  新建对话");
    case AIHistoryType::LessonPlan:
        return QStringLiteral("➕  新建教案会话");
    case AIHistoryType::AnalyticsReport:
        return QStringLiteral("➕  新建分析报告");
    case AIHistoryType::CaseAnalysis:
        return QStringLiteral("➕  新建案例分析");
    }

    return QStringLiteral("➕  新建记录");
}

QString ModernMainWindow::formatHistoryTime(const QDateTime &time) const
{
    if (!time.isValid()) {
        return QStringLiteral("未知时间");
    }
    return time.toString("M月d日 HH:mm");
}

void ModernMainWindow::refreshHistorySidebar()
{
    if (!m_chatHistoryWidget) {
        return;
    }

    m_chatHistoryWidget->setHeaderTitle(historyHeaderTitle(m_activeHistoryType));
    m_chatHistoryWidget->setNewButtonText(historyNewButtonText(m_activeHistoryType));
    m_chatHistoryWidget->clearHistory();

    QList<AIHistoryEntry> entries;
    for (auto it = m_historyEntries.cbegin(); it != m_historyEntries.cend(); ++it) {
        if (it.value().type == m_activeHistoryType) {
            entries.append(it.value());
        }
    }

    std::sort(entries.begin(), entries.end(), [](const AIHistoryEntry &a, const AIHistoryEntry &b) {
        return a.updatedAt > b.updatedAt;
    });

    for (const AIHistoryEntry &entry : entries) {
        const QString title = entry.title.isEmpty()
            ? historyHeaderTitle(entry.type)
            : entry.title;
        m_chatHistoryWidget->addHistoryItem(entry.conversationId, title, formatHistoryTime(entry.updatedAt));
    }
}

void ModernMainWindow::setActiveHistoryType(AIHistoryType type)
{
    m_activeHistoryType = type;
    refreshHistorySidebar();
}

void ModernMainWindow::resetConversationForType(AIHistoryType type)
{
    resetPPTQuestionFlow();
    m_selectedHistoryConversationId.clear();
    setActiveHistoryType(type);

    if (type == AIHistoryType::Chat || type == AIHistoryType::CaseAnalysis) {
        if (m_bubbleChatWidget) {
            m_bubbleChatWidget->clearMessages();
            m_bubbleChatWidget->addMessage("老师您好！我是智慧课堂助手，请问有什么可以帮你？", false);
        }
        if (m_difyService) {
            m_difyService->clearConversation();
        }
        m_isConversationStarted = false;
    } else if (type == AIHistoryType::LessonPlan) {
        if (m_lessonPlanEditor) {
            m_lessonPlanEditor->clear();
            m_lessonPlanEditor->setConversationId(QString());
        }
        if (m_aiTabWidget && m_lessonPlanEditor) {
            m_aiTabWidget->setCurrentWidget(m_lessonPlanEditor);
        }
        if (contentStack && dashboardWidget) {
            contentStack->setCurrentWidget(dashboardWidget);
        }
        if (m_mainStack && m_chatContainer) {
            m_mainStack->setCurrentWidget(m_chatContainer);
        }
        if (m_difyService) {
            m_difyService->clearConversation();
        }
    } else if (type == AIHistoryType::AnalyticsReport) {
        if (m_dataAnalyticsWidget) {
            contentStack->setCurrentWidget(m_dataAnalyticsWidget);
            m_dataAnalyticsWidget->showOverview();
            m_dataAnalyticsWidget->setReportContent(QString());
        }
    }

    if (m_chatHistoryWidget) {
        m_chatHistoryWidget->clearSelection();
    }
}

void ModernMainWindow::handleHistorySelection(const QString &conversationId)
{
    const AIHistoryEntry entry = m_historyEntries.value(conversationId);
    if (entry.conversationId.isEmpty()) {
        return;
    }

    resetPPTQuestionFlow();
    m_selectedHistoryConversationId = conversationId;

    setActiveHistoryType(entry.type);

    switch (entry.type) {
    case AIHistoryType::Chat:
    case AIHistoryType::CaseAnalysis:
    {
        const bool isLocalPptHistory = entry.conversationId.startsWith(QStringLiteral("ppt_"));
        QString localPptPath = entry.localFilePath;
        if (localPptPath.isEmpty() && isLocalPptHistory) {
            localPptPath = findBundledDemoPPTPath();
        }

        if (!localPptPath.isEmpty() || isLocalPptHistory) {
            if (QFileInfo::exists(localPptPath)) {
                showPPTPreviewPage(localPptPath, entry.title);
            } else {
                QMessageBox::warning(this,
                                     QStringLiteral("预览不可用"),
                                     QStringLiteral("该 PPT 文件已不存在，无法重新打开预览。"));
            }
            break;
        }
        if (contentStack && dashboardWidget) {
            contentStack->setCurrentWidget(dashboardWidget);
        }
        if (m_mainStack && m_chatContainer) {
            m_mainStack->setCurrentWidget(m_chatContainer);
        }
        if (m_aiTabWidget) {
            m_aiTabWidget->setCurrentIndex(0);
        }
        if (m_difyService) {
            m_difyService->setCurrentConversationId(conversationId);
            m_difyService->fetchMessages(conversationId);
        }
        m_isConversationStarted = true;
        break;
    }
    case AIHistoryType::LessonPlan:
        if (contentStack && dashboardWidget) {
            contentStack->setCurrentWidget(dashboardWidget);
        }
        if (m_mainStack && m_chatContainer) {
            m_mainStack->setCurrentWidget(m_chatContainer);
        }
        if (m_aiTabWidget && m_lessonPlanEditor) {
            m_aiTabWidget->setCurrentWidget(m_lessonPlanEditor);
            m_lessonPlanEditor->setConversationId(conversationId);
            m_lessonPlanEditor->setContent(entry.lessonContent);
        }
        break;
    case AIHistoryType::AnalyticsReport:
        if (m_dataAnalyticsWidget) {
            contentStack->setCurrentWidget(m_dataAnalyticsWidget);
            m_dataAnalyticsWidget->showOverview();
            m_dataAnalyticsWidget->setReportContent(entry.previewText);
        }
        break;
    }
}

void ModernMainWindow::onHistoryDeleteRequested(const QString &conversationId)
{
    const AIHistoryEntry entry = m_historyEntries.value(conversationId);
    if (entry.conversationId.isEmpty()) {
        return;
    }

    const QString displayTitle = entry.title.isEmpty()
        ? historyHeaderTitle(entry.type)
        : entry.title;
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        QStringLiteral("删除本地记录"),
        QStringLiteral("确定删除“%1”吗？\n\n此操作只会删除本地侧边栏记录，不会删除云端会话。").arg(displayTitle),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    const bool isCurrentSelection = (m_selectedHistoryConversationId == conversationId);

    m_deletedHistoryIds.insert(conversationId);
    m_historyEntries.remove(conversationId);
    saveDeletedHistoryIds();
    saveHistoryEntries();
    refreshHistorySidebar();

    if (m_chatHistoryWidget) {
        m_chatHistoryWidget->clearSelection();
    }

    if (isCurrentSelection) {
        if (contentStack && contentStack->currentWidget() == m_pptPreviewPage) {
            onPPTPreviewBackRequested();
        }
        resetConversationForType(entry.type);
    }

    statusBar()->showMessage(QStringLiteral("已删除本地历史记录"), 3000);
}

void ModernMainWindow::upsertHistoryEntry(const AIHistoryEntry &entry)
{
    if (entry.conversationId.isEmpty()) {
        return;
    }

    if (m_deletedHistoryIds.contains(entry.conversationId)) {
        return;
    }

    AIHistoryEntry merged = m_historyEntries.value(entry.conversationId);
    merged.conversationId = entry.conversationId;
    merged.type = entry.type;
    if (!entry.title.isEmpty()) {
        merged.title = entry.title;
    }
    if (entry.updatedAt.isValid()) {
        merged.updatedAt = entry.updatedAt;
    } else if (!merged.updatedAt.isValid()) {
        merged.updatedAt = QDateTime::currentDateTime();
    }
    if (!entry.previewText.isEmpty()) {
        merged.previewText = entry.previewText;
    }
    if (!entry.lessonContent.isEmpty()) {
        merged.lessonContent = entry.lessonContent;
    }
    if (!entry.localFilePath.isEmpty()) {
        merged.localFilePath = entry.localFilePath;
    }

    m_historyEntries.insert(merged.conversationId, merged);
    saveHistoryEntries();
    refreshHistorySidebar();
}

void ModernMainWindow::loadHistoryEntries()
{
    m_historyEntries.clear();

    QSettings settings;
    const QString raw = settings.value(QStringLiteral("aiHistory/entries")).toString();
    if (raw.trimmed().isEmpty()) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
    if (!doc.isArray()) {
        return;
    }

    const QJsonArray entries = doc.array();
    for (const QJsonValue &value : entries) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject obj = value.toObject();
        AIHistoryEntry entry;
        entry.conversationId = obj.value(QStringLiteral("conversationId")).toString();
        if (entry.conversationId.isEmpty() || m_deletedHistoryIds.contains(entry.conversationId)) {
            continue;
        }

        const QString type = obj.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("lesson_plan")) {
            entry.type = AIHistoryType::LessonPlan;
        } else if (type == QStringLiteral("analytics_report")) {
            entry.type = AIHistoryType::AnalyticsReport;
        } else if (type == QStringLiteral("case_analysis")) {
            entry.type = AIHistoryType::CaseAnalysis;
        } else {
            entry.type = AIHistoryType::Chat;
        }

        entry.title = obj.value(QStringLiteral("title")).toString();
        entry.previewText = obj.value(QStringLiteral("previewText")).toString();
        entry.lessonContent = obj.value(QStringLiteral("lessonContent")).toString();
        entry.localFilePath = obj.value(QStringLiteral("localFilePath")).toString();
        entry.updatedAt = QDateTime::fromString(
            obj.value(QStringLiteral("updatedAt")).toString(),
            Qt::ISODate);

        m_historyEntries.insert(entry.conversationId, entry);
    }
}

void ModernMainWindow::saveHistoryEntries() const
{
    QJsonArray entries;
    for (auto it = m_historyEntries.cbegin(); it != m_historyEntries.cend(); ++it) {
        const AIHistoryEntry &entry = it.value();
        QJsonObject obj;
        obj.insert(QStringLiteral("conversationId"), entry.conversationId);

        QString type = QStringLiteral("chat");
        switch (entry.type) {
        case AIHistoryType::LessonPlan:
            type = QStringLiteral("lesson_plan");
            break;
        case AIHistoryType::AnalyticsReport:
            type = QStringLiteral("analytics_report");
            break;
        case AIHistoryType::CaseAnalysis:
            type = QStringLiteral("case_analysis");
            break;
        case AIHistoryType::Chat:
            break;
        }

        obj.insert(QStringLiteral("type"), type);
        obj.insert(QStringLiteral("title"), entry.title);
        obj.insert(QStringLiteral("updatedAt"), entry.updatedAt.toString(Qt::ISODate));
        obj.insert(QStringLiteral("previewText"), entry.previewText);
        obj.insert(QStringLiteral("lessonContent"), entry.lessonContent);
        obj.insert(QStringLiteral("localFilePath"), entry.localFilePath);
        entries.append(obj);
    }

    QSettings settings;
    settings.setValue(QStringLiteral("aiHistory/entries"), QString::fromUtf8(QJsonDocument(entries).toJson(QJsonDocument::Compact)));
}

void ModernMainWindow::loadDeletedHistoryIds()
{
    QSettings settings;
    const QStringList ids = settings.value(QStringLiteral("aiHistory/deletedEntryIds")).toStringList();
    m_deletedHistoryIds = QSet<QString>(ids.cbegin(), ids.cend());
}

void ModernMainWindow::saveDeletedHistoryIds() const
{
    QSettings settings;
    settings.setValue(QStringLiteral("aiHistory/deletedEntryIds"), QStringList(m_deletedHistoryIds.cbegin(), m_deletedHistoryIds.cend()));
}

void ModernMainWindow::recordLessonPlanHistory(const QString &conversationId, const QString &title, const QString &content)
{
    AIHistoryEntry entry;
    entry.conversationId = conversationId;
    entry.type = AIHistoryType::LessonPlan;
    entry.title = title.isEmpty() ? QStringLiteral("教案编辑") : QStringLiteral("教案：") + title;
    entry.lessonContent = content;
    entry.updatedAt = QDateTime::currentDateTime();
    upsertHistoryEntry(entry);
}

void ModernMainWindow::recordAnalyticsHistory(const QString &conversationId, const QString &title, const QString &content)
{
    AIHistoryEntry entry;
    entry.conversationId = conversationId;
    entry.type = AIHistoryType::AnalyticsReport;
    entry.title = title;
    entry.previewText = content;
    entry.updatedAt = QDateTime::currentDateTime();
    upsertHistoryEntry(entry);
}

void ModernMainWindow::recordCaseAnalysisHistory(const QString &conversationId, const QString &title)
{
    AIHistoryEntry entry;
    entry.conversationId = conversationId;
    entry.type = AIHistoryType::CaseAnalysis;
    entry.title = title;
    entry.updatedAt = QDateTime::currentDateTime();
    upsertHistoryEntry(entry);
}

void ModernMainWindow::swapToHistorySidebar()
{
    if (m_sidebarStack) {
        m_sidebarStack->setCurrentIndex(1);  // 历史记录侧边栏
    }
    refreshHistorySidebar();
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

    if (m_isPptQuestionFlowActive) {
        handlePPTQuestionAnswer(message);
        return;
    }

    // PPT 请求先走伪提问流程
    if (isPPTGenerationRequest(message)) {
        startPPTQuestionFlow(message);
        return;
    }

    // 清空累积响应
    m_currentAIResponse.clear();

    // 发送到 Dify
    if (m_difyService) {
        m_pendingHistoryType = AIHistoryType::Chat;
        m_pendingHistoryTitle = message.left(24);
        m_pendingLessonPlanContent.clear();
        m_pendingAnalyticsContent.clear();
        m_pendingCasePrompt.clear();
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

    // 过滤 Markdown 格式符号用于显示
    QString displayText = m_currentAIResponse;
    displayText.remove(QRegularExpression("^##\\s*", QRegularExpression::MultilineOption));
    displayText.remove(QRegularExpression("//+\\s*"));
    displayText.remove(QRegularExpression("\\*\\*"));

    // 如果是第一个 chunk，先添加一个空的 AI 消息
    if (m_currentAIResponse.length() == chunk.length()) {
        qDebug() << "[ModernMainWindow] Adding first AI message placeholder";
        m_bubbleChatWidget->addMessage("", false); // 添加空的 AI 消息占位
        // 第一个 chunk 立即更新
        m_bubbleChatWidget->updateLastAIMessage(displayText);
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
    // 保留完整响应，供 onAIRequestFinished 阶段解析 PPT JSON 使用
    if (!response.trimmed().isEmpty()) {
        m_currentAIResponse = response;
    }
}

void ModernMainWindow::onAIError(const QString &error)
{
    // 添加调试输出
    qDebug() << "[ModernMainWindow] AI Error occurred:" << error;

    // 直接在主页面聊天组件中显示错误消息
    QString errorMessage = QString("[!] 错误：%1").arg(error);
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
    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->setInputEnabled(true);
        m_bubbleChatWidget->focusInput();
    }

    m_currentAIResponse.clear();

    // 刷新对话列表以显示新创建的对话
    if (m_difyService && (m_activeHistoryType == AIHistoryType::Chat || m_activeHistoryType == AIHistoryType::CaseAnalysis)) {
        m_difyService->fetchConversations();
    }
    refreshHistorySidebar();
}

void ModernMainWindow::onPPTPreviewBackRequested()
{
    resetAllSidebarButtons();
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    if (contentStack && dashboardWidget) {
        contentStack->setCurrentWidget(dashboardWidget);
    }
    if (m_mainStack && m_chatContainer) {
        m_mainStack->setCurrentWidget(m_chatContainer);
    }
    if (m_sidebarStack) {
        swapToHistorySidebar();
    }

    statusBar()->showMessage(QStringLiteral("AI智能备课"));
}

void ModernMainWindow::onPPTPreviewDownloadRequested()
{
    if (savePPTToUserLocation(m_previewPPTPath, m_previewPPTTitle)) {
        statusBar()->showMessage(QStringLiteral("PPT 已保存"), 3000);
    }
}

void ModernMainWindow::showPPTPreviewPage(const QString &pptPath, const QString &title)
{
    m_previewPPTPath = pptPath;
    m_previewPPTTitle = title;

    if (m_pptPreviewPage) {
        m_pptPreviewPage->setPresentation(title, pptPath);
    }

    resetAllSidebarButtons();
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    if (contentStack && m_pptPreviewPage) {
        contentStack->setCurrentWidget(m_pptPreviewPage);
    }
    if (m_sidebarStack) {
        swapToHistorySidebar();
    }

    statusBar()->showMessage(QStringLiteral("PPT 预览"));
}

bool ModernMainWindow::savePPTToUserLocation(const QString &sourcePath, const QString &title)
{
    if (sourcePath.isEmpty() || !QFileInfo::exists(sourcePath)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), QStringLiteral("当前没有可下载的 PPT 文件。"));
        return false;
    }

    QString suggestedName = title;
    suggestedName.remove(QRegularExpression("[/\\:*?\"<>|]"));
    if (suggestedName.isEmpty()) {
        suggestedName = QStringLiteral("思政课PPT");
    }
    if (!suggestedName.endsWith(QStringLiteral(".pptx"), Qt::CaseInsensitive)) {
        suggestedName += QStringLiteral(".pptx");
    }

    QString targetPath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("保存PPT"),
        QDir::homePath() + "/Desktop/" + suggestedName,
        QStringLiteral("PowerPoint 演示文稿 (*.pptx)")
    );

    if (targetPath.isEmpty()) {
        return false;
    }

    if (!targetPath.endsWith(QStringLiteral(".pptx"), Qt::CaseInsensitive)) {
        targetPath += QStringLiteral(".pptx");
    }

    if (QFile::exists(targetPath) && !QFile::remove(targetPath)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), QStringLiteral("目标文件已存在且无法覆盖，请检查权限。"));
        return false;
    }

    if (!QFile::copy(sourcePath, targetPath)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), QStringLiteral("PPT 文件复制失败，请稍后重试。"));
        return false;
    }

    return true;
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

QString ModernMainWindow::extractPPTTopic(const QString &message) const
{
    // 从用户消息中提取 PPT 主题
    QString msg = message.trimmed();

    auto cleanupTopic = [](QString topic) {
        topic = topic.trimmed();
        topic.remove(QRegularExpression(R"(^(?:[0-9一二两三四五六七八九十百]+)\s*(?:页|张|份|个|套))"));
        topic.remove(QRegularExpression(R"(^(?:关于))"));
        topic.remove(QRegularExpression("^[的了一个]+"));
        topic.remove(QRegularExpression("[的了吧呢]+$"));
        return topic.trimmed();
    };

    // 常见句式：「帮我生成一个关于XX的PPT」「制作XX课件」等
    QStringList patterns = {
        "关于(.+?)的(?:ppt|PPT|课件|幻灯片|演示文稿)",
        "(?:生成|制作|做一个|创建)(.+?)(?:ppt|PPT|课件|幻灯片|演示文稿)",
        "(.+?)(?:ppt|PPT|课件|幻灯片|演示文稿)",
    };

    for (const QString &pat : patterns) {
        QRegularExpression re(pat);
        QRegularExpressionMatch match = re.match(msg);
        if (match.hasMatch()) {
            QString topic = cleanupTopic(match.captured(1));
            if (!topic.isEmpty() && topic.length() > 1) {
                return topic;
            }
        }
    }

    // 兜底：去掉关键词后剩余部分作为主题
    QString fallback = msg;
    for (const QString &kw : {"帮我", "请", "生成", "制作", "做一个", "创建", "一个", "ppt", "PPT", "课件", "幻灯片", "演示文稿"}) {
        fallback.remove(kw);
    }
    fallback = cleanupTopic(fallback);
    return fallback.isEmpty() ? "思政教学" : fallback;
}

void ModernMainWindow::startPPTQuestionFlow(const QString &userMessage)
{
    if (!m_bubbleChatWidget) {
        return;
    }

    resetPPTQuestionFlow();
    m_isPptQuestionFlowActive = true;
    m_pptQuestionStep = 0;
    m_pendingPptRequest = userMessage;
    m_pptTopic = extractPPTTopic(userMessage);
    m_pptUserPreferences.clear();  // 清空上次的偏好

    m_bubbleChatWidget->setPlaceholderText(QStringLiteral("可直接输入回答，或点击上方选项"));
    m_bubbleChatWidget->addMessage(QStringLiteral("好的，我先快速了解一下这次课件偏好，这样更像真实备课流程。"), false);

    QTimer::singleShot(260, this, [this]() {
        if (m_isPptQuestionFlowActive) {
            askNextPPTQuestion();
        }
    });
}

void ModernMainWindow::askNextPPTQuestion()
{
    if (!m_bubbleChatWidget || !m_isPptQuestionFlowActive) {
        return;
    }

    const QList<PPTQuestionConfig> questions = buildPPTQuestionFlow();
    if (m_pptQuestionStep < 0 || m_pptQuestionStep >= questions.size()) {
        return;
    }

    const PPTQuestionConfig &question = questions.at(m_pptQuestionStep);
    m_bubbleChatWidget->addMessage(question.question, false);
    m_bubbleChatWidget->addQuickReplyOptions(question.options);
}

void ModernMainWindow::handlePPTQuestionAnswer(const QString &answer)
{
    if (!m_isPptQuestionFlowActive) {
        return;
    }

    // 收集用户偏好——根据当前步骤存入对应 key
    switch (m_pptQuestionStep) {
    case 0: m_pptUserPreferences["pref_scene"] = answer; break;  // 授课场景
    case 1: m_pptUserPreferences["pref_style"] = answer; break;  // 表达风格
    case 2: m_pptUserPreferences["pref_focus"] = answer; break;  // 内容重点
    case 3: m_pptUserPreferences["pref_pace"]  = answer; break;  // 呈现节奏
    default: break;
    }
    qDebug() << "[PPT] 用户偏好收集 step" << m_pptQuestionStep << ":" << answer;

    ++m_pptQuestionStep;
    const int totalQuestions = buildPPTQuestionFlow().size();
    if (m_pptQuestionStep < totalQuestions) {
        QTimer::singleShot(260, this, [this]() {
            if (m_isPptQuestionFlowActive) {
                askNextPPTQuestion();
            }
        });
        return;
    }

    const QString pendingRequest = m_pendingPptRequest;
    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->addMessage(QStringLiteral("好的，偏好我记下了，现在开始为你整理课件。"), false);
    }

    resetPPTQuestionFlow();

    QTimer::singleShot(420, this, [this, pendingRequest]() {
        if (!pendingRequest.isEmpty()) {
            startPPTSimulation(pendingRequest);
        }
    });
}

void ModernMainWindow::resetPPTQuestionFlow()
{
    m_isPptQuestionFlowActive = false;
    m_pptQuestionStep = 0;
    m_pendingPptRequest.clear();

    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->setPlaceholderText(QStringLiteral("向AI助手发送信息..."));
    }
}

void ModernMainWindow::startPPTSimulation(const QString &userMessage)
{
    if (!m_bubbleChatWidget) return;

    // 提取主题
    m_pptTopic = extractPPTTopic(userMessage);
    qDebug() << "[PPT] 模拟生成，提取主题:" << m_pptTopic;

    // 如果之前有正在进行的 PPT 生成，先停止
    if (m_pptAgentService->currentState() != ZhipuPPTAgentService::State::Idle &&
        m_pptAgentService->currentState() != ZhipuPPTAgentService::State::Finished &&
        m_pptAgentService->currentState() != ZhipuPPTAgentService::State::Failed) {
        m_pptAgentService->cancel();
    }
    
    // 在最后添加 AI 回复占位
    if (m_bubbleChatWidget) {
        m_bubbleChatWidget->addMessage("好的，我来帮您生成关于「" + m_pptTopic + "」的课件...\n\n⏳ 正在准备生成环境...", false);
    }

    // 构建生成参数
    QMap<QString, QString> params;
    params["topic"] = m_pptTopic;
    params["userRequest"] = userMessage;

    // 注入用户偏好（从提问流程收集而来）
    for (auto it = m_pptUserPreferences.constBegin(); it != m_pptUserPreferences.constEnd(); ++it) {
        params[it.key()] = it.value();
    }
    if (!m_pptUserPreferences.isEmpty()) {
        qDebug() << "[PPT] 已注入用户偏好:" << m_pptUserPreferences;
    }
    
    // 启动 PPT Agent
    m_pptAgentService->generate(params);
}

void ModernMainWindow::handleChatPPTComplete(const QStringList &svgCodes, const QVector<QImage> &previews)
{
    // 生成 PPTX 文件
    QString pptxPath;
    QString pptxError;
    if (!m_pptxGenerator) {
        pptxError = "PPTXGenerator 未初始化";
        qWarning() << "[PPTAgent]" << pptxError;
    } else if (previews.isEmpty() && svgCodes.isEmpty()) {
        pptxError = "生成结果为空，无法导出 PPTX";
        qWarning() << "[PPTAgent]" << pptxError;
    } else {
        QString safeName = m_pptTopic;
        safeName.remove(QRegularExpression(R"([\\/:*?"<>|])"));
        if (safeName.isEmpty()) safeName = "ppt_output";

        // 使用持久化数据目录保存 PPT，而非临时目录（避免退出后文件丢失）
        QString pptDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/ppt";
        QDir().mkpath(pptDir);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        pptxPath = pptDir + "/" + safeName + "_" + timestamp + ".pptx";
        
        qDebug() << "[PPTAgent] 聊天模式 PPTX 生成开始，目标路径:" << pptxPath
                 << "preview数量:" << previews.size()
                 << "svg数量:" << svgCodes.size();
        
        bool ok = !previews.isEmpty()
            ? m_pptxGenerator->generateFromImages(m_pptTopic, previews, pptxPath)
            : false;
        qDebug() << "[PPTAgent] 聊天模式 PPTX 生成:" << (ok ? "成功" : "失败") << pptxPath;
        if (!ok) {
            pptxError = m_pptxGenerator->lastError();
            qWarning() << "[PPTAgent] PPTXGenerator error:" << pptxError;
            if (!QFile::exists(pptxPath)) {
                pptxPath.clear();
            }
        }
    }

    if (!pptxPath.isEmpty() && QFile::exists(pptxPath)) {
        m_pendingPPTPath = pptxPath;

        QString finalMsg =
            "好的，我来帮您生成关于「" + m_pptTopic + "」的课件...\n\n"
            "✅ **PPT 生成完成！**\n\n"
            "---\n\n"
            "📎 **" + QFileInfo(m_pendingPPTPath).fileName() + "**\n\n"
            "已为您打开应用内预览页面，您可以先查看效果，再点击下载保存。";

        m_bubbleChatWidget->updateLastAIMessage(finalMsg);
        showPPTPreviewPage(m_pendingPPTPath, m_pptTopic);

        AIHistoryEntry entry;
        entry.conversationId = "ppt_" + QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
        entry.type = AIHistoryType::Chat;
        entry.title = "PPT生成：" + m_pptTopic;
        entry.updatedAt = QDateTime::currentDateTime();
        entry.localFilePath = m_pendingPPTPath;
        upsertHistoryEntry(entry);
    } else {
        QString errMsg = "抱歉，PPT 生成完成但文件保存失败";
        if (!pptxError.isEmpty()) {
            errMsg += "：" + pptxError;
        }
        errMsg += "\n\n请稍后重试。";
        m_bubbleChatWidget->updateLastAIMessage(errMsg);
    }
}

void ModernMainWindow::onPPTSimulationStep()
{
    // 该方法已废弃，由真实的 ZhipuPPTAgentService 替代
}

// ========== 通知系统相关槽函数 ==========

void ModernMainWindow::onNotificationBtnClicked()
{
    if (!m_notificationWidget) return;

    if (m_notificationWidget->isPopupVisible()) {
        m_notificationWidget->hidePopup();
    } else {
        // 计算弹窗位置（在通知按钮下方）
        QPoint btnPos = notificationBtn->mapToGlobal(QPoint(0, notificationBtn->height()));
        // 弹窗右对齐到按钮
        int popupX = btnPos.x() + notificationBtn->width() - m_notificationWidget->width();
        int popupY = btnPos.y() + 8;
        m_notificationWidget->move(popupX, popupY);
        m_notificationWidget->showPopup();
    }
}

void ModernMainWindow::onUnreadCountChanged(int count)
{
    if (m_notificationBadge) {
        m_notificationBadge->setCount(count);
    }
}
