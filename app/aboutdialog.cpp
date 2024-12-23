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
#include "pathmanager.h"
#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    // Contributors
    ui->setupUi(this);
    //ui->lwContributors->addItem("Icons made by...");

    //ui->lwContributors->addItem("Different languages translations: " \
     //"???");
    //ui->lwContributors->addItem("French UI: ???");
    //ui->lwContributors->addItem("Bugfixes: ???");
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
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
