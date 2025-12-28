#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "LibraryManager.h"

#include <algorithm>
#include <ctime>
#include <cctype>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

// ===== 构造：自动加载 =====
LibraryManager::LibraryManager() {
    loadData();
}

// ===== 工具 =====
static std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return r;
}

bool LibraryManager::containsIgnoreCase(const std::string& src,
    const std::string& key) {
    if (key.empty()) return true;
    return toLower(src).find(toLower(key)) != std::string::npos;
}

// ===== 持久化实现 =====
void LibraryManager::loadData() {
    // 1. 加载图书
    QFile bf(QString::fromStdString(bookFile_));
    if (bf.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(bf.readAll());
        bf.close();
        if (doc.isArray()) {
            books_.clear();
            for (auto v : doc.array()) {
                QJsonObject o = v.toObject();
                books_.emplace_back(
                    o["isbn"].toString().toStdString(),
                    o["title"].toString().toStdString(),
                    o["author"].toString().toStdString(),
                    o["publisher"].toString().toStdString(),
                    o["year"].toInt(),
                    o["category"].toString().toStdString(),
                    o["stock"].toInt()
                );
            }
        }
    }

    // 2. 加载记录
    QFile rf(QString::fromStdString(recordFile_));
    if (rf.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(rf.readAll());
        rf.close();
        if (doc.isArray()) {
            records_.clear();
            for (auto v : doc.array()) {
                QJsonObject o = v.toObject();
                // 构造基础记录
                BorrowRecord r(
                    o["id"].toString().toStdString(),
                    o["username"].toString().toStdString(),
                    o["isbn"].toString().toStdString(),
                    QDateTime::fromString(o["borrowTime"].toString(), Qt::ISODate)
                );
                // 如果已归还，标记归还
                if (o["returned"].toBool()) {
                    r.markReturned(QDateTime::fromString(o["returnTime"].toString(), Qt::ISODate));
                }
                records_.push_back(r);
            }
        }
    }
}

void LibraryManager::saveData() {
    // 1. 保存图书
    QJsonArray bArr;
    for (const auto& b : books_) {
        QJsonObject o;
        o["isbn"] = QString::fromStdString(b.isbn());
        o["title"] = QString::fromStdString(b.title());
        o["author"] = QString::fromStdString(b.author());
        o["publisher"] = QString::fromStdString(b.publisher());
        o["year"] = b.publishYear();
        o["category"] = QString::fromStdString(b.category());
        o["stock"] = b.stock();
        bArr.append(o);
    }
    QFile bf(QString::fromStdString(bookFile_));
    if (bf.open(QIODevice::WriteOnly)) {
        bf.write(QJsonDocument(bArr).toJson());
        bf.close();
    }

    // 2. 保存记录
    QJsonArray rArr;
    for (const auto& r : records_) {
        QJsonObject o;
        o["id"] = QString::fromStdString(r.id());
        o["username"] = QString::fromStdString(r.username());
        o["isbn"] = QString::fromStdString(r.isbn());
        o["borrowTime"] = r.borrowTime().toString(Qt::ISODate);
        o["returned"] = r.returned();
        if (r.returned()) {
            o["returnTime"] = r.returnTime().toString(Qt::ISODate);
        }
        rArr.append(o);
    }
    QFile rf(QString::fromStdString(recordFile_));
    if (rf.open(QIODevice::WriteOnly)) {
        rf.write(QJsonDocument(rArr).toJson());
        rf.close();
    }
}

// ===== 图书管理 =====
bool LibraryManager::addBook(const Book& book) {
    if (findBook(book.isbn())) return false;
    books_.push_back(book);
    saveData(); // ★ 保存
    return true;
}

bool LibraryManager::deleteBookByISBN(const std::string& isbn) {
    // 有未归还记录不能删
    for (const auto& r : records_) {
        if (r.isbn() == isbn && !r.returned())
            return false;
    }

    auto it = std::remove_if(books_.begin(), books_.end(),
        [&](const Book& b) { return b.isbn() == isbn; });
    if (it == books_.end()) return false;

    books_.erase(it, books_.end());
    saveData(); // ★ 保存
    return true;
}

bool LibraryManager::modifyBookByISBN(const std::string& isbn,
    const Book& newBook) {
    Book* b = findBook(isbn);
    if (!b) return false;
    *b = newBook;
    saveData(); // ★ 保存
    return true;
}

// ===== 图书查询 =====
std::vector<Book> LibraryManager::searchBooks(const std::string& titleKey,
    const std::string& authorKey,
    const std::string& categoryKey) const {
    std::vector<Book> res;
    for (const auto& b : books_) {
        if (containsIgnoreCase(b.title(), titleKey) &&
            containsIgnoreCase(b.author(), authorKey) &&
            containsIgnoreCase(b.category(), categoryKey)) {
            res.push_back(b);
        }
    }
    return res;
}

std::vector<Book> LibraryManager::getAllBooks() const {
    return books_;
}

std::vector<BorrowRecord> LibraryManager::getAllRecords() const {
    return records_;
}

// ===== 借阅 =====
bool LibraryManager::borrowBook(const std::string& username,
    const std::string& isbn,
    std::string& outRecordID) {
    Book* b = findBook(isbn);
    if (!b || b->stock() <= 0) return false;

    b->setStock(b->stock() - 1);

    std::string id = generateRecordID();
    records_.emplace_back(id, username, isbn, QDateTime::currentDateTime());
    outRecordID = id;

    saveData(); // ★ 保存（同时更新了库存和记录）
    return true;
}

bool LibraryManager::returnBookByRecordID(const std::string& recordID,
    int& outDays,
    bool& outOverdue) {
    BorrowRecord* r = findRecord(recordID);
    if (!r || r->returned()) return false;

    r->markReturned(QDateTime::currentDateTime());

    Book* b = findBook(r->isbn());
    if (b) b->setStock(b->stock() + 1);

    qint64 secs = r->borrowTime().secsTo(r->returnTime());
    outDays = static_cast<int>(secs / (24 * 3600));
    outOverdue = outDays > 30;

    saveData(); // ★ 保存
    return true;
}

// ===== 查询 =====
std::vector<BorrowRecord> LibraryManager::recordsByUser(const std::string& username) const {
    std::vector<BorrowRecord> res;
    for (const auto& r : records_) {
        if (r.username() == username)
            res.push_back(r);
    }
    return res;
}

std::vector<BorrowRecord> LibraryManager::recordsByISBN(const std::string& isbn) const {
    std::vector<BorrowRecord> res;
    for (const auto& r : records_) {
        if (r.isbn() == isbn)
            res.push_back(r);
    }
    return res;
}

// ===== 支撑 UserManager =====
bool LibraryManager::hasUnreturned(const std::string& username) const {
    for (const auto& r : records_) {
        if (r.username() == username && !r.returned())
            return true;
    }
    return false;
}

int LibraryManager::currentBorrowCount(const std::string& username) const {
    int cnt = 0;
    for (const auto& r : records_) {
        if (r.username() == username && !r.returned())
            ++cnt;
    }
    return cnt;
}

// ===== 内部查找 =====
Book* LibraryManager::findBook(const std::string& isbn) {
    for (auto& b : books_) {
        if (b.isbn() == isbn)
            return &b;
    }
    return nullptr;
}

BorrowRecord* LibraryManager::findRecord(const std::string& id) {
    for (auto& r : records_) {
        if (r.id() == id)
            return &r;
    }
    return nullptr;
}

std::string LibraryManager::generateRecordID() const {
    return "R" + std::to_string(std::time(nullptr)) +
        "_" + std::to_string(records_.size() + 1);
}