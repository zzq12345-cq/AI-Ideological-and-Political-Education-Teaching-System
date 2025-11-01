#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QIcon>

class ImageButtonExample : public QWidget
{
    Q_OBJECT

public:
    ImageButtonExample(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
    }

private slots:
    void onImageClicked()
    {
        qDebug() << "图片按钮被点击!";
    }

private:
    void setupUI()
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // 1. 带图标的按钮
        QPushButton *iconButton = new QPushButton("登录");
        QIcon icon(":/images/login_icon.png");
        iconButton->setIcon(icon);
        iconButton->setIconSize(QSize(32, 32));
        iconButton->setStyleSheet(
            "QPushButton {"
            "   padding: 10px;"
            "   font-size: 16px;"
            "}"
        );
        connect(iconButton, &QPushButton::clicked, this, &ImageButtonExample::onImageClicked);

        // 2. 纯图片按钮
        QPushButton *imageButton = new QPushButton();
        QPixmap pixmap(":/images/button_image.png");
        QIcon buttonIcon(pixmap);
        imageButton->setIcon(buttonIcon);
        imageButton->setIconSize(QSize(100, 100));
        imageButton->setFixedSize(120, 120);
        imageButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
        connect(imageButton, &QPushButton::clicked, this, &ImageButtonExample::onImageClicked);

        // 3. 悬停效果的图片按钮
        QPushButton *hoverButton = new QPushButton();
        QPixmap hoverPixmap(":/images/hover.png");
        QIcon hoverIcon(hoverPixmap);
        hoverButton->setIcon(hoverIcon);
        hoverButton->setIconSize(QSize(80, 80));
        hoverButton->setFixedSize(100, 100);
        hoverButton->setStyleSheet(
            "QPushButton {"
            "   border: 2px solid #ccc;"
            "   border-radius: 10px;"
            "   background-color: white;"
            "}"
            "QPushButton:hover {"
            "   border-color: #007bff;"
            "   background-color: #f0f8ff;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #e6f3ff;"
            "}"
        );

        layout->addWidget(new QLabel("1. 带图标的按钮:"));
        layout->addWidget(iconButton);

        layout->addWidget(new QLabel("2. 纯图片按钮:"));
        layout->addWidget(imageButton);

        layout->addWidget(new QLabel("3. 悬停效果图片按钮:"));
        layout->addWidget(hoverButton);

        setLayout(layout);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ImageButtonExample window;
    window.setWindowTitle("Qt 图片按钮示例");
    window.resize(300, 400);
    window.show();

    return app.exec();
}

#include "image_button_example.moc"