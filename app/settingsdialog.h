#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent);
    ~SettingsDialog();
    bool setData();
    bool getData();
    bool langChanged();
private slots:
    void on_cbUseSystemDateTimeFormat_clicked(bool checked);
    void on_cbUseSystemFontsAndColors_clicked(bool checked);
    void on_btnTableFont_clicked();
    void on_btnGridColor1_clicked();
    void on_btnGridColor2_clicked();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SettingsDialog *ui;
    QString _lang;
    bool _langChanged;
};

#endif // SETTINGSDIALOG_H
