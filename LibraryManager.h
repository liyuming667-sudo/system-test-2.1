#pragma once
#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <vector>
#include <string>

#include "Book.h"
#include "BorrowRecord.h"

class LibraryManager {
public:
    LibraryManager(); // 构造函数需要改为非默认，以便调用 loadData

    // ===== 图书管理（管理员）=====
    bool addBook(const Book& book);
    bool deleteBookByISBN(const std::string& isbn);
    bool modifyBookByISBN(const std::string& isbn, const Book& newBook);

    // ===== 图书查询（所有人）=====
    std::vector<Book> searchBooks(const std::string& titleKey,
        const std::string& authorKey,
        const std::string& categoryKey) const;

    std::vector<Book> getAllBooks() const;
    std::vector<BorrowRecord> getAllRecords() const;

    // ===== 借阅管理 =====
    bool borrowBook(const std::string& username,
        const std::string& isbn,
        std::string& outRecordID);

    bool returnBookByRecordID(const std::string& recordID,
        int& outDays,
        bool& outOverdue);

    // ===== 查询借阅记录 =====
    std::vector<BorrowRecord> recordsByUser(const std::string& username) const;
    std::vector<BorrowRecord> recordsByISBN(const std::string& isbn) const;

    // ===== 支撑 UserManager =====
    bool hasUnreturned(const std::string& username) const;
    int currentBorrowCount(const std::string& username) const;

    // ===== 数据持久化 =====
    void loadData();
    void saveData();

private:
    std::vector<Book> books_;
    std::vector<BorrowRecord> records_;

    const std::string bookFile_ = "books.json";
    const std::string recordFile_ = "records.json";

    // 工具
    Book* findBook(const std::string& isbn);
    BorrowRecord* findRecord(const std::string& id);

    static bool containsIgnoreCase(const std::string& src,
        const std::string& key);

    std::string generateRecordID() const;
};

#endif // LIBRARYMANAGER_H