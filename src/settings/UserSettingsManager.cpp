#include "UserSettingsManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QUrl>

UserSettingsManager* UserSettingsManager::s_instance = nullptr;

UserSettingsManager* UserSettingsManager::instance()
{
    if (!s_instance) {
        s_instance = new UserSettingsManager();
    }
    return s_instance;
}

UserSettingsManager::UserSettingsManager(QObject *parent)
    : QObject(parent)
{
    // 使用应用程序的配置目录
    m_settings = new QSettings(
        QSettings::IniFormat,
        QSettings::UserScope,
        QCoreApplication::organizationName().isEmpty() ? "AIPoliticalEducation" : QCoreApplication::organizationName(),
        "UserSettings"
    );

    load();
    qDebug() << "[UserSettingsManager] Initialized, nickname:" << m_nickname;
}

UserSettingsManager::~UserSettingsManager()
{
    save();
    delete m_settings;
}

QString UserSettingsManager::nickname() const
{
    return m_nickname;
}

void UserSettingsManager::setNickname(const QString &name)
{
    if (m_nickname != name) {
        m_nickname = name;
        emit settingsChanged();
    }
}

QString UserSettingsManager::department() const
{
    return m_department;
}

void UserSettingsManager::setDepartment(const QString &dept)
{
    if (m_department != dept) {
        m_department = dept;
        emit settingsChanged();
    }
}

QString UserSettingsManager::title() const
{
    return m_title;
}

void UserSettingsManager::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit settingsChanged();
    }
}

QString UserSettingsManager::honorific() const
{
    return m_honorific;
}

void UserSettingsManager::setHonorific(const QString &honorific)
{
    if (m_honorific != honorific) {
        m_honorific = honorific;
        emit settingsChanged();
    }
}

QString UserSettingsManager::avatarInitial() const
{
    // 取昵称的第一个字符作为头像
    if (!m_nickname.isEmpty()) {
        return m_nickname.left(1);
    }
    return "U";  // 默认
}

QString UserSettingsManager::aiGreetingTemplate() const
{
    return m_aiGreetingTemplate;
}

void UserSettingsManager::setAIGreetingTemplate(const QString &tmpl)
{
    if (m_aiGreetingTemplate != tmpl) {
        m_aiGreetingTemplate = tmpl;
        emit settingsChanged();
    }
}

QString UserSettingsManager::email() const
{
    return m_email;
}

void UserSettingsManager::setEmail(const QString &email)
{
    QString normalizedEmail = email.trimmed();
    if (normalizedEmail.contains("@")) {
        normalizedEmail = normalizedEmail.toLower();
    }
    if (m_email == normalizedEmail) {
        return;
    }

    if (!m_email.isEmpty()) {
        save();
    }

    m_email = normalizedEmail;
    loadFromCurrentScope();
    emit settingsChanged();
}

QString UserSettingsManager::role() const
{
    return m_role;
}

void UserSettingsManager::setRole(const QString &role)
{
    if (m_role != role) {
        m_role = role;
        emit settingsChanged();
    }
}

QString UserSettingsManager::displayName() const
{
    if (m_role == "学生" || m_honorific.isEmpty()) {
        return m_nickname;
    }
    return m_nickname + m_honorific;
}

QString UserSettingsManager::aiUserTitle() const
{
    // 学生直接返回昵称
    if (m_role == "学生") {
        return m_nickname.isEmpty() ? "同学" : m_nickname;
    }

    // 教师：取姓 + 称呼，如 "周老师"
    if (m_nickname.isEmpty()) {
        return m_honorific.isEmpty() ? "您" : m_honorific;
    }

    QString surname = m_nickname.left(1);

    if (m_honorific.isEmpty()) {
        return surname;
    }

    return surname + m_honorific;
}

void UserSettingsManager::save()
{
    m_settings->setValue(scopedKey("user/nickname"), m_nickname);
    m_settings->setValue(scopedKey("user/department"), m_department);
    m_settings->setValue(scopedKey("user/title"), m_title);
    m_settings->setValue(scopedKey("user/honorific"), m_honorific);
    m_settings->setValue(scopedKey("user/email"), m_email);
    m_settings->setValue(scopedKey("user/role"), m_role);
    m_settings->setValue(scopedKey("ai/greetingTemplate"), m_aiGreetingTemplate);

    m_settings->setValue("user/email", m_email);
    m_settings->setValue("user/role", m_role);
    m_settings->sync();
    qDebug() << "[UserSettingsManager] Settings saved";
}

void UserSettingsManager::load()
{
    m_email = m_settings->value("user/email", "").toString();
    if (!m_email.isEmpty()) {
        loadFromCurrentScope();
        qDebug() << "[UserSettingsManager] Settings loaded - nickname:" << m_nickname
                 << "role:" << m_role << "department:" << m_department;
        return;
    }

    m_nickname = m_settings->value("user/nickname", "").toString();
    m_role = m_settings->value("user/role", "学生").toString();
    m_department = m_settings->value("user/department",
        m_role == "学生" ? "" : "思政教研组").toString();
    m_title = m_settings->value("user/title", "").toString();
    m_honorific = m_settings->value("user/honorific",
        m_role == "学生" ? "" : "老师").toString();
    m_aiGreetingTemplate = m_settings->value(
        "ai/greetingTemplate",
        "{honorific}您好！我是智慧课堂助手，请问有什么可以帮您？"
    ).toString();

    qDebug() << "[UserSettingsManager] Settings loaded - nickname:" << m_nickname
             << "role:" << m_role << "department:" << m_department;
}

QString UserSettingsManager::scopedKey(const QString &key) const
{
    if (m_email.isEmpty()) {
        return key;
    }

    const QString accountKey = QString::fromUtf8(QUrl::toPercentEncoding(m_email));
    return QString("accounts/%1/%2").arg(accountKey, key);
}

void UserSettingsManager::loadFromCurrentScope()
{
    const QString role = m_settings->value(scopedKey("user/role"), "学生").toString();
    m_role = role;
    m_nickname = m_settings->value(scopedKey("user/nickname"), "").toString();
    m_department = m_settings->value(scopedKey("user/department"),
        m_role == "学生" ? "" : "思政教研组").toString();
    m_title = m_settings->value(scopedKey("user/title"), "").toString();
    m_honorific = m_settings->value(scopedKey("user/honorific"),
        m_role == "学生" ? "" : "老师").toString();
    m_aiGreetingTemplate = m_settings->value(scopedKey("ai/greetingTemplate"),
        "{honorific}您好！我是智慧课堂助手，请问有什么可以帮您？").toString();
}

void UserSettingsManager::resetToDefaults()
{
    m_nickname = "";
    m_department = (m_role == "学生") ? "" : "思政教研组";
    m_title = "";
    m_honorific = (m_role == "学生") ? "" : "老师";
    m_aiGreetingTemplate = "{honorific}您好！我是智慧课堂助手，请问有什么可以帮您？";
    emit settingsChanged();
}
