#include "HelpCenterWidget.h"
#include "../shared/StyleConfig.h"
#include <QApplication>
#include <QFile>
#include <QDateTime>
#include <QStandardPaths>
#include "../shared/ModernDialogHelper.h"
#include <QGridLayout>
#include <QDir>
#include <QGraphicsDropShadowEffect>

// ==================== 样式常量 ====================
static const char* SECTION_TITLE_STYLE =
    "font-size: 20px; font-weight: 700; color: #0F172A; margin: 0; padding: 0;";

static const char* CARD_STYLE =
    "QFrame { background: #FFFFFF; border: 1px solid #F1F5F9; border-radius: 16px; }";

static const char* ACCORDION_BTN_STYLE =
    "QPushButton { background: transparent; border: none; text-align: left;"
    "  font-size: 15px; font-weight: 600; color: #1E293B; padding: 16px 20px;"
    "  border-radius: 8px; }"
    "QPushButton:hover { background: #F8FAFC; color: #6366F1; }";

static const char* KBD_STYLE =
    "background: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 6px;"
    "padding: 4px 10px; font-family: 'SF Mono', Menlo, monospace; font-size: 13px;"
    "color: #475569; font-weight: 600;";

// ==================== 构造/析构 ====================

HelpCenterWidget::HelpCenterWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}
HelpCenterWidget::~HelpCenterWidget() {}

// ==================== 辅助方法 ====================
static void applyCardShadow(QWidget* widget) {
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(15, 23, 42, 12));
    widget->setGraphicsEffect(shadow);
}

// ==================== 主布局 ====================

void HelpCenterWidget::setupUI() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: #F8FAFC; border: none; }"
        "QScrollBar:vertical { width: 8px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #CBD5E1; border-radius: 4px; min-height: 40px; }"
        "QScrollBar::handle:vertical:hover { background: #64748B; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background: #F8FAFC;");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(60, 48, 60, 60);
    m_contentLayout->setSpacing(32);

    // 各板块
    m_contentLayout->addWidget(createHeaderSection());
    m_quickStartSection = createQuickStartSection();
    m_contentLayout->addWidget(m_quickStartSection);
    m_featureGuideSection = createFeatureGuideSection();
    m_contentLayout->addWidget(m_featureGuideSection);
    m_faqSection = createFAQSection();
    m_contentLayout->addWidget(m_faqSection);
    m_shortcutsSection = createShortcutsSection();
    m_contentLayout->addWidget(m_shortcutsSection);
    m_aboutSection = createAboutSection();
    m_contentLayout->addWidget(m_aboutSection);
    m_feedbackSection = createFeedbackSection();
    m_contentLayout->addWidget(m_feedbackSection);
    m_contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea);
}

// ==================== 顶部标题 + 搜索 ====================

QWidget* HelpCenterWidget::createHeaderSection() {
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 16);
    lay->setSpacing(16);

    auto *title = new QLabel("帮助中心");
    title->setStyleSheet("font-size: 32px; font-weight: 800; color: #0F172A; letter-spacing: 1px;");
    lay->addWidget(title);

    auto *subtitle = new QLabel("您好，这里是系统使用帮助。输入关键词可快速搜索。");
    subtitle->setStyleSheet("font-size: 15px; color: #64748B;");
    lay->addWidget(subtitle);

    // 搜索框
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("搜索帮助内容...");
    m_searchInput->setClearButtonEnabled(true);
    m_searchInput->setFixedHeight(48);
    m_searchInput->setStyleSheet(
        "QLineEdit { background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px;"
        "  padding: 0 20px; font-size: 15px; color: #1E293B; }"
        "QLineEdit:focus { border-color: #6366F1; background: #FFFFFF; }");
    applyCardShadow(m_searchInput);

    connect(m_searchInput, &QLineEdit::textChanged, this, &HelpCenterWidget::onSearchTextChanged);
    lay->addWidget(m_searchInput);

    m_searchResultLabel = new QLabel();
    m_searchResultLabel->setStyleSheet("font-size: 14px; color: #64748B; font-weight: 500;");
    m_searchResultLabel->hide();
    lay->addWidget(m_searchResultLabel);

    return w;
}

// ==================== 1. 快速入门 ====================

QWidget* HelpCenterWidget::createQuickStartSection() {
    auto *section = new QFrame();
    section->setStyleSheet(CARD_STYLE);
    applyCardShadow(section);

    auto *lay = new QVBoxLayout(section);
    lay->setContentsMargins(32, 28, 32, 32);
    lay->setSpacing(24);

    auto *title = new QLabel("快速入门");
    title->setStyleSheet(SECTION_TITLE_STYLE);
    lay->addWidget(title);

    struct Step { QString num; QString name; QString desc; };
    QVector<Step> steps = {
        {"01", "登录系统",   "使用邮箱注册并登录"},
        {"02", "完善信息",   "在设置中填写昵称与部门"},
        {"03", "智能备课",   "与 AI 对话生成教案和 PPT"},
        {"04", "试题管理",   "导入、编辑、AI 生成试题"},
        {"05", "班级管理",   "创建班级、添加学生成员"},
        {"06", "数据分析",   "查看教学数据与学情报告"},
    };

    auto *grid = new QGridLayout();
    grid->setSpacing(20);
    for (int i = 0; i < steps.size(); ++i) {
        auto *card = new QFrame();
        card->setStyleSheet(
            "QFrame { background: #F8FAFC; border: none;"
            "  border-radius: 12px; }"
            "QFrame:hover { background: #F1F5F9; }");
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(20, 20, 20, 20);
        cl->setSpacing(8);

        auto *num = new QLabel(steps[i].num);
        num->setStyleSheet("font-size: 28px; font-weight: 800; color: #C7D2FE; font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;");
        cl->addWidget(num);

        auto *name = new QLabel(steps[i].name);
        name->setStyleSheet("font-size: 16px; font-weight: 700; color: #1E293B;");
        cl->addWidget(name);

        auto *desc = new QLabel(steps[i].desc);
        desc->setStyleSheet("font-size: 13px; color: #64748B; line-height: 1.5;");
        desc->setWordWrap(true);
        cl->addWidget(desc);

        grid->addWidget(card, i / 3, i % 3);
    }
    lay->addLayout(grid);
    return section;
}

// ==================== Accordion 辅助 ====================

QWidget* HelpCenterWidget::createAccordionItem(const QString &title, const QString &content) {
    auto *container = new QWidget();
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    auto *btn = new QPushButton("+  " + title);
    btn->setStyleSheet(ACCORDION_BTN_STYLE);
    btn->setCursor(Qt::PointingHandCursor);
    lay->addWidget(btn);

    auto *contentW = new QWidget();
    contentW->setVisible(false);
    auto *cLay = new QVBoxLayout(contentW);
    cLay->setContentsMargins(20, 4, 20, 20);

    auto *label = new QLabel(content);
    label->setWordWrap(true);
    label->setStyleSheet("font-size: 14px; color: #475569; line-height: 1.6;");
    label->setTextFormat(Qt::RichText);
    cLay->addWidget(label);
    lay->addWidget(contentW);

    // 分隔线
    auto *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: #F1F5F9; max-height: 1px; border: none;");
    lay->addWidget(sep);

    connect(btn, &QPushButton::clicked, this, [btn, contentW, title]() {
        bool show = !contentW->isVisible();
        contentW->setVisible(show);
        if (show) {
            btn->setText("-  " + title);
            btn->setStyleSheet(
                "QPushButton { background: #EEF2FF; border: none; text-align: left;"
                "  font-size: 15px; font-weight: 600; color: #4F46E5; padding: 16px 20px;"
                "  border-radius: 8px; }"
            );
        } else {
            btn->setText("+  " + title);
            btn->setStyleSheet(ACCORDION_BTN_STYLE);
        }
    });

    AccordionEntry entry{container, btn, contentW, title, content};
    m_accordionItems.append(entry);

    return container;
}

// ==================== 2. 功能模块指引 ====================

QWidget* HelpCenterWidget::createFeatureGuideSection() {
    auto *section = new QFrame();
    section->setStyleSheet(CARD_STYLE);
    applyCardShadow(section);

    auto *lay = new QVBoxLayout(section);
    lay->setContentsMargins(32, 28, 32, 24);
    lay->setSpacing(8);

    auto *title = new QLabel("功能模块指引");
    title->setStyleSheet(SECTION_TITLE_STYLE);
    lay->addWidget(title);
    lay->addSpacing(12);

    struct Guide { QString name; QString desc; };
    QVector<Guide> guides = {
        {"教师中心", "首页仪表板，查看教学概览、快捷入口和 AI 对话。支持流式响应，可直接在首页与 AI 助手交流备课问题。"},
        {"AI 智能备课", "与 AI 深度对话，自动生成教案和 PPT 演示文稿。支持多轮对话，AI 可理解上下文并持续优化输出。教案编辑器支持实时预览和导出。"},
        {"试题库", "管理海量试题，支持手动添加、AI 解析文档自动生成试题、批量导入导出。内置智能组卷和质量检查功能。"},
        {"时政新闻", "自动追踪和采集思政相关时政热点新闻，按分类浏览，帮助教师将时事融入课堂教学。"},
        {"考勤管理", "快速发起考勤活动，学生扫码或手动签到。支持查看历史记录、统计出勤率、导出考勤报表。"},
        {"数据分析", "教学数据可视化面板，包含个人和班级维度的成绩统计、知识点掌握雷达图、学情趋势分析报告。"},
        {"我的班级", "班级全生命周期管理：创建班级、邀请学生加入、分页浏览成员、发布作业、上传教学资料。"},
        {"系统设置", "个人信息管理：修改昵称、部门、称呼，输入邀请码升级教师角色。"},
    };

    for (auto &g : guides)
        lay->addWidget(createAccordionItem(g.name, g.desc));

    return section;
}

// ==================== 3. FAQ ====================

QWidget* HelpCenterWidget::createFAQSection() {
    auto *section = new QFrame();
    section->setStyleSheet(CARD_STYLE);
    applyCardShadow(section);

    auto *lay = new QVBoxLayout(section);
    lay->setContentsMargins(32, 28, 32, 24);
    lay->setSpacing(8);

    auto *title = new QLabel("常见问题");
    title->setStyleSheet(SECTION_TITLE_STYLE);
    lay->addWidget(title);
    lay->addSpacing(12);

    struct FAQ { QString q; QString a; };
    QVector<FAQ> faqs = {
        {"如何升级为教师角色？",
         "点击侧边栏头像旁的设置图标，进入系统设置，在「邀请码」一栏中输入管理员提供的教师邀请码，保存后即可升级。"},
        {"AI 对话没有响应怎么办？",
         "1. 检查网络连接是否正常<br>2. 确认环境变量 <b>DIFY_API_KEY</b> 已正确设置<br>3. 确保 api.dify.ai 地址可访问（不被代理拦截）"},
        {"网络错误：2 是什么意思？",
         "<b>RemoteHostClosedError</b> — 远程服务器关闭了连接。常见原因：代理配置不当或网络不稳定。请检查代理设置，确保 <b>*.supabase.co</b> 和 <b>api.dify.ai</b> 在白名单中。"},
        {"如何导入试题？",
         "进入试题库页面 → 点击「批量导入」→ 选择 Word (.docx) 或 PDF 文件 → AI 将自动解析文档内容并生成结构化试题。"},
        {"如何生成 PPT？",
         "在 AI 备课对话中发送包含 PPT、演示文稿或课件等关键词的消息，AI 会进入 PPT 生成流程，引导您完善主题后自动生成。"},
        {"考勤功能如何使用？",
         "教师端：进入考勤管理 → 选择班级 → 发起新的考勤活动<br>学生端：在班级详情中找到考勤入口 → 点击签到"},
    };

    for (auto &f : faqs)
        lay->addWidget(createAccordionItem(f.q, f.a));

    return section;
}

// ==================== 4. 快捷键速查 ====================

QWidget* HelpCenterWidget::createShortcutsSection() {
    auto *section = new QFrame();
    section->setStyleSheet(CARD_STYLE);
    applyCardShadow(section);

    auto *lay = new QVBoxLayout(section);
    lay->setContentsMargins(32, 28, 32, 32);
    lay->setSpacing(24);

    auto *title = new QLabel("快捷键速查");
    title->setStyleSheet(SECTION_TITLE_STYLE);
    lay->addWidget(title);

    struct Shortcut { QString keys; QString desc; };
    QVector<Shortcut> shortcuts = {
        {"Ctrl + N",     "新建对话"},
        {"Ctrl + S",     "保存教案"},
        {"Ctrl + Enter", "发送消息"},
        {"Esc",          "关闭弹窗 / 返回"},
        {"Ctrl + F",     "搜索试题"},
        {"Ctrl + E",     "导出文件"},
    };

    auto *grid = new QGridLayout();
    grid->setSpacing(16);
    for (int i = 0; i < shortcuts.size(); ++i) {
        auto *row = new QWidget();
        auto *rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        rl->setSpacing(16);

        auto *kbd = new QLabel(shortcuts[i].keys);
        kbd->setStyleSheet(KBD_STYLE);
        kbd->setFixedHeight(30);
        kbd->setAlignment(Qt::AlignCenter);
        rl->addWidget(kbd);

        auto *desc = new QLabel(shortcuts[i].desc);
        desc->setStyleSheet("font-size: 14px; font-weight: 500; color: #334155;");
        rl->addWidget(desc);
        rl->addStretch();

        grid->addWidget(row, i / 2, i % 2);
    }
    lay->addLayout(grid);
    return section;
}

// ==================== 5. 关于系统 ====================

QWidget* HelpCenterWidget::createAboutSection() {
    auto *section = new QFrame();
    section->setStyleSheet(CARD_STYLE);
    applyCardShadow(section);

    auto *lay = new QVBoxLayout(section);
    lay->setContentsMargins(32, 28, 32, 32);
    lay->setSpacing(16);

    auto *title = new QLabel("关于系统");
    title->setStyleSheet(SECTION_TITLE_STYLE);
    lay->addWidget(title);

    struct Info { QString label; QString value; };
    QVector<Info> infos = {
        {"系统名称", "AI 思政智慧课堂系统"},
        {"当前版本", "v1.0.0"},
        {"技术栈",   "Qt 6 · C++17 · Dify AI · Supabase"},
        {"开发框架", "CMake 构建 · SSE 流式通信 · 讯飞 PPT API"},
        {"开源协议", "MIT License"},
    };

    for (auto &info : infos) {
        auto *row = new QWidget();
        auto *rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        auto *lbl = new QLabel(info.label);
        lbl->setStyleSheet("font-size: 14px; color: #64748B; min-width: 100px;");
        auto *val = new QLabel(info.value);
        val->setStyleSheet("font-size: 14px; color: #0F172A; font-weight: 600;");
        rl->addWidget(lbl);
        rl->addWidget(val);
        rl->addStretch();
        lay->addWidget(row);
    }
    return section;
}

// ==================== 6. 反馈与支持 ====================

QWidget* HelpCenterWidget::createFeedbackSection() {
    auto *section = new QFrame();
    section->setStyleSheet(CARD_STYLE);
    applyCardShadow(section);

    auto *lay = new QVBoxLayout(section);
    lay->setContentsMargins(32, 28, 32, 32);
    lay->setSpacing(20);

    auto *title = new QLabel("反馈与支持");
    title->setStyleSheet(SECTION_TITLE_STYLE);
    lay->addWidget(title);

    auto *emailRow = new QWidget();
    auto *el = new QHBoxLayout(emailRow);
    el->setContentsMargins(0, 0, 0, 0);
    auto *emailLbl = new QLabel("联系邮箱");
    emailLbl->setStyleSheet("font-size: 14px; color: #64748B; min-width: 100px;");
    auto *emailVal = new QLabel("2477664538@qq.com");
    emailVal->setStyleSheet("font-size: 14px; color: #6366F1; font-weight: 600;");
    emailVal->setTextInteractionFlags(Qt::TextSelectableByMouse);
    el->addWidget(emailLbl);
    el->addWidget(emailVal);
    el->addStretch();
    lay->addWidget(emailRow);

    auto *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: #F1F5F9; max-height: 1px; border: none; margin: 8px 0;");
    lay->addWidget(sep);

    auto *fbLabel = new QLabel("意见反馈");
    fbLabel->setStyleSheet("font-size: 15px; font-weight: 600; color: #1E293B;");
    lay->addWidget(fbLabel);

    m_feedbackEdit = new QTextEdit();
    m_feedbackEdit->setPlaceholderText("请输入您的建议或遇到的问题...");
    m_feedbackEdit->setFixedHeight(120);
    m_feedbackEdit->setStyleSheet(
        "QTextEdit { background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px;"
        "  padding: 12px; font-size: 14px; color: #334155; }"
        "QTextEdit:focus { border-color: #6366F1; background: #FFFFFF; }");
    lay->addWidget(m_feedbackEdit);

    auto *submitBtn = new QPushButton("提交反馈");
    submitBtn->setFixedSize(120, 40);
    submitBtn->setCursor(Qt::PointingHandCursor);
    submitBtn->setStyleSheet(
        "QPushButton { background: #6366F1; color: white; border: none; border-radius: 8px;"
        "  font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #4F46E5; }"
        "QPushButton:pressed { background: #4338CA; }");
    connect(submitBtn, &QPushButton::clicked, this, &HelpCenterWidget::onSubmitFeedback);

    auto *btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(0, 0, 0, 0);
    btnLay->addWidget(submitBtn);
    btnLay->addStretch();
    lay->addLayout(btnLay);

    return section;
}

// ==================== 搜索功能 ====================

void HelpCenterWidget::onSearchTextChanged(const QString &text) {
    if (text.trimmed().isEmpty()) {
        resetAllSections();
        m_searchResultLabel->hide();
    } else {
        performSearch(text.trimmed());
    }
}

void HelpCenterWidget::performSearch(const QString &keyword) {
    int matchCount = 0;

    // 搜索折叠面板
    for (auto &entry : m_accordionItems) {
        bool match = entry.titleText.contains(keyword, Qt::CaseInsensitive) ||
                     entry.contentText.contains(keyword, Qt::CaseInsensitive);
        entry.container->setVisible(match);
        if (match) {
            entry.contentWidget->setVisible(true);
            entry.titleBtn->setText("-  " + entry.titleText);
            entry.titleBtn->setStyleSheet(
                "QPushButton { background: #EEF2FF; border: none; text-align: left;"
                "  font-size: 15px; font-weight: 600; color: #4F46E5; padding: 16px 20px;"
                "  border-radius: 8px; }"
            );
            matchCount++;
        }
    }

    // 板块级别 — 保持 quickStart / shortcuts / about / feedback 始终可见
    m_quickStartSection->setVisible(true);
    m_shortcutsSection->setVisible(true);
    m_aboutSection->setVisible(true);
    m_feedbackSection->setVisible(true);

    m_searchResultLabel->setText(
        QString("找到 %1 条相关结果").arg(matchCount));
    m_searchResultLabel->show();
}

void HelpCenterWidget::resetAllSections() {
    for (auto &entry : m_accordionItems) {
        entry.container->setVisible(true);
        entry.contentWidget->setVisible(false);
        entry.titleBtn->setText("+  " + entry.titleText);
        entry.titleBtn->setStyleSheet(ACCORDION_BTN_STYLE);
    }
    m_quickStartSection->setVisible(true);
    m_featureGuideSection->setVisible(true);
    m_faqSection->setVisible(true);
    m_shortcutsSection->setVisible(true);
    m_aboutSection->setVisible(true);
    m_feedbackSection->setVisible(true);
}

// ==================== 反馈提交 ====================

void HelpCenterWidget::onSubmitFeedback() {
    QString text = m_feedbackEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        ModernDialogHelper::warning(this, "提示", "请输入反馈内容后再提交。");
        return;
    }

    // 保存到本地文件
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    QString path = dir + "/feedback.log";
    QFile file(path);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << "=== " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " ===\n";
        out << text << "\n\n";
        file.close();

        m_feedbackEdit->clear();
        ModernDialogHelper::info(this, "感谢反馈", "您的反馈已保存，感谢您的建议！");
    } else {
        ModernDialogHelper::warning(this, "保存失败", "无法保存反馈内容，请稍后重试。");
    }
}
