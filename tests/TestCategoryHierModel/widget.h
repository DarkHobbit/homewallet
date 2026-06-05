#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "categoryhiermodel.h"
#include "hwdatabase.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget() override;

private slots:
    void on_btnAdd_clicked();
    void on_btnRemove_clicked();

private:
    Ui::Widget *ui;
    HwDatabase db;
    CategoryHierModel* mC;
};
#endif // WIDGET_H
