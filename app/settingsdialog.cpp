#include <QColorDialog>
#include <QFont>
#include <QFontDialog>

#include "configmanager.h"
#include "globals.h"
#include "languagemanager.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , _lang(""), _langChanged(false)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::setData()
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
    ui->cbShowTableGrid->setChecked(gd.showTableGrid);
    ui->cbShowLineNumbers->setChecked(gd.showLineNumbers);
    ui->cbResizeTableRowsToContents->setChecked(gd.resizeTableRowsToContents);
    ui->cbShowSumsWithCurrency->setChecked(gd.showSumsWithCurrency);
    ui->cbUseTableAlternateColors->setChecked(gd.useTableAlternateColors);
    ui->cbUseSystemFontsAndColors->setChecked(gd.useSystemFontsAndColors);
    on_cbUseSystemFontsAndColors_clicked(ui->cbUseSystemFontsAndColors->isChecked());
    // Column view
    // TODO

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
    gd.showTableGrid = ui->cbShowTableGrid->isChecked();
    gd.showLineNumbers = ui->cbShowLineNumbers->isChecked();
    gd.resizeTableRowsToContents = ui->cbResizeTableRowsToContents->isChecked();
    gd.showSumsWithCurrency = ui->cbShowSumsWithCurrency->isChecked();
    gd.useTableAlternateColors = ui->cbUseTableAlternateColors->isChecked();
    gd.useSystemFontsAndColors = ui->cbUseSystemFontsAndColors->isChecked();
    // Column view
    // TODO

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
