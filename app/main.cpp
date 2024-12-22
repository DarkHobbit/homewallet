/* Home Wallet
 *
 * Module: Main
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
#include <QDir>
#include <QMessageBox>
#include <QTranslator>

#include "configmanager.h"
#include "globals.h"
#include "languagemanager.h"
#include "languageselectdialog.h"
#include "mainwindow.h"
#include "pathmanager.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    configManager.prepare();
    pathManager.prepare();
    // Set UI language
    QString language = configManager.readLanguage();
    if (!languageManager.load(pathManager.transPath()+QDir::separator()+QString("iso639-1.utf8")))
        QMessageBox::warning(0, S_WARNING, "Language list loading error");
    else {
        if (language.isEmpty()) {
            language = LanguageSelectDialog::selectLanguage();
            configManager.writeLanguage(language);
        }
        bool qtOk;
        if (!languageManager.loadCodecs(language, qtOk))
            QMessageBox::warning(0, S_WARNING, "UI loading error");
        else {
            // It's not error, because not all supported languages have qt_*.ts files
            /*if (!qtOk)
                QMessageBox::warning(0, S_WARNING, "Qt standard UI loading error");*/
            // TODO here add initialization of any translated lists, as in DC
        }
    }
    // Arguments parse TODO m.b. move this code to separate file for QML support
    gd.fullScreenMode = false;
    for (int i=1; i<qApp->arguments().count(); i++) { // foreach not work, because arg0 is program name
        const QString ar = qApp->arguments()[i];
        if (ar=="--fullscreen" || ar=="-f")
            gd.fullScreenMode = true;
    }
    // Main Window
    MainWindow w;
    if (!w.db.isOpen())
        return 1;
    if (gd.fullScreenMode)
        w.showMaximized();
    else
        w.show();
    return a.exec();
}
