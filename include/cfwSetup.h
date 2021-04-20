#ifndef CFWSETUP_H
#define CFWSETUP_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class CFWSetup;
}

class CFWSetup : public QDialog
{
    Q_OBJECT

public:
    explicit CFWSetup(QWidget *parent = 0);
    ~CFWSetup();

    void loadFilterSetup();

private slots:
    void on_spinBox_filterSlotsNum_valueChanged(int arg1);

    void on_tableWidget_cfwSetup_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

    void on_tableWidget_cfwSetup_itemChanged(QTableWidgetItem *item);

    void on_pushButton_cancle_clicked();

    void on_pushButton_ok_clicked();

signals:
    void updateFilterNames();

private:
    Ui::CFWSetup *ui;

    QString previousText;
};

extern class CFWSetup *cfwSetup_dialog;
#endif // CFWSETUP_H
