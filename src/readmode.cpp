#include "include/readmode.h"
#include "ui_readmode.h"
#include "ezCap.h"
//#include "qhyccdStatus.h"
#include "include/dllqhyccd.h"
#include <QDebug>
#include <QListView>
extern struct IX ix;
extern qhyccd_handle *camhandle;
ReadMode *readMode;

ReadMode::ReadMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReadMode)
{
    //qDebug()<<"ReadMode::ReadMode()";
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->comboBox_readmode->setView(new QListView());
    mainWidget = (EZCAP*)this->parentWidget();//获取父窗口指针
    //qDebug()<<"ReadMode::ReadMode() end";
}

ReadMode::~ReadMode()
{
    //qDebug()<<"ReadMode::~~ReadMode()";
    delete ui;
    //qDebug()<<"ReadMode::~~ReadMode() end";
}

void ReadMode::on_comboBox_readmode_currentIndexChanged(int index)
{
    //qDebug()<<"ReadMode::on_comboBox_readmode_currentIndexChanged()";
    if(index == -1)
    {
        qDebug() << "no readmode selected";
        ui->okpBtn_readmode->setEnabled(false);
    }
    else
    {
        //qDebug() << "------------ManagementMenu::on_comboBox_readmode_currentIndexChanged()-----------";
        ix.currentReadMode=index;
        ui->okpBtn_readmode->setEnabled(true);
    }
    //qDebug()<<"ReadMode::on_comboBox_readmode_currentIndexChanged() end";
}
void ReadMode::resetUI()
{
    //qDebug()<<"ReadMode::resetUI()";
    ui->retranslateUi(this);
}

void ReadMode::on_okpBtn_readmode_clicked()
{
    //qDebug()<<"ReadMode::ok  pBtn  clicked()";
    unsigned int ret;
    ui->comboBox_readmode->setDisabled(true);
    ret = libqhyccd->SetQHYCCDReadMode(camhandle, ix.currentReadMode);
    //ret = SetQHYCCDReadMode(camhandle, ix.currentReadMode);
    if(ret != QHYCCD_SUCCESS)
        qCritical("SetQHYCCDReadMode: failed");
    else
        qDebug() << "SetQHYCCDReadMode success"<<ix.currentReadMode;
    /*unsigned int test,width,height;
    ret = GetQHYCCDReadMode(camhandle, &test);
    if(ret != QHYCCD_SUCCESS)
        qCritical("GetQHYCCDStreamMode: failed");
    else
        qDebug() << "GetQHYCCDStreamMode success"<<test;
    ret=GetQHYCCDReadModeResolution(camhandle,test,&width, &height);
    if(ret != QHYCCD_SUCCESS)
        qCritical("GetQHYCCDReadModeResolution: failed");
    else
        qDebug() << "GetQHYCCDReadModeResolution success"<<test<<"width:"<<width<<"height:"<<height;*/
    ui->okpBtn_readmode->setEnabled(false);
    //ui->cancelpBtn_readmode->setEnabled(false);
    //qDebug()<<"ReadMode::ok  pBtn  clicked() end";
    this->close();//20200226 lyl close dialog after setting
}
