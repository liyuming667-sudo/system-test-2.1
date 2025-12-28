#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "ReaderListDialog.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QStringList>

#include "UserManager.h"
#include "LibraryManager.h"
#include "User.h"
#include "BorrowRecord.h"

ReaderListDialog::ReaderListDialog(UserManager* users,
    LibraryManager* library,
    QWidget* parent)
    : QDialog(parent), users_(users), library_(library)
{
    setWindowTitle("读者 / 管理员列表");
    resize(900, 420);

    initUI();
    loadData();
}

void ReaderListDialog::initUI() {
    auto* layout = new QVBoxLayout(this);

    table_ = new QTableWidget(this);
    table_->setColumnCount(9);
    table_->setHorizontalHeaderLabels({
        "账号",
        "身份",
        "封禁状态",
        "姓名",
        "联系方式",
        "最大借书数",
        "当前借书数",
        "正在借阅的书",
        "备注"
        });

    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(table_);
}

void ReaderListDialog::loadData() {
    if (!users_ || !library_) return;

    auto users = users_->allUsers();
    table_->setRowCount(static_cast<int>(users.size()));

    int row = 0;
    for (const auto& u : users) {
        QString roleStr = "读者";
        if (u.isSuperAdmin()) roleStr = "最高管理员";
        else if (u.isAdmin()) roleStr = "管理员";

        QString bannedStr = u.banned() ? "是" : "否";

        int curBorrow = library_->currentBorrowCount(u.username());

        // 拼接正在借的书 ISBN
        QStringList borrowingBooks;
        auto records = library_->recordsByUser(u.username());
        for (const auto& r : records) {
            if (!r.returned()) {
                borrowingBooks << QString::fromStdString(r.isbn());
            }
        }

        auto setItem = [&](int col, const QString& text) {
            table_->setItem(row, col, new QTableWidgetItem(text));
            };

        setItem(0, QString::fromStdString(u.username()));
        setItem(1, roleStr);
        setItem(2, bannedStr);
        setItem(3, QString::fromStdString(u.name()));
        setItem(4, QString::fromStdString(u.contact()));
        setItem(5, QString::number(u.maxBorrow()));
        setItem(6, QString::number(curBorrow));
        setItem(7, borrowingBooks.join(", "));
        setItem(8, u.banned() ? "账号被封禁，无法登录" : "");

        ++row;
    }
}