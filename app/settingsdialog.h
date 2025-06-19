#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QMap>
#include "filteredquerymodel.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent);
    ~SettingsDialog();
    bool setData(FQMlist* dbModels);
    bool getData();
    bool langChanged();
private slots:
    void on_cbUseSystemDateTimeFormat_clicked(bool checked);
    void on_cbUseSystemFontsAndColors_clicked(bool checked);
    void on_btnTableFont_clicked();
    void on_btnGridColor1_clicked();
    void on_btnGridColor2_clicked();

    void on_cbTableNames_currentIndexChanged(int indexOfModel);
    void on_btnAddCol_clicked();
    void on_btnDelCol_clicked();
    void on_btnUpCol_clicked();
    void on_btnDownCol_clicked();
    void on_lwAvailableColumns_itemDoubleClicked(QListWidgetItem *item);
    void on_lwVisibleColumns_itemDoubleClicked(QListWidgetItem *item);

    void on_btnResetColumnToDefaults_clicked();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SettingsDialog *ui;
    QString _lang;
    bool _langChanged;
    bool columnsChanged;
    FilteredQueryModel* prevModel;
    QMap<FilteredQueryModel*, QStringList> columnsChangeCache;
    FQMlist* _dbModels;
};

#endif // SETTINGSDIALOG_H
