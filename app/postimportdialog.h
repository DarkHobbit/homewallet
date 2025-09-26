/* Home Wallet
 *
 * Module: Dialog for interactive import
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#ifndef POSTIMPORTDIALOG_H
#define POSTIMPORTDIALOG_H

#include <QDialog>
#include <QTableView>

#include "hwdatabase.h"
#include "formats/interactiveformat.h"
#include "importmodelset.h"

namespace Ui {
class PostImportDialog;
}

class PostImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PostImportDialog(QWidget *parent);
    ~PostImportDialog();
    void setData(InteractiveFormat* _intFile, HwDatabase* _db);

private:
    Ui::PostImportDialog *ui;
    InteractiveFormat* intFile;
    HwDatabase* db;
    ImportModelSet* mSet;
    // Potentially unsafe pointers (covered by activeTab() in all changed)
    ImportCandidatesModel* activeModel;
    QTableView* activeView;
    // End of potentially unsafe pointers

    enum ActiveTab {
        atIncomes,
        atExpenses,
        atReceipt,
        atTransfer,
        atDebtor,
        atCreditor,
    };

    virtual void showEvent(QShowEvent*);
    ActiveTab activeTab();
private slots:
    void setOkAccessibility();
    void setAddAliasAccessibility();
    void on_actExpCandState_triggered();
    void on_btnAddAlias_clicked();
    void on_actAddAlias_triggered();
};

#endif // POSTIMPORTDIALOG_H
