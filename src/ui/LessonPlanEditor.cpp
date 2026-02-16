#include "LessonPlanEditor.h"
#include "../services/CurriculumService.h"
#include "../services/DifyService.h"
#include "../utils/MarkdownRenderer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QFont>
#include <QTextCursor>
#include <QTextList>
#include <QTextListFormat>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QGraphicsDropShadowEffect>
#include <QIcon>
#include <QSize>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QRegularExpression>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>

// 样式常量 - 思政红主题（专业版）
namespace {
    // 主色调
    const QString PATRIOTIC_RED = "#C62828";           // 主红色
    const QString PATRIOTIC_RED_DARK = "#8E0000";      // 深红色
    const QString PATRIOTIC_RED_LIGHT = "#FFCDD2";     // 浅红色
    const QString PATRIOTIC_RED_BG = "#FFF8F8";        // 红色背景

    // 中性色
    const QString PRIMARY_TEXT = "#1A1A1A";            // 主文本
    const QString SECONDARY_TEXT = "#666666";          // 次要文本
    const QString TERTIARY_TEXT = "#999999";           // 辅助文本
    const QString BORDER_COLOR = "#E8E8E8";            // 边框色
    const QString BORDER_FOCUS = "#BDBDBD";            // 聚焦边框
    const QString BACKGROUND_COLOR = "#F8F9FA";        // 背景色
    const QString CARD_BACKGROUND = "#FFFFFF";         // 卡片背景

    // 功能色
    const QString SUCCESS_COLOR = "#2E7D32";           // 成功绿
    const QString WARNING_COLOR = "#F57C00";           // 警告橙
}

LessonPlanEditor::LessonPlanEditor(QWidget *parent)
    : QWidget(parent)
    , m_gradeCombo(nullptr)
    , m_semesterCombo(nullptr)
    , m_unitCombo(nullptr)
    , m_lessonCombo(nullptr)
    , m_aiGenerateBtn(nullptr)
    , m_saveBtn(nullptr)
    , m_toolbar(nullptr)
    , m_editor(nullptr)
    , m_wordCountLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_difyService(nullptr)
    , m_markdownRenderer(nullptr)
    , m_isGenerating(false)
    , m_isModified(false)
{
    initUI();
    connectSignals();
    loadCurriculum();
}

LessonPlanEditor::~LessonPlanEditor()
{
    if (m_markdownRenderer) {
        delete m_markdownRenderer;
        m_markdownRenderer = nullptr;
    }
}

void LessonPlanEditor::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // 设置整体背景
    setStyleSheet(QString("QWidget { background-color: %1; }").arg(BACKGROUND_COLOR));

    // 初始化各个部分
    initCourseSelector();
    initToolbar();
    initEditor();
    initStatusBar();

    // 创建Markdown渲染器
    m_markdownRenderer = new MarkdownRenderer();
}

void LessonPlanEditor::initCourseSelector()
{
    // 课程选择容器 - 现代卡片风格
    QFrame *selectorFrame = new QFrame(this);
    selectorFrame->setObjectName("courseSelectorCard");
    selectorFrame->setStyleSheet(QString(
        "#courseSelectorCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 12px;"
        "}"
    ).arg(CARD_BACKGROUND, BORDER_COLOR));

    // 添加阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(selectorFrame);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 4);
    selectorFrame->setGraphicsEffect(shadow);

    QHBoxLayout *selectorLayout = new QHBoxLayout(selectorFrame);
    selectorLayout->setContentsMargins(20, 16, 20, 16);
    selectorLayout->setSpacing(16);

    // 课程标签 - 带图标
    QLabel *courseLabel = new QLabel(this);
    courseLabel->setText("课程选择");
    courseLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: 15px;"
        "font-weight: 600;"
        "letter-spacing: 0.5px;"
    ).arg(PRIMARY_TEXT));
    selectorLayout->addWidget(courseLabel);

    // 下拉框样式 - 现代扁平化
    QString comboStyle = QString(
        "QComboBox {"
        "  padding: 10px 16px;"
        "  padding-right: 36px;"
        "  border: 1px solid %1;"
        "  border-radius: 8px;"
        "  background-color: %2;"
        "  color: %3;"
        "  font-size: 14px;"
        "  min-width: 110px;"
        "}"
        "QComboBox:hover {"
        "  border-color: %4;"
        "  background-color: %5;"
        "}"
        "QComboBox:focus {"
        "  border-color: %6;"
        "  border-width: 2px;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "  width: 32px;"
        "  padding-right: 8px;"
        "}"
        "QComboBox::down-arrow {"
        "  image: url(:/icons/resources/icons/arrow-down.svg);"
        "  width: 12px;"
        "  height: 12px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: white;"
        "  border: 1px solid %1;"
        "  border-radius: 8px;"
        "  padding: 4px;"
        "  selection-background-color: %5;"
        "  selection-color: %6;"
        "}"
    ).arg(BORDER_COLOR, CARD_BACKGROUND, PRIMARY_TEXT,
          BORDER_FOCUS, PATRIOTIC_RED_BG, PATRIOTIC_RED);

    // 年级下拉框
    m_gradeCombo = new QComboBox(this);
    m_gradeCombo->setStyleSheet(comboStyle);
    m_gradeCombo->setPlaceholderText("选择年级");
    selectorLayout->addWidget(m_gradeCombo);

    // 学期下拉框
    m_semesterCombo = new QComboBox(this);
    m_semesterCombo->setStyleSheet(comboStyle);
    m_semesterCombo->setPlaceholderText("选择学期");
    m_semesterCombo->setEnabled(false);
    selectorLayout->addWidget(m_semesterCombo);

    // 单元下拉框
    m_unitCombo = new QComboBox(this);
    m_unitCombo->setStyleSheet(comboStyle);
    m_unitCombo->setPlaceholderText("选择单元");
    m_unitCombo->setEnabled(false);
    selectorLayout->addWidget(m_unitCombo);

    // 课时下拉框
    m_lessonCombo = new QComboBox(this);
    m_lessonCombo->setStyleSheet(comboStyle);
    m_lessonCombo->setPlaceholderText("选择课时");
    m_lessonCombo->setEnabled(false);
    selectorLayout->addWidget(m_lessonCombo);

    selectorLayout->addStretch();

    // AI生成按钮 - 使用SVG图标
    m_aiGenerateBtn = new QPushButton(this);
    m_aiGenerateBtn->setIcon(QIcon(":/icons/resources/icons/ai-sparkle.svg"));
    m_aiGenerateBtn->setIconSize(QSize(18, 18));
    m_aiGenerateBtn->setText(" AI生成教案");
    m_aiGenerateBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 #E53935);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 10px 24px;"
        "  font-weight: 600;"
        "  font-size: 14px;"
        "  letter-spacing: 0.5px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #D32F2F, stop:1 #F44336);"
        "}"
        "QPushButton:pressed {"
        "  background: %2;"
        "}"
        "QPushButton:disabled {"
        "  background: #BDBDBD;"
        "  color: #FAFAFA;"
        "}"
    ).arg(PATRIOTIC_RED, PATRIOTIC_RED_DARK));
    m_aiGenerateBtn->setEnabled(false);
    m_aiGenerateBtn->setCursor(Qt::PointingHandCursor);
    selectorLayout->addWidget(m_aiGenerateBtn);

    // 保存按钮 - 使用SVG图标
    m_saveBtn = new QPushButton(this);
    m_saveBtn->setIcon(QIcon(":/icons/resources/icons/save.svg"));
    m_saveBtn->setIconSize(QSize(16, 16));
    m_saveBtn->setText(" 保存");
    m_saveBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 2px solid %2;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  font-weight: 600;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background-color: %3;"
        "  border-color: %4;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %5;"
        "}"
    ).arg(CARD_BACKGROUND, PATRIOTIC_RED, PATRIOTIC_RED_BG, PATRIOTIC_RED_DARK, PATRIOTIC_RED_LIGHT));
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    selectorLayout->addWidget(m_saveBtn);

    // 添加到主布局
    static_cast<QVBoxLayout*>(layout())->addWidget(selectorFrame);
}

void LessonPlanEditor::initToolbar()
{
    // 工具栏容器 - 现代简洁风格
    QFrame *toolbarFrame = new QFrame(this);
    toolbarFrame->setObjectName("toolbarCard");
    toolbarFrame->setStyleSheet(QString(
        "#toolbarCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 10px;"
        "}"
    ).arg(CARD_BACKGROUND, BORDER_COLOR));

    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarFrame);
    toolbarLayout->setContentsMargins(12, 8, 12, 8);
    toolbarLayout->setSpacing(4);

    // 工具按钮样式 - 简洁图标风格
    QString toolBtnStyle = QString(
        "QPushButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px;"
        "  min-width: 36px;"
        "  max-width: 36px;"
        "  min-height: 36px;"
        "  max-height: 36px;"
        "}"
        "QPushButton:hover {"
        "  background-color: %1;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %2;"
        "}"
    ).arg(BACKGROUND_COLOR, PATRIOTIC_RED_LIGHT);

    // 文本格式按钮样式（保留文字）
    QString textBtnStyle = QString(
        "QPushButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "  color: %1;"
        "}"
        "QPushButton:hover {"
        "  background-color: %2;"
        "  color: %3;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %4;"
        "}"
    ).arg(SECONDARY_TEXT, PATRIOTIC_RED_BG, PATRIOTIC_RED, PATRIOTIC_RED_LIGHT);

    // 加粗按钮 - SVG图标
    m_boldBtn = new QPushButton(this);
    m_boldBtn->setIcon(QIcon(":/icons/resources/icons/bold.svg"));
    m_boldBtn->setIconSize(QSize(18, 18));
    m_boldBtn->setStyleSheet(toolBtnStyle);
    m_boldBtn->setToolTip("加粗 (Ctrl+B)");
    m_boldBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(m_boldBtn);

    // 斜体按钮 - SVG图标
    m_italicBtn = new QPushButton(this);
    m_italicBtn->setIcon(QIcon(":/icons/resources/icons/italic.svg"));
    m_italicBtn->setIconSize(QSize(18, 18));
    m_italicBtn->setStyleSheet(toolBtnStyle);
    m_italicBtn->setToolTip("斜体 (Ctrl+I)");
    m_italicBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(m_italicBtn);

    // 分隔符
    QFrame *sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet(QString("background-color: %1; margin: 4px 8px;").arg(BORDER_COLOR));
    sep1->setFixedWidth(1);
    sep1->setFixedHeight(24);
    toolbarLayout->addWidget(sep1);

    // 标题按钮 - 保留文字更直观
    m_heading1Btn = new QPushButton("H1", this);
    m_heading1Btn->setStyleSheet(textBtnStyle);
    m_heading1Btn->setToolTip("一级标题");
    m_heading1Btn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(m_heading1Btn);

    m_heading2Btn = new QPushButton("H2", this);
    m_heading2Btn->setStyleSheet(textBtnStyle);
    m_heading2Btn->setToolTip("二级标题");
    m_heading2Btn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(m_heading2Btn);

    m_heading3Btn = new QPushButton("H3", this);
    m_heading3Btn->setStyleSheet(textBtnStyle);
    m_heading3Btn->setToolTip("三级标题");
    m_heading3Btn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(m_heading3Btn);

    // 分隔符
    QFrame *sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::VLine);
    sep2->setStyleSheet(QString("background-color: %1; margin: 4px 8px;").arg(BORDER_COLOR));
    sep2->setFixedWidth(1);
    sep2->setFixedHeight(24);
    toolbarLayout->addWidget(sep2);

    // 列表按钮 - SVG图标
    m_bulletListBtn = new QPushButton(this);
    m_bulletListBtn->setIcon(QIcon(":/icons/resources/icons/list-bullet.svg"));
    m_bulletListBtn->setIconSize(QSize(18, 18));
    m_bulletListBtn->setStyleSheet(toolBtnStyle);
    m_bulletListBtn->setToolTip("无序列表");
    m_bulletListBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(m_bulletListBtn);

    m_numberedListBtn = new QPushButton(this);
    m_numberedListBtn->setIcon(QIcon(":/icons/resources/icons/list-numbered.svg"));
    m_numberedListBtn->setIconSize(QSize(18, 18));
    m_numberedListBtn->setStyleSheet(toolBtnStyle);
    m_numberedListBtn->setToolTip("有序列表");
    m_numberedListBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(m_numberedListBtn);

    toolbarLayout->addStretch();

    // 添加到主布局
    static_cast<QVBoxLayout*>(layout())->addWidget(toolbarFrame);
}

void LessonPlanEditor::initEditor()
{
    // 编辑器容器
    QFrame *editorFrame = new QFrame(this);
    editorFrame->setObjectName("editorCard");
    editorFrame->setStyleSheet(QString(
        "#editorCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 12px;"
        "}"
    ).arg(CARD_BACKGROUND, BORDER_COLOR));

    // 添加阴影
    QGraphicsDropShadowEffect *editorShadow = new QGraphicsDropShadowEffect(editorFrame);
    editorShadow->setBlurRadius(16);
    editorShadow->setColor(QColor(0, 0, 0, 20));
    editorShadow->setOffset(0, 2);
    editorFrame->setGraphicsEffect(editorShadow);

    QVBoxLayout *editorLayout = new QVBoxLayout(editorFrame);
    editorLayout->setContentsMargins(4, 4, 4, 4);
    editorLayout->setSpacing(0);

    // 编辑器
    m_editor = new QTextEdit(this);
    m_editor->setAcceptRichText(true);
    m_editor->setPlaceholderText("选择课时后点击「AI生成教案」，或直接输入教案内容...");
    m_editor->setStyleSheet(QString(
        "QTextEdit {"
        "  background-color: %1;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 20px;"
        "  font-size: 15px;"
        "  line-height: 1.8;"
        "  color: %2;"
        "  selection-background-color: #B4D5FE;"
        "  selection-color: #1A1A1A;"
        "}"
        "QTextEdit:focus {"
        "  outline: none;"
        "}"
        "QScrollBar:vertical {"
        "  background: transparent;"
        "  width: 6px;"
        "  margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #D1D5DB;"
        "  border-radius: 3px;"
        "  min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #9CA3AF;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0;"
        "}"
    ).arg(CARD_BACKGROUND, PRIMARY_TEXT, BORDER_COLOR, BORDER_FOCUS));

    // 设置默认字体 - 使用更现代的字体栈
    QFont editorFont;
    editorFont.setFamilies({"PingFang SC", "Microsoft YaHei", "Noto Sans SC", "sans-serif"});
    editorFont.setPointSize(12);
    editorFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.3);
    m_editor->setFont(editorFont);

    editorLayout->addWidget(m_editor);

    // 添加到主布局，设置拉伸因子
    static_cast<QVBoxLayout*>(layout())->addWidget(editorFrame, 1);
}

void LessonPlanEditor::initStatusBar()
{
    // 状态栏容器 - 简约风格
    QFrame *statusFrame = new QFrame(this);
    statusFrame->setObjectName("statusBar");
    statusFrame->setStyleSheet(QString(
        "#statusBar {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "  padding: 2px 0;"
        "}"
    ).arg(CARD_BACKGROUND, BORDER_COLOR));

    QHBoxLayout *statusLayout = new QHBoxLayout(statusFrame);
    statusLayout->setContentsMargins(16, 8, 16, 8);

    // 字数统计 - 使用柔和颜色
    m_wordCountLabel = new QLabel("字数：0", this);
    m_wordCountLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: 13px;"
        "font-weight: 500;"
    ).arg(TERTIARY_TEXT));
    statusLayout->addWidget(m_wordCountLabel);

    statusLayout->addStretch();

    // 状态提示 - 成功/警告状态会变色
    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: 13px;"
        "font-weight: 500;"
    ).arg(TERTIARY_TEXT));
    statusLayout->addWidget(m_statusLabel);

    // 添加到主布局
    static_cast<QVBoxLayout*>(layout())->addWidget(statusFrame);
}

void LessonPlanEditor::connectSignals()
{
    // 课程选择信号
    connect(m_gradeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LessonPlanEditor::onGradeChanged);
    connect(m_semesterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LessonPlanEditor::onSemesterChanged);
    connect(m_unitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LessonPlanEditor::onUnitChanged);
    connect(m_lessonCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LessonPlanEditor::onLessonChanged);

    // 按钮信号
    connect(m_aiGenerateBtn, &QPushButton::clicked, this, &LessonPlanEditor::onAIGenerateClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &LessonPlanEditor::onSaveClicked);

    // 格式化工具栏信号
    connect(m_boldBtn, &QPushButton::clicked, this, &LessonPlanEditor::onBoldClicked);
    connect(m_italicBtn, &QPushButton::clicked, this, &LessonPlanEditor::onItalicClicked);
    connect(m_heading1Btn, &QPushButton::clicked, this, &LessonPlanEditor::onHeading1Clicked);
    connect(m_heading2Btn, &QPushButton::clicked, this, &LessonPlanEditor::onHeading2Clicked);
    connect(m_heading3Btn, &QPushButton::clicked, this, &LessonPlanEditor::onHeading3Clicked);
    connect(m_bulletListBtn, &QPushButton::clicked, this, &LessonPlanEditor::onBulletListClicked);
    connect(m_numberedListBtn, &QPushButton::clicked, this, &LessonPlanEditor::onNumberedListClicked);

    // 编辑器内容变化信号
    connect(m_editor, &QTextEdit::textChanged, this, &LessonPlanEditor::onTextChanged);
}

void LessonPlanEditor::loadCurriculum()
{
    qDebug() << "[LessonPlanEditor] ========== 开始加载课程目录 ==========";

    // 加载课程目录
    CurriculumService *curriculum = CurriculumService::instance();

    // 检查资源是否存在
    QString jsonPath = ":/data/resources/data/curriculum_morality_law.json";
    QFile testFile(jsonPath);
    qDebug() << "[LessonPlanEditor] 资源路径:" << jsonPath;
    qDebug() << "[LessonPlanEditor] 文件存在:" << testFile.exists();

    // 尝试读取文件内容来验证
    if (testFile.open(QIODevice::ReadOnly)) {
        QByteArray content = testFile.readAll();
        qDebug() << "[LessonPlanEditor] 文件大小:" << content.size() << "字节";
        qDebug() << "[LessonPlanEditor] 文件内容前200字符:" << QString::fromUtf8(content.left(200));
        testFile.close();
    } else {
        qWarning() << "[LessonPlanEditor] 艹！无法打开资源文件！错误:" << testFile.errorString();
    }

    // 如果还没加载，先加载
    if (curriculum->getTotalGrades() == 0) {
        qDebug() << "[LessonPlanEditor] 课程目录未加载，开始加载...";

        bool success = curriculum->loadCurriculum(jsonPath);
        qDebug() << "[LessonPlanEditor] 加载结果:" << (success ? "成功" : "失败");

        if (!success) {
            qWarning() << "[LessonPlanEditor] 艹！课程目录加载失败！";
        }
    } else {
        qDebug() << "[LessonPlanEditor] 课程目录已加载，跳过";
    }

    // 填充年级下拉框
    m_gradeCombo->clear();
    m_gradeCombo->addItem("请选择年级");
    QStringList grades = curriculum->getGrades();
    qDebug() << "[LessonPlanEditor] 获取到年级列表:" << grades;
    qDebug() << "[LessonPlanEditor] 年级数量:" << grades.size();

    for (const QString &grade : grades) {
        m_gradeCombo->addItem(grade);
        qDebug() << "[LessonPlanEditor] 添加年级:" << grade;
    }

    qDebug() << "[LessonPlanEditor] ComboBox项数:" << m_gradeCombo->count();
    qDebug() << "[LessonPlanEditor] ========== 课程目录加载完成 ==========";
}

// ================== 课程选择级联 ==================

void LessonPlanEditor::onGradeChanged(int index)
{
    // 清空下级选择
    m_semesterCombo->clear();
    m_unitCombo->clear();
    m_lessonCombo->clear();
    m_semesterCombo->setEnabled(false);
    m_unitCombo->setEnabled(false);
    m_lessonCombo->setEnabled(false);
    m_aiGenerateBtn->setEnabled(false);

    if (index <= 0) return;

    QString grade = m_gradeCombo->currentText();
    CurriculumService *curriculum = CurriculumService::instance();
    QStringList semesters = curriculum->getSemesters(grade);

    m_semesterCombo->addItem("请选择学期");
    for (const QString &semester : semesters) {
        m_semesterCombo->addItem(semester);
    }
    m_semesterCombo->setEnabled(true);
}

void LessonPlanEditor::onSemesterChanged(int index)
{
    m_unitCombo->clear();
    m_lessonCombo->clear();
    m_unitCombo->setEnabled(false);
    m_lessonCombo->setEnabled(false);
    m_aiGenerateBtn->setEnabled(false);

    if (index <= 0) return;

    QString grade = m_gradeCombo->currentText();
    QString semester = m_semesterCombo->currentText();
    CurriculumService *curriculum = CurriculumService::instance();
    QStringList units = curriculum->getUnits(grade, semester);

    m_unitCombo->addItem("请选择单元");
    for (const QString &unit : units) {
        m_unitCombo->addItem(unit);
    }
    m_unitCombo->setEnabled(true);
}

void LessonPlanEditor::onUnitChanged(int index)
{
    m_lessonCombo->clear();
    m_lessonCombo->setEnabled(false);
    m_aiGenerateBtn->setEnabled(false);

    if (index <= 0) return;

    QString grade = m_gradeCombo->currentText();
    QString semester = m_semesterCombo->currentText();
    QString unit = m_unitCombo->currentText();
    CurriculumService *curriculum = CurriculumService::instance();
    QStringList lessons = curriculum->getLessons(grade, semester, unit);

    m_lessonCombo->addItem("请选择课时");
    for (const QString &lesson : lessons) {
        m_lessonCombo->addItem(lesson);
    }
    m_lessonCombo->setEnabled(true);
}

void LessonPlanEditor::onLessonChanged(int index)
{
    m_aiGenerateBtn->setEnabled(index > 0);
}

// ================== AI生成教案 ==================

void LessonPlanEditor::onAIGenerateClicked()
{
    if (m_isGenerating) return;

    // 确认覆盖现有内容
    if (!m_editor->toPlainText().isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "确认", "AI生成将覆盖当前内容，是否继续？",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply != QMessageBox::Yes) return;
    }

    m_isGenerating = true;
    m_accumulatedMarkdown.clear();
    m_editor->clear();
    m_aiGenerateBtn->setEnabled(false);
    m_aiGenerateBtn->setIcon(QIcon(":/icons/resources/icons/loading-spinner.svg"));
    m_aiGenerateBtn->setText(" 生成中...");
    m_statusLabel->setText("AI正在生成教案...");
    emit aiGenerationStarted();

    // 创建DifyService（如果还没有）
    if (!m_difyService) {
        m_difyService = new DifyService(this);

        // 从环境变量获取API Key
        QString apiKey = qgetenv("DIFY_API_KEY");
        if (!apiKey.isEmpty()) {
            m_difyService->setApiKey(apiKey);
        }

        // 连接信号
        connect(m_difyService, &DifyService::streamChunkReceived,
                this, &LessonPlanEditor::onAIStreamChunk);
        connect(m_difyService, &DifyService::requestFinished,
                this, &LessonPlanEditor::onAIFinished);
        connect(m_difyService, &DifyService::errorOccurred,
                this, &LessonPlanEditor::onAIError);
    }

    // 构建提示词并发送
    QString prompt = buildAIPrompt();
    qDebug() << "[LessonPlanEditor] 发送AI生成请求：" << prompt.left(100) << "...";
    m_difyService->sendMessage(prompt, m_currentConversationId);
}

QString LessonPlanEditor::buildAIPrompt() const
{
    QString grade = m_gradeCombo->currentText();
    QString semester = m_semesterCombo->currentText();
    QString unit = m_unitCombo->currentText();
    QString lesson = m_lessonCombo->currentText();

    // 获取课时小节
    CurriculumService *curriculum = CurriculumService::instance();
    QStringList sections = curriculum->getSections(grade, semester, unit, lesson);
    QString sectionsStr = sections.join("、");

    // 构建更精确的prompt，严格限制AI只围绕指定章节生成内容
    QString prompt = QString(
        "# 教案生成任务\n\n"
        "## 严格约束（必须遵守）\n"
        "⚠️ **重要**：你只能围绕下面指定的【课时】和【小节】内容生成教案。\n"
        "- ❌ 禁止引用、提及或穿插其他单元、其他课时的内容\n"
        "- ❌ 禁止扩展到本课时以外的知识点\n"
        "- ✅ 所有教学内容、案例、练习都必须紧扣指定的小节主题\n\n"
        "## 课程信息\n"
        "| 项目 | 内容 |\n"
        "|------|------|\n"
        "| 学科 | 道德与法治（部编版） |\n"
        "| 年级 | %1 |\n"
        "| 学期 | %2 |\n"
        "| 单元 | %3 |\n"
        "| **本节课时** | **%4** |\n"
        "| **小节内容** | **%5** |\n\n"
        "## 教案结构要求\n"
        "请严格按照以下结构生成教案，每个部分都必须紧扣【%4】这一课时：\n\n"
        "### 1. 教学目标\n"
        "分三个维度撰写，所有目标都必须围绕「%5」这些小节内容：\n"
        "- **知识与技能**：学生应掌握的具体知识点\n"
        "- **过程与方法**：培养的思维方式和学习方法\n"
        "- **情感态度与价值观**：思政教育的核心价值引导\n\n"
        "### 2. 教学重难点\n"
        "- **重点**：本课时最核心的1-2个知识点\n"
        "- **难点**：学生理解起来有挑战的内容\n\n"
        "### 3. 教学过程\n"
        "请设计完整的45分钟课堂流程：\n"
        "- **导入新课**（5分钟）：用贴近学生生活的情境引入「%4」主题\n"
        "- **新课讲授**（25分钟）：分层讲解「%5」的核心内容\n"
        "- **课堂练习**（10分钟）：围绕本课内容设计互动讨论或练习\n"
        "- **课堂小结**（5分钟）：总结本节课的核心要点\n\n"
        "### 4. 板书设计\n"
        "用简洁的结构图或提纲展示本课时的知识框架。\n\n"
        "### 5. 作业布置\n"
        "布置1-2道与「%4」直接相关的课后思考题或实践任务。\n\n"
        "### 6. 教学反思\n"
        "留空供教师课后填写。\n\n"
        "---\n"
        "请使用清晰的Markdown格式输出，确保内容专业、符合初中思政课教学规范。"
    ).arg(grade, semester, unit, lesson, sectionsStr);

    return prompt;
}

void LessonPlanEditor::onAIStreamChunk(const QString &chunk)
{
    // 累积Markdown内容
    m_accumulatedMarkdown += chunk;

    // 实时渲染并显示
    QString html = m_markdownRenderer->renderToHtml(m_accumulatedMarkdown);
    m_editor->setHtml(html);

    // 滚动到底部
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_editor->setTextCursor(cursor);

    updateWordCount();
}

void LessonPlanEditor::onAIFinished()
{
    m_isGenerating = false;
    m_aiGenerateBtn->setEnabled(true);
    m_aiGenerateBtn->setIcon(QIcon(":/icons/resources/icons/ai-sparkle.svg"));
    m_aiGenerateBtn->setText(" AI生成教案");
    m_statusLabel->setText("生成完成");
    m_isModified = true;
    emit aiGenerationFinished();

    qDebug() << "[LessonPlanEditor] AI生成完成，内容长度：" << m_accumulatedMarkdown.length();
}

void LessonPlanEditor::onAIError(const QString &error)
{
    m_isGenerating = false;
    m_aiGenerateBtn->setEnabled(true);
    m_aiGenerateBtn->setIcon(QIcon(":/icons/resources/icons/ai-sparkle.svg"));
    m_aiGenerateBtn->setText(" AI生成教案");
    m_statusLabel->setText("生成失败");

    QMessageBox::warning(this, "AI生成失败", QString("生成教案时出错：%1").arg(error));
    qWarning() << "[LessonPlanEditor] AI生成失败：" << error;
}

// ================== 保存 ==================

void LessonPlanEditor::onSaveClicked()
{
    QString content = m_editor->toHtml();
    QString plainText = m_editor->toPlainText();

    if (content.isEmpty() || plainText.trimmed().isEmpty()) {
        QMessageBox::warning(this, "保存失败", "教案内容不能为空");
        return;
    }

    // 获取课时标题作为默认文件名
    QString lessonTitle = getCurrentLessonTitle();
    QString defaultFileName = lessonTitle.isEmpty() ? "教案" : lessonTitle;
    // 清理文件名中的非法字符
    defaultFileName.replace(QRegularExpression("[/\\\\:*?\"<>|]"), "_");

    // 获取桌面路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString defaultPath = desktopPath + "/" + defaultFileName + ".pdf";

    // 打开保存对话框 - 支持PDF、Word、HTML等格式
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "保存教案",
        defaultPath,
        "PDF文件 (*.pdf);;"
        "Word文档 (*.doc);;"
        "HTML文件 (*.html);;"
        "Markdown文件 (*.md);;"
        "纯文本文件 (*.txt)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    bool success = false;
    QString errorMsg;

    if (filePath.endsWith(".pdf", Qt::CaseInsensitive)) {
        // 保存为PDF
        success = saveToPdf(filePath, lessonTitle);
        if (!success) errorMsg = "PDF导出失败";
    } else if (filePath.endsWith(".doc", Qt::CaseInsensitive)) {
        // 保存为Word（实际是HTML格式，Word可以打开）
        success = saveToWord(filePath, lessonTitle, content);
        if (!success) errorMsg = "Word文档导出失败";
    } else {
        // 其他文本格式
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "保存失败",
                QString("无法打开文件：%1").arg(file.errorString()));
            return;
        }

        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);

        if (filePath.endsWith(".txt", Qt::CaseInsensitive)) {
            out << plainText;
        } else if (filePath.endsWith(".md", Qt::CaseInsensitive)) {
            out << (m_accumulatedMarkdown.isEmpty() ? plainText : m_accumulatedMarkdown);
        } else {
            // HTML格式
            out << buildHtmlDocument(lessonTitle, content);
        }

        file.close();
        success = true;
    }

    if (success) {
        m_isModified = false;
        m_statusLabel->setText("已保存：" + QFileInfo(filePath).fileName());
        emit saveRequested(lessonTitle, content);

        qDebug() << "[LessonPlanEditor] 教案已保存到：" << filePath;

        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "保存成功",
            QString("教案已保存到：\n%1\n\n是否立即打开？").arg(filePath),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    } else {
        QMessageBox::critical(this, "保存失败", errorMsg);
    }
}

bool LessonPlanEditor::saveToPdf(const QString &filePath, const QString &title)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    // 创建临时文档用于打印
    QTextDocument doc;
    doc.setHtml(buildHtmlDocument(title, m_editor->toHtml()));
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());

    doc.print(&printer);

    qDebug() << "[LessonPlanEditor] PDF导出成功：" << filePath;
    return true;
}

bool LessonPlanEditor::saveToWord(const QString &filePath, const QString &title, const QString &content)
{
    // Word可以直接打开HTML格式的.doc文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // 使用Word兼容的HTML格式
    out << "<!DOCTYPE html>\n"
        << "<html xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
        << "xmlns:w=\"urn:schemas-microsoft-com:office:word\">\n"
        << "<head>\n"
        << "<meta charset=\"UTF-8\">\n"
        << "<meta name=\"ProgId\" content=\"Word.Document\">\n"
        << "<title>" << (title.isEmpty() ? "教案" : title) << "</title>\n"
        << "<style>\n"
        << "@page { size: A4; margin: 2.5cm; }\n"
        << "body { font-family: '宋体', SimSun, serif; font-size: 12pt; line-height: 1.8; }\n"
        << "h1 { font-size: 18pt; color: #C62828; text-align: center; border-bottom: 2px solid #C62828; padding-bottom: 10px; }\n"
        << "h2 { font-size: 14pt; color: #1565C0; margin-top: 20pt; }\n"
        << "h3 { font-size: 12pt; color: #2E7D32; }\n"
        << "table { border-collapse: collapse; width: 100%; margin: 10pt 0; }\n"
        << "th, td { border: 1px solid #000; padding: 8pt; }\n"
        << "th { background-color: #f0f0f0; }\n"
        << "</style>\n"
        << "</head>\n<body>\n"
        << content
        << "\n</body>\n</html>";

    file.close();

    qDebug() << "[LessonPlanEditor] Word文档导出成功：" << filePath;
    return true;
}

QString LessonPlanEditor::buildHtmlDocument(const QString &title, const QString &content)
{
    return QString(
        "<!DOCTYPE html>\n<html>\n<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>%1</title>\n"
        "<style>\n"
        "body { font-family: 'PingFang SC', 'Microsoft YaHei', sans-serif; "
        "line-height: 1.8; padding: 40px; max-width: 800px; margin: 0 auto; }\n"
        "h1 { color: #C62828; border-bottom: 2px solid #C62828; padding-bottom: 10px; }\n"
        "h2 { color: #1565C0; margin-top: 30px; }\n"
        "h3 { color: #2E7D32; }\n"
        "table { border-collapse: collapse; width: 100%; margin: 15px 0; }\n"
        "th, td { border: 1px solid #ddd; padding: 10px; text-align: left; }\n"
        "th { background-color: #f5f5f5; }\n"
        "</style>\n"
        "</head>\n<body>\n%2\n</body>\n</html>"
    ).arg(title.isEmpty() ? "教案" : title, content);
}

// ================== 格式化工具栏 ==================

void LessonPlanEditor::onBoldClicked()
{
    QTextCharFormat format;
    QTextCursor cursor = m_editor->textCursor();

    if (cursor.charFormat().fontWeight() == QFont::Bold) {
        format.setFontWeight(QFont::Normal);
    } else {
        format.setFontWeight(QFont::Bold);
    }

    cursor.mergeCharFormat(format);
    m_editor->setTextCursor(cursor);
}

void LessonPlanEditor::onItalicClicked()
{
    QTextCharFormat format;
    QTextCursor cursor = m_editor->textCursor();

    format.setFontItalic(!cursor.charFormat().fontItalic());
    cursor.mergeCharFormat(format);
    m_editor->setTextCursor(cursor);
}

void LessonPlanEditor::onHeading1Clicked()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);

    QTextCharFormat charFormat;
    charFormat.setFontPointSize(24);
    charFormat.setFontWeight(QFont::Bold);
    charFormat.setForeground(QColor(PATRIOTIC_RED));

    cursor.mergeCharFormat(charFormat);
    m_editor->setTextCursor(cursor);
}

void LessonPlanEditor::onHeading2Clicked()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);

    QTextCharFormat charFormat;
    charFormat.setFontPointSize(18);
    charFormat.setFontWeight(QFont::Bold);

    cursor.mergeCharFormat(charFormat);
    m_editor->setTextCursor(cursor);
}

void LessonPlanEditor::onHeading3Clicked()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);

    QTextCharFormat charFormat;
    charFormat.setFontPointSize(14);
    charFormat.setFontWeight(QFont::Bold);

    cursor.mergeCharFormat(charFormat);
    m_editor->setTextCursor(cursor);
}

void LessonPlanEditor::onBulletListClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextListFormat listFormat;

    if (cursor.currentList()) {
        // 已经是列表，取消
        QTextBlockFormat blockFormat;
        blockFormat.setIndent(0);
        cursor.setBlockFormat(blockFormat);
    } else {
        // 创建无序列表
        listFormat.setStyle(QTextListFormat::ListDisc);
        cursor.createList(listFormat);
    }
}

void LessonPlanEditor::onNumberedListClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextListFormat listFormat;

    if (cursor.currentList()) {
        // 已经是列表，取消
        QTextBlockFormat blockFormat;
        blockFormat.setIndent(0);
        cursor.setBlockFormat(blockFormat);
    } else {
        // 创建有序列表
        listFormat.setStyle(QTextListFormat::ListDecimal);
        cursor.createList(listFormat);
    }
}

// ================== 其他方法 ==================

void LessonPlanEditor::onTextChanged()
{
    m_isModified = true;
    updateWordCount();
    emit contentChanged();
}

void LessonPlanEditor::updateWordCount()
{
    QString text = m_editor->toPlainText();
    int count = text.length();
    m_wordCountLabel->setText(QString("字数：%1").arg(count));
}

QString LessonPlanEditor::getContent() const
{
    return m_editor->toHtml();
}

void LessonPlanEditor::setContent(const QString &html)
{
    m_editor->setHtml(html);
    m_isModified = false;
}

QString LessonPlanEditor::getCurrentLessonTitle() const
{
    if (m_lessonCombo->currentIndex() <= 0) {
        return QString();
    }

    QString grade = m_gradeCombo->currentText();
    QString semester = m_semesterCombo->currentText();
    QString unit = m_unitCombo->currentText();
    QString lesson = m_lessonCombo->currentText();

    return CurriculumService::instance()->buildLessonTitle(grade, semester, unit, lesson);
}

void LessonPlanEditor::clear()
{
    m_editor->clear();
    m_gradeCombo->setCurrentIndex(0);
    m_semesterCombo->clear();
    m_unitCombo->clear();
    m_lessonCombo->clear();
    m_isModified = false;
    m_statusLabel->setText("就绪");
    updateWordCount();
}
