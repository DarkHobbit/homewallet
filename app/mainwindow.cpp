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
#include <QResizeEvent>
#include <QSqlError>
#include <QTime>

#include "aboutdialog.h"
#include "configmanager.h"
#include "globals.h"
#include "helpers.h"
#include "logwindow.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "testmanager.h"
#include "ui_mainwindow.h"
#include "formats/interactiveformat.h"
#include "formatsgui/exportdialog.h"
#include "formatsgui/preimporthbguihelper.h"
#include "formatsgui/postimportdialog.h"

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
    mdlLend = new CreditModel(this, true);
    proxyLend = new QSortFilterProxyModel(this);
    prepareModel(mdlLend, proxyLend, ui->tvLend, "Lend");
    ui->tvLend->insertAction(0, ui->actionShowRepaymentHistory);
    mdlBorrow = new CreditModel(this, false);
    proxyBorrow = new QSortFilterProxyModel(this);
    prepareModel(mdlBorrow, proxyBorrow, ui->tvBorrow, "Borrow");
    ui->tvBorrow->insertAction(0, ui->actionShowRepaymentHistory);
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
                S_NEED_UPGRADE.arg(dbPath).arg(db.lastError()),
                QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes)
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
    case atLend:
        updateOneView(ui->tvLend, true);
        break;
    case atBorrow:
        updateOneView(ui->tvBorrow, true);
        break;
    // Here process other tabs
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
    ui->tvExchange->resizeColumnsToContents();
    ui->tvLend->resizeColumnsToContents();
    ui->tvBorrow->resizeColumnsToContents();
    // Here process other tabs
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
        if (!m)
            return;
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
        FileFormat* impFile = factory.getObject(path, QIODevice::ReadOnly);
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
        if (!PreImportHbGuiHelper::check(path, impFile, db))
            return;

        // Check if already
        QFileInfo fi(path);
        int idImp = db.findImportFile(fi.fileName());
        if (idImp>-1) {
            if (QMessageBox::question(0, S_CONFIRM, S_REPEAT_IMPORT.arg(fi.fileName()),
                              QMessageBox::Yes, QMessageBox::No)!=QMessageBox::Yes)
                return;
        }
        else
            idImp = db.addImportFile(fi.fileName(), impFile->formatAbbr());
        impFile->setIdImp(idImp);

        // Actually, import
        bool res = impFile->importRecords(path, db);
        processImpExErrors(impFile, res, path);
        // Post-import
        if (res) {
            if (impFile->isDialogRequired()) {
                InteractiveFormat* intFile = dynamic_cast<InteractiveFormat*>(impFile);
                if (intFile) {
                    PostImportDialog* d = new PostImportDialog(0);
                    d->setData(intFile, &db);
                    d->exec();
                    if (d->result()==QDialog::Accepted)
                        impFile->postImport(db);
                    delete d;
                }
                else
                    QMessageBox::warning(0, S_WARNING, tr("Format require dialog, but dialog not provided. Contact author"));
            }
            else
                impFile->postImport(db);
        }
        processImpExSuccess(impFile);
        updateViews();
    }
}

void MainWindow::on_action_Export_triggered()
{
    ExportDialog* d = new ExportDialog(&factory, 0);
    int res = d->exec();
    FileFormat* expFile = d->getCurrentFormat();
    FileFormat::SubTypeFlags subTypes = d->getSubTypes();
    QString path = d->getPath();
    bool isDir = d->isDir();
    delete d;
    if (res!=QDialog::Accepted)
        return;
    bool eRes = true;
    if (isDir) { // Separate file in dir by each info type
        int test = 1;
        for (int i=0; i<FileFormat::subTypeFlagsCount; i++) {
            if (subTypes.testFlag((FileFormat::SubType)test)) {
                QString fPath = path+QDir::separator()+subTypeFileNames[i]+"."
                    +expFile->supportedExtensions()[0];
                eRes = expFile->exportRecords(fPath, db, (FileFormat::SubType)test);
                if (!eRes)
                    break;
            }
            test = test << 1;
        }
    }
    else { // Entire file with multi-info-type
        // TODO
    }
    processImpExErrors(expFile, eRes, path);
    processImpExSuccess(expFile);
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
    source->setDefaultVisibleColumns();
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
    // Context menu
    view->setContextMenuPolicy(Qt::ActionsContextMenu);
    view->insertAction(0, ui->actionEdit);
    view->insertAction(0, ui->actionDelete);
    QAction* actSep = new QAction(this);
    actSep->setSeparator(true);
    view->insertAction(0, actSep);
    view->insertAction(0, ui->actionTechInfo);
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
        activeView = ui->tvExpenses;
        return atExpenses;
    }
    else if (curW==ui->tabIncomes) {
        activeModel = mdlIncomes;
        activeView = ui->tvIncomes;
        return atIncomes;
    }
    else if (curW==ui->tabTransferAndExchange) {
        if (ui->tabWidgetTransferAndExchange->currentWidget()==ui->tabTransfer) {
            activeModel = mdlTransfer;
            activeView = ui->tvTransfer;
            return atTransfer;
        }
        else {
            activeModel = mdlCurrConv;
            activeView = ui->tvExchange;
            return atExchange;
        }
    }
    else if (curW==ui->tabLendAndBorrow) {
        if (ui->tabWidgetLendAndBorrow->currentWidget()==ui->tabLend) {
            activeModel = mdlLend;
            activeView = ui->tvLend;
            return atLend;
        }
        else {
            activeModel = mdlBorrow;
            activeView = ui->tvBorrow;
            return atBorrow;
        }
    }
    // Here process other tabs
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
    case atTransfer:
        proxy = proxyTransfer;
        break;
    case atExchange:
        proxy = proxyCurrConv;
        break;
    case atLend:
        proxy = proxyLend;
        break;
    case atBorrow:
        proxy = proxyBorrow;
        break;
    // Here process other tabs
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

void MainWindow::on_tabWidgetLendAndBorrow_currentChanged(int)
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
    updateTableConfig(ui->tvLend);
    updateTableConfig(ui->tvBorrow);
    // Here process other tabs
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
    case atLend:
        ui->tvLend->resizeColumnsToContents();
        break;
    case atBorrow:
        ui->tvBorrow->resizeColumnsToContents();
        break;
    // Here process other tabs
    default:
        ui->cbCategory->clear();
    }
    fillComboByDict(ui->cbCategory, collCat, true);
    on_cbCategory_activated(0); // fill subcategories, where needed, and...
    // Filter config (model-dependent, see also constructor)
    QString category, subcategory;
    configManager.readCategoriesFilter(activeModel, category, subcategory);
    if (!category.isEmpty()) {
        ui->cbCategory->setCurrentIndex(ui->cbCategory->findText(category));
        on_cbCategory_activated(0);
        if (!subcategory.isEmpty())
            ui->cbSubcategory->setCurrentIndex(ui->cbSubcategory->findText(subcategory));
    }
    updateViews();
}

void MainWindow::processImpExErrors(FileFormat *f, bool res, const QString& path)
{
    QString fatalError = f->fatalError();
    QStringList errors = f->errors();
    if (!errors.isEmpty()) {
        LogWindow* w = new LogWindow(0);
        w->setData(path, f->processedRecordsCount(), errors);
        w->exec();
        delete w;
    }
    if (fatalError.isEmpty() && !res) {
        fatalError = S_ERR_UNKNOWN_IMP_EX;
    }
    if (!fatalError.isEmpty()) {
        fatalError += S_ERR_REC_NUM
                          .arg(f->processedRecordsCount())
                          .arg(f->totalRecordsCount());
        QMessageBox::critical(0, S_ERROR, fatalError);
    }
}

void MainWindow::processImpExSuccess(FileFormat *f)
{
    QMessageBox::information(0, S_INFORM,
        S_INFO_IMP_STAT.arg(f->processedRecordsCount()));
}

void MainWindow::on_btn_Add_clicked()
{
    QMessageBox::critical(0, S_ERROR, "=|=UNDER CONSTRUCTION=|=");
}

void MainWindow::on_btn_Edit_clicked()
{
    QMessageBox::critical(0, S_ERROR, "=|=UNDER CONSTRUCTION=|=");
}

void MainWindow::on_btn_Delete_clicked()
{
    activeTab();
    if (!checkSelection()) return;
    if (QMessageBox::question(0, S_CONFIRM, S_REMOVE_CONFIRM,
                              QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes)
    {
        if (activeModel->removeAnyRows(selection))
            updateViews(); // TODO restore prev position from removes
    }
}


bool MainWindow::checkSelection(bool errorIfNoSelected, bool onlyOneRowAllowed)
{
    QModelIndexList proxySelection = activeView->selectionModel()->selectedRows();
    if (proxySelection.count()==0) {
        if (errorIfNoSelected)
            QMessageBox::critical(0, S_ERROR, S_REC_NOT_SEL);
        return false;
    }
    if (onlyOneRowAllowed && (proxySelection.count()>1)) {
        QMessageBox::critical(0, S_ERROR, S_ONLY_ONE_REC);
        return false;
    }
    // Map indices from proxy to source
    QSortFilterProxyModel* selectedProxy = dynamic_cast<QSortFilterProxyModel*>(activeView->model());
    selection.clear();
    foreach(QModelIndex index, proxySelection)
        selection << selectedProxy->mapToSource(index);
    return true;
}


void MainWindow::on_actionTechInfo_triggered()
{
    activeTab();
    if (!checkSelection(true, true)) return;
    QMessageBox::information(0, S_INFORM, activeModel->techInfo(selection.first()));
}


void MainWindow::on_actionEdit_triggered()
{
    on_btn_Edit_clicked();
}


void MainWindow::on_actionDelete_triggered()
{
    on_btn_Delete_clicked();
}

void MainWindow::on_actionShowRepaymentHistory_triggered()
{
    activeTab();
    if (!checkSelection(true, true)) return;
    CreditModel* mdl = dynamic_cast<CreditModel*>(activeModel);
    if (!mdl)
        return;
    QWidget* wRep = new QWidget(0);
    QVBoxLayout* l = new QVBoxLayout(wRep);
    QTableView* tabRep = new QTableView(this);
    l->addWidget(tabRep);
    wRep->setMinimumWidth(640);
    wRep->setMinimumHeight(480);
    wRep->setWindowTitle(tr("Repayment history for: ")+mdl->recordLabel(selection.first()));
    QSqlQueryModel* mdlRep = mdl->createRepaymentModelForRecord(selection.first());
    if (!mdlRep)
        return;
    mdlRep->setParent(wRep);
    tabRep->setModel(mdlRep);
    tabRep->horizontalHeader()->setStretchLastSection(true);
    tabRep->resizeColumnsToContents();
    updateTableConfig(tabRep); // TODO sorting
    wRep->setGeometry(this->x()+this->width()/2, this->y()+this->height()/2, -1, -1);
    wRep->show();
}

