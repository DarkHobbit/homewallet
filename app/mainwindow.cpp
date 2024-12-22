/* Home Wallet
 *
 * Module: Main window
 *
 * Copyright 2023 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#include <QApplication>
#include <QDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QSqlError>
#include <QTime>
#include <QVBoxLayout>

#include "aboutdialog.h"
#include "globals.h"
#include "configmanager.h"
#include "logwindow.h"
#include "mainwindow.h"
#include "testmanager.h"
#include "ui_mainwindow.h"
#include "formats/xmlhbfile.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Models
    mdlExpenses = new ExpenseModel(this);
    proxyExpenses  = new QSortFilterProxyModel(this);
    prepareModel(mdlExpenses, proxyExpenses, ui->tvExpenses);
    ui->leExpFilter->installEventFilter(this);
    showMaximized(); // TODO to settings
    // Status bar
    lbCounts = new QLabel(0);
    statusBar()->addWidget(lbCounts);
    // Database open (and create, if needed)
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
    case HwDatabase::Alien:
        QMessageBox::critical(0, S_ERROR, S_ALIEN_DB.arg(dbPath));
        return;
    case HwDatabase::NeedUpgrade:
        if (QMessageBox::question(0, S_CONFIRM,
                S_NEED_UPGRADE.arg(dbPath).arg(db.lastError()))==QMessageBox::Yes)
            db.upgrade(dbPath);
        else
            return;
    default:
        break;
    }
    openDb(dbPath);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openDb(const QString &path)
{
    if (!db.open(path)) {
        QMessageBox::critical(0, S_ERROR,
        S_CANT_OPEN_DB.arg(path).arg(db.lastError()));
        return;
    }
    updateViews();
}

void MainWindow::updateViews()
{
    mdlExpenses->update();
    // ui->tvExpenses->resizeRowsToContents(); // very slow
    const int rH = 20;
#if QT_VERSION >= 0x050200
    ui->tvExpenses->verticalHeader()->setMaximumSectionSize(rH);
#endif
    for (int i=0; i<ui->tvExpenses->model()->rowCount(); i++) // M.b. slow on some computers!
        ui->tvExpenses->setRowHeight(i, rH);
    // Status bar
    lbCounts->setText(tr("Expenses: %1").arg(mdlExpenses->rowCount()));
}

void MainWindow::resizeEvent(QResizeEvent* e)
{
    QMainWindow::resizeEvent(e);
    int w = ui->tvExpenses->width() - ui->tvExpenses->verticalHeader()->width() - 64;
    int cc = ui->tvExpenses->model()->columnCount();
    if (cc) {
        int w1col = w/cc;
        for (int i=0; i<cc; i++)
            if (i==3) // TODO tune it if column view can changed
                ui->tvExpenses->setColumnWidth(i, qMax(w1col/4, 30));
            else if (i==5 || i==4)
                ui->tvExpenses->setColumnWidth(i, qMax(w1col/2, 60));
            else
                ui->tvExpenses->setColumnWidth(i, w1col);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEv = static_cast<QKeyEvent *>(event);
        if (kEv->key()==Qt::Key_Enter || kEv->key()==Qt::Key_Return) {
            if (focusWidget()==ui->leExpFilter)
                on_btn_Exp_Filter_Apply_clicked();
        }
        else
            return false;
        return true;
    } else // default
        return QObject::eventFilter(obj, event);
}

void MainWindow::on_action_About_triggered()
{
    AboutDialog* d = new AboutDialog(0);
    d->exec();
    delete d;
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    qApp->aboutQt();
}

void MainWindow::on_action_DbDebug_triggered()
{
    QString sQuery = QInputDialog::getText(
        0, tr("DB query"), tr("Query text"), QLineEdit::Normal, "select ");
    if (!sQuery.isEmpty()) {
        QSqlQueryModel* m = TestManager::dbDebug(sQuery, db);
        QString err = m->lastError().text();
        if (err.trimmed().isEmpty()) {
            QMessageBox::information(0, S_INFORM, QString("%1 records got").arg(m->rowCount()));
            QTableView* t = new QTableView(0);
            t->setModel(m);
            m->setParent(t);
            t->setWindowTitle(tr("Query result"));
            t->show();
        }
        else
            QMessageBox::critical(0, S_ERROR, err);
    }
}

void MainWindow::on_actionE_xit_triggered()
{
    close();
}

void MainWindow::on_action_Import_triggered()
{
    QString supportedFiltersForRead = "XML (*.xml *.XML)"; // TODO move to FormatFactory
    //QString supportedFiltersForWrite = ;
    QString selectedFilter;
    QString path = QFileDialog::getOpenFileName(0, tr("Open file for import"),
        configManager.lastImportedFile(),
        supportedFiltersForRead,
        &selectedFilter);
    if (!path.isEmpty()) {
        configManager.setLastImportedFile(path);
        XmlHbFile* impFile = new XmlHbFile(); // TODO move to FormatFactory, detect HK XML or HW XML (also gnucache, ledger, etc)
        if (!impFile->detect(path)) {
            delete impFile;
            return;            
        }
        // HB-specific stuff
        XmlHbFile* hbFile = dynamic_cast<XmlHbFile*>(impFile);
        if (hbFile) {
            XmlHbFile::SubType sType = hbFile->fileSubType();
            if (hbFile->isAmbiguous()) {
                QString categorySamples = hbFile->categorySamples();
                if (categorySamples.isEmpty()) {
                    QMessageBox::critical(0, S_ERROR, S_EMPTY_FILE.arg(path));
                    return;
                }
                else {
                    // Ambiguous HB file subtype
                    QString altType, alt1, alt2;
                    XmlHbFile::SubType res1, res2;
                    switch (sType) {
                    case XmlHbFile::IncomesOrExpenses:
                        altType = S_HB_SELECT_AMBIG_IE;
                        alt1 = S_TREAT_AS_INCOMES;
                        alt2 = S_TREAT_AS_EXPENSES;
                        res1 = XmlHbFile::Incomes;
                        res2 = XmlHbFile::Expenses;
                        break;
                    case XmlHbFile::DebtorsOrCreditors:
                        altType = S_HB_SELECT_AMBIG_DC;
                        alt1 = S_TREAT_AS_DEBTORS;
                        alt2 = S_TREAT_AS_CREDITORS;
                        res1 = XmlHbFile::Debtors;
                        res2 = XmlHbFile::Creditors;
                        break;
                    case XmlHbFile::IncomeOrExpensePlan:
                    default:
                        altType = S_HB_SELECT_AMBIG_PIE;
                        alt1 = S_TREAT_AS_INC_PLAN;
                        alt2 = S_TREAT_AS_EXP_PLAN;
                        res1 = XmlHbFile::IncomePlan;
                        res2 = XmlHbFile::ExpensePlan;
                        break;
                    }
                    QMessageBox mbAmbig(QMessageBox::Question,
                        S_HB_SELECT_AMBIG_TITLE,
                        S_HB_SELECT_AMBIG_ASK
                            .arg(altType).arg(categorySamples));
                    QPushButton* btnAlt1 = mbAmbig.addButton(alt1, QMessageBox::YesRole);
                    mbAmbig.addButton(alt2, QMessageBox::NoRole);
                    QPushButton* btnCancel = mbAmbig.addButton(QMessageBox::Cancel);
                    mbAmbig.exec();
                    QAbstractButton* btnRes = mbAmbig.clickedButton();
                    if (btnRes==btnCancel) {
                        delete impFile;
                        return;
                    }
                    hbFile->setFileSubType((btnRes==btnAlt1) ? res1 : res2);
                }
            }
            else if (sType==XmlHbFile::AccountsInDetail) {
                QMessageBox::critical(0, S_ERROR, S_ACC_DET_NOT_SUPPORTED);
                return;
            }
        }
        // Actually, import
        bool res = impFile->importRecords(path, db);
        QString fatalError = impFile->fatalError();
        QStringList errors = impFile->errors();
        if (!errors.isEmpty()) {
            LogWindow* w = new LogWindow(0);
            w->setData(path, impFile->importedRecordsCount(), errors);
            w->exec();
            delete w;
        }
        if (fatalError.isEmpty() && !res)
            fatalError = tr("Unknown import error");
        if (!fatalError.isEmpty()) {
            fatalError += tr("\nat record %1 from %2")
                .arg(impFile->importedRecordsCount())
                .arg(impFile->totalRecordsCount());
            QMessageBox::critical(0, S_ERROR, fatalError);
        }
        delete impFile;
        updateViews();
    }
}

void MainWindow::on_action_Settings_triggered()
{
    // TODO At first, drag view (table) and locale settings from DC
}

#include <iostream>
void MainWindow::on_leExpFilter_textChanged(const QString&)
{
    if (false) // TODO to Settings
        on_btn_Exp_Filter_Apply_clicked();
}


void MainWindow::on_actionFilter_triggered()
{
    switch (activeTab()) {
    case atExpenses:
        ui->leExpFilter->setFocus();
        break;
    default:
        break;
    }
}

void MainWindow::prepareModel(QAbstractItemModel *source, QSortFilterProxyModel *proxy, QTableView *view)
{
    proxy->setSourceModel(source);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive); // Driver == driver
    proxy->setSortRole(SortStringRole);
    view->setModel(proxy);
    // TODO implement selection and checkSelection() and mapToSource for all cases, see DoubleContact's checkSelection()
    view->setSortingEnabled(true); // TODO to settings
    view->horizontalHeader()->setStretchLastSection(true);
}

MainWindow::ActiveTab MainWindow::activeTab()
{
    QTabWidget* t = ui->tabWidget;
    QString s = t->tabText(t->indexOf(t->currentWidget()));
    if (s==tr("Expenses"))
        return atExpenses;
    else if (s==tr("Incomes"))
        return atIncomes;
    else
        return atAccounts;
}

void MainWindow::on_btn_Exp_Filter_Apply_clicked()
{
    if (ui->leExpFilter->text().isEmpty())
        proxyExpenses->setFilterWildcard("");
    else
        proxyExpenses->setFilterWildcard(ui->leExpFilter->text());
    updateViews();
}
