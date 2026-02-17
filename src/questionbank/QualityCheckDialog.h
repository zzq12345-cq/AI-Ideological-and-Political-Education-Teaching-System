#ifndef QUALITYCHECKDIALOG_H
#define QUALITYCHECKDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>

class QuestionQualityService;
class PaperService;

/**
 * @brief 题库质量检查对话框
 *
 * 全库去重扫描，带统计报告。
 */
class QualityCheckDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QualityCheckDialog(QuestionQualityService *qualityService,
                                 PaperService *paperService,
                                 QWidget *parent = nullptr);

private slots:
    void onStartScan();
    void onScanProgress(int current, int total);
    void onScanCompleted(const QList<QPair<QString,QString>> &duplicatePairs);
    void onScanError(const QString &error);

private:
    void initUI();

    QuestionQualityService *m_qualityService;
    PaperService *m_paperService;

    // UI
    QWidget *m_idleView;
    QWidget *m_scanningView;
    QWidget *m_resultView;

    QPushButton *m_scanBtn;
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    QLabel *m_resultSummary;
    QVBoxLayout *m_resultListLayout;
};

#endif // QUALITYCHECKDIALOG_H
