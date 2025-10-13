/* Home Wallet
 *
 * Module: Dialog for export details
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include "formats/formatfactory.h"

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(FormatFactory* _factory, QWidget *parent = nullptr);
    ~ExportDialog();
    FileFormat* getCurrentFormat();
    FileFormat::SubTypeFlags getSubTypes();
    QString getPath();
    bool isDir();

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_cbFormat_currentIndexChanged(int);
    void on_btnSelectAll_clicked();
    void on_btnUnselectAll_clicked();
    virtual void accept();
    void on_btnSelectPath_clicked();

private:
    Ui::ExportDialog *ui;
    FormatFactory* factory;
    FileFormat* currentFormat;
    void setSubTypeEnabled(QCheckBox* cb, FileFormat::SubType typeFlag);
    void selectSubTypeIfEnabled(QCheckBox* cb);
    void checkSubType(FileFormat::SubTypeFlags subTypes, QCheckBox* cb, FileFormat::SubType typeFlag);
};

#endif // EXPORTDIALOG_H
