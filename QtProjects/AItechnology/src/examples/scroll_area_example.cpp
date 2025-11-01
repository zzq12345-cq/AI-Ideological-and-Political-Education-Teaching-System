#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

class ScrollAreaExample : public QWidget
{
    Q_OBJECT

public:
    ScrollAreaExample(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
    }

private slots:
    void loadImage()
    {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "选择图片",
            "",
            "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"
        );

        if (!fileName.isEmpty()) {
            QPixmap pixmap(fileName);
            if (!pixmap.isNull()) {
                imageLabel->setPixmap(pixmap);
                imageLabel->resize(pixmap.size());
            }
        }
    }

private:
    QLabel *imageLabel;

    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // 加载图片按钮
        QPushButton *loadBtn = new QPushButton("加载图片");
        connect(loadBtn, &QPushButton::clicked, this, &ScrollAreaExample::loadImage);

        // 创建滚动区域
        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(false);  // 不自动调整大小
        scrollArea->setMinimumHeight(400);

        // 创建图片标签
        imageLabel = new QLabel();
        imageLabel->setAlignment(Qt::AlignCenter);

        // 加载默认图片
        QPixmap defaultPixmap(":/images/default.jpg");
        if (defaultPixmap.isNull()) {
            // 创建默认图片
            defaultPixmap = QPixmap(1200, 800);
            defaultPixmap.fill(QColor(200, 220, 240));
            QPainter painter(&defaultPixmap);
            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", 32));
            painter.drawText(defaultPixmap.rect(), Qt::AlignCenter,
                "大尺寸图片示例\n1200 x 800\n\n可以滚动查看");
        }

        imageLabel->setPixmap(defaultPixmap);
        imageLabel->resize(defaultPixmap.size());

        // 设置滚动区域的内容
        scrollArea->setWidget(imageLabel);

        mainLayout->addWidget(loadBtn);
        mainLayout->addWidget(scrollArea);

        setLayout(mainLayout);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ScrollAreaExample window;
    window.setWindowTitle("Qt QScrollArea 图片显示示例");
    window.resize(600, 500);
    window.show();

    return app.exec();
}

#include "scroll_area_example.moc"