#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "MainWindow.h"
#include "ReaderListDialog.h"

#include <functional>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>

#include "UserManager.h"
#include "LibraryManager.h"
#include "Book.h"
#include "BorrowRecord.h"

MainWindow::MainWindow(UserManager* users,
    LibraryManager* library,
    const QString& me,
    QWidget* parent)
    : QMainWindow(parent), users_(users), lib_(library), me_(me)
{
    setWindowTitle("图书管理系统");
    resize(760, 540);

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
    b->setMinimumHeight(40);
    layout_->addWidget(b);
    connect(b, &QPushButton::clicked, this, [fn]() { fn(); });
}

void MainWindow::rebuildUI() {
    // 清空布局
    while (QLayoutItem* item = layout_->takeAt(0)) {
        if (QWidget* w = item->widget()) w->deleteLater();
        delete item;
    }

    // ===== 顶部登录状态 =====
    QString roleStr = "读者";
    if (isSuperAdmin()) roleStr = "最高管理员";
    else if (isAdmin()) roleStr = "管理员";

    auto* info = new QLabel(QString("当前登录：%1    身份：%2").arg(me_).arg(roleStr), this);
    info->setStyleSheet("font-weight:600;font-size:14px;");
    layout_->addWidget(info);

    // ===== 首页 =====
    if (page_ == Page::Home) {
        addBtn("图书管理", [this]() { goBooks(); });
        addBtn("借阅管理", [this]() { goBorrows(); });

        if (isAdmin()) {
            addBtn("读者/账号管理（管理员）", [this]() { goReaders(); });
        }

        addBtn("退出登录（返回欢迎界面）", [this]() { onLogout(); });
        return;
    }

    // ===== 图书模块 =====
    if (page_ == Page::Books) {
        if (isAdmin()) {
            addBtn("添加图书（管理员）", [this]() { addBookUI(); });
            addBtn("删除图书（管理员）", [this]() { deleteBookUI(); });
            addBtn("修改图书信息（管理员）", [this]() { modifyBookUI(); });
        }
        addBtn("查询图书（多条件：书名/作者/类别）", [this]() { searchBookUI(); });
        addBtn("显示全部图书", [this]() { showAllBooksUI(); });
        addBtn("返回", [this]() { goHome(); });
        return;
    }

    // ===== 读者/账号管理 =====
    if (page_ == Page::Readers) {
        if (!isAdmin()) {
            QMessageBox::warning(this, "无权限", "只有管理员可以进入该模块。");
            page_ = Page::Home;
            rebuildUI();
            return;
        }

        addBtn("查看全部账号（文本）", [this]() { listUsersUI(); });
        addBtn("查看读者清单（表格）", [this]() { showReaderTableUI(); });

        addBtn("修改读者信息（姓名/联系方式/最大借书数）", [this]() { editUserUI(); });

        addBtn("封禁账号", [this]() { banUserUI(); });
        addBtn("解封账号", [this]() { unbanUserUI(); });
        addBtn("查看封禁名单", [this]() { listBannedUI(); });

        if (isSuperAdmin()) {
            addBtn("授予管理员权限（最高管理员）", [this]() { promoteToAdminUI(); });
            addBtn("撤销管理员权限（最高管理员）", [this]() { demoteToReaderUI(); });
        }

        addBtn("返回", [this]() { goHome(); });
        return;
    }

    // ===== 借阅模块 =====
    addBtn("借书", [this]() { borrowBookUI(); });
    addBtn("还书", [this]() { returnBookUI(); });
    addBtn("查看我的借阅记录", [this]() { myRecordsUI(); });

    if (isAdmin()) {
        addBtn("查看全部借阅记录（管理员）", [this]() { allRecordsUI(); });
    }

    // 读者注销
    if (!isAdmin()) {
        addBtn("注销我的账号（仅读者本人）", [this]() { deleteSelfUI(); });
    }

    addBtn("返回", [this]() { goHome(); });
}

void MainWindow::goHome() { page_ = Page::Home; rebuildUI(); }
void MainWindow::goBooks() { page_ = Page::Books; rebuildUI(); }
void MainWindow::goReaders() { page_ = Page::Readers; rebuildUI(); }
void MainWindow::goBorrows() { page_ = Page::Borrows; rebuildUI(); }

void MainWindow::onLogout() {
    emit logoutRequested();
}

// ===== 文本拼接 =====
QString MainWindow::booksToText(const std::vector<Book>& v) const {
    if (v.empty()) return "暂无图书。";
    QString s;
    for (const auto& b : v) {
        s += QString("ISBN=%1 | 书名=%2 | 作者=%3 | 出版社=%4 | 年份=%5 | 类别=%6 | 库存=%7\n")
            .arg(QString::fromStdString(b.isbn()))
            .arg(QString::fromStdString(b.title()))
            .arg(QString::fromStdString(b.author()))
            .arg(QString::fromStdString(b.publisher()))
            .arg(b.publishYear())
            .arg(QString::fromStdString(b.category()))
            .arg(b.stock());
    }
    return s;
}

QString MainWindow::recordsToText(const std::vector<BorrowRecord>& v) const {
    if (v.empty()) return "暂无借阅记录。";
    QString s;
    for (const auto& r : v) {
        s += QString("记录ID=%1 | 用户=%2 | ISBN=%3 | 借阅时间=%4 | 是否归还=%5\n")
            .arg(QString::fromStdString(r.id()))
            .arg(QString::fromStdString(r.username()))
            .arg(QString::fromStdString(r.isbn()))
            .arg(r.borrowTime().toString("yyyy-MM-dd HH:mm:ss"))
            .arg(r.returned() ? "是" : "否");
    }
    return s;
}

QString MainWindow::usersToText(const std::vector<User>& v) const {
    if (v.empty()) return "暂无账号。";
    QString s;
    for (const auto& u : v) {
        QString role = "读者";
        if (u.isSuperAdmin()) role = "最高管理员";
        else if (u.isAdmin()) role = "管理员";

        s += QString("账号=%1 | 身份=%2 | 封禁=%3 | 姓名=%4 | 联系方式=%5 | 最大借书=%6\n")
            .arg(QString::fromStdString(u.username()))
            .arg(role)
            .arg(u.banned() ? "是" : "否")
            .arg(QString::fromStdString(u.name()))
            .arg(QString::fromStdString(u.contact()))
            .arg(u.maxBorrow());
    }
    return s;
}

// ================= 图书模块 =================
void MainWindow::addBookUI() {
    if (!isAdmin()) { QMessageBox::warning(this, "无权限", "只有管理员可以添加图书。"); return; }

    bool ok = false;
    QString isbn = QInputDialog::getText(this, "添加图书", "请输入ISBN：", QLineEdit::Normal, "", &ok);
    if (!ok || isbn.isEmpty()) return;

    QString title = QInputDialog::getText(this, "添加图书", "请输入书名：", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;

    QString author = QInputDialog::getText(this, "添加图书", "请输入作者：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString publisher = QInputDialog::getText(this, "添加图书", "请输入出版社：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    int year = QInputDialog::getInt(this, "添加图书", "请输入出版年份：", 2024, 0, 3000, 1, &ok);
    if (!ok) return;

    QString category = QInputDialog::getText(this, "添加图书", "请输入类别：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    int stock = QInputDialog::getInt(this, "添加图书", "请输入库存数量：", 1, 0, 1000000, 1, &ok);
    if (!ok) return;

    Book b(isbn.toStdString(), title.toStdString(), author.toStdString(),
        publisher.toStdString(), year, category.toStdString(), stock);

    if (lib_->addBook(b)) QMessageBox::information(this, "成功", "添加图书成功。");
    else QMessageBox::warning(this, "失败", "添加失败：ISBN已存在或信息不合法。");
}

void MainWindow::deleteBookUI() {
    if (!isAdmin()) { QMessageBox::warning(this, "无权限", "只有管理员可以删除图书。"); return; }

    bool ok = false;
    QString isbn = QInputDialog::getText(this, "删除图书", "请输入要删除的ISBN：", QLineEdit::Normal, "", &ok);
    if (!ok || isbn.isEmpty()) return;

    if (lib_->deleteBookByISBN(isbn.toStdString()))
        QMessageBox::information(this, "成功", "删除图书成功。");
    else
        QMessageBox::warning(this, "失败", "删除失败：图书不存在或仍有未归还记录。");
}

void MainWindow::modifyBookUI() {
    if (!isAdmin()) { QMessageBox::warning(this, "无权限", "只有管理员可以修改图书信息。"); return; }

    bool ok = false;
    QString isbn = QInputDialog::getText(this, "修改图书", "请输入要修改的ISBN：", QLineEdit::Normal, "", &ok);
    if (!ok || isbn.isEmpty()) return;

    QString title = QInputDialog::getText(this, "修改图书", "新书名：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString author = QInputDialog::getText(this, "修改图书", "新作者：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString publisher = QInputDialog::getText(this, "修改图书", "新出版社：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    int year = QInputDialog::getInt(this, "修改图书", "新出版年份：", 2024, 0, 3000, 1, &ok);
    if (!ok) return;

    QString category = QInputDialog::getText(this, "修改图书", "新类别：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    int stock = QInputDialog::getInt(this, "修改图书", "新库存数量：", 1, 0, 1000000, 1, &ok);
    if (!ok) return;

    Book nb(isbn.toStdString(), title.toStdString(), author.toStdString(),
        publisher.toStdString(), year, category.toStdString(), stock);

    if (lib_->modifyBookByISBN(isbn.toStdString(), nb))
        QMessageBox::information(this, "成功", "修改图书信息成功。");
    else
        QMessageBox::warning(this, "失败", "修改失败：图书不存在或ISBN不一致。");
}

void MainWindow::searchBookUI() {
    bool ok = false;
    QString titleKey = QInputDialog::getText(this, "查询图书", "书名关键词（可空）：", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    QString authorKey = QInputDialog::getText(this, "查询图书", "作者关键词（可空）：", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    QString cateKey = QInputDialog::getText(this, "查询图书", "类别关键词（可空）：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    auto res = lib_->searchBooks(titleKey.toStdString(), authorKey.toStdString(), cateKey.toStdString());
    QMessageBox::information(this, "查询结果", booksToText(res));
}

void MainWindow::showAllBooksUI() {
    QMessageBox::information(this, "全部图书", booksToText(lib_->getAllBooks()));
}

// ================= 账号管理（管理员）=================
void MainWindow::listUsersUI() {
    QMessageBox::information(this, "全部账号", usersToText(users_->allUsers()));
}

void MainWindow::editUserUI() {
    bool ok = false;
    QString u = QInputDialog::getText(this, "修改读者信息", "请输入目标账号：", QLineEdit::Normal, "", &ok);
    if (!ok || u.isEmpty()) return;

    QString name = QInputDialog::getText(this, "修改读者信息", "新姓名：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString contact = QInputDialog::getText(this, "修改读者信息", "新联系方式：", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    int maxBorrow = QInputDialog::getInt(this, "修改读者信息", "最大借书数量：", 3, 1, 100, 1, &ok);
    if (!ok) return;

    if (users_->updateUserInfoByAdmin(u.toStdString(), name.toStdString(), contact.toStdString(), maxBorrow))
        QMessageBox::information(this, "成功", "修改成功。");
    else
        QMessageBox::warning(this, "失败", "修改失败：账号不存在或不允许修改该账号。");
}

void MainWindow::banUserUI() {
    bool ok = false;
    QString u = QInputDialog::getText(this, "封禁账号", "请输入要封禁的账号：", QLineEdit::Normal, "", &ok);
    if (!ok || u.isEmpty()) return;

    if (users_->setBanned(u.toStdString(), true))
        QMessageBox::information(this, "成功", "封禁成功。");
    else
        QMessageBox::warning(this, "失败", "封禁失败：账号不存在或不允许封禁。");
}

void MainWindow::unbanUserUI() {
    bool ok = false;
    QString u = QInputDialog::getText(this, "解封账号", "请输入要解封的账号：", QLineEdit::Normal, "", &ok);
    if (!ok || u.isEmpty()) return;

    if (users_->setBanned(u.toStdString(), false))
        QMessageBox::information(this, "成功", "解封成功。");
    else
        QMessageBox::warning(this, "失败", "解封失败：账号不存在或不允许解封。");
}

void MainWindow::listBannedUI() {
    QMessageBox::information(this, "封禁名单", usersToText(users_->bannedUsers()));
}

void MainWindow::promoteToAdminUI() {
    bool ok = false;
    QString u = QInputDialog::getText(this, "授予管理员权限", "请输入目标账号：", QLineEdit::Normal, "", &ok);
    if (!ok || u.isEmpty()) return;

    if (users_->setRoleBySuperAdmin(u.toStdString(), UserRole::Admin))
        QMessageBox::information(this, "成功", "已授予管理员权限。");
    else
        QMessageBox::warning(this, "失败", "操作失败：账号不存在或不允许修改该账号。");
}

void MainWindow::demoteToReaderUI() {
    bool ok = false;
    QString u = QInputDialog::getText(this, "撤销管理员权限", "请输入目标账号：", QLineEdit::Normal, "", &ok);
    if (!ok || u.isEmpty()) return;

    if (users_->setRoleBySuperAdmin(u.toStdString(), UserRole::Reader))
        QMessageBox::information(this, "成功", "已撤销管理员权限。");
    else
        QMessageBox::warning(this, "失败", "操作失败：账号不存在或不允许修改该账号。");
}

void MainWindow::showReaderTableUI() {
    ReaderListDialog dlg(users_, lib_, this);
    dlg.exec();
}

// ================= 借阅模块 =================
void MainWindow::borrowBookUI() {
    auto u = users_->findUser(me_.toStdString());
    if (!u.has_value()) {
        QMessageBox::warning(this, "错误", "当前账号不存在。");
        return;
    }

    int cur = lib_->currentBorrowCount(me_.toStdString());
    if (cur >= u->maxBorrow()) {
        QMessageBox::warning(this, "额度不足",
            QString("借书数量已达上限：%1/%2").arg(cur).arg(u->maxBorrow()));
        return;
    }

    bool ok = false;
    QString isbn = QInputDialog::getText(this, "借书", "请输入要借的图书ISBN：", QLineEdit::Normal, "", &ok);
    if (!ok || isbn.isEmpty()) return;

    std::string recID;
    if (lib_->borrowBook(me_.toStdString(), isbn.toStdString(), recID)) {
        QMessageBox::information(this, "成功",
            QString("借书成功！\n借阅记录ID：%1").arg(QString::fromStdString(recID)));
    }
    else {
        QMessageBox::warning(this, "失败", "借书失败：图书不存在或库存不足。");
    }
}

void MainWindow::returnBookUI() {
    bool ok = false;
    QString rec = QInputDialog::getText(this, "还书", "请输入借阅记录ID：", QLineEdit::Normal, "", &ok);
    if (!ok || rec.isEmpty()) return;

    int days = 0;
    bool overdue = false;

    if (lib_->returnBookByRecordID(rec.toStdString(), days, overdue)) {
        QString msg = QString("还书成功！\n借阅天数：%1 天\n是否超期：%2")
            .arg(days)
            .arg(overdue ? "是" : "否");
        QMessageBox::information(this, "成功", msg);
    }
    else {
        QMessageBox::warning(this, "失败", "还书失败：记录不存在或已归还。");
    }
}

void MainWindow::myRecordsUI() {
    auto v = lib_->recordsByUser(me_.toStdString());
    QMessageBox::information(this, "我的借阅记录", recordsToText(v));
}

void MainWindow::allRecordsUI() {
    QMessageBox::information(this, "全部借阅记录", recordsToText(lib_->getAllRecords()));
}

// ================= 读者注销（仅本人且仅读者）=================
void MainWindow::deleteSelfUI() {
    if (isAdmin()) {
        QMessageBox::warning(this, "不允许", "按演示规则：管理员不允许在此界面注销账号。");
        return;
    }

    auto reply = QMessageBox::question(this, "确认注销",
        "确定要注销你的账号吗？该操作不可恢复。", QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    std::string msg;
    if (users_->deleteSelf(me_.toStdString(), *lib_, msg)) {
        QMessageBox::information(this, "成功", QString::fromStdString(msg));
        emit logoutRequested();
    }
    else {
        QMessageBox::warning(this, "失败", QString::fromStdString(msg));
    }
}