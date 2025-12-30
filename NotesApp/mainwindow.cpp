#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_currentNote(nullptr)
    , m_selectedNoteIndex(-1)
{
    setupUI();

    // 设置笔记目录
    m_notesDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/MyNotes";
    QDir dir;
    if (!dir.exists(m_notesDir)) {
        dir.mkpath(m_notesDir);
    }

    // 加载笔记
    loadNotes();

    // 如果没有笔记，创建一个欢迎笔记
    if (m_notes.isEmpty()) {
        Note welcomeNote;
        welcomeNote.setTitle("欢迎使用 Markdown 笔记");
        welcomeNote.setContent("# 欢迎使用 Markdown 笔记应用\n\n"
                              "这是一个功能强大的笔记应用，支持以下特性：\n\n"
                              "## 功能特性\n\n"
                              "- **Markdown 支持**: 完整的 Markdown 语法支持\n"
                              "- **实时预览**: 编辑时实时预览 Markdown 渲染效果\n"
                              "- **搜索功能**: 快速搜索笔记标题和内容\n"
                              "- **语法高亮**: 编辑时自动高亮 Markdown 语法\n\n"
                              "## Markdown 语法示例\n\n"
                              "### 文本格式\n"
                              "- **粗体文本**: 使用 `**文本**` 或 `__文本__`\n"
                              "- *斜体文本*: 使用 `*文本*` 或 `_文本_`\n"
                              "- `代码`: 使用反引号包围\n\n"
                              "### 代码块\n"
                              "```\n"
                              "代码块使用三个反引号包围\n"
                              "```\n\n"
                              "### 列表\n"
                              "- 无序列表项 1\n"
                              "- 无序列表项 2\n\n"
                              "1. 有序列表项 1\n"
                              "2. 有序列表项 2\n\n"
                              "### 链接\n"
                              "[链接文本](https://example.com)\n\n"
                              "开始创建你的笔记吧！");
"
        m_notes.append(welcomeNote);
        saveNoteToFile(welcomeNote);
        updateNotesList(m_notes);
    }

    m_statusLabel->setText(QStringLiteral("就绪 | 笔记数量: %1").arg(m_notes.size()));
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle(QStringLiteral("Markdown 笔记应用"));
    resize(1200, 800);

    // 创建中心部件和主布局
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 创建工具栏
    QHBoxLayout* toolBarLayout = new QHBoxLayout();

    QPushButton* newBtn = new QPushButton(QStringLiteral("新建笔记"), this);
    QPushButton* deleteBtn = new QPushButton(QStringLiteral("删除笔记"), this);
    QPushButton* saveBtn = new QPushButton(QStringLiteral("保存"), this);
    QPushButton* exportBtn = new QPushButton(QStringLiteral("导出"), this);
    QPushButton* aboutBtn = new QPushButton(QStringLiteral("关于"), this);

    newBtn->setMaximumWidth(100);
    deleteBtn->setMaximumWidth(100);
    saveBtn->setMaximumWidth(100);
    exportBtn->setMaximumWidth(100);
    aboutBtn->setMaximumWidth(100);

    toolBarLayout->addWidget(newBtn);
    toolBarLayout->addWidget(deleteBtn);
    toolBarLayout->addWidget(saveBtn);
    toolBarLayout->addWidget(exportBtn);
    toolBarLayout->addStretch();
    toolBarLayout->addWidget(aboutBtn);

    connect(newBtn, &QPushButton::clicked, this, &MainWindow::createNewNote);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteCurrentNote);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveCurrentNote);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::exportNote);
    connect(aboutBtn, &QPushButton::clicked, this, &MainWindow::showAbout);

    mainLayout->addLayout(toolBarLayout);

    // 创建搜索栏
    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel(QStringLiteral("搜索:"), this);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("输入关键词搜索笔记..."));

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchEdit);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::searchNotes);

    mainLayout->addLayout(searchLayout);

    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);

    // 左侧面板 - 笔记列表
    QWidget* leftPanel = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    m_notesList = new QListWidget(this);
    m_notesList->setMinimumWidth(250);

    leftLayout->addWidget(m_notesList);
    leftPanel->setLayout(leftLayout);

    // 右侧面板 - 编辑器和预览
    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    // 标题编辑
    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* titleLabel = new QLabel(QStringLiteral("标题:"), this);
    m_titleEdit = new QTextEdit(this);
    m_titleEdit->setMaximumHeight(50);
    m_titleEdit->setPlaceholderText(QStringLiteral("输入笔记标题..."));

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(m_titleEdit);

    rightLayout->addLayout(titleLayout);

    // 创建编辑器和预览的分割器
    QSplitter* editPreviewSplitter = new QSplitter(Qt::Vertical, this);

    // 编辑器
    QWidget* editorWidget = new QWidget(this);
    QVBoxLayout* editorLayout = new QVBoxLayout(editorWidget);

    QLabel* editLabel = new QLabel(QStringLiteral("编辑 (Markdown)"), this);
    m_contentEdit = new QTextEdit(this);
    m_contentEdit->setPlaceholderText(QStringLiteral("输入 Markdown 内容..."));

    // 添加 Markdown 高亮
    m_highlighter = new MarkdownHighlighter(m_contentEdit->document());

    editorLayout->addWidget(editLabel);
    editorLayout->addWidget(m_contentEdit);

    editorWidget->setLayout(editorLayout);

    // 预览
    QWidget* previewWidget = new QWidget(this);
    QVBoxLayout* previewLayout = new QVBoxLayout(previewWidget);

    QLabel* previewLabel = new QLabel(QStringLiteral("预览"), this);
    m_previewEdit = new QTextEdit(this);
    m_previewEdit->setReadOnly(true);

    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(m_previewEdit);

    previewWidget->setLayout(previewLayout);

    editPreviewSplitter->addWidget(editorWidget);
    editPreviewSplitter->addWidget(previewWidget);
    editPreviewSplitter->setStretchFactor(0, 1);
    editPreviewSplitter->setStretchFactor(1, 1);

    rightLayout->addWidget(editPreviewSplitter);

    // 连接信号
    connect(m_notesList, &QListWidget::itemClicked, this, &MainWindow::onNoteSelected);
    connect(m_titleEdit, &QTextEdit::textChanged, [this]() {
        if (m_currentNote && m_selectedNoteIndex >= 0) {
            m_notes[m_selectedNoteIndex].setTitle(m_titleEdit->toPlainText());
        }
    });
    connect(m_contentEdit, &QTextEdit::textChanged, this, &MainWindow::onContentChanged);

    rightPanel->setLayout(rightLayout);

    // 添加到主分割器
    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(rightPanel);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 3);

    mainLayout->addWidget(m_splitter);

    // 状态栏
    m_statusLabel = new QLabel(QStringLiteral("就绪"), this);
    mainLayout->addWidget(m_statusLabel);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::loadNotes()
{
    QDir dir(m_notesDir);
    QStringList filters;
    filters << "*.json";
    dir.setNameFilters(filters);

    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo& fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                Note note;
                note.setTitle(obj["title"].toString());
                note.setContent(obj["content"].toString());
                note.setCreatedAt(QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate));
                note.setUpdatedAt(QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate));
                note.setFilePath(fileInfo.absoluteFilePath());

                m_notes.append(note);
            }
        }
    }

    // 按更新时间排序
    std::sort(m_notes.begin(), m_notes.end(), [](const Note& a, const Note& b) {
        return a.updatedAt() > b.updatedAt();
    });

    updateNotesList(m_notes);
}

void MainWindow::saveNoteToFile(const Note& note)
{
    QString fileName = note.createdAt().toString("yyyyMMdd-hhmmss") + ".json";
    QString filePath = m_notesDir + "/" + fileName;

    QJsonObject obj;
    obj["title"] = note.title();
    obj["content"] = note.content();
    obj["createdAt"] = note.createdAt().toString(Qt::ISODate);
    obj["updatedAt"] = note.updatedAt().toString(Qt::ISODate);

    QJsonDocument doc(obj);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void MainWindow::deleteNoteFile(const QString& filePath)
{
    QFile file(filePath);
    if (file.exists()) {
        file.remove();
    }
}

void MainWindow::updateNotesList(const QVector<Note>& notes)
{
    m_notesList->clear();

    for (const Note& note : notes) {
        QListWidgetItem* item = new QListWidgetItem(note.title());
        item->setData(Qt::UserRole, note.updatedAt().toString(Qt::ISODate));
        m_notesList->addItem(item);
    }
}

void MainWindow::filterNotes(const QString& query)
{
    if (query.isEmpty()) {
        updateNotesList(m_notes);
        return;
    }

    QVector<Note> filtered = filterNotesByQuery(query);
    updateNotesList(filtered);
}

QVector<Note> MainWindow::filterNotesByQuery(const QString& query)
{
    QVector<Note> filtered;

    for (const Note& note : m_notes) {
        if (note.title().contains(query, Qt::CaseInsensitive) ||
            note.content().contains(query, Qt::CaseInsensitive)) {
            filtered.append(note);
        }
    }

    return filtered;
}

QString MainWindow::renderMarkdown(const QString& markdown)
{
    QString html = markdown;

    // 转义 HTML 特殊字符
    html.replace("&", "&amp;");
    html.replace("<", "&lt;");
    html.replace(">", "&gt;");

    // Headers
    html.replace(QRegularExpression("^######\\s(.*?)$"), "<h6>\\1</h6>");
    html.replace(QRegularExpression("^#####\\s(.*?)$"), "<h5>\\1</h5>");
    html.replace(QRegularExpression("^####\\s(.*?)$"), "<h4>\\1</h4>");
    html.replace(QRegularExpression("^###\\s(.*?)$"), "<h3>\\1</h3>");
    html.replace(QRegularExpression("^##\\s(.*?)$"), "<h2>\\1</h2>");
    html.replace(QRegularExpression("^#\\s(.*?)$"), "<h1>\\1</h1>");

    // Bold
    html.replace(QRegularExpression("\\*\\*(.*?)\\*\\*"), "<b>\\1</b>");
    html.replace(QRegularExpression("__(.*?)__"), "<b>\\1</b>");

    // Italic
    html.replace(QRegularExpression("\\*(.*?)\\*"), "<i>\\1</i>");
    html.replace(QRegularExpression("_(.*?)_"), "<i>\\1</i>");

    // Code
    html.replace(QRegularExpression("`(.*?)`"), "<code style='background-color: #f4f4f4; padding: 2px 4px; border-radius: 3px;'>\\1</code>");

    // Links
    html.replace(QRegularExpression("\\[(.*?)\\]\\((.*?)\\)"), "<a href='\\2'>\\1</a>");

    // Unordered lists
    html.replace(QRegularExpression("^\\s*[-*+]\\s+(.*?)$", QRegularExpression::MultilineOption), "<li>\\1</li>");
    html.replace(QRegularExpression("(<li>.*?</li>)\\n(<li>.*?</li>)", QRegularExpression::MultilineOption), "<ul>\\1\\2</ul>");

    // Ordered lists
    html.replace(QRegularExpression("^\\s*\\d+\\.\\s+(.*?)$", QRegularExpression::MultilineOption), "<li>\\1</li>");

    // Line breaks
    html.replace("\n\n", "<br><br>");

    return html;
}

void MainWindow::createNewNote()
{
    Note newNote;
    newNote.setTitle(QStringLiteral("新笔记"));
    newNote.setContent("");

    m_notes.append(newNote);
    saveNoteToFile(newNote);

    updateNotesList(m_notes);

    // 选中新创建的笔记
    m_notesList->setCurrentRow(m_notes.size() - 1);
    onNoteSelected(m_notesList->currentItem());

    m_statusLabel->setText(QStringLiteral("已创建新笔记 | 笔记数量: %1").arg(m_notes.size()));
}

void MainWindow::deleteCurrentNote()
{
    if (m_selectedNoteIndex < 0 || m_selectedNoteIndex >= m_notes.size()) {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("请先选择要删除的笔记"));
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, QStringLiteral("确认删除"),
                                  QStringLiteral("确定要删除笔记 \"%1\" 吗？")
                                  .arg(m_notes[m_selectedNoteIndex].title()),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        deleteNoteFile(m_notes[m_selectedNoteIndex].filePath());
        m_notes.removeAt(m_selectedNoteIndex);

        m_titleEdit->clear();
        m_contentEdit->clear();
        m_previewEdit->clear();
        m_selectedNoteIndex = -1;
        m_currentNote = nullptr;

        updateNotesList(m_notes);
        m_statusLabel->setText(QStringLiteral("已删除笔记 | 笔记数量: %1").arg(m_notes.size()));
    }
}

void MainWindow::saveCurrentNote()
{
    if (m_selectedNoteIndex < 0 || m_selectedNoteIndex >= m_notes.size()) {
        return;
    }

    Note& note = m_notes[m_selectedNoteIndex];
    note.setTitle(m_titleEdit->toPlainText());
    note.setContent(m_contentEdit->toPlainText());
    note.setUpdatedAt(QDateTime::currentDateTime());

    saveNoteToFile(note);
    m_statusLabel->setText(QStringLiteral("已保存: %1").arg(note.title()));
}

void MainWindow::onNoteSelected(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    // 找到对应的笔记
    QString title = item->text();
    for (int i = 0; i < m_notes.size(); ++i) {
        if (m_notes[i].title() == title) {
            m_selectedNoteIndex = i;
            m_currentNote = &m_notes[i];

            m_titleEdit->setPlainText(m_currentNote->title());
            m_contentEdit->setPlainText(m_currentNote->content());

            // 更新预览
            QString html = renderMarkdown(m_currentNote->content());
            m_previewEdit->setHtml(html);

            break;
        }
    }

    m_statusLabel->setText(QStringLiteral("已打开: %1").arg(m_currentNote->title()));
}

void MainWindow::onTitleChanged(const QString& title)
{
    if (m_currentNote) {
        m_currentNote->setTitle(title);
    }
}

void MainWindow::onContentChanged()
{
    if (m_currentNote && m_selectedNoteIndex >= 0) {
        m_notes[m_selectedNoteIndex].setContent(m_contentEdit->toPlainText());

        // 实时更新预览
        QString html = renderMarkdown(m_contentEdit->toPlainText());
        m_previewEdit->setHtml(html);
    }
}

void MainWindow::searchNotes(const QString& query)
{
    filterNotes(query);

    if (!query.isEmpty()) {
        QVector<Note> filtered = filterNotesByQuery(query);
        m_statusLabel->setText(QStringLiteral("搜索结果: %1 个笔记").arg(filtered.size()));
    } else {
        m_statusLabel->setText(QStringLiteral("笔记数量: %1").arg(m_notes.size()));
    }
}

void MainWindow::exportNote()
{
    if (m_selectedNoteIndex < 0 || m_selectedNoteIndex >= m_notes.size()) {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("请先选择要导出的笔记"));
        return;
    }

    const Note& note = m_notes[m_selectedNoteIndex];

    QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("导出笔记"),
                                                     note.title() + ".md",
                                                     QStringLiteral("Markdown 文件 (*.md);;所有文件 (*)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "# " << note.title() << "\n\n";
            out << note.content();
            file.close();

            QMessageBox::information(this, QStringLiteral("成功"),
                                    QStringLiteral("笔记已导出到:\n%1").arg(fileName));
        }
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, QStringLiteral("关于"),
                      QStringLiteral("<h3>Markdown 笔记应用</h3>"
                                    "<p>版本: 1.0</p>"
                                    "<p>一个功能强大的 Markdown 笔记应用</p>"
                                    "<p><b>功能特性:</b></p>"
                                    "<ul>"
                                    "<li>完整的 Markdown 语法支持</li>"
                                    "<li>实时预览和语法高亮</li>"
                                    "<li>强大的搜索功能</li>"
                                    "<li>导出为 Markdown 文件</li>"
                                    "</ul>"));
}
