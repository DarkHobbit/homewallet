/* Home Wallet
 *
 * Module: About dialog
 *
 * Copyright 2023 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <limits.h>
#include <QApplication>

#include "pathmanager.h"
#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent, const QString& dbInfo) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    // Version
    ui->lbVer->setText(QString("v ") + qApp->applicationVersion());
    // Contributors
    ui->lwContributors->addItem("Icons made by:");
    ui->lwContributors->addItem("");
    ui->lwContributors->setItemWidget(ui->lwContributors->item(1), new QLabel("<a href=\"https://www.flaticon.com/free-icons/google-plus\" title=\"google-plus icons\">Google-plus icons created by Pixel perfect - Flaticon</a>"));
    ui->lwContributors->addItem("");
    ui->lwContributors->setItemWidget(ui->lwContributors->item(2),  new QLabel("<a href=\"https://www.flaticon.com/free-icons/pencil\" title=\"pencil icons\">Pencil icons created by Freepik - Flaticon</a>"));
    ui->lwContributors->addItem("");
    ui->lwContributors->setItemWidget(ui->lwContributors->item(3),  new QLabel("<a href=\"https://www.flaticon.com/free-icons/stop\" title=\"stop icons\">Stop icons created by Alfredo Hernandez - Flaticon</a>"));
    /*
    ui->lwContributors->addItem("Different languages translations: " \
     "???");
    ui->lwContributors->addItem("French UI: ???");
    ui->lwContributors->addItem("Bugfixes: ???");
    */
    // Additional info
    QString compiler;
#if defined __GNUC_MINOR__
    compiler = QString("GCC %1.%2").arg(__GNUC__).arg(__GNUC_MINOR__);
#if defined __GNUC_PATCHLEVEL__
    compiler += QString(".%1").arg(__GNUC_PATCHLEVEL__);
#endif
#endif
#ifdef __WORDSIZE
    compiler += QString(" word size: %1 bit").arg(__WORDSIZE);
#endif
    if (!compiler.isEmpty())
        ui->lbCompilerValue->setText(compiler);
    ui->lbTrasPathValue->setText(pathManager.transPath());
    ui->lbDbInfo->setText(dbInfo);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
