#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simplexlsxreader.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnOpen_clicked()
{
    QString path = QFileDialog::getOpenFileName(0,
                "Select XLSX file",
                qApp->applicationDirPath(),
                "OOXML Excel file (*.xlsx)");
    SimpleXlsxReader reader(path);
    if (reader.read()) {
        if (reader.rowCount(0)==0 || reader.columnCount(0)==0)
            QMessageBox::warning(0, "Warning", "Empty table");
        else {
            ui->table->setRowCount(reader.rowCount(0));
            ui->table->setColumnCount(reader.columnCount(0));
            for (int i=0; i<reader.rowCount(0); i++)
                for (int j=0; j<reader.columnCount(0); j++)
                    ui->table->setItem(i, j, new QTableWidgetItem(reader.cellValue(i, j)));
        }
    }
    else
        QMessageBox::critical(0, "Error", reader.lastError());

}
