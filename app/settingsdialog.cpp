#include <QColorDialog>
#include <QFont>
#include <QFontDialog>
#include <QMessageBox>

#include "configmanager.h"
#include "globals.h"
#include "helpers.h"
#include "languagemanager.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , _lang(""), _langChanged(false)
    , columnsChanged(false), prevModel(0)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::setData(FQMlist* dbModels)
{
    // Language
    ui->cbLanguage->insertItems(0, languageManager.nativeNames());
    _lang = configManager.readLanguage();
    ui->cbLanguage->setCurrentIndex(ui->cbLanguage->findText(_lang));
    // Locale
    ui->leDateFormat->setText(gd.dateFormat);
    ui->leTimeFormat->setText(gd.timeFormat);
    ui->cbUseSystemDateTimeFormat->setChecked(gd.useSystemDateTimeFormat);
    on_cbUseSystemDateTimeFormat_clicked(ui->cbUseSystemDateTimeFormat->isChecked());
    // View
    ui->cbFullScreenMode->setChecked(gd.fullScreenMode);
    ui->cbShowTableGrid->setChecked(gd.showTableGrid);
    ui->cbShowLineNumbers->setChecked(gd.showLineNumbers);
    ui->cbResizeTableRowsToContents->setChecked(gd.resizeTableRowsToContents);
    ui->cbShowSumsWithCurrency->setChecked(gd.showSumsWithCurrency);
    ui->cbUseTableAlternateColors->setChecked(gd.useTableAlternateColors);
    ui->cbUseSystemFontsAndColors->setChecked(gd.useSystemFontsAndColors);
    on_cbUseSystemFontsAndColors_clicked(ui->cbUseSystemFontsAndColors->isChecked());
    // Column view
    _dbModels = dbModels;
    for (FilteredQueryModel* mdl: *_dbModels)
        ui->cbTableNames->addItem(mdl->localizedName());
    return true;
}

bool SettingsDialog::getData()
{
    // Language
    QString newLang = ui->cbLanguage->currentText();
    configManager.writeLanguage(newLang);
    if (newLang!=_lang) {
        _langChanged = true;
        _lang = newLang;
    }
    // Locale
    gd.dateFormat = ui->leDateFormat->text();
    gd.timeFormat = ui->leTimeFormat->text();
    gd.useSystemDateTimeFormat = ui->cbUseSystemDateTimeFormat->isChecked();
    // View
    gd.fullScreenMode = ui->cbFullScreenMode->isChecked();
    gd.showTableGrid = ui->cbShowTableGrid->isChecked();
    gd.showLineNumbers = ui->cbShowLineNumbers->isChecked();
    gd.resizeTableRowsToContents = ui->cbResizeTableRowsToContents->isChecked();
    gd.showSumsWithCurrency = ui->cbShowSumsWithCurrency->isChecked();
    gd.useTableAlternateColors = ui->cbUseTableAlternateColors->isChecked();
    gd.useSystemFontsAndColors = ui->cbUseSystemFontsAndColors->isChecked();
    // Column view:
    // - from cache
    for (FilteredQueryModel* model: columnsChangeCache.keys())
        model->setVisibleColumns(columnsChangeCache[model]);
    // - from current tab
    if (columnsChanged)
        prevModel->setVisibleColumns(getListItems(ui->lwVisibleColumns));
    return true;
}

bool SettingsDialog::langChanged()
{
    return _langChanged;
}

void SettingsDialog::on_cbUseSystemDateTimeFormat_clicked(bool checked)
{
    ui->leDateFormat->setEnabled(!checked);
    ui->leTimeFormat->setEnabled(!checked);
}

void SettingsDialog::on_cbUseSystemFontsAndColors_clicked(bool checked)
{
    ui->btnTableFont->setEnabled(!checked);
    ui->btnGridColor1->setEnabled(!checked);
    ui->btnGridColor2->setEnabled(!checked);
}

void SettingsDialog::on_btnTableFont_clicked()
{
    QFontDialog* d = new QFontDialog;
    QFont f;
    f.fromString(gd.tableFont);
    d->setCurrentFont(f);
    if (d->exec()==QDialog::Accepted)
        gd.tableFont = d->currentFont().toString();
    delete d;
}

void SettingsDialog::on_btnGridColor1_clicked()
{
    QColorDialog* d = new QColorDialog;
    d->setCurrentColor(QColor(gd.gridColor1));
    if (d->exec()==QDialog::Accepted)
        gd.gridColor1 = d->currentColor().name();
    delete d;
}

void SettingsDialog::on_btnGridColor2_clicked()
{
    QColorDialog* d = new QColorDialog;
    d->setCurrentColor(QColor(gd.gridColor2));
    if (d->exec()==QDialog::Accepted)
        gd.gridColor2 = d->currentColor().name();
    delete d;
}

void SettingsDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SettingsDialog::on_cbTableNames_currentIndexChanged(int indexOfModel)
{
    // if prev table was change, save it
    if (columnsChanged) {
        columnsChangeCache[prevModel] = getListItems(ui->lwVisibleColumns);
        columnsChanged = false;
    }
    // Load visible columns either from model,
    // or from cache (if changed)
    ui->lwVisibleColumns->clear();
    FilteredQueryModel* model = (*_dbModels)[indexOfModel];
    QStringList visColumns =
            columnsChangeCache.keys().contains(model) ?
                columnsChangeCache[model] :
                model->getVisibleColumns();
    ui->lwVisibleColumns->addItems(visColumns);
    // Recalc available columns
    ui->lwAvailableColumns->clear();
    for(const QString& col: model->getAllColumns())
        if (!visColumns.contains(col))
            ui->lwAvailableColumns->addItem(col);
    prevModel = model;
}

void SettingsDialog::on_btnAddCol_clicked()
{
    foreach (QListWidgetItem* item, ui->lwAvailableColumns->selectedItems()) {
        ui->lwVisibleColumns->addItem(item->text());
        delete item;
        columnsChanged = true;
    }
}

void SettingsDialog::on_btnDelCol_clicked()
{
    if (ui->lwVisibleColumns->selectedItems().count()>=ui->lwVisibleColumns->count()) {
        QMessageBox::critical(0, S_ERROR, tr("List must contain at least one visible column"));
    }
    else
    foreach (QListWidgetItem* item, ui->lwVisibleColumns->selectedItems()) {
        ui->lwAvailableColumns->addItem(item->text());
        delete item;
        columnsChanged = true;
    }
}

void SettingsDialog::on_btnUpCol_clicked()
{
    for (int i=1; i<ui->lwVisibleColumns->count(); i++) {
        QListWidgetItem* item = ui->lwVisibleColumns->item(i);
        if (item->isSelected()) {
            QString colName = item->text();
            delete item;
            ui->lwVisibleColumns->insertItem(i-1, colName);
            ui->lwVisibleColumns->item(i-1)->setSelected(true);
            columnsChanged = true;
        }
    }
}

void SettingsDialog::on_btnDownCol_clicked()
{
    for (int i=ui->lwVisibleColumns->count()-2; i>=0; i--) {
        QListWidgetItem* item = ui->lwVisibleColumns->item(i);
        if (item->isSelected()) {
            QString colName = item->text();
            delete item;
            ui->lwVisibleColumns->insertItem(i+1, colName);
            ui->lwVisibleColumns->item(i+1)->setSelected(true);
            columnsChanged = true;
        }
    }
}

void SettingsDialog::on_lwAvailableColumns_itemDoubleClicked(QListWidgetItem *item)
{
    item->setSelected(true);
    on_btnAddCol_clicked();
}

void SettingsDialog::on_lwVisibleColumns_itemDoubleClicked(QListWidgetItem *item)
{
    item->setSelected(true);
    on_btnDelCol_clicked();
}


void SettingsDialog::on_btnResetColumnToDefaults_clicked()
{
    int indexOfModel = ui->cbTableNames->currentIndex();
    FilteredQueryModel* model = (*_dbModels)[indexOfModel];
    columnsChangeCache.remove(model);
    columnsChanged = false;
    model->setDefaultVisibleColumns();
    on_cbTableNames_currentIndexChanged(indexOfModel);
}

