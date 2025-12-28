#pragma once
#ifndef BORROWRECORD_H
#define BORROWRECORD_H

#include <string>
#include <QDateTime>

class BorrowRecord {
public:
    BorrowRecord() = default;

    BorrowRecord(std::string id,
        std::string username,
        std::string isbn,
        QDateTime borrowTime)
        : id_(std::move(id)),
        username_(std::move(username)),
        isbn_(std::move(isbn)),
        borrowTime_(std::move(borrowTime)) {
    }

    const std::string& id() const { return id_; }
    const std::string& username() const { return username_; }
    const std::string& isbn() const { return isbn_; }

    const QDateTime& borrowTime() const { return borrowTime_; }

    bool returned() const { return returned_; }
    const QDateTime& returnTime() const { return returnTime_; }

    void markReturned(const QDateTime& t) {
        returned_ = true;
        returnTime_ = t;
    }

private:
    std::string id_;
    std::string username_;
    std::string isbn_;

    QDateTime borrowTime_;
    bool returned_ = false;
    QDateTime returnTime_;   
};

#endif // BORROWRECORD_H