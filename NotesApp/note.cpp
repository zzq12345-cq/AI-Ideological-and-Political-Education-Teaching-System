#include "note.h"

Note::Note()
    : m_createdAt(QDateTime::currentDateTime())
    , m_updatedAt(QDateTime::currentDateTime())
{
}

Note::Note(const QString& title, const QString& content)
    : m_title(title)
    , m_content(content)
    , m_createdAt(QDateTime::currentDateTime())
    , m_updatedAt(QDateTime::currentDateTime())
{
}
