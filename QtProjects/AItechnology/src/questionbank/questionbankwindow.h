#ifndef QUESTIONBANKWINDOW_H
#define QUESTIONBANKWINDOW_H

#include <QWidget>
#include <QStringList>

class QButtonGroup;

class QuestionBankWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuestionBankWindow(QWidget *parent = nullptr);

private:
    void buildUI();
    QWidget *createNavBar();
    QWidget *createBody();
    QWidget *createSidebar();
    QWidget *createContentCard();
    QWidget *createFilterGroup(const QString &title,
                               const QStringList &options,
                               const QString &groupName,
                               int columns = 2);
    void loadStyle();

    QButtonGroup *m_optionGroup = nullptr;
};

#endif // QUESTIONBANKWINDOW_H
