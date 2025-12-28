#pragma once
#ifndef READERLISTDIALOG_H
#define READERLISTDIALOG_H

#include <QDialog>

class QTableWidget;
class UserManager;
class LibraryManager;

class ReaderListDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReaderListDialog(UserManager* users,
        LibraryManager* library,
        QWidget* parent = nullptr);

private:
    void initUI();
    void loadData();

private:
    UserManager* users_ = nullptr;
    LibraryManager* library_ = nullptr;

    QTableWidget* table_ = nullptr;
};

#endif // READERLISTDIALOG_H