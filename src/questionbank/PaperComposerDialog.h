#ifndef PAPERCOMPOSERDIALOG_H
#define PAPERCOMPOSERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

class QScrollArea;
class QTableWidget;
struct PaperQuestion;

/**
 * @brief 组卷对话框
 *
 * 用于预览和编辑试题篮中的试题
 * 支持设置试卷标题、调整题目顺序、设置分值
 * 可导出为试卷
 */
class PaperComposerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaperComposerDialog(QWidget *parent = nullptr);

signals:
    // 导出试卷时发出
    void paperExported(const QString &filePath);

private slots:
    void onRefreshList();
    void onMoveUp();
    void onMoveDown();
    void onRemoveSelected();
    void onExportPaper();
    void onScoreChanged(int row, int score);

private:
    void setupUI();
    void populateTable();
    QString generatePaperHtml();

    // UI 组件
    QLineEdit *m_titleEdit = nullptr;
    QTableWidget *m_questionTable = nullptr;
    QLabel *m_summaryLabel = nullptr;  // 合并后的统计标签
    QPushButton *m_exportButton = nullptr;

    void updateSummary();
};

#endif // PAPERCOMPOSERDIALOG_H
