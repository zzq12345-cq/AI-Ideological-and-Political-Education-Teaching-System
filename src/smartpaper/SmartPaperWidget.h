#ifndef SMARTPAPERWIDGET_H
#define SMARTPAPERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QStackedWidget>
#include <QProgressBar>
#include <QList>

#include "SmartPaperConfig.h"
#include "SmartPaperService.h"

class PaperService;

/**
 * @brief 智能组卷 UI 组件
 *
 * 配置面板 + 结果预览 + 操作按钮
 * 参考 AIPreparationWidget 的布局模式。
 */
class SmartPaperWidget : public QWidget
{
    Q_OBJECT

public:
    enum class AssemblyState { Idle, Generating, Success, Failed };

    explicit SmartPaperWidget(QWidget *parent = nullptr);
    ~SmartPaperWidget();

private slots:
    void onAddTypeRow();
    void onRemoveTypeRow(int index);
    void onGenerateClicked();
    void onSwapQuestion(const QString &questionId, const QString &questionType);
    void onSaveToCloud();
    void onImportToBasket();
    void onEditExport();
    void onRegenerate();

    // SmartPaperService 信号处理
    void onGenerationCompleted(const SmartPaperResult &result);
    void onProgressUpdated(int percent, const QString &message);
    void onGenerationFailed(const QString &error);

    // PaperService 信号处理（保存到云端）
    void onPaperCreated(const Paper &paper);

private:
    void initUI();
    void setupTitleSection(QVBoxLayout *mainLayout);
    void setupConfigCard(QVBoxLayout *mainLayout);
    void setupStatusArea(QVBoxLayout *mainLayout);
    void setupResultPreview(QVBoxLayout *mainLayout);
    void setupBottomActions(QVBoxLayout *mainLayout);

    // 题型分布表管理
    QWidget *createTypeRow(int index);
    void updateTotalScoreLabel();
    void updateGenerateButton();

    // 结果预览构建
    void buildResultPreview();
    void clearResultPreview();

    // 题型显示名称映射
    QString questionTypeDisplayName(const QString &type) const;
    QString difficultyDisplayName(const QString &diff) const;
    QColor difficultyColor(const QString &diff) const;

    // 状态切换
    void setState(AssemblyState state);

    // 服务
    SmartPaperService *m_smartPaperService = nullptr;
    PaperService *m_paperService = nullptr;

    // 状态
    AssemblyState m_state = AssemblyState::Idle;
    SmartPaperConfig m_config;
    SmartPaperResult m_currentResult;

    // 配置区UI
    QLineEdit *m_titleEdit = nullptr;
    QComboBox *m_subjectCombo = nullptr;
    QComboBox *m_gradeCombo = nullptr;
    QComboBox *m_durationCombo = nullptr;

    // 题型分布表
    QVBoxLayout *m_typeRowsLayout = nullptr;
    QList<QWidget *> m_typeRows;
    QLabel *m_totalScoreLabel = nullptr;

    // 难度比例
    QSpinBox *m_easyRatioSpin = nullptr;
    QSpinBox *m_mediumRatioSpin = nullptr;
    QSpinBox *m_hardRatioSpin = nullptr;

    // 状态区
    QStackedWidget *m_statusStack = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_errorLabel = nullptr;
    QPushButton *m_retryBtn = nullptr;

    // 结果预览区
    QWidget *m_resultWidget = nullptr;
    QVBoxLayout *m_resultLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;  // 外层滚动区域，用于自动滚到结果

    // 底部操作栏
    QFrame *m_bottomActions = nullptr;

    // 开始组卷按钮
    QPushButton *m_generateBtn = nullptr;

    // 用于保存到云端的临时数据
    QList<PaperQuestion> m_pendingSaveQuestions;
};

#endif // SMARTPAPERWIDGET_H
