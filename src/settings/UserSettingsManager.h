#ifndef USERSETTINGSMANAGER_H
#define USERSETTINGSMANAGER_H

#include <QObject>
#include <QString>
#include <QSettings>

/**
 * @brief 用户设置管理器（单例）
 *
 * 管理用户个人信息设置，存储到本地配置文件
 * 老王说：这玩意儿就是个配置管理器，别瞎改
 */
class UserSettingsManager : public QObject
{
    Q_OBJECT

public:
    static UserSettingsManager* instance();

    // 用户昵称（显示名称）
    QString nickname() const;
    void setNickname(const QString &name);

    // 用户部门/组织
    QString department() const;
    void setDepartment(const QString &dept);

    // 用户职称
    QString title() const;
    void setTitle(const QString &title);

    // 称呼偏好（老师/教授/同志等）
    QString honorific() const;
    void setHonorific(const QString &honorific);

    // 头像首字（用于头像显示）
    QString avatarInitial() const;

    // AI 问候语模板
    QString aiGreetingTemplate() const;
    void setAIGreetingTemplate(const QString &tmpl);

    // 用户邮箱（登录后设置）
    QString email() const;
    void setEmail(const QString &email);

    // 获取完整显示名称（昵称 + 称呼）
    QString displayName() const;

    // 获取 AI 对用户的称呼（姓 + 称呼，如"周老师"）
    QString aiUserTitle() const;

    // 保存/加载
    void save();
    void load();

    // 恢复默认
    void resetToDefaults();

signals:
    void settingsChanged();

private:
    explicit UserSettingsManager(QObject *parent = nullptr);
    ~UserSettingsManager();

    static UserSettingsManager *s_instance;
    QSettings *m_settings;

    QString m_nickname;
    QString m_department;
    QString m_title;
    QString m_honorific;
    QString m_aiGreetingTemplate;
    QString m_email;
};

#endif // USERSETTINGSMANAGER_H
