/* Home Wallet
 *
 * Module: Dialog for date-based reports
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef SIMPLEREPORTDIALOG_H
#define SIMPLEREPORTDIALOG_H

#include <QDialog>
#include <QBoxLayout>

#include "hwdatabase.h"

namespace Ui {
class SimpleReportDialog;
}

class SimpleReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimpleReportDialog(const QString& title, HwDatabase* db,  QWidget *parent = 0);
    ~SimpleReportDialog();
    QBoxLayout* mainLayout();
    QWidget* lastBaseWidget();
    void getData(QString& path, QDate &dtMin, QDate &dtMax);

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_btnSetPath_clicked();

private:
    Ui::SimpleReportDialog *ui;
};

#endif // SIMPLEREPORTDIALOG_H
