#ifndef STUDENTMATERIALWIDGET_H
#define STUDENTMATERIALWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QEvent>
#include "MaterialManager.h"

// 面包屑点击
class StudentBreadcrumbFilter : public QObject
{
    Q_OBJECT
public:
    StudentBreadcrumbFilter(int index, std::function<void(int)> cb, QObject *p = nullptr)
        : QObject(p), m_index(index), m_cb(cb) {}
protected:
    bool eventFilter(QObject *, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonRelease) { m_cb(m_index); return true; }
        return false;
    }
private:
    int m_index;
    std::function<void(int)> m_cb;
};

// 行点击
class StudentRowFilter : public QObject
{
    Q_OBJECT
public:
    StudentRowFilter(bool isFolder, const QString &folderId, const QString &folderName,
                     const QString &fileUrl, std::function<void()> onFolder,
                     std::function<void(const QString &)> onFile, QObject *p = nullptr)
        : QObject(p), m_isFolder(isFolder), m_folderId(folderId), m_folderName(folderName),
          m_fileUrl(fileUrl), m_onFolder(onFolder), m_onFile(onFile) {}
protected:
    bool eventFilter(QObject *, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (m_isFolder) m_onFolder();
            else if (!m_fileUrl.isEmpty()) m_onFile(m_fileUrl);
            return true;
        }
        return false;
    }
private:
    bool m_isFolder;
    QString m_folderId, m_folderName, m_fileUrl;
    std::function<void()> m_onFolder;
    std::function<void(const QString &)> m_onFile;
};

class StudentMaterialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StudentMaterialWidget(const QString &classId, QWidget *parent = nullptr);

signals:
    void backRequested();

private:
    void setupUI();
    void loadFolder(const QString &folderId, const QString &folderName = QString());
    void navigateToBreadcrumb(int index);
    void refreshCurrentFolder();
    void updateBreadcrumbUI();
    QWidget* createMaterialRow(const MaterialManager::MaterialInfo &info);
    void clearList();
    QString formatFileSize(qint64 bytes) const;
    QString getFileIcon(const QString &mimeType, const QString &type) const;

    QString m_classId;
    QString m_currentFolderId;
    QList<QPair<QString, QString>> m_breadcrumb;

    QFrame *m_breadcrumbBar = nullptr;
    QHBoxLayout *m_breadcrumbLayout = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QWidget *m_listContainer = nullptr;
    QScrollArea *m_scrollArea = nullptr;
};

#endif
