#ifndef READMODE_H
#define READMODE_H

#include <QDialog>

class EZCAP;
namespace Ui {
class ReadMode;
}

class ReadMode : public QDialog
{
    Q_OBJECT

public:
    explicit ReadMode(QWidget *parent = 0);
    ~ReadMode();

private slots:
    //void on_cancelpBtn_readmode_clicked();

    void on_comboBox_readmode_currentIndexChanged(int index);

    void resetUI();
    void on_okpBtn_readmode_clicked();

public://private
    Ui::ReadMode *ui;
};
extern class ReadMode *readMode;
#endif // READMODE_H
