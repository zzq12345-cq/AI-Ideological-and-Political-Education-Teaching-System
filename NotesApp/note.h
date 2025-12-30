#ifndef NOTE_H
#define NOTE_H

#include <QString>
#include <QDateTime>

class Note
{
public:
    Note();
    Note(const QString& title, const QString& content);

    QString title() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    QString content() const { return m_content; }
    void setContent(const QString& content) { m_content = content; }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& date) { m_createdAt = date; }

    QDateTime updatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& date) { m_updatedAt = date; }

    QString filePath() const { return m_filePath; }
    void setFilePath(const QString& path) { m_filePath = path; }

private:
    QString m_title;
    QString m_content;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QString m_filePath;
};

#endif // NOTE_H
