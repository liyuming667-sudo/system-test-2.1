#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "BookListDialog.h"
#include "LibraryManager.h"
#include "Book.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QStringList>

BookListDialog::BookListDialog(LibraryManager* lib, QWidget* parent)
    : QDialog(parent), lib_(lib)
{
    setWindowTitle("全部图书列表");
    resize(1000, 600); // 窗口稍微大一点

    initUI();
    loadData();
}

void BookListDialog::initUI() {
    auto* layout = new QVBoxLayout(this);

    // 7列数据
    table_ = new QTableWidget(this);
    table_->setColumnCount(7);
    table_->setHorizontalHeaderLabels({
        "ISBN",
        "书名",
        "作者",
        "出版社",
        "年份",
        "类别",
        "库存"
        });

    // 样式优化
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自动填满宽度
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);             // 禁止编辑
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);            // 选中整行
    table_->setSelectionMode(QAbstractItemView::SingleSelection);           // 单选
    table_->setAlternatingRowColors(true);                                  // 隔行变色

    layout->addWidget(table_);
}

void BookListDialog::loadData() {
    if (!lib_) return;

    auto books = lib_->getAllBooks();
    table_->setRowCount(static_cast<int>(books.size()));

    int row = 0;
    for (const auto& b : books) {
        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter); // 居中显示
            table_->setItem(row, col, item);
            };

        setItem(0, QString::fromStdString(b.isbn()));
        setItem(1, QString::fromStdString(b.title()));
        setItem(2, QString::fromStdString(b.author()));
        setItem(3, QString::fromStdString(b.publisher()));
        setItem(4, QString::number(b.publishYear()));
        setItem(5, QString::fromStdString(b.category()));
        setItem(6, QString::number(b.stock()));

        ++row;
    }
}