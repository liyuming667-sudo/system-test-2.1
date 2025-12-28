#pragma once
#ifndef BOOKLISTDIALOG_H
#define BOOKLISTDIALOG_H

#include <QDialog>

class QTableWidget;
class LibraryManager;

class BookListDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookListDialog(LibraryManager* lib, QWidget* parent = nullptr);

private:
    void initUI();
    void loadData();

private:
    LibraryManager* lib_ = nullptr;
    QTableWidget* table_ = nullptr;
};

#endif // BOOKLISTDIALOG_H