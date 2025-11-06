// ============================================================================
// AI 智能备课信号连接片段 - modernmainwindow.cpp
// ============================================================================

// 1. 在头文件中声明 AIPreparationWidget（已存在于 modernmainwindow.h）
// #include "../ui/aipreparationwidget.h"

// 2. 在 ModernMainWindow 类中添加成员变量（已存在于 modernmainwindow.h）
// AIPreparationWidget *m_prepWidget;

// 3. 在 createAIPreparation() 函数中初始化组件并连接信号
void ModernMainWindow::createAIPreparation()
{
    // 创建 AIPreparationWidget 实例
    // enableOnlineEdit = true 启用在线编辑按钮，false 则隐藏
    m_prepWidget = new AIPreparationWidget(this, /* enableOnlineEdit */ false);
    m_prepWidget->setObjectName("AIPreparationWidget");

    // 创建容器
    QFrame *container = new QFrame();
    container->setObjectName("AIPreparationContainer");
    container->setStyleSheet(QString(
        "QFrame#AIPreparationContainer {"
        "  background-color: %1;"
        "  border-radius: 12px;"
        "}"
    ).arg(BACKGROUND_LIGHT));

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(24, 24, 24, 24);
    containerLayout->addWidget(m_prepWidget);

    // ========================================================================
    // 核心：信号连接
    // ========================================================================

    // 3.1 生成相关信号
    connect(m_prepWidget, &AIPreparationWidget::generateRequested,
            this, &ModernMainWindow::onGeneratePPTRequested);

    // 3.2 预览相关信号
    connect(m_prepWidget, &AIPreparationWidget::previewRequested,
            this, &ModernMainWindow::onPreviewSlideRequested);
    connect(m_prepWidget, &AIPreparationWidget::slidePreviewRequested,
            this, &ModernMainWindow::onSlidePreviewRequested);

    // 3.3 操作按钮信号
    connect(m_prepWidget, &AIPreparationWidget::downloadRequested,
            this, &ModernMainWindow::onDownloadPPTRequested);
    connect(m_prepWidget, &AIPreparationWidget::saveToLibraryRequested,
            this, &ModernMainWindow::onSaveToLibraryRequested);
    connect(m_prepWidget, &AIPreparationWidget::onlineEditRequested,
            this, &ModernMainWindow::onOnlineEditRequested);
    connect(m_prepWidget, &AIPreparationWidget::regenerateRequested,
            this, &ModernMainWindow::onRegenerateRequested);

    // 3.4 拖拽排序信号
    connect(m_prepWidget, &AIPreparationWidget::slidesReordered,
            this, &ModernMainWindow::onSlidesReordered);

    // 添加到内容栈
    contentStack->addWidget(container);
    qDebug() << "AI 智能备课组件已创建并连接信号";
}

// 4. 槽函数实现示例
// ========================================================================

void ModernMainWindow::onGeneratePPTRequested(const QMap<QString, QString> &params)
{
    qDebug() << "收到 PPT 生成请求:";
    qDebug() << "  教材版本:" << params["textbook"];
    qDebug() << "  年级:" << params["grade"];
    qDebug() << "  章节:" << params["chapter"];
    qDebug() << "  模板:" << params["template"];
    qDebug() << "  课时长度:" << params["duration"];
    qDebug() << "  内容侧重:" << params["contentFocus"];

    // TODO: 调用 AI 生成逻辑
    // 示例：模拟生成过程
    simulateGeneration();
}

void ModernMainWindow::onPreviewSlideRequested(int index)
{
    qDebug() << "预览第" << (index + 1) << "张幻灯片";
    // TODO: 打开预览对话框
}

void ModernMainWindow::onSlidePreviewRequested(int index)
{
    qDebug() << "幻灯片预览请求:" << index;

    // 获取幻灯片图片并打开预览对话框
    // 这里需要根据实际数据结构获取图片
    // QImage slideImage = getSlideImage(index);
    // SlidePreviewDialog *dialog = new SlidePreviewDialog(slideImage, index, this);
    // dialog->exec();

    // 示例：打开对话框（需要先创建对话框实例或使用现有逻辑）
}

void ModernMainWindow::onDownloadPPTRequested()
{
    qDebug() << "下载 PPT 请求";

    // TODO: 实现 PPT 下载逻辑
    // 1. 生成 PPT 文件
    // 2. 打开保存对话框
    // 3. 保存到指定位置
}

void ModernMainWindow::onSaveToLibraryRequested()
{
    qDebug() << "保存到我的备课库请求";

    // TODO: 实现保存逻辑
    // 1. 保存 PPT 元数据
    // 2. 保存到用户备课库
    // 3. 更新界面显示
}

void ModernMainWindow::onOnlineEditRequested()
{
    qDebug() << "在线编辑请求";

    // TODO: 实现在线编辑逻辑
    // 1. 打开在线编辑器
    // 2. 或跳转到编辑页面
}

void ModernMainWindow::onRegenerateRequested()
{
    qDebug() << "重新生成请求";

    // TODO: 实现重新生成逻辑
    // 1. 重新调用生成函数
    // 2. 重置状态
}

void ModernMainWindow::onSlidesReordered(const QList<int> &newOrder)
{
    qDebug() << "幻灯片顺序已更改:" << newOrder;

    // TODO: 更新幻灯片顺序
    // 1. 更新数据结构
    // 2. 更新显示
}

// 5. 测试示例：模拟生成过程和状态更新
// ========================================================================

void ModernMainWindow::simulateGeneration()
{
    // 设置生成状态
    m_prepWidget->setGenerationState(AIPreparationWidget::GenerationState::Generating);

    // 模拟进度更新
    QTimer *timer = new QTimer(this);
    int progress = 0;

    connect(timer, &QTimer::timeout, this, [this, &progress, timer]() {
        progress += 10;
        m_prepWidget->setProgress(progress);

        if (progress >= 100) {
            timer->stop();
            timer->deleteLater();

            // 生成完成后设置幻灯片示例
            QVector<QImage> slides;
            // slides.append(QImage("slide1.png"));
            // slides.append(QImage("slide2.png"));
            // slides.append(QImage("slide3.png"));
            // slides.append(QImage("slide4.png"));

            // m_prepWidget->setSlides(slides);
        }
    });

    timer->start(500); // 每 500ms 更新一次
}

// 6. 在导航菜单中添加"AI 智能备课"按钮（如果需要）
// ========================================================================
/*
void ModernMainWindow::setupSidebarButtons()
{
    // 在现有按钮后添加
    QPushButton *prepButton = new QPushButton("🤖 AI 智能备课");
    prepButton->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(MEDIUM_GRAY).arg(LIGHT_GRAY));
    prepButton->setProperty("pageName", "AIPreparation");
    sidebarLayout->addWidget(prepButton);

    // 连接按钮点击
    connect(prepButton, &QPushButton::clicked, this, [this]() {
        // 如果页面未创建，先创建
        if (!contentStack->widget(contentStack->indexOf(findChild<QFrame*>("AIPreparationContainer")))) {
            createAIPreparation();
        }
        contentStack->setCurrentWidget(findChild<QFrame*>("AIPreparationContainer"));
    });
}
*/

// 7. 在 setupCentralWidget() 中调用 createAIPreparation()
// ========================================================================
/*
void ModernMainWindow::setupCentralWidget()
{
    // ... 现有代码 ...

    // 创建 AI 智能备课页面
    createAIPreparation();

    // ... 其他页面 ...
}
*/

// ============================================================================
// 使用示例
// ============================================================================

/*
// 在 ModernMainWindow 构造函数中调用：
ModernMainWindow::ModernMainWindow(const QString &userRole, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , currentUserRole(userRole)
    , currentUsername(username)
{
    // ... 初始化代码 ...

    // 在 createDashboard() 之后创建 AI 智能备课页面
    createAIPreparation();

    // ... 其他初始化 ...
}
*/