#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "MainWindow.h"
#include "ReaderListDialog.h"
#include "BookListDialog.h"
#include "UserManager.h"
#include "LibraryManager.h"
#include "Book.h"
#include "BorrowRecord.h"

#include <functional>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>

MainWindow::MainWindow(UserManager* users,
    LibraryManager* library,
    const QString& me,
    QWidget* parent)
    : QMainWindow(parent), users_(users), lib_(library), me_(me)
{
    setWindowTitle("图书管理系统");
    resize(800, 600);

    central_ = new QWidget(this);
    layout_ = new QVBoxLayout(central_);
    setCentralWidget(central_);

    rebuildUI();
}

bool MainWindow::isAdmin() const {
    auto u = users_->findUser(me_.toStdString());
    return u.has_value() && u->isAdmin();
}

bool MainWindow::isSuperAdmin() const {
    auto u = users_->findUser(me_.toStdString());
    return u.has_value() && u->isSuperAdmin();
}

void MainWindow::addBtn(const QString& text, const std::function<void()>& fn) {
    auto* b = new QPushButton(text, this);
    // 【关键】移除所有 setMinimumHeight 或 setStyleSheet
    layout_->addWidget(b);
    connect(b, &QPushButton::clicked, this, [fn]() { fn(); });
}

void MainWindow::rebuildUI() {
    while (QLayoutItem* item = layout_->takeAt(0)) {
        if (QWidget* w = item->widget()) w->deleteLater();
        delete item;
    }

    QString roleStr = "读者";
    if (isSuperAdmin()) roleStr = "最高管理员";
    else if (isAdmin()) roleStr = "管理员";

    auto* info = new QLabel(QString("当前用户: %1  身份: %2").arg(me_).arg(roleStr), this);
    layout_->addWidget(info);

    if (page_ == Page::Home) {
        addBtn("图书管理", [this]() { goBooks(); });
        addBtn("借阅管理", [this]() { goBorrows(); });
        if (isAdmin()) addBtn("账号管理", [this]() { goReaders(); });
        addBtn("退出登录", [this]() { onLogout(); });
        return;
    }

    if (page_ == Page::Books) {
        if (isAdmin()) {
            addBtn("添加图书", [this]() { addBookUI(); });
            addBtn("删除图书", [this]() { deleteBookUI(); });
            addBtn("修改图书", [this]() { modifyBookUI(); });
        }
        addBtn("查询图书", [this]() { searchBookUI(); });
        addBtn("显示全部图书", [this]() { showAllBooksUI(); });
        addBtn("返回", [this]() { goHome(); });
        return;
    }

    if (page_ == Page::Readers) {
        if (!isAdmin()) { page_ = Page::Home; rebuildUI(); return; }
        addBtn("所有账号列表", [this]() { listUsersUI(); });
        addBtn("修改信息", [this]() { editUserUI(); });
        addBtn("封禁账号", [this]() { banUserUI(); });
        addBtn("解封账号", [this]() { unbanUserUI(); });
        addBtn("封禁名单", [this]() { listBannedUI(); });
        if (isSuperAdmin()) {
            addBtn("设为管理员", [this]() { promoteToAdminUI(); });
            addBtn("撤销管理员", [this]() { demoteToReaderUI(); });
        }
        addBtn("返回", [this]() { goHome(); });
        return;
    }

    addBtn("借书", [this]() { borrowBookUI(); });
    addBtn("还书", [this]() { returnBookUI(); });
    addBtn("我的记录", [this]() { myRecordsUI(); });
    if (isAdmin()) addBtn("所有记录", [this]() { allRecordsUI(); });
    if (!isAdmin()) addBtn("注销账号", [this]() { deleteSelfUI(); });
    addBtn("返回", [this]() { goHome(); });
}

// ... 保持原有的逻辑函数实现不变 ...
// (为了篇幅，这里假设你保留了 goHome, addBookUI 等逻辑函数的原始实现)
// 只要确保 addBtn 里面没有 setStyleSheet 即可。

void MainWindow::goHome() { page_ = Page::Home; rebuildUI(); }
void MainWindow::goBooks() { page_ = Page::Books; rebuildUI(); }
void MainWindow::goReaders() { page_ = Page::Readers; rebuildUI(); }
void MainWindow::goBorrows() { page_ = Page::Borrows; rebuildUI(); }
void MainWindow::onLogout() { emit logoutRequested(); }

QString MainWindow::booksToText(const std::vector<Book>& v) const {
    QString s;
    for (const auto& b : v) s += QString("ISBN: %1 书名: %2\n").arg(QString::fromStdString(b.isbn())).arg(QString::fromStdString(b.title()));
    return s.isEmpty() ? "无数据" : s;
}
QString MainWindow::recordsToText(const std::vector<BorrowRecord>& v) const {
    QString s;
    for (const auto& r : v) s += QString("ID: %1 书号: %2\n").arg(QString::fromStdString(r.id())).arg(QString::fromStdString(r.isbn()));
    return s.isEmpty() ? "无数据" : s;
}
QString MainWindow::usersToText(const std::vector<User>& v) const {
    QString s;
    for (const auto& u : v) s += QString("用户: %1\n").arg(QString::fromStdString(u.username()));
    return s.isEmpty() ? "无数据" : s;
}

void MainWindow::addBookUI() {
    if(!isAdmin()) return;
    bool ok;
    QString isbn = QInputDialog::getText(this, "添加", "ISBN:", QLineEdit::Normal, "", &ok);
    if(ok && !isbn.isEmpty()) {
        QString title = QInputDialog::getText(this, "添加", "书名:", QLineEdit::Normal, "", &ok);
        lib_->addBook(Book(isbn.toStdString(), title.toStdString(), "未知", "未知", 2024, "通用", 10));
        QMessageBox::information(this, "成功", "已添加");
    }
}
void MainWindow::deleteBookUI() {
    if(!isAdmin()) return;
    bool ok;
    QString isbn = QInputDialog::getText(this, "删除", "ISBN:", QLineEdit::Normal, "", &ok);
    if(ok && !isbn.isEmpty()) {
        if(lib_->deleteBookByISBN(isbn.toStdString())) QMessageBox::information(this, "成功", "已删除");
        else QMessageBox::warning(this, "失败", "删除失败");
    }
}
void MainWindow::modifyBookUI() { 
    if(!isAdmin()) return;
    QMessageBox::information(this,"提示","请完善修改逻辑"); 
}
void MainWindow::searchBookUI() {
    bool ok;
    QString key = QInputDialog::getText(this, "查询", "书名关键字:", QLineEdit::Normal, "", &ok);
    if(ok) {
        auto res = lib_->searchBooks(key.toStdString(), "", "");
        QMessageBox::information(this, "结果", booksToText(res));
    }
}
void MainWindow::showAllBooksUI() {
    BookListDialog dlg(lib_, this);
    dlg.exec();
}
void MainWindow::listUsersUI() {
    ReaderListDialog dlg(users_, lib_, this);
    dlg.exec();
}
void MainWindow::editUserUI() { QMessageBox::information(this,"提示","功能未实现"); }
void MainWindow::banUserUI() { QMessageBox::information(this,"提示","功能未实现"); }
void MainWindow::unbanUserUI() { QMessageBox::information(this,"提示","功能未实现"); }
void MainWindow::listBannedUI() { QMessageBox::information(this,"提示","功能未实现"); }
void MainWindow::promoteToAdminUI() { QMessageBox::information(this,"提示","功能未实现"); }
void MainWindow::demoteToReaderUI() { QMessageBox::information(this,"提示","功能未实现"); }

void MainWindow::borrowBookUI() {
    bool ok;
    QString isbn = QInputDialog::getText(this, "借书", "ISBN:", QLineEdit::Normal, "", &ok);
    std::string id;
    if(ok && !isbn.isEmpty()) {
        if(lib_->borrowBook(me_.toStdString(), isbn.toStdString(), id)) 
            QMessageBox::information(this, "成功", "借阅成功");
        else 
            QMessageBox::warning(this, "失败", "库存不足或书不存在");
    }
}
void MainWindow::returnBookUI() {
    bool ok;
    QString id = QInputDialog::getText(this, "还书", "记录ID:", QLineEdit::Normal, "", &ok);
    int d; bool o;
    if(ok && !id.isEmpty()) {
        if(lib_->returnBookByRecordID(id.toStdString(), d, o))
             QMessageBox::information(this, "成功", "还书成功");
        else
             QMessageBox::warning(this, "失败", "还书失败");
    }
}
void MainWindow::myRecordsUI() {
    QMessageBox::information(this, "我的记录", recordsToText(lib_->recordsByUser(me_.toStdString())));
}
void MainWindow::allRecordsUI() {
    QMessageBox::information(this, "所有记录", recordsToText(lib_->getAllRecords()));
}
void MainWindow::deleteSelfUI() {
    std::string msg;
    if(users_->deleteSelf(me_.toStdString(), *lib_, msg)) emit logoutRequested();
    else QMessageBox::warning(this, "失败", QString::fromStdString(msg));
}
