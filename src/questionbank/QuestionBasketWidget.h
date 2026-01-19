#ifndef QUESTIONBASKETWIDGET_H
#define QUESTIONBASKETWIDGET_H

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QVBoxLayout>

class PaperQuestion;
class QScrollArea;

/**
 * @brief 试题篮悬浮组件
 *
 * 悬浮在右下角，显示已选试题数量
 * 点击展开可查看已选试题列表，支持移除试题
 * 提供"生成试卷"按钮进入组卷流程
 */
class QuestionBasketWidget : public QFrame
{
    Q_OBJECT

public:
    explicit QuestionBasketWidget(QWidget *parent = nullptr);

    // 刷新显示
    void refresh();

signals:
    // 点击"生成试卷"时发出
    void composePaperRequested();

    // 试题被移除时发出
    void questionRemoved(const QString &questionId);

    // 大小变化时发出（用于父窗口重新定位）
    void sizeChanged();

private slots:
    void onBasketCountChanged(int count);
    void onToggleExpand();
    void onComposePaper();
    void onClearBasket();
    void onRemoveQuestion(const QString &questionId);

private:
    void setupUI();
    void setupConnections();
    QWidget *createQuestionItem(const PaperQuestion &question, int index);
    void updateQuestionList();

    // UI 组件
    QFrame *m_collapsedView = nullptr;   // 收起状态的视图
    QFrame *m_expandedView = nullptr;    // 展开状态的视图
    QLabel *m_countBadge = nullptr;      // 数量徽章
    QLabel *m_countLabel = nullptr;      // 展开视图中的数量标签
    QPushButton *m_toggleButton = nullptr;
    QPushButton *m_composeButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QVBoxLayout *m_questionListLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;

    bool m_expanded = false;

protected:
    bool event(QEvent *event) override;
};

#endif // QUESTIONBASKETWIDGET_H
