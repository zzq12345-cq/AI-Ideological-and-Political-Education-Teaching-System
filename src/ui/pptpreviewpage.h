#ifndef PPTPREVIEWPAGE_H
#define PPTPREVIEWPAGE_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class MacOSQuickLookPreview;

class PPTPreviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit PPTPreviewPage(QWidget *parent = nullptr);

    void setPresentation(const QString &title, const QString &filePath);
    QString presentationPath() const;

signals:
    void backRequested();
    void downloadRequested();
    void modifySuggestionSubmitted(const QString &suggestion);

private:
    void updateState();
    void onSendSuggestion();

    QString m_title;
    QString m_filePath;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_hintLabel = nullptr;
    QLabel *m_fileLabel = nullptr;
    QPushButton *m_backButton = nullptr;
    QPushButton *m_downloadButton = nullptr;
    MacOSQuickLookPreview *m_previewWidget = nullptr;

    // 修改建议输入区
    QLineEdit *m_suggestionInput = nullptr;
    QPushButton *m_sendSuggestionBtn = nullptr;
};

#endif // PPTPREVIEWPAGE_H
