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
#include "postimportdialog.h"
#include "settingsdialog.h"
#include "testmanager.h"
#include "ui_mainwindow.h"
#include "formats/interactiveformat.h"
#include "formats/xmlhbfile.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), activeModel(0)
{
    ui->setupUi(this);
    // Configuration
    configManager.setDefaults(ui->tvExpenses->font().toString(),
        ui->tvExpenses->palette().color(QPalette::Base).name(),
        ui->tvExpenses->palette().color(QPalette::AlternateBase).name());
    configManager.readConfig(); // TODO see comment in doublecontact
    // Filter config (model-independent, see also updateViews())
    QDate dtFilterFrom, dtFilterTo;
    switch (gd.filterDatesOnStartup) {
    case GlobalConfig::fdRestorePrevRange: {
        bool useDateFrom, useDateTo;
        configManager.readDateFilter(useDateFrom, useDateTo, dtFilterFrom, dtFilterTo);
        ui->dteDateFrom->setDate(dtFilterFrom);
        ui->dteDateTo->setDate(dtFilterTo);
        ui->cbDateFrom->setChecked(useDateFrom);
        ui->cbDateTo->setChecked(useDateTo);
        break;
    }
    case GlobalConfig::fdShowLastNMonths: {
        QDate now = QDate::currentDate();
        ui->dteDateFrom->setDate(now.addMonths(-gd.monthsInFilter));
        ui->dteDateTo->setDate(now);
        ui->cbDateFrom->setChecked(true);
        ui->cbDateTo->setChecked(true);
        break;
    }
    default: // dShowAllRecords - do nothing
        break;
    }
    // Models
    mdlExpenses = new ExpenseModel(this);
    proxyExpenses  = new QSortFilterProxyModel(this);
    prepareModel(mdlExpenses, proxyExpenses, ui->tvExpenses, "Expenses");
    mdlIncomes = new IncomeModel(this);
    proxyIncomes = new QSortFilterProxyModel(this);
    prepareModel(mdlIncomes, proxyIncomes, ui->tvIncomes, "Incomes");
    mdlTransfer = new TransferModel(this);
    proxyTransfer = new QSortFilterProxyModel(this);
    prepareModel(mdlTransfer, proxyTransfer, ui->tvTransfer, "Transfer");
    mdlCurrConv = new CurrConvModel(this);
    proxyCurrConv = new QSortFilterProxyModel(this);
    prepareModel(mdlCurrConv, proxyCurrConv, ui->tvExchange, "CurrConv");
    // Table columns
    for (FilteredQueryModel* mdl: dbModels)
        configManager.readTableConfig(mdl);
    // Filter
    ui->leQuickFilter->installEventFilter(this);
    ui->dteDateFrom->installEventFilter(this);
    ui->dteDateTo->installEventFilter(this);
    if (gd.fullScreenMode)
        showMaximized();
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
    case HwDatabase::OpenError:
        QMessageBox::critical(0, S_ERROR, db.lastError());
        return;
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
            if (TestManager::createTestData(db, 40000)) {
                on_tabWidgetMain_currentChanged(0);
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
    for (const QString& ws: db.warnings())
        QMessageBox::warning(0, S_WARNING, ws);
    on_tabWidgetMain_currentChanged(0);
}

void MainWindow::updateViews()
{
    // Filter
    on_cbDateFrom_stateChanged(0);
    on_cbDateTo_stateChanged(0);
    ActiveTab _activeTab = activeTab(); // set activeModel!
    updateOneModel(activeModel);
    switch (_activeTab) {
    case atExpenses:
        updateOneView(ui->tvExpenses, true);
        break;
    case atIncomes:
        updateOneView(ui->tvIncomes, true);
        break;
    case atTransfer:
        updateOneView(ui->tvTransfer, true);
        break;
    case atExchange:
        updateOneView(ui->tvExchange, true);
        break;
    default:
        break;
    }
    resizeViews();
    // Status bar
    int totalInCount, totalExpCount;
    db.getCounts(totalInCount, totalExpCount);
    lbCounts->setText(tr("Expenses: %1 (%2) Incomes: %3 (%4)")
        .arg(mdlExpenses->rowCount())
        .arg(totalExpCount)
        .arg(mdlIncomes->rowCount())
        .arg(totalInCount));
}

void MainWindow::resizeViews()
{
    ui->tvExpenses->resizeColumnsToContents();
    ui->tvIncomes->resizeColumnsToContents();
    ui->tvTransfer->resizeColumnsToContents();
    // Alternatives is ui->tvExpenses->setColumnWidth(), autoresized columns
    // in some cases can be too big (subcategory)
}

void MainWindow::resizeEvent(QResizeEvent* e)
{
    QMainWindow::resizeEvent(e);
    resizeViews();
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
    //QString supportedFiltersForWrite = ;
    QString selectedFilter;
    QString path = QFileDialog::getOpenFileName(0, tr("Open file for import"),
        configManager.lastImportedFile(),
        factory.supportedFilters(QIODevice::ReadOnly, false).join(";;"),
        &selectedFilter);
    if (!path.isEmpty()) {
        configManager.setLastImportedFile(path);
        // If format is unknown...
        FileFormat* impFile = factory.createObject(path, QIODevice::ReadOnly);
        if (!impFile) {
            QMessageBox::critical(0, S_ERROR, factory.error);
            return;
        }
        // If format was detected but cannot be imported...
        if (!impFile->fatalError().isEmpty()) {
            QMessageBox::critical(0, S_ERROR, impFile->fatalError());
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
        else if (impFile->isDialogRequired()) {
            InteractiveFormat* intFile = dynamic_cast<InteractiveFormat*>(impFile);
            if (intFile) {
                PostImportDialog* d = new PostImportDialog(0);
                d->setData(&intFile->candidates);
                d->exec();
                if (d->result()==QDialog::Accepted)
                    impFile->postImport(db);
                delete d;
            }
        }
        updateViews();
    }
}

void MainWindow::on_action_Settings_triggered()
{
    SettingsDialog* setDlg = new SettingsDialog(0);
    setDlg->setData(&dbModels);
    setDlg->exec();
    if (setDlg->result()==QDialog::Accepted) {
        setDlg->getData();
        configManager.writeConfig();
        updateViews();
        updateConfig();
        // Language
        if (setDlg->langChanged())
            QMessageBox::information(0, S_INFORM, tr("Restart program to apply language change"));
        // Table columns
        for (FilteredQueryModel* mdl: dbModels)
            configManager.writeTableConfig(mdl);
    }
    delete setDlg;
}

void MainWindow::on_leQuickFilter_textChanged(const QString&)
{
    if (gd.applyQuickFilterImmediately)
        on_btn_Quick_Filter_Apply_clicked();
}

void MainWindow::on_actionFilter_triggered()
{
    ui->leQuickFilter->setFocus();
}

void MainWindow::prepareModel(FilteredQueryModel *source, QSortFilterProxyModel *proxy, QTableView *view, const QString& nameForDebug)
{
    source->setObjectName(QString("mdl")+nameForDebug);
    proxy->setSourceModel(source);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive); // Driver == driver
    proxy->setSortRole(SortStringRole);
    proxy->setObjectName(QString("proxy")+nameForDebug);
    view->setModel(proxy);
    dbModels << source;
    // TODO implement selection and checkSelection() and mapToSource for all cases, see DoubleContact's checkSelection()
//    view->horizontalHeader()->setResizeContentsPrecision(64);
    view->horizontalHeader()->setStretchLastSection(true);
    view->setObjectName(QString("tv")+nameForDebug);
    // Error handling
    connect(source, SIGNAL(modelError(QString)), this, SLOT(processModelError(QString)));
}

void MainWindow::updateOneModel(FilteredQueryModel* source)
{
    source->setFilterDates(
        ui->cbDateFrom->isChecked() ? ui->dteDateFrom->date() : QDate(),
        ui->cbDateTo->isChecked() ? ui->dteDateTo->date() : QDate());
    CategoriesBasedQueryModel* catSource = dynamic_cast<CategoriesBasedQueryModel*>(source);
    if (catSource)
        catSource->setFilterCategories(
            getComboCurrentId(ui->cbCategory),
            ui->cbSubcategory->count()==0 ? -2 : getComboCurrentId(ui->cbSubcategory));
    else {
        ui->cbCategory->clear();
        ui->cbSubcategory->clear();
    }
    source->update();
}

MainWindow::ActiveTab MainWindow::activeTab()
{
    QTabWidget* t = ui->tabWidgetMain;
    QWidget* curW = t->currentWidget();
    if (curW==ui->tabExpenses) {
        activeModel = mdlExpenses;
        return atExpenses;
    }
    else if (curW==ui->tabIncomes) {
        activeModel = mdlIncomes;
        return atIncomes;
    }
    else if (curW==ui->tabTransferAndExchange) {
        if (ui->tabWidgetTransferAndExchange->currentWidget()==ui->tabTransfer) {
            activeModel = mdlTransfer;
            return atTransfer;
        }
        else {
            activeModel = mdlCurrConv;
            return atExchange;
        }
    }
    // TODO other tabs
    else {
        activeModel = 0; // TODO m.b. accounts also will be on main window, m.b. no
        return atAccounts;
    }
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
    // Save filter config
    configManager.writeDateFilter(
        ui->cbDateFrom->isChecked(), ui->cbDateTo->isChecked(),
        ui->dteDateFrom->date(), ui->dteDateTo->date());
    configManager.writeCategoriesFilter(activeModel, ui->cbCategory->currentText(), ui->cbSubcategory->currentText());
}

void MainWindow::on_btn_Filter_Reset_clicked()
{
    ui->cbDateFrom->setChecked(false);
    ui->cbDateTo->setChecked(false);
    on_tabWidgetMain_currentChanged(0);
    updateViews();
    // Save filter config
    configManager.writeDateFilter(false, false, QDate(), QDate());
    configManager.writeCategoriesFilter(activeModel, "", "");
}

void MainWindow::on_cbDateFrom_stateChanged(int)
{
    ui->dteDateFrom->setEnabled(ui->cbDateFrom->isChecked());
}

void MainWindow::on_cbDateTo_stateChanged(int)
{
    ui->dteDateTo->setEnabled(ui->cbDateTo->isChecked());
}

void MainWindow::on_tabWidgetMain_currentChanged(int)
{
    updateTabsAndFilters();
}

void MainWindow::on_tabWidgetTransferAndExchange_currentChanged(int)
{
    updateTabsAndFilters();
}

void MainWindow::on_cbCategory_activated(int)
{
    GenericDatabase::DictColl collSubcat;
    int idCat = getComboCurrentId(ui->cbCategory);
    switch (activeTab()) {
    case atExpenses:
        db.collectDict(collSubcat, "hw_ex_subcat", "name", "id", QString("where id_ecat=%1").arg(idCat));
        break;
    case atIncomes:
        db.collectDict(collSubcat, "hw_in_subcat", "name", "id", QString("where id_icat=%1").arg(idCat));
        break;
    default:
        ui->cbSubcategory->clear();
    }
    ui->btn_Filter_Apply->setEnabled(ui->cbCategory->count()>0);
    if (!collSubcat.isEmpty())
        fillComboByDict(ui->cbSubcategory, collSubcat, true);
    else
        ui->cbSubcategory->clear();
}

void MainWindow::processModelError(const QString &message)
{
    QMessageBox::critical(0, S_ERROR, message);
}

void MainWindow::showEvent(QShowEvent*)
{
    updateConfig();
}

// Manage immediately applied but settings window managed options
void MainWindow::updateConfig()
{
    // Table(s) general config (must be after columns config, because resizeRowsToContents())
    updateTableConfig(ui->tvExpenses);
    updateTableConfig(ui->tvIncomes);
    updateTableConfig(ui->tvTransfer);
    updateTableConfig(ui->tvExchange);
}

void MainWindow::updateTabsAndFilters()
{
    GenericDatabase::DictColl collCat;
    switch (activeTab()) {
    case atExpenses:
        ui->tvExpenses->resizeColumnsToContents();
        db.collectDict(collCat, "hw_ex_cat");
        break;
    case atIncomes:
        ui->tvIncomes->resizeColumnsToContents();
        db.collectDict(collCat, "hw_in_cat");
        break;
    case atTransfer:
        ui->tvTransfer->resizeColumnsToContents();
        db.collectDict(collCat, "hw_transfer_type");
        break;
    case atExchange:
        ui->tvExchange->resizeColumnsToContents();
        break;
    default:
        ui->cbCategory->clear();
    }
    fillComboByDict(ui->cbCategory, collCat, true);
    on_cbCategory_activated(0); // fill subcategories, where needed, and...
    // Filter config (model-dependent, see also constructor)
    QString category, subcategory;
    configManager.readCategoriesFilter(activeModel, category, subcategory);
    if (!category.isEmpty()) {
        ui->cbCategory->setCurrentText(category);
        on_cbCategory_activated(0);
        if (!subcategory.isEmpty())
            ui->cbSubcategory->setCurrentText(subcategory);
    }
    updateViews();
}

