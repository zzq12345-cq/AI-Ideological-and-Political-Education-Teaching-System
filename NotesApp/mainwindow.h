#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "note.h"
#include "markdownhighlighter.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void createNewNote();
    void deleteCurrentNote();
    void saveCurrentNote();
    void onNoteSelected(QListWidgetItem* item);
    void onTitleChanged(const QString& title);
    void onContentChanged();
    void searchNotes(const QString& query);
    void exportNote();
    void showAbout();

private:
    void setupUI();
    void loadNotes();
    void saveNoteToFile(const Note& note);
    void deleteNoteFile(const QString& filePath);
    void updateNotesList(const QVector<Note>& notes);
    void filterNotes(const QString& query);
    QString renderMarkdown(const QString& markdown);
    QVector<Note> filterNotesByQuery(const QString& query);

    // UI Components
    QSplitter* m_splitter;
    QListWidget* m_notesList;
    QLineEdit* m_searchEdit;
    QTextEdit* m_titleEdit;
    QTextEdit* m_contentEdit;
    QTextEdit* m_previewEdit;
    QLabel* m_statusLabel;

    MarkdownHighlighter* m_highlighter;

    // Data
    QVector<Note> m_notes;
    Note* m_currentNote;
    QString m_notesDir;
    int m_selectedNoteIndex;
};

#endif // MAINWINDOW_H
