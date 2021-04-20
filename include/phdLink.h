#ifndef PHDLINK_H
#define PHDLINK_H
//#include "include/dithercontrol.h"
#include "include/dllqhyccd.h"
#include <QDialog>

namespace Ui {
class PHDLink;
}

class PHDLink : public QDialog
{
    Q_OBJECT

public:
    explicit PHDLink(QWidget *parent = 0);
    ~PHDLink();

    bool IsDitherEnabled();
    int GetDitherLevel();
    double GetSettleRange();

    bool isPHDConnect;

private slots:
    void on_pBtn_connect_clicked();

    void on_pushButton_clicked();

    void aTimer_timeout();

private:
    Ui::PHDLink *ui;

    QTimer *aTimer;//温度控制定时器
};

extern class PHDLink *phdLink_dialog;//定义全局类对象，供主界面类中使用

#endif // PHDLINK_H
