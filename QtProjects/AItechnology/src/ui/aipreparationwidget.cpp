#include "aipreparationwidget.h"

#include <QApplication>
#include <QDebug>
#include <QDrag>
#include <QEasingCurve>
#include <QFileDialog>
#include <QGridLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QMimeData>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyle>
#include <QTimer>

namespace {
constexpr char kPreviewDragMimeType[] = "application/x-aipreview-index";
constexpr int kVisibleSlideSlots = 3;
}

AIPreparationWidget::AIPreparationWidget(QWidget *parent, bool enableOnlineEdit)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_subtitleLabel(nullptr)
    , m_textbookCombo(nullptr)
    , m_gradeCombo(nullptr)
    , m_chapterCombo(nullptr)
    , m_selectionCard(nullptr)
    , m_generateBtn(nullptr)
    , m_toggleAdvancedBtn(nullptr)
    , m_advancedSection(nullptr)
    , m_advancedLayout(nullptr)
    , m_advancedAnimation(nullptr)
    , m_advancedMaxHeight(0)
    , m_animationProgress(0)
    , m_selectedTemplateKey()
    , m_durationCombo(nullptr)
    , m_contentFocusEdit(nullptr)
    , m_previewContainer(nullptr)
    , m_previewGrid(nullptr)
    , m_statusStack(nullptr)
    , m_generatingStatus(nullptr)
    , m_successStatus(nullptr)
    , m_failedStatus(nullptr)
    , m_progressDot(nullptr)
    , m_progressLabel(nullptr)
    , m_successChipLabel(nullptr)
    , m_failedChipLabel(nullptr)
    , m_progressTimer(nullptr)
    , m_progressDotVisible(true)
    , m_actionsBar(nullptr)
    , m_downloadBtn(nullptr)
    , m_saveBtn(nullptr)
    , m_onlineEditBtn(nullptr)
    , m_regenerateBtn(nullptr)
    , m_previewPlaceholderCard(nullptr)
    , m_placeholderLabel(nullptr)
    , m_placeholderIcon(nullptr)
    , m_enableOnlineEdit(enableOnlineEdit)
    , m_dragStartPos()
    , m_dragSourceIndex(-1)
    , m_dragging(false)
    , m_activePreviewDialog(nullptr)
    , m_state(GenerationState::Idle)
    , m_currentProgress(0)
{
    setObjectName(QStringLiteral("AIPreparationWidget"));
    setAttribute(Qt::WA_StyledBackground);
    setAcceptDrops(true);

    setupConstants();
    initUI();
}

AIPreparationWidget::~AIPreparationWidget() = default;

void AIPreparationWidget::setupConstants()
{
    COLOR_PRIMARY = "#c00000";
    COLOR_ACCENT = "#409EFF";
    COLOR_TEXT = "#303133";
    COLOR_SUBTEXT = "#909399";
    COLOR_BORDER = "#e4e7ed";
    COLOR_SURFACE = "#ffffff";
    COLOR_BG_LIGHT = "#f5f7fa";
    COLOR_BG_DARK = "#1d1d1f";
    COLOR_SUCCESS = "#67C23A";
    COLOR_FAILED = "#F56C6C";
    WIDGET_HEIGHT = 36;
    CORNER_RADIUS = 12;
    BORDER_WIDTH = 2;
    SPACING_SMALL = 8;
    SPACING_MEDIUM = 16;
    SPACING_LARGE = 24;
    TEMPLATE_CARD_WIDTH = 228;
    TEMPLATE_CARD_HEIGHT = 150;
    PREVIEW_CARD_HEIGHT = 180;
}

void AIPreparationWidget::initUI()
{
    setStyleSheet(QStringLiteral("QWidget#AIPreparationWidget { background-color: %1; }").arg(COLOR_BG_LIGHT));

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(32, 32, 32, 32);
    m_mainLayout->setSpacing(SPACING_LARGE);

    setupTopSection();
    setupSelectionSection();
    setupAdvancedSection();
    setupPreviewSection();
    setupActionsBar();

    m_mainLayout->addStretch();
}

void AIPreparationWidget::setupTopSection()
{
    QHBoxLayout *breadcrumbLayout = new QHBoxLayout();
    breadcrumbLayout->setContentsMargins(0, 0, 0, 0);
    breadcrumbLayout->setSpacing(SPACING_SMALL);

    QLabel *homeLabel = new QLabel(QStringLiteral("首页"));
    homeLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 500;").arg(COLOR_SUBTEXT));
    homeLabel->setCursor(Qt::PointingHandCursor);

    QLabel *separator = new QLabel(QStringLiteral("/"));
    separator->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 500;").arg(COLOR_SUBTEXT));

    QLabel *currentLabel = new QLabel(QStringLiteral("AI智能备课"));
    currentLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 500;").arg(COLOR_TEXT));

    breadcrumbLayout->addWidget(homeLabel);
    breadcrumbLayout->addWidget(separator);
    breadcrumbLayout->addWidget(currentLabel);
    breadcrumbLayout->addStretch();

    m_mainLayout->addLayout(breadcrumbLayout);

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(SPACING_SMALL);

    m_titleLabel = new QLabel(QStringLiteral("AI智能备课 · PPT生成"));
    m_titleLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 30px; font-weight: 700;").arg(COLOR_TEXT));

    m_subtitleLabel = new QLabel(QStringLiteral("根据年级和章节，一键生成思政课堂教学PPT。"));
    m_subtitleLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 16px;").arg(COLOR_SUBTEXT));

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(m_subtitleLabel);

    m_mainLayout->addLayout(titleLayout);
}

QWidget *AIPreparationWidget::createLabeledField(const QString &labelText, QWidget *field)
{
    QWidget *wrapper = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wrapper);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(SPACING_SMALL);

    QLabel *label = new QLabel(labelText);
    label->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 500;").arg(COLOR_TEXT));

    layout->addWidget(label);
    layout->addWidget(field);

    wrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return wrapper;
}

void AIPreparationWidget::setupSelectionSection()
{
    m_selectionCard = new QFrame();
    m_selectionCard->setObjectName(QStringLiteral("selectionCard"));
    m_selectionCard->setStyleSheet(QStringLiteral(
        "QFrame#selectionCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 12px;"
        "}"
    ).arg(COLOR_SURFACE, COLOR_BORDER));

    QVBoxLayout *cardLayout = new QVBoxLayout(m_selectionCard);
    cardLayout->setContentsMargins(SPACING_LARGE, SPACING_LARGE, SPACING_LARGE, SPACING_LARGE);
    cardLayout->setSpacing(SPACING_MEDIUM);

    QLabel *sectionTitle = new QLabel(QStringLiteral("选择课程范围并生成PPT"));
    sectionTitle->setStyleSheet(QStringLiteral("color: %1; font-size: 18px; font-weight: 600;").arg(COLOR_TEXT));
    cardLayout->addWidget(sectionTitle);

    const QString comboStyle = QStringLiteral(
        "QComboBox {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "  padding: 0 12px;"
        "  color: %3;"
        "  font-size: 14px;"
        "}"
        "QComboBox:focus {"
        "  border: 2px solid %4;"
        "}"
        "QComboBox::drop-down {"
        "  width: 28px;"
        "  border: none;"
        "}"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 5px solid transparent;"
        "  border-right: 5px solid transparent;"
        "  border-top: 6px solid %3;"
        "  margin-right: 6px;"
        "}"
    ).arg(COLOR_SURFACE, COLOR_BORDER, COLOR_TEXT, COLOR_ACCENT);

    m_textbookCombo = new QComboBox();
    m_textbookCombo->addItems({QStringLiteral("人教版"), QStringLiteral("部编版"), QStringLiteral("粤教版")});
    m_textbookCombo->setCurrentIndex(0);
    m_textbookCombo->setFixedHeight(WIDGET_HEIGHT);
    m_textbookCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_textbookCombo->setStyleSheet(comboStyle);

    m_gradeCombo = new QComboBox();
    m_gradeCombo->addItems({QStringLiteral("七年级"), QStringLiteral("八年级"), QStringLiteral("九年级")});
    m_gradeCombo->setCurrentIndex(1);
    m_gradeCombo->setFixedHeight(WIDGET_HEIGHT);
    m_gradeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_gradeCombo->setStyleSheet(comboStyle);

    m_chapterCombo = new QComboBox();
    m_chapterCombo->addItems({QStringLiteral("第一章：爱我中华"),
                              QStringLiteral("第二章：国家利益至上"),
                              QStringLiteral("第三章：法治中国")});
    m_chapterCombo->setCurrentIndex(1);
    m_chapterCombo->setFixedHeight(WIDGET_HEIGHT);
    m_chapterCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_chapterCombo->setStyleSheet(comboStyle);

    QGridLayout *rowLayout = new QGridLayout();
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setHorizontalSpacing(SPACING_MEDIUM);
    rowLayout->setVerticalSpacing(0);

    rowLayout->addWidget(createLabeledField(QStringLiteral("选择教材版本"), m_textbookCombo), 0, 0);
    rowLayout->addWidget(createLabeledField(QStringLiteral("选择年级"), m_gradeCombo), 0, 1);
    rowLayout->addWidget(createLabeledField(QStringLiteral("选择章节"), m_chapterCombo), 0, 2);

    QWidget *buttonsWrapper = new QWidget();
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsWrapper);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(SPACING_SMALL);
    buttonsLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_generateBtn = new QPushButton(QStringLiteral("生成PPT"));
    m_generateBtn->setCursor(Qt::PointingHandCursor);
    m_generateBtn->setFixedSize(120, WIDGET_HEIGHT);
    m_generateBtn->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    m_generateBtn->setIconSize(QSize(18, 18));
    m_generateBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: %1;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #a00000; }"
        "QPushButton:pressed { background-color: #880000; }"
        "QPushButton:focus { border: 2px solid %2; outline: none; }"
    ).arg(COLOR_PRIMARY, COLOR_ACCENT));

    m_toggleAdvancedBtn = new QPushButton();
    m_toggleAdvancedBtn->setCursor(Qt::PointingHandCursor);
    m_toggleAdvancedBtn->setCheckable(true);
    m_toggleAdvancedBtn->setFixedSize(80, WIDGET_HEIGHT);
    m_toggleAdvancedBtn->setText(QStringLiteral("高级"));
    m_toggleAdvancedBtn->setIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    m_toggleAdvancedBtn->setIconSize(QSize(18, 18));
    m_toggleAdvancedBtn->setToolTip(QStringLiteral("高级选项（展开/收起）"));
    m_toggleAdvancedBtn->setStatusTip(QStringLiteral("打开或收起高级选项面板"));
    m_toggleAdvancedBtn->setWhatsThis(QStringLiteral("点击以展开或收起高级选项，包括 PPT 模板、课时长度与内容侧重等设置。"));
    m_toggleAdvancedBtn->setAccessibleName(QStringLiteral("高级选项切换"));
    m_toggleAdvancedBtn->setAccessibleDescription(QStringLiteral("展开或收起高级选项面板"));
    m_toggleAdvancedBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}"
        "QPushButton:hover { border-color: %3; }"
        "QPushButton:checked { background-color: %3; color: white; }"
        "QPushButton:focus { border: 2px solid %3; outline: none; }"
    ).arg(COLOR_SURFACE, COLOR_BORDER, COLOR_ACCENT));

    connect(m_generateBtn, &QPushButton::clicked, this, &AIPreparationWidget::onGenerateClicked);
    connect(m_toggleAdvancedBtn, &QPushButton::toggled, this, &AIPreparationWidget::onToggleAdvanced);

    buttonsLayout->addWidget(m_generateBtn);
    buttonsLayout->addWidget(m_toggleAdvancedBtn);
    buttonsWrapper->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    rowLayout->addWidget(buttonsWrapper, 0, 3, Qt::AlignRight);

    for (int col = 0; col < 3; ++col) {
        rowLayout->setColumnStretch(col, 1);
    }
    rowLayout->setColumnStretch(3, 0);

    cardLayout->addLayout(rowLayout);

    m_mainLayout->addWidget(m_selectionCard);
}

void AIPreparationWidget::setupAdvancedSection()
{
    m_advancedSection = new QFrame();
    m_advancedSection->setObjectName(QStringLiteral("advancedSection"));
    m_advancedSection->setStyleSheet(QStringLiteral(
        "QFrame#advancedSection {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}"
    ).arg(COLOR_BG_LIGHT, COLOR_BORDER));
    m_advancedSection->setVisible(false);
    m_advancedSection->setMaximumHeight(0);

    m_advancedLayout = new QVBoxLayout(m_advancedSection);
    m_advancedLayout->setContentsMargins(SPACING_MEDIUM, SPACING_MEDIUM, SPACING_MEDIUM, SPACING_MEDIUM);
    m_advancedLayout->setSpacing(SPACING_MEDIUM);

    QLabel *title = new QLabel(QStringLiteral("高级选项"));
    title->setStyleSheet(QStringLiteral("color: %1; font-size: 16px; font-weight: 600;").arg(COLOR_TEXT));
    m_advancedLayout->addWidget(title);

    QWidget *rowOne = new QWidget();
    QHBoxLayout *rowOneLayout = new QHBoxLayout(rowOne);
    rowOneLayout->setContentsMargins(0, 0, 0, 0);
    rowOneLayout->setSpacing(SPACING_MEDIUM);

    QHBoxLayout *templatesLayout = new QHBoxLayout();
    templatesLayout->setContentsMargins(0, 0, 0, 0);
    templatesLayout->setSpacing(SPACING_MEDIUM);

    m_templateCards.clear();
    m_templateNameLabels.clear();

    createTemplateCard(QStringLiteral("red"), QStringLiteral("党政红"), QColor(COLOR_PRIMARY), true);
    createTemplateCard(QStringLiteral("business"), QStringLiteral("商务蓝"), QColor(COLOR_ACCENT), false);
    createTemplateCard(QStringLiteral("clean"), QStringLiteral("简约白"), QColor("#d9d9d9"), false);

    templatesLayout->addWidget(m_templateCards.value(QStringLiteral("red")));
    templatesLayout->addWidget(m_templateCards.value(QStringLiteral("business")));
    templatesLayout->addWidget(m_templateCards.value(QStringLiteral("clean")));

    QWidget *templatesColumnWidget = new QWidget();
    templatesColumnWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    templatesColumnWidget->setMinimumHeight(TEMPLATE_CARD_HEIGHT + 50);

    QVBoxLayout *templatesColumn = new QVBoxLayout(templatesColumnWidget);
    templatesColumn->setContentsMargins(0, 0, 0, 0);
    templatesColumn->setSpacing(SPACING_SMALL);

    QLabel *templatesLabel = new QLabel(QStringLiteral("PPT风格模板"));
    templatesLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 500;").arg(COLOR_TEXT));
    templatesColumn->addWidget(templatesLabel);
    templatesColumn->addLayout(templatesLayout);

    rowOneLayout->addWidget(templatesColumnWidget, 1, Qt::AlignTop);

    m_durationCombo = new QComboBox();
    m_durationCombo->addItems({QStringLiteral("45分钟标准课时"), QStringLiteral("20分钟精讲"), QStringLiteral("60分钟深化")});
    m_durationCombo->setCurrentIndex(0);
    m_durationCombo->setFixedHeight(WIDGET_HEIGHT);
    m_durationCombo->setStyleSheet(QStringLiteral(
        "QComboBox {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "  padding: 0 12px;"
        "  color: %3;"
        "  font-size: 14px;"
        "}"
        "QComboBox:focus { border: 2px solid %4; }"
        "QComboBox::drop-down { width: 28px; border: none; }"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 5px solid transparent;"
        "  border-right: 5px solid transparent;"
        "  border-top: 6px solid %3;"
        "  margin-right: 6px;"
        "}"
    ).arg(COLOR_SURFACE, COLOR_BORDER, COLOR_TEXT, COLOR_ACCENT));

    QWidget *durationWrapper = createLabeledField(QStringLiteral("课时长度"), m_durationCombo);
    durationWrapper->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    durationWrapper->setMinimumWidth(220);
    rowOneLayout->addWidget(durationWrapper, 0, Qt::AlignTop);
    rowOneLayout->setStretch(0, 1);
    rowOneLayout->setStretch(1, 0);

    m_advancedLayout->addWidget(rowOne);
    m_advancedLayout->addSpacing(SPACING_LARGE);  // 增加模板区与内容侧重的间距

    m_contentFocusEdit = new QLineEdit();
    m_contentFocusEdit->setPlaceholderText(QStringLiteral("例如：重点讲解英美案例、增加爱国主义教育内容"));
    m_contentFocusEdit->setFixedHeight(WIDGET_HEIGHT);
    m_contentFocusEdit->setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "  padding: 0 12px;"
        "  color: %3;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus { border: 2px solid %4; }"
    ).arg(COLOR_SURFACE, COLOR_BORDER, COLOR_TEXT, COLOR_ACCENT));
    QWidget *focusWrapper = createLabeledField(QStringLiteral("内容侧重"), m_contentFocusEdit);
    focusWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_advancedLayout->addWidget(focusWrapper);

    m_mainLayout->addWidget(m_advancedSection);

    m_advancedMaxHeight = m_advancedSection->sizeHint().height() + 40;  // 增加40px缓冲

    m_advancedAnimation = new QPropertyAnimation(m_advancedSection, "maximumHeight", this);
    m_advancedAnimation->setDuration(320);  // 延长动画时长
    m_advancedAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    connect(m_advancedAnimation, &QPropertyAnimation::finished, this, [this]() {
        if (!m_toggleAdvancedBtn->isChecked()) {
            m_advancedSection->setVisible(false);
        }
    });
}

void AIPreparationWidget::setupStatusStack(QHBoxLayout *headerLayout)
{
    m_statusStack = new QStackedWidget();
    m_statusStack->setVisible(false);
    m_statusStack->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    m_generatingStatus = new QFrame();
    QHBoxLayout *generatingLayout = new QHBoxLayout(m_generatingStatus);
    generatingLayout->setContentsMargins(SPACING_SMALL, 0, SPACING_SMALL, 0);
    generatingLayout->setSpacing(SPACING_SMALL);

    m_progressDot = new QLabel();
    m_progressDot->setFixedSize(10, 10);
    m_progressDot->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 5px;").arg(COLOR_ACCENT));

    m_progressLabel = new QLabel(QStringLiteral("正在分析教学目标，智能生成中… 0%"));
    m_progressLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 500;").arg(COLOR_ACCENT));

    generatingLayout->addWidget(m_progressDot);
    generatingLayout->addWidget(m_progressLabel);

    m_statusStack->addWidget(m_generatingStatus);

    m_successStatus = new QFrame();
    QHBoxLayout *successLayout = new QHBoxLayout(m_successStatus);
    successLayout->setContentsMargins(0, 0, 0, 0);
    successLayout->setSpacing(0);

    m_successChipLabel = new QLabel(QStringLiteral("生成完毕！"));
    m_successChipLabel->setAlignment(Qt::AlignCenter);
    m_successChipLabel->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  background-color: %1;"
        "  color: white;"
        "  border-radius: 16px;"
        "  padding: 6px 16px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
    ).arg(COLOR_SUCCESS));

    successLayout->addWidget(m_successChipLabel);
    m_statusStack->addWidget(m_successStatus);

    m_failedStatus = new QFrame();
    QHBoxLayout *failedLayout = new QHBoxLayout(m_failedStatus);
    failedLayout->setContentsMargins(SPACING_SMALL, 0, SPACING_SMALL, 0);
    failedLayout->setSpacing(SPACING_SMALL);

    QLabel *failedIcon = new QLabel();
    failedIcon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-error")).pixmap(18, 18));

    m_failedChipLabel = new QLabel(QStringLiteral("生成失败，请重试"));
    m_failedChipLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 500;").arg(COLOR_FAILED));

    failedLayout->addWidget(failedIcon);
    failedLayout->addWidget(m_failedChipLabel);
    m_statusStack->addWidget(m_failedStatus);

    headerLayout->addWidget(m_statusStack);
}

void AIPreparationWidget::setupPreviewSection()
{
    m_previewContainer = new QFrame();
    QVBoxLayout *containerLayout = new QVBoxLayout(m_previewContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(SPACING_MEDIUM);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(SPACING_SMALL);

    QLabel *title = new QLabel(QStringLiteral("PPT生成预览"));
    title->setStyleSheet(QStringLiteral("color: %1; font-size: 18px; font-weight: 600;").arg(COLOR_TEXT));
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    setupStatusStack(headerLayout);

    containerLayout->addLayout(headerLayout);

    m_previewGrid = new QGridLayout();
    m_previewGrid->setContentsMargins(0, 0, 0, 0);
    m_previewGrid->setHorizontalSpacing(SPACING_MEDIUM);
    m_previewGrid->setVerticalSpacing(SPACING_MEDIUM);

    m_previewCards.clear();
    m_previewOverlays.clear();
    m_previewButtons.clear();
    m_previewImageLabels.clear();
    m_previewCaptionLabels.clear();

    for (int i = 0; i < kVisibleSlideSlots; ++i) {
        createPreviewCard(i);
        m_previewGrid->addWidget(m_previewCards[i], 0, i);
    }

    m_previewPlaceholderCard = new QFrame();
    m_previewPlaceholderCard->setObjectName(QStringLiteral("previewPlaceholder"));
    m_previewPlaceholderCard->setFixedHeight(PREVIEW_CARD_HEIGHT);
    m_previewPlaceholderCard->setStyleSheet(QStringLiteral(
        "QFrame#previewPlaceholder {"
        "  border: 1px dashed %1;"
        "  border-radius: 12px;"
        "  background-color: rgba(255,255,255,0.4);"
        "}"
    ).arg(COLOR_BORDER));

    QVBoxLayout *placeholderLayout = new QVBoxLayout(m_previewPlaceholderCard);
    placeholderLayout->setContentsMargins(0, 0, 0, 0);
    placeholderLayout->setSpacing(SPACING_SMALL);
    placeholderLayout->setAlignment(Qt::AlignCenter);

    m_placeholderIcon = new QLabel();
    m_placeholderIcon->setAlignment(Qt::AlignCenter);
    placeholderLayout->addWidget(m_placeholderIcon);

    m_placeholderLabel = new QLabel();
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 500;").arg(COLOR_SUBTEXT));
    placeholderLayout->addWidget(m_placeholderLabel);

    m_previewGrid->addWidget(m_previewPlaceholderCard, 0, kVisibleSlideSlots);

    containerLayout->addLayout(m_previewGrid);

    m_mainLayout->addWidget(m_previewContainer);

    updatePlaceholderCard();
}

void AIPreparationWidget::setupActionsBar()
{
    m_actionsBar = new QFrame();
    m_actionsBar->setVisible(false);
    m_actionsBar->setObjectName(QStringLiteral("actionsBar"));
    m_actionsBar->setStyleSheet(QStringLiteral(
        "QFrame#actionsBar {"
        "  border: none;"
        "}"
    ));

    QHBoxLayout *actionsLayout = new QHBoxLayout(m_actionsBar);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(12);
    actionsLayout->setAlignment(Qt::AlignRight);

    auto outlineStyle = QStringLiteral(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: %1;"
        "  border: 1px solid %1;"
        "  border-radius: 8px;"
        "  padding: 0 20px;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  min-width: 120px;"
        "}"
        "QPushButton:hover { background-color: rgba(192,0,0,0.06); }"
        "QPushButton:focus { border: 2px solid %2; outline: none; }"
    ).arg(COLOR_PRIMARY, COLOR_ACCENT);

    m_downloadBtn = new QPushButton(QStringLiteral("下载PPT"));
    m_downloadBtn->setCursor(Qt::PointingHandCursor);
    m_downloadBtn->setFixedHeight(WIDGET_HEIGHT);
    m_downloadBtn->setIcon(QIcon::fromTheme(QStringLiteral("download")));
    m_downloadBtn->setIconSize(QSize(18, 18));
    m_downloadBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: %1;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 0 24px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #a00000; }"
        "QPushButton:focus { border: 2px solid %2; outline: none; }"
    ).arg(COLOR_PRIMARY, COLOR_ACCENT));

    m_saveBtn = new QPushButton(QStringLiteral("保存到我的备课"));
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setFixedHeight(WIDGET_HEIGHT);
    m_saveBtn->setIcon(QIcon::fromTheme(QStringLiteral("bookmark-new")));
    m_saveBtn->setIconSize(QSize(18, 18));
    m_saveBtn->setStyleSheet(outlineStyle);

    if (m_enableOnlineEdit) {
        m_onlineEditBtn = new QPushButton(QStringLiteral("在线编辑"));
        m_onlineEditBtn->setCursor(Qt::PointingHandCursor);
        m_onlineEditBtn->setFixedHeight(WIDGET_HEIGHT);
        m_onlineEditBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
        m_onlineEditBtn->setIconSize(QSize(18, 18));
        m_onlineEditBtn->setStyleSheet(outlineStyle);
    }

    m_regenerateBtn = new QPushButton(QStringLiteral("重新生成"));
    m_regenerateBtn->setCursor(Qt::PointingHandCursor);
    m_regenerateBtn->setFixedHeight(WIDGET_HEIGHT);
    m_regenerateBtn->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    m_regenerateBtn->setIconSize(QSize(18, 18));
    m_regenerateBtn->setStyleSheet(outlineStyle);

    connect(m_downloadBtn, &QPushButton::clicked, this, &AIPreparationWidget::onDownloadClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &AIPreparationWidget::onSaveClicked);
    if (m_onlineEditBtn) {
        connect(m_onlineEditBtn, &QPushButton::clicked, this, &AIPreparationWidget::onOnlineEditClicked);
    }
    connect(m_regenerateBtn, &QPushButton::clicked, this, &AIPreparationWidget::onRegenerateClicked);

    actionsLayout->addWidget(m_downloadBtn);
    actionsLayout->addWidget(m_saveBtn);
    if (m_onlineEditBtn) {
        actionsLayout->addWidget(m_onlineEditBtn);
    }
    actionsLayout->addWidget(m_regenerateBtn);

    m_mainLayout->addWidget(m_actionsBar);
}

void AIPreparationWidget::createTemplateCard(const QString &key, const QString &name, const QColor &thumbnailColor, bool isSelected)
{
    QFrame *card = new QFrame();
    card->setObjectName(QStringLiteral("templateCard_%1").arg(key));
    card->setCursor(Qt::PointingHandCursor);
    card->setFocusPolicy(Qt::StrongFocus);
    card->setFixedHeight(TEMPLATE_CARD_HEIGHT);
    card->setFixedWidth(TEMPLATE_CARD_WIDTH);
    card->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    card->setProperty("templateKey", key);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(SPACING_SMALL, SPACING_SMALL, SPACING_SMALL, SPACING_SMALL);
    cardLayout->setSpacing(SPACING_SMALL);

    QLabel *thumbnail = new QLabel();
    thumbnail->setAlignment(Qt::AlignCenter);
    thumbnail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    thumbnail->setMinimumHeight(64);

    const int thumbWidth = TEMPLATE_CARD_WIDTH - SPACING_SMALL * 2;
    const int reservedForName = 22;
    const int reservedMargins = 0;
    const int thumbHeight = qMax(64, TEMPLATE_CARD_HEIGHT - reservedForName - reservedMargins - SPACING_SMALL * 3);
    thumbnail->setFixedSize(thumbWidth, thumbHeight);
    thumbnail->setStyleSheet(QStringLiteral(
        "background-color: %1;"
        "border-radius: 8px;"
    ).arg(thumbnailColor.name()));

    QLabel *nameLabel = new QLabel(name);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setFixedHeight(22);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setWordWrap(false);

    cardLayout->addWidget(thumbnail, 0, Qt::AlignHCenter);
    cardLayout->addWidget(nameLabel);

    m_templateCards.insert(key, card);
    m_templateNameLabels.insert(key, nameLabel);

    card->installEventFilter(this);

    if (isSelected) {
        m_selectedTemplateKey = key;
    }

    updateTemplateCardStyle(key, isSelected);
}

void AIPreparationWidget::updateTemplateCardStyle(const QString &key, bool isSelected)
{
    QFrame *card = m_templateCards.value(key, nullptr);
    if (!card) {
        return;
    }

    const QString borderColor = isSelected ? COLOR_PRIMARY : COLOR_BORDER;
    QString style = QStringLiteral(
        "QFrame#templateCard_%1 {"
        "  background-color: %2;"
        "  border: %3px solid %4;"
        "  border-radius: 12px;"
        "}"
        "QFrame#templateCard_%1:hover {"
        "  border: 2px solid %5;"
        "}"
        "QFrame#templateCard_%1:focus {"
        "  border: 2px solid %6;"
        "}"
    ).arg(key, COLOR_SURFACE, isSelected ? QStringLiteral("2") : QStringLiteral("1"), borderColor, COLOR_PRIMARY, COLOR_ACCENT);
    card->setStyleSheet(style);

    QLabel *nameLabel = m_templateNameLabels.value(key, nullptr);
    if (nameLabel) {
        nameLabel->setStyleSheet(QStringLiteral(
            "color: %1;"
            "font-size: 14px;"
            "font-weight: %2;"
        ).arg(isSelected ? COLOR_PRIMARY : "#333333", isSelected ? QStringLiteral("600") : QStringLiteral("500")));
        nameLabel->setWordWrap(false);
    }
}

void AIPreparationWidget::createPreviewCard(int index, const QImage *image)
{
    QFrame *card = new QFrame();
    card->setObjectName(QStringLiteral("previewCard_%1").arg(index));
    card->setFixedHeight(PREVIEW_CARD_HEIGHT);
    card->setCursor(Qt::PointingHandCursor);
    card->setFocusPolicy(Qt::StrongFocus);
    card->setStyleSheet(QStringLiteral(
        "QFrame#previewCard_%1 {"
        "  background-color: rgba(20,20,24,0.95);"
        "  border: 1px solid rgba(255,255,255,0.06);"
        "  border-radius: 12px;"
        "}"
        "QFrame#previewCard_%1:hover { border-color: %2; }"
        "QFrame#previewCard_%1:focus { border: 2px solid %3; }"
    ).arg(index).arg(COLOR_PRIMARY, COLOR_ACCENT));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(SPACING_MEDIUM, SPACING_MEDIUM, SPACING_MEDIUM, SPACING_MEDIUM);
    cardLayout->setSpacing(SPACING_SMALL);

    QLabel *imageLabel = new QLabel();
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumHeight(PREVIEW_CARD_HEIGHT - 70);
    imageLabel->setStyleSheet(QStringLiteral(
        "background-color: %1;"
        "border-radius: 12px;"
        "color: white;"
        "font-size: 14px;"
    ).arg(COLOR_BG_DARK));

    if (image && !image->isNull()) {
        QPixmap pixmap = QPixmap::fromImage(*image).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(pixmap);
        imageLabel->setText(QString());
    } else {
        imageLabel->setText(QStringLiteral("待生成"));
    }

    QLabel *captionLabel = new QLabel(QStringLiteral("第 %1 张").arg(index + 1));
    captionLabel->setAlignment(Qt::AlignCenter);
    captionLabel->setStyleSheet(QStringLiteral("color: white; font-size: 14px; font-weight: 600;"));

    cardLayout->addWidget(imageLabel);
    cardLayout->addWidget(captionLabel);

    QWidget *overlay = new QWidget(card);
    overlay->setObjectName(QStringLiteral("previewOverlay_%1").arg(index));
    overlay->setStyleSheet(QStringLiteral(
        "background-color: rgba(0,0,0,0.65);"
        "border-radius: 12px;"
    ));
    overlay->hide();

    QVBoxLayout *overlayLayout = new QVBoxLayout(overlay);
    overlayLayout->setContentsMargins(SPACING_MEDIUM, SPACING_MEDIUM, SPACING_MEDIUM, SPACING_MEDIUM);
    overlayLayout->addStretch();

    QPushButton *previewBtn = new QPushButton(QStringLiteral("预览"));
    previewBtn->setCursor(Qt::PointingHandCursor);
    previewBtn->setFixedSize(92, WIDGET_HEIGHT);
    previewBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: rgba(255,255,255,0.9);"
        "  color: %1;"
        "  border-radius: 18px;"
        "  border: none;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: white; }"
    ).arg(COLOR_PRIMARY));
    overlayLayout->addWidget(previewBtn, 0, Qt::AlignHCenter);
    overlayLayout->addStretch();

    overlay->installEventFilter(this);
    card->installEventFilter(this);
    overlay->setGeometry(card->rect());

    connect(previewBtn, &QPushButton::clicked, this, [this, index]() {
        onPreviewClicked(index);
    });

    m_previewCards.append(card);
    m_previewOverlays.append(overlay);
    m_previewButtons.append(previewBtn);
    m_previewImageLabels.append(imageLabel);
    m_previewCaptionLabels.append(captionLabel);
}

void AIPreparationWidget::updatePreviewCard(int index, const QImage &image)
{
    if (index < 0 || index >= m_previewImageLabels.size()) {
        return;
    }

    QLabel *imageLabel = m_previewImageLabels.at(index);
    if (!imageLabel) {
        return;
    }

    if (!image.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(image).scaled(QSize(320, 180), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(pixmap);
        imageLabel->setText(QString());
    }
}

void AIPreparationWidget::refreshPreviewCards()
{
    for (int i = 0; i < m_previewCards.size(); ++i) {
        const bool hasSlide = i < m_slideImages.size();

        if (hasSlide) {
            updatePreviewCard(i, m_slideImages.at(i));
            m_previewCaptionLabels[i]->setText(QStringLiteral("第 %1 张").arg(i + 1));
            m_previewImageLabels[i]->setStyleSheet(QStringLiteral(
                "background-color: %1;"
                "border-radius: 12px;"
                "color: white;"
                "font-size: 14px;"
            ).arg(COLOR_BG_DARK));
            m_previewCards[i]->setCursor(Qt::PointingHandCursor);
            if (m_previewButtons[i]) {
                m_previewButtons[i]->setEnabled(true);
            }
        } else {
            QLabel *imageLabel = m_previewImageLabels[i];
            imageLabel->clear();
            imageLabel->setText(QStringLiteral("待生成"));
            imageLabel->setStyleSheet(QStringLiteral(
                "background-color: rgba(255,255,255,0.08);"
                "border-radius: 12px;"
                "color: %1;"
                "font-size: 14px;"
            ).arg(COLOR_SUBTEXT));
            m_previewCaptionLabels[i]->setText(QStringLiteral("尚未生成"));
            QWidget *overlay = m_previewOverlays[i];
            if (overlay) {
                overlay->hide();
            }
            if (m_previewButtons[i]) {
                m_previewButtons[i]->setEnabled(false);
            }
            m_previewCards[i]->setCursor(Qt::ArrowCursor);
        }
    }
}

void AIPreparationWidget::updatePlaceholderCard()
{
    if (!m_placeholderLabel || !m_placeholderIcon) {
        return;
    }

    QString text;
    QIcon icon;

    switch (m_state) {
    case GenerationState::Idle:
        text = QStringLiteral("等待生成");
        icon = QIcon::fromTheme(QStringLiteral("hourglass"));
        break;
    case GenerationState::Generating:
        text = QStringLiteral("生成中…");
        icon = QIcon::fromTheme(QStringLiteral("view-refresh"));
        break;
    case GenerationState::Success:
        text = m_slideImages.size() >= kVisibleSlideSlots ? QStringLiteral("预览已完成") : QStringLiteral("继续生成更多页");
        icon = QIcon::fromTheme(QStringLiteral("emblem-default"));
        break;
    case GenerationState::Failed:
        text = QStringLiteral("生成失败，请重试");
        icon = QIcon::fromTheme(QStringLiteral("dialog-error"));
        break;
    }

    m_placeholderLabel->setText(text);
    m_placeholderIcon->setPixmap(icon.pixmap(32, 32));
}

void AIPreparationWidget::showPreviewDialog(int index)
{
    if (index < 0 || index >= m_slideImages.size()) {
        return;
    }

    emit previewRequested(index);
    emit slidePreviewRequested(index);

    SlidePreviewDialog *dialog = new SlidePreviewDialog(m_slideImages.at(index), index, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setTotalSlides(m_slideImages.size());

    connect(dialog, &SlidePreviewDialog::navigateRequested, this, &AIPreparationWidget::onPreviewNavigate);
    connect(dialog, &SlidePreviewDialog::deleteRequested, this, &AIPreparationWidget::onPreviewDelete);

    m_activePreviewDialog = dialog;
    dialog->exec();
    m_activePreviewDialog = nullptr;
}

void AIPreparationWidget::updateActionsVisibility()
{
    if (!m_actionsBar) {
        return;
    }

    const bool visible = (m_state == GenerationState::Success);
    m_actionsBar->setVisible(visible);
    if (m_onlineEditBtn) {
        m_onlineEditBtn->setVisible(m_enableOnlineEdit);
    }
}

void AIPreparationWidget::updateStatusForCurrentState()
{
    if (!m_statusStack) {
        return;
    }

    switch (m_state) {
    case GenerationState::Idle:
        m_statusStack->setVisible(false);
        stopProgressAnimation();
        break;
    case GenerationState::Generating:
        m_statusStack->setVisible(true);
        m_statusStack->setCurrentWidget(m_generatingStatus);
        if (m_progressLabel) {
            m_progressLabel->setText(QStringLiteral("正在分析教学目标，智能生成中… %1%").arg(m_currentProgress));
        }
        startProgressAnimation();
        break;
    case GenerationState::Success:
        m_statusStack->setVisible(true);
        m_statusStack->setCurrentWidget(m_successStatus);
        stopProgressAnimation();
        break;
    case GenerationState::Failed:
        m_statusStack->setVisible(true);
        m_statusStack->setCurrentWidget(m_failedStatus);
        stopProgressAnimation();
        break;
    }

    updatePlaceholderCard();
}

void AIPreparationWidget::startProgressAnimation()
{
    if (!m_progressTimer) {
        m_progressTimer = new QTimer(this);
        m_progressTimer->setInterval(500);
        connect(m_progressTimer, &QTimer::timeout, this, &AIPreparationWidget::onProgressTimerTimeout);
    }
    if (!m_progressTimer->isActive()) {
        m_progressTimer->start();
    }
    m_progressDotVisible = true;
    if (m_progressDot) {
        m_progressDot->setVisible(true);
    }
}

void AIPreparationWidget::stopProgressAnimation()
{
    if (m_progressTimer) {
        m_progressTimer->stop();
    }
    if (m_progressDot) {
        m_progressDot->setVisible(true);
    }
    m_progressDotVisible = true;
}

bool AIPreparationWidget::eventFilter(QObject *obj, QEvent *event)
{
    for (auto it = m_templateCards.cbegin(); it != m_templateCards.cend(); ++it) {
        if (obj == it.value()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *mouseEvent = static_cast<QMouseEvent *>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    onTemplateSelected(it.key());
                    return true;
                }
            } else if (event->type() == QEvent::KeyPress) {
                auto *keyEvent = static_cast<QKeyEvent *>(event);
                if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Space) {
                    onTemplateSelected(it.key());
                    return true;
                }
            } else if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
                updateTemplateCardStyle(it.key(), m_selectedTemplateKey == it.key());
            }
            return QWidget::eventFilter(obj, event);
        }
    }

    for (int i = 0; i < m_previewCards.size(); ++i) {
        if (obj == m_previewCards[i]) {
            switch (event->type()) {
            case QEvent::Enter:
                if (m_previewOverlays[i]) {
                    m_previewOverlays[i]->setGeometry(m_previewCards[i]->rect());
                    if (i < m_slideImages.size()) {
                        m_previewOverlays[i]->show();
                    }
                }
                break;
            case QEvent::Leave:
                if (m_previewOverlays[i]) {
                    m_previewOverlays[i]->hide();
                }
                break;
            case QEvent::Resize:
                if (m_previewOverlays[i]) {
                    m_previewOverlays[i]->setGeometry(m_previewCards[i]->rect());
                }
                break;
            case QEvent::MouseButtonPress: {
                auto *mouseEvent = static_cast<QMouseEvent *>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    m_dragStartPos = mouseEvent->pos();
                    m_dragSourceIndex = i;
                    m_dragging = false;
                }
                break;
            }
            case QEvent::MouseMove: {
                auto *mouseEvent = static_cast<QMouseEvent *>(event);
                if (m_dragSourceIndex == i && (mouseEvent->buttons() & Qt::LeftButton)) {
                    if (!m_dragging && (mouseEvent->pos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance()) {
                        if (i >= m_slideImages.size()) {
                            break;
                        }
                        m_dragging = true;
                        QDrag *drag = new QDrag(this);
                        QMimeData *mime = new QMimeData();
                        mime->setData(kPreviewDragMimeType, QByteArray::number(i));
                        drag->setMimeData(mime);
                        drag->exec(Qt::MoveAction);
                        m_dragSourceIndex = -1;
                        m_dragging = false;
                    }
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                auto *mouseEvent = static_cast<QMouseEvent *>(event);
                if (mouseEvent->button() == Qt::LeftButton && m_dragSourceIndex == i) {
                    if (!m_dragging && i < m_slideImages.size()) {
                        onPreviewClicked(i);
                    }
                    m_dragSourceIndex = -1;
                    m_dragging = false;
                }
                break;
            }
            default:
                break;
            }
            return QWidget::eventFilter(obj, event);
        }

        if (obj == m_previewOverlays.value(i, nullptr)) {
            if (event->type() == QEvent::Leave) {
                m_previewOverlays[i]->hide();
            }
            return QWidget::eventFilter(obj, event);
        }
    }

    return QWidget::eventFilter(obj, event);
}

void AIPreparationWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(kPreviewDragMimeType)) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void AIPreparationWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(kPreviewDragMimeType)) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void AIPreparationWidget::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat(kPreviewDragMimeType)) {
        return;
    }

    bool ok = false;
    int fromIndex = event->mimeData()->data(kPreviewDragMimeType).toInt(&ok);
    if (!ok || fromIndex < 0 || fromIndex >= m_slideImages.size()) {
        return;
    }

    int toIndex = -1;
    for (int i = 0; i < m_previewCards.size(); ++i) {
        if (m_previewCards[i]->geometry().contains(event->position().toPoint())) {
            toIndex = i;
            break;
        }
    }

    if (toIndex < 0) {
        toIndex = m_slideImages.size() - 1;
    }
    toIndex = qBound(0, toIndex, m_slideImages.size() - 1);

    if (fromIndex == toIndex) {
        event->acceptProposedAction();
        return;
    }

    m_slideImages.move(fromIndex, toIndex);
    if (fromIndex < m_slideOrder.size() && toIndex < m_slideOrder.size()) {
        m_slideOrder.move(fromIndex, toIndex);
    }

    refreshPreviewCards();

    QList<int> order;
    for (int id : m_slideOrder) {
        order.append(id);
    }
    emit slidesReordered(order);

    event->acceptProposedAction();
}

void AIPreparationWidget::setAnimationProgress(int progress)
{
    m_animationProgress = progress;
    updateAdvancedSectionHeight();
}

void AIPreparationWidget::updateAdvancedSectionHeight()
{
    if (!m_advancedSection) {
        return;
    }

    if (m_animationProgress <= 0) {
        m_advancedSection->setMaximumHeight(0);
        m_advancedSection->setVisible(false);
        return;
    }

    int height = m_advancedMaxHeight * m_animationProgress / 100;
    m_advancedSection->setVisible(true);
    m_advancedSection->setMaximumHeight(height);
}

QString AIPreparationWidget::selectedTextbook() const
{
    return m_textbookCombo ? m_textbookCombo->currentText() : QString();
}

QString AIPreparationWidget::selectedGrade() const
{
    return m_gradeCombo ? m_gradeCombo->currentText() : QString();
}

QString AIPreparationWidget::selectedChapter() const
{
    return m_chapterCombo ? m_chapterCombo->currentText() : QString();
}

QString AIPreparationWidget::selectedTemplateKey() const
{
    return m_selectedTemplateKey;
}

QString AIPreparationWidget::selectedDuration() const
{
    return m_durationCombo ? m_durationCombo->currentText() : QString();
}

QString AIPreparationWidget::contentFocus() const
{
    return m_contentFocusEdit ? m_contentFocusEdit->text() : QString();
}

void AIPreparationWidget::setProgress(int percent)
{
    m_currentProgress = qBound(0, percent, 100);
    if (m_currentProgress < 100) {
        if (m_state != GenerationState::Generating) {
            setGenerationState(GenerationState::Generating);
        } else if (m_progressLabel) {
            m_progressLabel->setText(QStringLiteral("正在分析教学目标，智能生成中… %1%").arg(m_currentProgress));
        }
    } else {
        setGenerationState(GenerationState::Success);
    }

    updatePlaceholderCard();
}

void AIPreparationWidget::setGenerationState(GenerationState state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
    updateStatusForCurrentState();
    updateActionsVisibility();
}

void AIPreparationWidget::setSlides(const QVector<QImage> &images)
{
    m_slideImages = images;
    m_slideOrder.resize(images.size());
    for (int i = 0; i < m_slideOrder.size(); ++i) {
        m_slideOrder[i] = i;
    }

    refreshPreviewCards();
    updatePlaceholderCard();
    updateActionsVisibility();
}

void AIPreparationWidget::onGenerateClicked()
{
    QMap<QString, QString> params;
    params.insert(QStringLiteral("textbook"), selectedTextbook());
    params.insert(QStringLiteral("grade"), selectedGrade());
    params.insert(QStringLiteral("chapter"), selectedChapter());
    params.insert(QStringLiteral("template"), selectedTemplateKey());
    params.insert(QStringLiteral("duration"), selectedDuration());
    params.insert(QStringLiteral("contentFocus"), contentFocus());

    emit generateRequested(params);
    setGenerationState(GenerationState::Generating);
    setProgress(0);
}

void AIPreparationWidget::onToggleAdvanced(bool checked)
{
    if (!m_advancedSection || !m_advancedAnimation) {
        return;
    }

    // 动态更新提示文案
    if (checked) {
        m_toggleAdvancedBtn->setToolTip(QStringLiteral("高级选项（已展开，点击收起）"));
        m_toggleAdvancedBtn->setStatusTip(QStringLiteral("高级选项面板已展开，点击收起"));
        m_toggleAdvancedBtn->setWhatsThis(QStringLiteral("高级选项面板当前已展开，包含 PPT 模板、课时长度与内容侧重等设置。点击可收起。"));

        m_advancedSection->setVisible(true);
        if (m_advancedMaxHeight == 0) {
            m_advancedMaxHeight = m_advancedSection->sizeHint().height();
        }
        m_advancedAnimation->stop();
        m_advancedAnimation->setStartValue(m_advancedSection->maximumHeight());
        m_advancedAnimation->setEndValue(m_advancedMaxHeight);
        m_advancedAnimation->start();
    } else {
        m_toggleAdvancedBtn->setToolTip(QStringLiteral("高级选项（已收起，点击展开）"));
        m_toggleAdvancedBtn->setStatusTip(QStringLiteral("高级选项面板已收起，点击展开"));
        m_toggleAdvancedBtn->setWhatsThis(QStringLiteral("高级选项面板当前已收起，包括 PPT 模板、课时长度与内容侧重等设置。点击可展开。"));

        m_advancedAnimation->stop();
        m_advancedAnimation->setStartValue(m_advancedSection->maximumHeight());
        m_advancedAnimation->setEndValue(0);
        m_advancedAnimation->start();
    }
}

void AIPreparationWidget::onTemplateSelected(const QString &key)
{
    if (m_selectedTemplateKey == key) {
        return;
    }

    if (!m_selectedTemplateKey.isEmpty()) {
        updateTemplateCardStyle(m_selectedTemplateKey, false);
    }
    m_selectedTemplateKey = key;
    updateTemplateCardStyle(key, true);
}

void AIPreparationWidget::onPreviewClicked(int index)
{
    if (index < 0 || index >= m_slideImages.size()) {
        return;
    }
    showPreviewDialog(index);
}

void AIPreparationWidget::onDownloadClicked()
{
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("保存PPT"),
        QStringLiteral("思政课堂.pptx"),
        QStringLiteral("PowerPoint 演示文稿 (*.pptx)")
    );

    if (!fileName.isEmpty()) {
        emit downloadRequested();
    }
}

void AIPreparationWidget::onSaveClicked()
{
    emit saveToLibraryRequested();
}

void AIPreparationWidget::onOnlineEditClicked()
{
    emit onlineEditRequested();
}

void AIPreparationWidget::onRegenerateClicked()
{
    emit regenerateRequested();
}

void AIPreparationWidget::onPreviewNavigate(int offset)
{
    if (!m_activePreviewDialog || m_slideImages.isEmpty()) {
        return;
    }

    int target = qBound(0, m_activePreviewDialog->index() + offset, m_slideImages.size() - 1);
    if (target == m_activePreviewDialog->index()) {
        return;
    }

    m_activePreviewDialog->setIndex(target);
    m_activePreviewDialog->setImage(m_slideImages.at(target));
    m_activePreviewDialog->setTotalSlides(m_slideImages.size());

    emit slidePreviewRequested(target);
}

void AIPreparationWidget::onPreviewDelete(int index)
{
    if (index < 0 || index >= m_slideImages.size()) {
        return;
    }

    m_slideImages.removeAt(index);
    if (index < m_slideOrder.size()) {
        m_slideOrder.removeAt(index);
    }

    refreshPreviewCards();
    updatePlaceholderCard();
    updateActionsVisibility();

    QList<int> order;
    for (int id : m_slideOrder) {
        order.append(id);
    }
    emit slidesReordered(order);

    if (m_activePreviewDialog) {
        if (m_slideImages.isEmpty()) {
            m_activePreviewDialog->close();
            return;
        }
        int nextIndex = qBound(0, index, m_slideImages.size() - 1);
        m_activePreviewDialog->setIndex(nextIndex);
        m_activePreviewDialog->setImage(m_slideImages.at(nextIndex));
        m_activePreviewDialog->setTotalSlides(m_slideImages.size());
        emit slidePreviewRequested(nextIndex);
    }
}

void AIPreparationWidget::onProgressTimerTimeout()
{
    if (!m_progressDot) {
        return;
    }
    m_progressDotVisible = !m_progressDotVisible;
    m_progressDot->setVisible(m_progressDotVisible);
}

// ------------------- SlidePreviewDialog -------------------

SlidePreviewDialog::SlidePreviewDialog(const QImage &image, int index, QWidget *parent)
    : QDialog(parent)
    , m_image(image)
    , m_index(index)
    , m_total(1)
    , m_scaleFactor(1.0)
    , m_fitScale(1.0)
{
    setModal(true);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet(QStringLiteral("QDialog { background-color: rgba(0, 0, 0, 200); }"));
    setMinimumSize(960, 640);

    setupUI();
    setImage(image);
    updateNavigationButtons();
}

void SlidePreviewDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(48, 48, 48, 48);
    mainLayout->setSpacing(24);

    QFrame *contentFrame = new QFrame();
    contentFrame->setStyleSheet(QStringLiteral(
        "QFrame {"
        "  background-color: rgba(22,22,27,0.95);"
        "  border-radius: 16px;"
        "}"
    ));

    QVBoxLayout *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(32, 24, 32, 24);
    contentLayout->setSpacing(16);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(12);

    m_indexLabel = new QLabel();
    m_indexLabel->setStyleSheet(QStringLiteral("color: white; font-size: 16px; font-weight: 600;"));

    m_prevBtn = new QPushButton(QStringLiteral("上一张"));
    m_prevBtn->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    m_prevBtn->setFixedHeight(36);
    m_prevBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: white;"
        "  border: 1px solid rgba(255,255,255,0.2);"
        "  border-radius: 8px;"
        "  padding: 0 18px;"
        "}"
        "QPushButton:disabled { color: rgba(255,255,255,0.4); border-color: rgba(255,255,255,0.1); }"
        "QPushButton:hover:!disabled { background-color: rgba(255,255,255,0.1); }"
    ));

    m_nextBtn = new QPushButton(QStringLiteral("下一张"));
    m_nextBtn->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    m_nextBtn->setFixedHeight(36);
    m_nextBtn->setStyleSheet(m_prevBtn->styleSheet());

    m_zoomOutBtn = new QPushButton(QStringLiteral("缩小 -"));
    m_zoomOutBtn->setFixedHeight(36);
    m_zoomOutBtn->setStyleSheet(m_prevBtn->styleSheet());

    m_fitBtn = new QPushButton(QStringLiteral("适合窗口"));
    m_fitBtn->setFixedHeight(36);
    m_fitBtn->setStyleSheet(m_prevBtn->styleSheet());

    m_zoomInBtn = new QPushButton(QStringLiteral("放大 +"));
    m_zoomInBtn->setFixedHeight(36);
    m_zoomInBtn->setStyleSheet(m_prevBtn->styleSheet());

    m_deleteBtn = new QPushButton(QStringLiteral("删除该页"));
    m_deleteBtn->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    m_deleteBtn->setFixedHeight(36);
    m_deleteBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: %1;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 0 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #d95454; }"
    ).arg("#F56C6C"));

    toolbarLayout->addWidget(m_indexLabel);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_prevBtn);
    toolbarLayout->addWidget(m_nextBtn);
    toolbarLayout->addWidget(m_zoomOutBtn);
    toolbarLayout->addWidget(m_fitBtn);
    toolbarLayout->addWidget(m_zoomInBtn);
    toolbarLayout->addWidget(m_deleteBtn);

    contentLayout->addLayout(toolbarLayout);

    m_scrollArea = new QScrollArea();
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(QStringLiteral(
        "QScrollArea {"
        "  background-color: rgba(0,0,0,0.85);"
        "  border: none;"
        "}"
    ));

    m_imageLabel = new QLabel();
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet(QStringLiteral("color: rgba(255,255,255,0.6); font-size: 14px;"));
    m_scrollArea->setWidget(m_imageLabel);

    contentLayout->addWidget(m_scrollArea);
    mainLayout->addWidget(contentFrame);

    connect(m_prevBtn, &QPushButton::clicked, this, &SlidePreviewDialog::onPrevClicked);
    connect(m_nextBtn, &QPushButton::clicked, this, &SlidePreviewDialog::onNextClicked);
    connect(m_zoomInBtn, &QPushButton::clicked, this, &SlidePreviewDialog::onZoomInClicked);
    connect(m_zoomOutBtn, &QPushButton::clicked, this, &SlidePreviewDialog::onZoomOutClicked);
    connect(m_fitBtn, &QPushButton::clicked, this, &SlidePreviewDialog::onFitToWindowClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SlidePreviewDialog::onDeleteClicked);
}

void SlidePreviewDialog::setImage(const QImage &image)
{
    m_image = image;
    m_scaleFactor = 1.0;
    if (!m_image.isNull()) {
        m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
    } else {
        m_imageLabel->setPixmap(QPixmap());
        m_imageLabel->setText(QStringLiteral("暂无预览内容"));
    }
    onFitToWindowClicked();
}

void SlidePreviewDialog::setTotalSlides(int total)
{
    m_total = qMax(1, total);
    updateNavigationButtons();
}

void SlidePreviewDialog::updateNavigationButtons()
{
    m_indexLabel->setText(QStringLiteral("第 %1 / %2 页").arg(m_index + 1).arg(m_total));
    m_prevBtn->setEnabled(m_index > 0);
    m_nextBtn->setEnabled(m_index + 1 < m_total);
}

void SlidePreviewDialog::updateScale()
{
    if (m_image.isNull()) {
        return;
    }

    QPixmap pixmap = QPixmap::fromImage(m_image);
    QSize scaledSize = QSize(qMax(1, static_cast<int>(pixmap.width() * m_scaleFactor)),
                             qMax(1, static_cast<int>(pixmap.height() * m_scaleFactor)));
    m_imageLabel->setPixmap(pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_imageLabel->resize(m_imageLabel->pixmap(Qt::ReturnByValue).size());
}

void SlidePreviewDialog::onPrevClicked()
{
    emit navigateRequested(-1);
}

void SlidePreviewDialog::onNextClicked()
{
    emit navigateRequested(1);
}

void SlidePreviewDialog::onZoomInClicked()
{
    m_scaleFactor *= 1.25;
    updateScale();
}

void SlidePreviewDialog::onZoomOutClicked()
{
    m_scaleFactor /= 1.25;
    if (m_scaleFactor < 0.1) {
        m_scaleFactor = 0.1;
    }
    updateScale();
}

void SlidePreviewDialog::onFitToWindowClicked()
{
    if (m_image.isNull()) {
        return;
    }

    QSize viewport = m_scrollArea->viewport()->size();
    if (viewport.isEmpty()) {
        return;
    }

    qreal scaleX = static_cast<qreal>(viewport.width()) / m_image.width();
    qreal scaleY = static_cast<qreal>(viewport.height()) / m_image.height();
    m_fitScale = qMin(scaleX, scaleY);
    m_scaleFactor = m_fitScale;
    updateScale();
}

void SlidePreviewDialog::onDeleteClicked()
{
    emit deleteRequested(m_index);
}
