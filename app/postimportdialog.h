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
#include "formats/interactiveformat.h"
#include "importmodelset.h"

namespace Ui {
class PostImportDialog;
}

class PostImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PostImportDialog(QWidget *parent = nullptr);
    ~PostImportDialog();
    void setData(ImpCandidates* _cands);

private:
    Ui::PostImportDialog *ui;
    ImpCandidates* cands;
    ImportModelSet* mSet;
private slots:
    void setOkAccessibility();
};

#endif // POSTIMPORTDIALOG_H
