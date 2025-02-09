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
#include "configmanager.h"
#include "globals.h"
#include "helpers.h"
#include "logwindow.h"
#include "mainwindow.h"
#include "testmanager.h"
#include "ui_mainwindow.h"
#include "formats/interactiveformat.h"
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
    mdlIncomes = new IncomeModel(this);
    proxyIncomes = new QSortFilterProxyModel(this);
    prepareModel(mdlIncomes, proxyIncomes, ui->tvIncomes);

    ui->leQuickFilter->installEventFilter(this);
    ui->dteDateFrom->installEventFilter(this);
    ui->dteDateTo->installEventFilter(this);
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
    if (gd.debugDataMode) {
        if (db.isEmpty()) {
            if (TestManager::createTestData(db)) {
                on_tabWidget_currentChanged(0);
                updateViews();
            }
            else
                QMessageBox::critical(0, S_ERROR,
                    tr("Test data create error"));
        }
        else
            QMessageBox::critical(0, S_ERROR,
                tr("Database not empty for testing"));
    }
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
    on_tabWidget_currentChanged(0);
    updateViews();
}

void MainWindow::updateViews()
{
    // Filter
    on_cbDateFrom_stateChanged(0);
    on_cbDateTo_stateChanged(0);
    switch (activeTab()) {
    case atExpenses:
        updateOneModel(mdlExpenses);
        updateOneView(ui->tvExpenses);
        break;
    // TODO unify all models with datefilters
    case atIncomes:
        updateOneModel(mdlIncomes);
        updateOneView(ui->tvIncomes);
        break;
    default:
        break;
    }
    // Status bar
    int totalInCount, totalExpCount;
    db.getCounts(totalInCount, totalExpCount);
    lbCounts->setText(tr("Expenses: %1 (%2) Incomes: %3 (%4)")
        .arg(mdlExpenses->rowCount())
        .arg(totalExpCount)
        .arg(mdlIncomes->rowCount())
        .arg(totalInCount));
}

void MainWindow::resizeEvent(QResizeEvent* e)
{
    QMainWindow::resizeEvent(e);
    ui->tvExpenses->resizeColumnsToContents();
    ui->tvIncomes->resizeColumnsToContents();
    // Alternatives is ui->tvExpenses->setColumnWidth(), autoresized columns
    // in some cases can be too big (subcategory)
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEv = static_cast<QKeyEvent *>(event);
        if (kEv->key()==Qt::Key_Enter || kEv->key()==Qt::Key_Return) {
            if (focusWidget()==ui->leQuickFilter)
                on_btn_Quick_Filter_Apply_clicked();
            else if (focusWidget()->parentWidget()==ui->gbFilter)
                on_btn_Filter_Apply_clicked();
        }
        else
            return false;
        return true;
    } else // default
        return QObject::eventFilter(obj, event);
}

void MainWindow::on_action_About_triggered()
{
    AboutDialog* d = new AboutDialog(0, db.dbInfo());
    d->exec();
    delete d;
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    qApp->aboutQt();
    QSqlDatabase sqlDb;
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
        FileFormat* impFile = new XmlHbFile(); // TODO move to FormatFactory, detect HK XML or HW XML (also gnucache, ledger, etc)
        if (!impFile->detect(path)) {
            // TODO type separate messages for defined format or auto-detect
            // And take different names for filters for HW and HK XML
            if (!impFile->fatalError().isEmpty())
                QMessageBox::critical(0, S_ERROR, impFile->fatalError());
            // TODO what if not one detected? Type all, or?..
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
        // Check if already
        QFileInfo fi(path);
        int idImp = db.findImportFile(fi.fileName());
        if (idImp>-1) {
            if (QMessageBox::question(0, S_CONFIRM, S_REPEAT_IMPORT.arg(fi.fileName()))!=QMessageBox::Yes)
                return;
        }
        else
            idImp = db.addImportFile(fi.fileName(), impFile->formatAbbr());
        impFile->setIdImp(idImp);

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
        if (impFile->isDialogRequired()) {
            InteractiveFormat* intFile = dynamic_cast<InteractiveFormat*>(impFile);
            if (intFile) {
                QDialog* d = new QDialog(0); // TODO PostImportDialog!
                //d->setData(intFile->candidates);
                d->exec();
                if (d->result()==QDialog::Accepted)
                    impFile->postImport(db);
                delete d;
            }
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
void MainWindow::on_leQuickFilter_textChanged(const QString&)
{
    if (false) // TODO to Settings
        on_btn_Quick_Filter_Apply_clicked();
}


void MainWindow::on_actionFilter_triggered()
{
    ui->leQuickFilter->setFocus();
}

void MainWindow::prepareModel(FilteredQueryModel *source, QSortFilterProxyModel *proxy, QTableView *view)
{
    proxy->setSourceModel(source);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive); // Driver == driver
    proxy->setSortRole(SortStringRole);
    view->setModel(proxy);
    // TODO implement selection and checkSelection() and mapToSource for all cases, see DoubleContact's checkSelection()
    view->setSortingEnabled(true); // TODO to settings
//    view->horizontalHeader()->setResizeContentsPrecision(64);
    view->horizontalHeader()->setStretchLastSection(true);
    // Error handling
    connect(source, SIGNAL(modelError(QString)), this, SLOT(on_Model_Error(QString)));
}

void MainWindow::updateOneModel(CategoriesBasedQueryModel *source)
{
    source->setFilterDates(
        ui->cbDateFrom->isChecked() ? ui->dteDateFrom->date() : QDate(),
        ui->cbDateTo->isChecked() ? ui->dteDateTo->date() : QDate());
    source->setFilterCategories(
        getComboCurrentId(ui->cbCategory),
        getComboCurrentId(ui->cbSubcategory));
    source->update();
}

void MainWindow::updateOneView(QTableView *view)
{
    view->setColumnHidden(0, true);
    // view->resizeRowsToContents(); // very slow, m.b. to settings
    const int rH = 20;
#if QT_VERSION >= 0x050200
    view->verticalHeader()->setMaximumSectionSize(rH);
#endif
    for (int i=0; i<view->model()->rowCount(); i++) // M.b. slow on some computers!
        view->setRowHeight(i, rH);
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

void MainWindow::on_btn_Quick_Filter_Apply_clicked()
{
    QSortFilterProxyModel* proxy;
    switch (activeTab()) {
    case atExpenses:
        proxy = proxyExpenses;
        break;
    case atIncomes:
        proxy = proxyIncomes;
        break;
    default:
        proxy = 0;
        break;
    }
    if (ui->leQuickFilter->text().isEmpty())
        proxy->setFilterWildcard("");
    else
        proxy->setFilterWildcard(ui->leQuickFilter->text());
    updateViews();
}

void MainWindow::on_btn_Filter_Apply_clicked()
{
    updateViews();
}

void MainWindow::on_btn_Filter_Reset_clicked()
{
    ui->cbDateFrom->setChecked(false);
    ui->cbDateTo->setChecked(false);
    on_tabWidget_currentChanged(0);
    updateViews();
}

void MainWindow::on_cbDateFrom_stateChanged(int)
{
    ui->dteDateFrom->setEnabled(ui->cbDateFrom->isChecked());
}

void MainWindow::on_cbDateTo_stateChanged(int)
{
    ui->dteDateTo->setEnabled(ui->cbDateTo->isChecked());
}

void MainWindow::on_tabWidget_currentChanged(int)
{
    GenericDatabase::DictColl collCat;
    // TODO here save combo indexes and restore it
    switch (activeTab()) {
    case atExpenses:
        db.collectDict(collCat, "hw_ex_cat");
        // TODO call collectSubDict if will be slow, but it's more complex
        fillComboByDict(ui->cbCategory, collCat, true);
        break;
    case atIncomes:
        db.collectDict(collCat, "hw_in_cat");
        fillComboByDict(ui->cbCategory, collCat, true);
        break;
    default:
        //
        break;
    }
    on_cbCategory_activated(0);
    updateViews();
}

void MainWindow::on_cbCategory_activated(int)
{
    GenericDatabase::DictColl collSubcat;
    int idCat = getComboCurrentId(ui->cbCategory);
    db.collectDict(collSubcat, "hw_ex_subcat", "name", "id", QString("where id_ecat=%1").arg(idCat));
    fillComboByDict(ui->cbSubcategory, collSubcat, true);
}

void MainWindow::on_Model_Error(const QString &message)
{
    QMessageBox::critical(0, S_ERROR, message);
}

