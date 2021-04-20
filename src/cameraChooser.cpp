#include "include/cameraChooser.h"
#include "ui_cameraChooser.h"
#include "ezCap.h"
//20200220 lyl Add ReadMode Dialog
//#include "readmode.h"
//#include "ui_readmode.h"
#include <QException>

//#include "qhyccdStatus.h"
#include "include/dllqhyccd.h"
#include <QDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QtCore/QtGlobal>

extern struct IX ix;

extern qhyccd_handle *camhandle;

CameraChooser *cameraChooser;

//char camid[64];
extern char camid[64];

CameraChooser::CameraChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraChooser)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    mainWidget = (EZCAP*)this->parentWidget();//获取父窗口指针
    
    //获取SDK支持的相机列表
#ifdef Q_OS_MAC
    int ret;
    QString path = QCoreApplication::applicationDirPath();
    ret =libqhyccd->OSXInitQHYCCDFirmware((char*)path.toStdString().c_str());//ret = OSXInitQHYCCDFirmware((char*)path.toStdString().c_str());
    if(ret == QHYCCD_SUCCESS)
    {
        sleep(5);
    }

#endif

    //20200226 lyl  add readmode
    //ui->comboBox_readmode->setDisabled(false);


}

CameraChooser::~CameraChooser()
{
    delete ui;
}

/**
 * @brief CameraChooser::on_coBox_cameraChooser_currentIndexChanged        
 */
void CameraChooser::on_coBox_cameraChooser_currentIndexChanged(const QString &arg1)
{
    qDebug() << "--------------- +1 ----------------"<<arg1;
    if(arg1 == "")//No Camera
    {
        qDebug() << "no camera selected";
        ui->okBtn_cameraChooser->setEnabled(false);
    }
    else
    {
        qDebug() << "--------------- +1 --------else--------";
        //set ok button enable
        ui->okBtn_cameraChooser->setEnabled(true);
        //test
        QByteArray pstr;
        int ret = QHYCCD_ERROR;
        memset(camid,'\0',64);
        pstr = ui->coBox_cameraChooser->currentText().toLocal8Bit();
        memcpy(camid,pstr.data(),pstr.size());
        qDebug() << "current camera:" << camid;
        camhandle = libqhyccd->OpenQHYCCD(camid);
        //camhandle = OpenQHYCCD(camid);
        if(camhandle != NULL)
        {
            try{
                ret=libqhyccd->GetQHYCCDNumberOfReadModes(camhandle,&ix.NumberOfReadModes);
                //ret=GetQHYCCDNumberOfReadModes(camhandle,&ix.NumberOfReadModes);
            }
            catch(QException e)
            {
                qCritical("GetQHYCCDNumberOfReadModes: have no this function !");
            }
            if(ret == QHYCCD_SUCCESS)
            {
                ix.ReadModeList.clear();//清空列表
                //qDebug() << "current GetQHYCCDNumberOfReadModes:"<<ix.NumberOfReadModes;
                for(int i=0; i<ix.NumberOfReadModes; i++)
                {
                    if (libqhyccd->GetQHYCCDReadModeName(camhandle, i, ix.currentReadModeName) == 0)
                    //if (GetQHYCCDReadModeName(camhandle, i, ix.currentReadModeName) == 0)
                        ix.ReadModeList.append(ix.currentReadModeName);//列表填充
                }
                ui->comboBox_readmode->clear();
                ui->comboBox_readmode->addItems(ix.ReadModeList);
            }
            else
                qDebug() << "GetQHYCCDNumberOfReadModes failed";
        }
        libqhyccd->CloseQHYCCD(camhandle);
        //CloseQHYCCD(camhandle);
        //ReleaseQHYCCDResource();
    }
}

/**
 * @brief CameraChooser::on_cancelBtn_cameraChooser_clicked       
 */
void CameraChooser::on_cancelBtn_cameraChooser_clicked()
{
    this->close();
}

/**
 * @brief CameraChooser::on_okBtn_cameraChooser_clicked 
 */        
void CameraChooser::on_okBtn_cameraChooser_clicked()
{
    QByteArray pstr;
    int ret = QHYCCD_ERROR;

    memset(camid,'\0',64);
    pstr = ui->coBox_cameraChooser->currentText().toLocal8Bit();

    memcpy(camid,pstr.data(),pstr.size());
    qDebug() << "chose camera:" << camid;
    camhandle =libqhyccd-> OpenQHYCCD(camid);
    //camhandle = OpenQHYCCD(camid);
    if(camhandle != NULL)
    {
        //qDebug() << "OpenQHYCCD success";
        ix.isConnected = true;
        ix.CamID = QString(camid);
        ix.CamModel = ix.CamID.left(ix.CamID.lastIndexOf('-'));
        /*//20200220 lyl Add ReadMode Dialog
        ix.ReadModeList.clear();//清空列表
        ret=GetQHYCCDNumberOfReadModes(camhandle,&ix.NumberOfReadModes);
        if(ret == QHYCCD_SUCCESS)
        {
            readMode = new ReadMode(this);
            qDebug() << "GetQHYCCDNumberOfReadModes:"<<ix.NumberOfReadModes;
            for(int i=0; i<ix.NumberOfReadModes; i++)
            {
                if (GetQHYCCDReadModeName(camhandle, i, ix.currentReadModeName) == 0)
                    ix.ReadModeList.append(ix.currentReadModeName);//列表填充
            }
            //20200220 lyl Add ReadMode 可在此处增加对话框显示，选定readmode之后进行SetQHYCCDReadMode()
            readMode->ui->comboBox_readmode->clear();
            readMode->ui->comboBox_readmode->addItems(ix.ReadModeList);
            readMode->setWindowModality(Qt::ApplicationModal);//设置父窗口不可控
            qDebug() << "Show readMode";//模态对话框//readMode->setModal(true);//readMode->show();
            readMode->exec();
        }
        else
            qDebug() << "GetQHYCCDNumberOfReadModes failed";*/

        //test
        //ui->comboBox_readmode->setDisabled(true);
        //ret = SetQHYCCDReadMode(camhandle, ix.currentReadMode);
        ret = libqhyccd->SetQHYCCDReadMode(camhandle, ix.currentReadMode);
        if(ret != QHYCCD_SUCCESS)
            qCritical("SetQHYCCDStreamMode: failed");
        else
            qDebug() << "current SetQHYCCDStreamMode success"<<ix.currentReadMode;
        ret = libqhyccd->SetQHYCCDStreamMode(camhandle, 0);
        //ret = SetQHYCCDStreamMode(camhandle, 0);
        if(ret != QHYCCD_SUCCESS)
            qCritical("SetQHYCCDStreamMode: failed");
        else
            qDebug() << "SetQHYCCDStreamMode success";
        ret =libqhyccd-> InitQHYCCD(camhandle);
       // ret = InitQHYCCD(camhandle);
        if(ret != QHYCCD_SUCCESS)
        {
            qCritical("InitQHYCCD: failed");
            QMessageBox::critical(this,tr("Error"),tr("Camera can not connect as init failure!"),QMessageBox::Ok);
        }
        else
        {
            qDebug() << "InitQHYCCD success";
            if(!mainWidget->getParamsFromCamera()){
                qCritical() << "getParamsFromCamera Error";
                return;
            }//设置界面控件初始状态

            emit connect_camera();
        }
    }
    else
    {
        qCritical("OpenQHYCCD: failure");
        QMessageBox::critical(this,tr("Warning"),tr("Camera Connect Failure!"),QMessageBox::Ok);
    }
    //ui->comboBox_readmode->setDisabled(false);
}

/**
 * @brief CameraChooser::on_logo_cameraChooser_clicked      
 */
void CameraChooser::on_logo_cameraChooser_clicked()
{
    QDesktopServices::openUrl(QUrl("http://www.qhyccd.com"));
}

void CameraChooser::resetUI()
{
    ui->retranslateUi(this);
}

void CameraChooser::on_comboBox_readmode_currentIndexChanged(int index)
{
    //qDebug()<<"cuurent   comboBox  readmode IndexChanged() start ";
    if(index == -1)
    {
        qDebug() << "no readmode selected";//ui->okpBtn_readmode->setEnabled(false);
    }
    else
    {
        qDebug() << "current index:"<<index;
        ix.currentReadMode=index;
    }
    //qDebug()<<"current   comboBox  readmode IndexChanged() end";
}
