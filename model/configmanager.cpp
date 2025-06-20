/* Home Wallet
 *
 * Module: Configuration manager
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLocale>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif
#include <QTextCodec>

#include "configmanager.h"
#include "globals.h"

ConfigManager::ConfigManager()
    :settings(0)
{}

ConfigManager::~ConfigManager()
{
    if (settings)
        delete settings;
}

void ConfigManager::prepare()
{
    // Check if program distribution is portable
    QString portableIniPath = QDir::toNativeSeparators(qApp->applicationDirPath() + "/") + "homewallet.ini";
    settings = new QSettings(portableIniPath, QSettings::IniFormat);
    bool isPortable = settings->value("General/IsPortable", false).toBool();
    // If no, use standard config location
    if (!isPortable) {
        delete settings;
        settings = new QSettings("DarkHobbit", "homewallet");
        settings->setIniCodec(QTextCodec::codecForName("UTF-8"));
    }
}

void ConfigManager::setDefaults(const QString &tableFont, const QString &gridColor1, const QString &gridColor2)
{
    gd.tableFont = tableFont;
    gd.gridColor1 = gridColor1;
    gd.gridColor2 = gridColor2;
}

void ConfigManager::readConfig()
{
    if (!settings)
        return;
    // View
    gd.fullScreenMode = settings->value("View/FullScreenMode", false).toBool();
    gd.showTableGrid = settings->value("View/ShowTableGrid", false).toBool();
    gd.showLineNumbers = settings->value("View/ShowLineNumbers", false).toBool();
    gd.resizeTableRowsToContents = settings->value("View/ResizeTableRowsToContents", false).toBool();
    gd.showSumsWithCurrency = settings->value("View/ShowSumsWithCurrency", true).toBool();
    gd.useTableAlternateColors = settings->value("View/UseTableAlternateColors", true).toBool();
    gd.useSystemFontsAndColors = settings->value("View/UseSystemFontsAndColors", false).toBool();
    gd.tableFont = settings->value("View/TableFont", gd.tableFont).toString();
    gd.gridColor1 = settings->value("View/GridColor1", gd.gridColor1).toString();
    gd.gridColor2 = settings->value("View/GridColor2", gd.gridColor2).toString();
    // Locale
    gd.dateFormat = settings->value("Locale/DateFormat", QLocale::system().dateFormat(QLocale::ShortFormat)).toString();
    gd.timeFormat = settings->value("Locale/TimeFormat", QLocale::system().timeFormat(QLocale::LongFormat)).toString();
    gd.useSystemDateTimeFormat = settings->value("Locale/UseSystemDateTimeFormat", false).toBool();
    updateFormats();
    // Filter & Sort
    gd.applyQuickFilterImmediately = settings->value("Filter/ApplyQuickFilterImmediately", false).toBool();
    gd.filterDatesOnStartup = (GlobalConfig::FilterDatesOnStartup)enFilterDatesOnStartup.load(settings);
    gd.monthsInFilter = settings->value("Filter/MonthsInFilter", 12).toInt();
    gd.saveCategoriesInFilter = settings->value("Filter/SaveCategoriesInFilter", true).toBool();
    gd.enableSorting = settings->value("Filter/EnableSorting", true).toBool();
}

void ConfigManager::writeConfig()
{
    if (!settings)
        return;
    // View
    settings->setValue("View/FullScreenMode", gd.fullScreenMode);
    settings->setValue("View/ShowTableGrid", gd.showTableGrid);
    settings->setValue("View/ShowLineNumbers", gd.showLineNumbers);
    settings->setValue("View/ResizeTableRowsToContents", gd.resizeTableRowsToContents);
    settings->setValue("View/ShowSumsWithCurrency", gd.showSumsWithCurrency);
    settings->setValue("View/UseTableAlternateColors", gd.useTableAlternateColors);
    settings->setValue("View/UseSystemFontsAndColors", gd.useSystemFontsAndColors);
    settings->setValue("View/TableFont", gd.tableFont);
    settings->setValue("View/GridColor1", gd.gridColor1);
    settings->setValue("View/GridColor2", gd.gridColor2);
    // Locale
    updateFormats();
    settings->setValue("Locale/DateFormat", gd.dateFormat);
    settings->setValue("Locale/TimeFormat", gd.timeFormat);
    settings->setValue("Locale/UseSystemDateTimeFormat", gd.useSystemDateTimeFormat);
    // Filter & Sort
    settings->setValue("Filter/ApplyQuickFilterImmediately", gd.applyQuickFilterImmediately);
    enFilterDatesOnStartup.save(settings, gd.filterDatesOnStartup);
    settings->setValue("Filter/MonthsInFilter", gd.monthsInFilter);
    settings->setValue("Filter/SaveCategoriesInFilter", gd.saveCategoriesInFilter);
    settings->setValue("Filter/EnableSorting", gd.enableSorting);
    settings->sync();
}

void ConfigManager::readTableConfig(FilteredQueryModel *model)
{
    if (!settings)
        return;
    QString sectName = sectionForModel(model);
    int visibleColumnCount = settings->value(sectName + "/ColumnsCount", 0).toInt();
    QStringList cols;
    for (int i=0; i<visibleColumnCount; i++)
        cols << settings->value(sectName + QString("/Column%1").arg(i+1)).toString();
    model->setVisibleColumns(cols);
}

void ConfigManager::writeTableConfig(FilteredQueryModel *model)
{
    if (!settings)
        return;
    QString sectName = sectionForModel(model);
    QStringList cols = model->getVisibleColumns();
    settings->setValue(sectName + "/ColumnsCount", cols.count());
    for (int i=0; i<cols.count(); i++)
        settings->setValue(sectName + QString("/Column%1").arg(i+1), cols[i]);
}

void ConfigManager::readDateFilter(bool& useDateFrom, bool& useDateTo, QDate &dtFilterFrom, QDate &dtFilterTo)
{
    if (!settings)
        return;
    useDateFrom = settings->value("Filter/UseDateFrom").toBool();
    useDateTo = settings->value("Filter/UseDateTo").toBool();
    dtFilterFrom =
        QDate::fromString(settings->value("Filter/DateFrom").toString(),
            Qt::ISODate);
    dtFilterTo =
        QDate::fromString(settings->value("Filter/DateTo").toString(),
            Qt::ISODate);
}

void ConfigManager::writeDateFilter(const bool& useDateFrom, const bool& useDateTo, const QDate &dtFilterFrom, const QDate &dtFilterTo)
{
    if (!settings)
        return;
    settings->setValue("Filter/UseDateFrom", useDateFrom);
    settings->setValue("Filter/UseDateTo", useDateTo);
    settings->setValue("Filter/DateFrom", dtFilterFrom.toString(Qt::ISODate));
    settings->setValue("Filter/DateTo", dtFilterTo.toString(Qt::ISODate));
}

void ConfigManager::readCategoriesFilter(FilteredQueryModel* model, QString &category, QString &subcategory)
{
    if (!settings)
        return;
    QString sectName = sectionForModel(model);
    category = settings->value(sectName + "/FilterCategory").toString();
    subcategory = settings->value(sectName + "/FilterSubcategory").toString();
}

void ConfigManager::writeCategoriesFilter(FilteredQueryModel* model, const QString &category, const QString &subcategory)
{
    if (!settings)
        return;
    QString sectName = sectionForModel(model);
    settings->setValue(sectName + "/FilterCategory", category);
    settings->setValue(sectName + "/FilterSubcategory", subcategory);
}

QString ConfigManager::readLanguage()
{
    if (!settings)
        return "";
    return settings->value("General/Language", "").toString();
}

void ConfigManager::writeLanguage(const QString &language)
{
    if (!settings)
        return;
    settings->setValue("General/Language", language);
}

QString ConfigManager::lastImportedFile()
{
    if (!settings)
        return "";
    return settings->value("General/LastImportedFile", defaultDocDir()).toString();
}

void ConfigManager::setLastImportedFile(const QString &path)
{
    if (!settings)
        return;
    settings->setValue("General/LastImportedFile", path);
}

QString ConfigManager::localDatabaseDir()
{
    if (!settings)
        return "";
    const QString sep = QDir::separator();
    return settings->value("General/LocalDatabaseDir", defaultDocDir()+sep+"HomeWallet"+sep+"db").toString();
}

void ConfigManager::setLocalDatabaseDir(const QString &dir)
{
    if (!settings)
        return;
    settings->setValue("General/LocalDatabaseDir", dir);
}

QString ConfigManager::defaultDocDir()
{
    return
        #if QT_VERSION >= 0x050000
                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        #else
                QDesktopServices::storageLocation( QDesktopServices::DocumentsLocation);
        #endif
}

void ConfigManager::updateFormats()
{
    if (gd.useSystemDateTimeFormat) {
        gd.dateFormat = QLocale::system().dateFormat();
        gd.timeFormat = QLocale::system().timeFormat();
    }
}

QString ConfigManager::sectionForModel(FilteredQueryModel *model)
{
    QString res = model->objectName();
    res.remove("mdl");
    return QString("TableOf") + res;
}

ConfigManager configManager;
