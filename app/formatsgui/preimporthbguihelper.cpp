/* Home Wallet
 *
 * Module: Pre-import GUI helper for Home Bookkeeping (keepsoft.ru) file formats
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QSqlError>
#include <QTableWidget>
#include <QVBoxLayout>

#include "genericdatabase.h"
#include "globals.h"
#include "helpers.h"
#include "preimporthbguihelper.h"
#include "formats/xmlhbfile.h"
#include "formats/xlsxrepaymentfile.h"


PreImportHbGuiHelper::PreImportHbGuiHelper()
{}

bool PreImportHbGuiHelper::check(const QString& path, FileFormat *impFile, HwDatabase& db)
{
    // HB-specific stuff (main files)
    XmlHbFile* hbFile = dynamic_cast<XmlHbFile*>(impFile);
    if (hbFile) {
        XmlHbFile::SubType sType = hbFile->fileSubType();
        // Amiguous subType resolve
        if (hbFile->isAmbiguous()) {
            QString categorySamples = hbFile->categorySamples();
            if (categorySamples.isEmpty()) {
                QMessageBox::critical(0, S_ERROR, S_EMPTY_FILE.arg(path));
                return false;
            }
            else {
                // Ambiguous HB file subtype
                QString altType, alt1, alt2;
                XmlHbFile::SubType res1, res2;
                switch (sType) {
                case XmlHbFile::IncomesOrExpenses:
                    altType = S_HB_SELECT_AMBIG_IE;
                    alt1 = S_TREAT_AS_INCOMES;
                    alt2 = S_TREAT_AS_EXPENSES;
                    res1 = XmlHbFile::Incomes;
                    res2 = XmlHbFile::Expenses;
                    break;
                case XmlHbFile::DebtorsOrCreditors:
                    altType = S_HB_SELECT_AMBIG_DC;
                    alt1 = S_TREAT_AS_DEBTORS;
                    alt2 = S_TREAT_AS_CREDITORS;
                    res1 = XmlHbFile::Debtors;
                    res2 = XmlHbFile::Creditors;
                    break;
                case XmlHbFile::IncomeOrExpensePlan:
                default:
                    altType = S_HB_SELECT_AMBIG_PIE;
                    alt1 = S_TREAT_AS_INC_PLAN;
                    alt2 = S_TREAT_AS_EXP_PLAN;
                    res1 = XmlHbFile::IncomePlan;
                    res2 = XmlHbFile::ExpensePlan;
                    break;
                }
                QMessageBox mbAmbig(QMessageBox::Question,
                    S_HB_SELECT_AMBIG_TITLE,
                    S_HB_SELECT_AMBIG_ASK
                        .arg(altType).arg(categorySamples));
                QPushButton* btnAlt1 = mbAmbig.addButton(alt1, QMessageBox::YesRole);
                mbAmbig.addButton(alt2, QMessageBox::NoRole);
                QPushButton* btnCancel = mbAmbig.addButton(QMessageBox::Cancel);
                mbAmbig.exec();
                QAbstractButton* btnRes = mbAmbig.clickedButton();
                if (btnRes==btnCancel) {
                    delete impFile;
                    return false;
                }
                hbFile->setFileSubType((btnRes==btnAlt1) ? res1 : res2);
            }
        }
        // Currency indicies resolve
        if (sType==XmlHbFile::CurrencyRate) {
            int minCurrIndex, maxCurrIndex;
            if (!hbFile->getCurrenciesRange(minCurrIndex, maxCurrIndex)) {
                QMessageBox::critical(0, S_ERROR, QObject::tr("Currency rates not found in file"));
                return false;
            }
            GenericDatabase::DictColl currs;
            db.collectDict(currs, "hw_currency", "full_name");
            QDialog* d = new QDialog(0);
            d->setWindowTitle(QObject::tr("Select Currency sequence in Home Bookkeeping file"));
            QVBoxLayout* l = new QVBoxLayout();
            d->setLayout(l);
            QLabel* lbWarn = new QLabel(QObject::tr("Currency order, main unit and rate directions must be identical!"));
            l->addWidget(lbWarn);
            QTableWidget* tb = new QTableWidget(0);
            tb->setColumnCount(2);
            tb->setRowCount(maxCurrIndex-minCurrIndex+1);
            tb->horizontalHeader()->setStretchLastSection(true);
            tb->setHorizontalHeaderLabels(QStringList()
                << QObject::tr("Order")
                << QObject::tr("HB currency"));
            tb->verticalHeader()->setVisible(false);
            for (int i=0; i<tb->rowCount(); i++) {
                tb->setItem(i, 0, new QTableWidgetItem(QString::number(minCurrIndex+i)));
                QComboBox* cbC = new QComboBox(0);
                fillComboByDict(cbC, currs, false);
                tb->setCellWidget(i, 1, cbC);
            }
            l->addWidget(tb);
            QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            QObject::connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
            QObject::connect(bb, SIGNAL(rejected()), d, SLOT(reject()));
            l->addWidget(bb);
            int res = d->exec();
            if (res==QDialog::Rejected) {
                delete d;
                return false;
            }
            XmlHbFile::CurrIdsByInd ids;
            for (int i=0; i<tb->rowCount(); i++) {
                QComboBox* cbC = dynamic_cast<QComboBox*>(tb->cellWidget(i, 1));
                ids[minCurrIndex+i] = currs[cbC->currentText()];
            }
            hbFile->setCurrIdsByInd(ids);
            delete d;
            if (res==QDialog::Rejected)
                return false;
        }
    }
    // HB-specific stuff (repayment files)
    XlsxRepaymentFile* repFile = dynamic_cast<XlsxRepaymentFile*>(impFile);
    if (repFile) {
        // Make candidates list
        QSqlQuery qCredList(db.sqlDbRef());
        // Either simple model (with combo box), or QInputDialog :(
        if (!qCredList.prepare(
                "select crd.id, crd.op_date, crs.name" \
                " from hw_credit crd " \
                " left join hw_correspondent crs on crd.id_crs=crs.id" \
                " order by op_date desc"))
        {
            QMessageBox::critical(0, S_ERROR, qCredList.lastError().text());
            return false;
        }
        if (!qCredList.exec()) {
            QMessageBox::critical(0, S_ERROR, qCredList.lastError().text());
            return false;
        }
        if (db.queryRecCount(qCredList)==0) {
            QMessageBox::critical(0, S_ERROR, S_ERR_NO_CREDITS);
            return false;
        }
        QStringList items;
        QMap<QString,int> credIds;
        int defaultIndex = -1;
        QDateTime estDate = repFile->getEstimatedOpDate();
        QString estCorr = repFile->getEstimatedCorrName();
        qCredList.first();
        while (qCredList.isValid()) {
            QDateTime dt = qCredList.value(1).toDateTime();
            QString corr = qCredList.value(2).toString();
            QString credName = dt.toString(gd.dateFormat) + " " + corr;
            items << credName;
            credIds[credName] = qCredList.value(0).toInt();
            // Try tune default item to date and correspondent
            if (dt==estDate) {
                if (defaultIndex==-1)
                    defaultIndex = items.count()-1;
                else {
                    if (corr==estCorr)
                        defaultIndex = items.count()-1;
                }
            }

            qCredList.next();
        }
        // Select candidate
        bool ok;
        QString it = QInputDialog::getItem(0,
            QObject::tr("Select credit or debt for this file"),
            path, items, defaultIndex, false, &ok);
        if (!ok || it.isEmpty())
            return false;
        repFile->setCredId(credIds[it]);

    }
    return true;
}
