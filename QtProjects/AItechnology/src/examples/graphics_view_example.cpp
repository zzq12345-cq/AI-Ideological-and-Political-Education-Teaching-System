#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>

class GraphicsViewExample : public QWidget
{
    Q_OBJECT

public:
    GraphicsViewExample(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
        setupGraphicsView();
    }

private slots:
    void zoomIn()
    {
        view->scale(1.2, 1.2);
    }

    void zoomOut()
    {
        view->scale(0.8, 0.8);
    }

    void resetView()
    {
        view->resetTransform();
    }

    void onZoomChanged(int value)
    {
        qreal scaleFactor = value / 100.0;
        view->resetTransform();
        view->scale(scaleFactor, scaleFactor);
    }

private:
    QGraphicsView *view;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;

    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // 控制按钮布局
        QHBoxLayout *buttonLayout = new QHBoxLayout();

        QPushButton *zoomInBtn = new QPushButton("放大");
        QPushButton *zoomOutBtn = new QPushButton("缩小");
        QPushButton *resetBtn = new QPushButton("重置");
        QSlider *zoomSlider = new QSlider(Qt::Horizontal);
        zoomSlider->setRange(10, 200);
        zoomSlider->setValue(100);
        zoomSlider->setMaximumWidth(200);

        connect(zoomInBtn, &QPushButton::clicked, this, &GraphicsViewExample::zoomIn);
        connect(zoomOutBtn, &QPushButton::clicked, this, &GraphicsViewExample::zoomOut);
        connect(resetBtn, &QPushButton::clicked, this, &GraphicsViewExample::resetView);
        connect(zoomSlider, &QSlider::valueChanged, this, &GraphicsViewExample::onZoomChanged);

        buttonLayout->addWidget(zoomInBtn);
        buttonLayout->addWidget(zoomOutBtn);
        buttonLayout->addWidget(resetBtn);
        buttonLayout->addWidget(new QLabel("缩放:"));
        buttonLayout->addWidget(zoomSlider);
        buttonLayout->addStretch();

        mainLayout->addLayout(buttonLayout);

        // QGraphicsView
        view = new QGraphicsView();
        view->setMinimumHeight(400);
        view->setDragMode(QGraphicsView::ScrollHandDrag);  // 鼠标拖拽模式
        view->setRenderHint(QPainter::Antialiasing);      // 抗锯齿
        mainLayout->addWidget(view);

        setLayout(mainLayout);
    }

    void setupGraphicsView()
    {
        // 创建场景
        scene = new QGraphicsScene();
        view->setScene(scene);

        // 加载图片
        QPixmap pixmap(":/images/large_image.jpg");
        if (pixmap.isNull()) {
            // 如果没有图片，创建一个测试图片
            pixmap = QPixmap(800, 600);
            pixmap.fill(QColor(100, 150, 200));
            QPainter painter(&pixmap);
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", 24));
            painter.drawText(pixmap.rect(), Qt::AlignCenter, "测试图片\n800x600");
        }

        // 添加图片到场景
        pixmapItem = scene->addPixmap(pixmap);
        pixmapItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);

        // 设置场景矩形
        scene->setSceneRect(pixmap.rect());

        // 启用鼠标交互
        view->setInteractive(true);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GraphicsViewExample window;
    window.setWindowTitle("Qt QGraphicsView 图片显示示例");
    window.resize(800, 600);
    window.show();

    return app.exec();
}

#include "graphics_view_example.moc"