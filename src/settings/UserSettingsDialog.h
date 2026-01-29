#ifndef USERSETTINGSDIALOG_H
#define USERSETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

class QSvgWidget;

/**
 * @brief 用户设置对话框
 *
 * 让用户编辑个人信息：昵称、部门、称呼等
 * 使用 SVG 图标，界面更专业统一
 */
class UserSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserSettingsDialog(QWidget *parent = nullptr);
    ~UserSettingsDialog();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onResetClicked();
    void updatePreview();

private:
    void setupUI();
    void loadSettings();
    QWidget* createUserCard();
    QWidget* createBasicInfoSection();
    QWidget* createFormRow(const QString &iconPath, const QString &label, QWidget *input);
    QWidget* createSectionHeader(const QString &iconPath, const QString &title);
    QString getInputStyle() const;
    QString getIconPath(const QString &name) const;

    // 输入控件
    QLineEdit *m_nicknameEdit;
    QLineEdit *m_departmentEdit;
    QLineEdit *m_titleEdit;

    // 预览
    QLabel *m_previewLabel;
    QLabel *m_avatarPreview;
    QLabel *m_emailLabel;

    // 按钮
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_resetBtn;
};

#endif // USERSETTINGSDIALOG_H
