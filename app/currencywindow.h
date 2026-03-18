/* Home Wallet
 *
 * Module: Currency window
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#ifndef CURRENCYWINDOW_H
#define CURRENCYWINDOW_H

#include <QWidget>
#include "helpers.h"
#include "hwdatabase.h"

namespace Ui {
class CurrencyWindow;
}

class CurrencyWindow : public QWidget, public SelecTables
{
    Q_OBJECT

public:
    explicit CurrencyWindow(QWidget *parent, HwDatabase& db);
    ~CurrencyWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CurrencyWindow *ui;
};

#endif // CURRENCYWINDOW_H
