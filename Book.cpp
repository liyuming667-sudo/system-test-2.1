#include "Book.h"

Book::Book(const std::string& isbn,
    const std::string& title,
    const std::string& author,
    const std::string& publisher,
    int publishYear,
    const std::string& category,
    int stock)
    : isbn_(isbn),
    title_(title),
    author_(author),
    publisher_(publisher),
    publishYear_(publishYear),
    category_(category),
    stock_(stock)
{
}

const std::string& Book::isbn() const {
    return isbn_;
}

const std::string& Book::title() const {
    return title_;
}

const std::string& Book::author() const {
    return author_;
}

const std::string& Book::publisher() const {
    return publisher_;
}

int Book::publishYear() const {
    return publishYear_;
}

const std::string& Book::category() const {
    return category_;
}

int Book::stock() const {
    return stock_;
}

bool Book::canBorrow() const {
    return stock_ > 0;
}

bool Book::borrowOne() {
    if (stock_ <= 0) return false;
    --stock_;
    return true;
}

void Book::returnOne() {
    ++stock_;
}