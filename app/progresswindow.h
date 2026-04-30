#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QWidget>

namespace Ui {
class ProgressWindow;
}

class ProgressWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressWindow(QWidget *parent = nullptr);
    ~ProgressWindow();

public slots:
    void onProgressUpdate(int progress, const QString& stage);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ProgressWindow *ui;
};

#endif // PROGRESSWINDOW_H
