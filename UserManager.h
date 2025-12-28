#pragma once
#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <vector>
#include <string>
#include <optional>

#include "User.h"

class LibraryManager;

class UserManager {
public:
    UserManager();

    // ===== 注册 & 登录 =====
    bool registerReader(const std::string& username,
        const std::string& password,
        const std::string& name,
        const std::string& contact,
        std::string& outMsg);

    std::optional<User> login(const std::string& username,
        const std::string& password,
        std::string& outMsg) const;

    // ===== 查询 =====
    std::optional<User> findUser(const std::string& username) const;
    std::vector<User> allUsers() const;
    std::vector<User> bannedUsers() const;

    // ===== 管理员操作 =====
    bool setBanned(const std::string& target, bool banned);
    bool updateUserInfoByAdmin(const std::string& target,
        const std::string& name,
        const std::string& contact,
        int maxBorrow);

    // ===== 最高管理员 =====
    bool setRoleBySuperAdmin(const std::string& target, UserRole role);

    // ===== 读者注销（本人）=====
    bool deleteSelf(const std::string& selfUsername,
        const LibraryManager& lib,
        std::string& outMsg);

    // ===== 数据持久化 =====
    void loadData(); // 从文件加载
    void saveData(); // 保存到文件

private:
    User* findUserMutable(const std::string& username);

private:
    std::vector<User> users_;
    const std::string fileName_ = "users.json"; // 数据文件名
};

#endif // USERMANAGER_H