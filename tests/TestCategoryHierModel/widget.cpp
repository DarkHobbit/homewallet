#include <QMessageBox>

#include "configmanager.h"
#include "globals.h"
#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    configManager.prepare();
    QString dbPath = configManager.localDatabaseDir();

    if (!db.exists(dbPath)) {
        QMessageBox::information(0, S_INFORM,
                                 S_FIRST_TIME+"\n"+S_WILL_CREAT.arg(dbPath));
        if (!db.create(dbPath)) {
            QMessageBox::critical(0, S_ERROR,
                                  S_CANT_CREAT_DB.arg(dbPath).arg(db.lastError()));
        }
    }
    HwDatabase::DBFileState dbState = db.test(dbPath);
    switch (dbState) {
    case HwDatabase::OpenError:
        QMessageBox::critical(0, S_ERROR, db.lastError());
        return;
    case HwDatabase::Alien:
        QMessageBox::critical(0, S_ERROR, S_ALIEN_DB.arg(dbPath));
        return;
    case HwDatabase::NeedUpgrade:
        if (QMessageBox::question(0, S_CONFIRM,
                                  S_NEED_UPGRADE.arg(dbPath).arg(db.lastError()),
                                  QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes)
            db.upgrade(dbPath);
        else
            return;
    default:
        break;
    }
    if (!db.open(dbPath)) {
        QMessageBox::critical(0, S_ERROR,
                              S_CANT_OPEN_DB.arg(dbPath).arg(db.lastError()));
        return;
    }
    for (const QString& ws: db.warnings())
        QMessageBox::warning(0, S_WARNING, ws);
    mC = new CategoryHierModel(true, &db, this);
    ui->treeView->setModel(mC);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnAdd_clicked()
{
    QModelIndexList sel = ui->treeView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        db.addExpenseCategory(QString::fromUtf8("Тапки"), QString::fromUtf8("Тапочки"));
    }
    else {
        QModelIndex p = sel.first();
        if (mC->isCategory(p)) {
            db.addExpenseSubCategory(mC->getId(p), QString::fromUtf8("Мона"), QString::fromUtf8("Бон аппетит    "));
        }
        else
            QMessageBox::critical(0, S_ERROR, "Select category or don't select nothing");
    }
    mC->refresh();
}

void Widget::on_btnRemove_clicked()
{
    QModelIndexList sel = ui->treeView->selectionModel()->selectedRows();
    if (sel.isEmpty())
        return;
    if (!mC->removeAnyRows(sel))
        QMessageBox::critical(0, S_ERROR, mC->lastError());
}
