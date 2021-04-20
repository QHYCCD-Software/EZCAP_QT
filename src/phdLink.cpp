#include "phdLink.h"
#include "ui_phdLink.h"
//#include "qhyccdStatus.h"
#include <QDebug>
#include <QTimer>
#include <QListView>

PHDLink *phdLink_dialog;

PHDLink::PHDLink(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PHDLink)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//fixed the size of the window
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    ui->comboBox_ditherLevel->setView(new QListView());
    ui->comboBox_settleRange->setView(new QListView());

    isPHDConnect = false;
    ui->grpBox_durDownload->setEnabled(false);
    ui->grpBox_ditherSetting->setEnabled(false);
    ui->pBtn_connect->setText(tr("Connect"));
    ui->label_connInfo->setText(tr("No Connection"));
    ui->label_connInfo->setStyleSheet("color: rgb(170, 0, 0);");

    aTimer = new QTimer();//温度 计时器
    aTimer->setInterval(500);//设置定时器时间间隔1s
    connect(aTimer, SIGNAL(timeout()), this, SLOT(aTimer_timeout()));

    aTimer->start();
}

PHDLink::~PHDLink()
{
    delete ui;
}

void PHDLink::on_pBtn_connect_clicked()
{
    int ret;

    if(!isPHDConnect)
    {
        //ret = Connect2PHD2();  //connect PHD2
        ret = libqhyccd->Connect2PHD2();
        if(ret == 0)
        {
            qDebug() << "connect phd2";
            isPHDConnect = true;

            ui->pBtn_connect->setText(tr("Disconnect"));
            ui->label_connInfo->setText(tr("Connection OK"));
            ui->label_connInfo->setStyleSheet("color: rgb(0, 170, 0);");

            ui->grpBox_durDownload->setEnabled(true);
            ui->grpBox_ditherSetting->setEnabled(true);
        }
        else
        {
            qDebug() << "Connect failed, PHD2 maybe not running.";
        }
    }
    else
    {
        //ret = DisConnect2PHD2();    //Dis PHD2
        ret = libqhyccd->DisConnect2PHD2();
        if(ret == 0)
        {
            qDebug() << "disconnect PHD2";
            isPHDConnect = false;

            ui->pBtn_connect->setText(tr("Connect"));
            ui->label_connInfo->setText(tr("No Connection"));
            ui->label_connInfo->setStyleSheet("color: rgb(170, 0, 0);");

            ui->grpBox_durDownload->setEnabled(false);
            ui->grpBox_ditherSetting->setEnabled(false);
        }
    }

}

bool PHDLink::IsDitherEnabled()
{
    bool ditherEnabled = false;
    if(isPHDConnect)
    {
        if(ui->comboBox_ditherLevel->currentIndex() == 0)
        {
            ditherEnabled = false;
        }
        else
        {
            ditherEnabled = true;
        }
    }

    return ditherEnabled;
}

int PHDLink::GetDitherLevel()
{
    int dither = 0;
    switch(ui->comboBox_ditherLevel->currentIndex())
    {
        case 0:
            dither = 1;
        break;
        case 1:
            dither = 2;
        break;
        case 2:
            dither = 3;
        break;
        case 3:
            dither = 4;
        break;
        case 4:
            dither = 5;
        break;
        case 5:
            dither = 6;
        break;
    }

    return dither;
}

double PHDLink::GetSettleRange()
{
    double settle = 0.1;
    switch(ui->comboBox_settleRange->currentIndex())
    {
        case 0:
            settle = 0.1;
        break;
        case 1:
            settle = 0.2;
        break;
        case 2:
            settle = 0.3;
        break;
        case 3:
            settle = 0.4;
        break;
        case 4:
            settle = 0.5;
        break;
        case 5:
            settle = 0.6;
        break;
        case 6:
            settle = 0.7;
        break;
        case 7:
            settle = 0.8;
        break;
        case 8:
            settle = 0.9;
        break;
        case 9:
            settle = 1.0;
        break;
        case 10:
            settle = 1.1;
        break;
        case 11:
            settle = 1.2;
        break;
        case 12:
            settle = 1.3;
        break;
        case 13:
            settle = 1.4;
        break;
        case 14:
            settle = 1.5;
        break;
    }

    return settle;
}

void PHDLink::on_pushButton_clicked()
{
/*
    if(IsDitherEnabled())
    {

        int dither = GetDitherLevel();
        double settle = GetSettleRange();

        qDebug() << dither << settle;

        int ret = ControlPHD2Dither(dither, settle);    //Control PHD2 Dither
        if(ret == 0)
        {

        }
    }
    */
}

void PHDLink::aTimer_timeout()
{
    /*
    if(isPHDConnect)
    {
        QString recStr;
        char str[1024];
        int ret = CheckPHD2Status(str);
        if(ret == 0)
        {
            recStr = QString(QLatin1String(str));
            qDebug() << recStr;
            if(recStr.compare(QString("SettleDone"), recStr) == 0)
            {
                qDebug() << "get SettleDone status, Dither Done.";
            }
        }
    }
    QApplication::processEvents();
    */
}

