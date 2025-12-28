#pragma once
#ifndef BOOK_H
#define BOOK_H

#include <string>

class Book {
public:
    Book() = default;
    Book(std::string isbn, std::string title, std::string author,
        std::string publisher, int publishYear, std::string category, int stock)
        : isbn_(std::move(isbn)), title_(std::move(title)), author_(std::move(author)),
        publisher_(std::move(publisher)), publishYear_(publishYear),
        category_(std::move(category)), stock_(stock) {
    }

    const std::string& isbn() const { return isbn_; }
    const std::string& title() const { return title_; }
    const std::string& author() const { return author_; }
    const std::string& publisher() const { return publisher_; }
    int publishYear() const { return publishYear_; }
    const std::string& category() const { return category_; }
    int stock() const { return stock_; }

    void setTitle(const std::string& v) { title_ = v; }
    void setAuthor(const std::string& v) { author_ = v; }
    void setPublisher(const std::string& v) { publisher_ = v; }
    void setPublishYear(int v) { publishYear_ = v; }
    void setCategory(const std::string& v) { category_ = v; }
    void setStock(int v) { stock_ = v; }

private:
    std::string isbn_;
    std::string title_;
    std::string author_;
    std::string publisher_;
    int publishYear_ = 0;
    std::string category_;
    int stock_ = 0;
};

#endif