#include <QApplication>
#include "progresswindow.h"
#include "ui_progresswindow.h"

ProgressWindow::ProgressWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProgressWindow)
{
    ui->setupUi(this);
}

ProgressWindow::~ProgressWindow()
{
    delete ui;
}

void ProgressWindow::onProgressUpdate(int progress, const QString &stage)
{
    ui->progressBar->setValue(progress);
    ui->lbStage->setText(stage);
    qApp->processEvents();
}

void ProgressWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
