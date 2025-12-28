#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "UserManager.h"
#include "LibraryManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

// ===== 构造：启动时自动加载 =====
UserManager::UserManager() {
    loadData();

    // 如果加载后发现没有用户（比如第一次运行），创建一个默认最高管理员
    if (users_.empty()) {
        users_.emplace_back(
            "admin",          // 账号
            "admin123",       // 密码
            "系统管理员",     // 姓名
            "系统内置",       // 联系方式
            999,              // 最大借书数
            UserRole::SuperAdmin,
            false             // 未封禁
        );
        saveData(); // 保存这个默认用户
    }
}

// ===== 持久化实现 =====
void UserManager::loadData() {
    QFile file(QString::fromStdString(fileName_));
    if (!file.open(QIODevice::ReadOnly)) {
        return; // 文件不存在，跳过
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;

    users_.clear();
    QJsonArray arr = doc.array();
    for (auto val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        users_.emplace_back(
            obj["username"].toString().toStdString(),
            obj["password"].toString().toStdString(),
            obj["name"].toString().toStdString(),
            obj["contact"].toString().toStdString(),
            obj["maxBorrow"].toInt(3),
            static_cast<UserRole>(obj["role"].toInt(0)),
            obj["banned"].toBool(false)
        );
    }
}

void UserManager::saveData() {
    QJsonArray arr;
    for (const auto& u : users_) {
        QJsonObject obj;
        obj["username"] = QString::fromStdString(u.username());
        obj["password"] = QString::fromStdString(u.password());
        obj["name"] = QString::fromStdString(u.name());
        obj["contact"] = QString::fromStdString(u.contact());
        obj["maxBorrow"] = u.maxBorrow();
        obj["role"] = static_cast<int>(u.role());
        obj["banned"] = u.banned();
        arr.append(obj);
    }

    QFile file(QString::fromStdString(fileName_));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
        file.close();
    }
}

// ===== 内部查找（可修改）=====
User* UserManager::findUserMutable(const std::string& username) {
    for (auto& u : users_) {
        if (u.username() == username)
            return &u;
    }
    return nullptr;
}

// ===== 只读查找 =====
std::optional<User> UserManager::findUser(const std::string& username) const {
    for (const auto& u : users_) {
        if (u.username() == username)
            return u;
    }
    return std::nullopt;
}

// ===== 注册读者 =====
bool UserManager::registerReader(const std::string& username,
    const std::string& password,
    const std::string& name,
    const std::string& contact,
    std::string& outMsg)
{
    if (username.empty() || password.empty() ||
        name.empty() || contact.empty()) {
        outMsg = "注册信息不完整。";
        return false;
    }

    if (findUser(username).has_value()) {
        outMsg = "该账号已存在。";
        return false;
    }

    users_.emplace_back(
        username,
        password,
        name,
        contact,
        3,                  // 默认最大借书数
        UserRole::Reader,   // 默认读者
        false               // 未封禁
    );

    saveData(); // ★ 保存
    outMsg = "注册成功。";
    return true;
}

// ===== 登录 =====
std::optional<User> UserManager::login(const std::string& username,
    const std::string& password,
    std::string& outMsg) const
{
    for (const auto& u : users_) {
        if (u.username() != username)
            continue;

        if (u.password() != password) {
            outMsg = "密码错误。";
            return std::nullopt;
        }

        if (u.banned()) {
            outMsg = "该账号已被封禁，无法登录。";
            return std::nullopt;
        }

        outMsg = "登录成功。";
        return u;
    }

    outMsg = "账号不存在。";
    return std::nullopt;
}

// ===== 查询 =====
std::vector<User> UserManager::allUsers() const {
    return users_;
}

std::vector<User> UserManager::bannedUsers() const {
    std::vector<User> res;
    for (const auto& u : users_) {
        if (u.banned())
            res.push_back(u);
    }
    return res;
}

// ===== 封禁 / 解封 =====
bool UserManager::setBanned(const std::string& target, bool banned) {
    User* u = findUserMutable(target);
    if (!u) return false;
    if (u->isSuperAdmin()) return false;

    u->setBanned(banned);
    saveData(); // ★ 保存
    return true;
}

// ===== 管理员修改读者信息 =====
bool UserManager::updateUserInfoByAdmin(const std::string& target,
    const std::string& name,
    const std::string& contact,
    int maxBorrow)
{
    User* u = findUserMutable(target);
    if (!u) return false;
    if (u->isSuperAdmin()) return false;
    if (maxBorrow < 1) return false;

    u->setName(name);
    u->setContact(contact);
    u->setMaxBorrow(maxBorrow);
    saveData(); // ★ 保存
    return true;
}

// ===== 超级管理员修改角色 =====
bool UserManager::setRoleBySuperAdmin(const std::string& target, UserRole role) {
    User* u = findUserMutable(target);
    if (!u) return false;
    if (u->isSuperAdmin()) return false;

    u->setRole(role);
    saveData(); // ★ 保存
    return true;
}

// ===== 读者注销（本人）=====
bool UserManager::deleteSelf(const std::string& selfUsername,
    const LibraryManager& lib,
    std::string& outMsg)
{
    if (lib.hasUnreturned(selfUsername)) {
        outMsg = "仍有未归还的书籍，无法注销账号。";
        return false;
    }

    for (auto it = users_.begin(); it != users_.end(); ++it) {
        if (it->username() == selfUsername) {
            if (it->isSuperAdmin()) {
                outMsg = "最高管理员账号不可注销。";
                return false;
            }
            users_.erase(it);
            saveData(); // ★ 保存
            outMsg = "账号已成功注销。";
            return true;
        }
    }

    outMsg = "账号不存在。";
    return false;
}