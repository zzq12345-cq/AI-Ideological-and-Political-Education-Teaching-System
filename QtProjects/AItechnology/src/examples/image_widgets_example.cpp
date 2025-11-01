#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QFileDialog>

class ImageExample : public QWidget
{
    Q_OBJECT

public:
    ImageExample(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
    }

private:
    void setupUI()
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // 1. QLabel 显示图片 - 最常用
        QLabel *imageLabel = new QLabel();
        QPixmap pixmap(":/images/logo.png");  // 从资源文件加载
        if (pixmap.isNull()) {
            // 如果资源文件不存在，使用绝对路径
            pixmap.load("/path/to/your/image.png");
        }

        if (!pixmap.isNull()) {
            imageLabel->setPixmap(pixmap);
            imageLabel->setAlignment(Qt::AlignCenter);

            // 设置图片缩放方式
            imageLabel->setScaledContents(false);  // 不拉伸，保持原始比例

            // 设置固定大小
            imageLabel->setFixedSize(200, 200);
        } else {
            imageLabel->setText("图片未找到");
            imageLabel->setAlignment(Qt::AlignCenter);
        }

        // 2. 显示缩略图的QLabel
        QLabel *thumbnailLabel = new QLabel();
        QPixmap thumbPixmap(":/images/thumbnail.png");
        if (!thumbPixmap.isNull()) {
            // 缩放图片适应标签大小
            QPixmap scaledPixmap = thumbPixmap.scaled(
                150, 150,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            thumbnailLabel->setPixmap(scaledPixmap);
            thumbnailLabel->setAlignment(Qt::AlignCenter);
        }

        // 3. 背景图片QLabel
        QLabel *backgroundLabel = new QLabel();
        backgroundLabel->setText("背景图片文字");
        backgroundLabel->setAlignment(Qt::AlignCenter);
        backgroundLabel->setStyleSheet(
            "QLabel {"
            "   background-image: url(:/images/background.jpg);"
            "   background-position: center;"
            "   background-repeat: no-repeat;"
            "   color: white;"
            "   font-size: 18px;"
            "   font-weight: bold;"
            "   padding: 50px;"
            "}"
        );
        backgroundLabel->setMinimumHeight(200);

        layout->addWidget(new QLabel("1. QLabel显示图片:"));
        layout->addWidget(imageLabel);

        layout->addWidget(new QLabel("2. 缩略图:"));
        layout->addWidget(thumbnailLabel);

        layout->addWidget(new QLabel("3. 背景图片:"));
        layout->addWidget(backgroundLabel);

        setLayout(layout);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ImageExample window;
    window.setWindowTitle("Qt 图片控件示例");
    window.resize(400, 600);
    window.show();

    return app.exec();
}

#include "image_widgets_example.moc"