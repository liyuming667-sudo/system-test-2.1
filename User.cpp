#include "User.h"

User::User(const std::string& username,
    const std::string& password,
    const std::string& name,
    const std::string& contact,
    int maxBorrow,
    UserRole role,
    bool banned)
    : m_username(username),
    m_password(password),
    m_name(name),
    m_contact(contact),
    m_maxBorrow(maxBorrow),
    m_role(role),
    m_banned(banned)
{
}

// ===== getters =====
const std::string& User::username() const { return m_username; }
const std::string& User::password() const { return m_password; }
const std::string& User::name() const { return m_name; }
const std::string& User::contact() const { return m_contact; }
int User::maxBorrow() const { return m_maxBorrow; }

UserRole User::role() const { return m_role; }
bool User::banned() const { return m_banned; }

// ===== 权限判断 =====
bool User::isAdmin() const {
    return m_role == UserRole::Admin || m_role == UserRole::SuperAdmin;
}

bool User::isSuperAdmin() const {
    return m_role == UserRole::SuperAdmin;
}

// ===== setters =====
void User::setName(const std::string& name) {
    m_name = name;
}

void User::setContact(const std::string& contact) {
    m_contact = contact;
}

void User::setMaxBorrow(int n) {
    m_maxBorrow = n;
}

void User::setRole(UserRole role) {
    m_role = role;
}

void User::setBanned(bool banned) {
    m_banned = banned;
}
