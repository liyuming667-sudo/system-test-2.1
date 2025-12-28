#pragma once
#ifndef USER_H
#define USER_H

#include <string>

enum class UserRole {
    Reader,       // 读者
    Admin,        // 管理员
    SuperAdmin    // 最高管理员
};

class User {
public:
    User(const std::string& username,
        const std::string& password,
        const std::string& name,
        const std::string& contact,
        int maxBorrow,
        UserRole role,
        bool banned);

    // ===== 基本信息 =====
    const std::string& username() const;
    const std::string& password() const;
    const std::string& name() const;
    const std::string& contact() const;
    int maxBorrow() const;

    // ===== 角色 / 状态 =====
    UserRole role() const;
    bool banned() const;

    bool isAdmin() const;        // Admin 或 SuperAdmin
    bool isSuperAdmin() const;   // 仅 SuperAdmin

    // ===== 修改 =====
    void setName(const std::string& name);
    void setContact(const std::string& contact);
    void setMaxBorrow(int n);
    void setRole(UserRole role);
    void setBanned(bool banned);

private:
    std::string m_username;
    std::string m_password;
    std::string m_name;
    std::string m_contact;
    int m_maxBorrow;
    UserRole m_role;
    bool m_banned;
};

#endif // USER_H
