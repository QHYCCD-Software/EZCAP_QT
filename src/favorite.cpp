#include "favorite.h"
#include "ui_favorite.h"
#include "include/dllqhyccd.h"

//#include "qhyccdStatus.h"
#include "myStruct.h"

#include <QDebug>
#include <QException>

Favorite *favorite_dialog;
extern qhyccd_handle *camhandle;

Favorite::Favorite(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Favorite)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//fixed the size of the window
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar
}

Favorite::~Favorite()
{
    delete ui;
}


void Favorite::on_cBox_trigIn_clicked(bool checked)
{
    ix.trigerInOrOut = checked;
}

void Favorite::on_spinBox_usbtraffic_valueChanged(int arg1)
{
    ix.usbtraffic = arg1;
}

//------------------------finetone 3210-------------------------------
/**
 * 改变Fine Tone的值
 * @brief Favorite::setFineToneValue
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 */
void Favorite::setFineToneValue(int value1, int value2, int value3, int value4,int value5, int value6, int value7, int value8)
{
    if(value1 < 0)
        value1 = ui->hSlider_1_favorite->value();
    if(value2 < 0)
        value2 = ui->hSlider_2_favorite->value();
    if(value3 < 0)
        value3 = ui->hSlider_3_favorite->value();
    if(value4 < 0)
        value4 = ui->hSlider_4_favorite->value();
    if(value5 < 0)
        value5 = ui->hSlider_5_favorite->value();
    if(value6 < 0)
        value6 = ui->hSlider_6_favorite->value();
    if(value7 < 0)
        value7 = ui->hSlider_7_favorite->value();
    if(value8 < 0)
        value8 = ui->hSlider_8_favorite->value();

    int value = (value1 * 64 + value2 * 16 + value3 * 4 + value4) * 256
            + value5 * 64 + value6 * 16 + value7 * 4 + value8;

    ui->label_fineTone_favorite->setText(QString::number(value));
}

/**
 * Fine Tone中value1值改变
 * @brief Favorite::on_hSlider_1_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_1_favorite_valueChanged(int value)
{
    setFineToneValue(value,-1,-1,-1,-1,-1,-1,-1);
}

/**
 * Fine Tone中value2值改变
 * @brief Favorite::on_hSlider_2_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_2_favorite_valueChanged(int value)
{
    setFineToneValue(-1,value,-1,-1,-1,-1,-1,-1);
}

/**
 * Fine Tone中value3值改变
 * @brief Favorite::on_hSlider_3_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_3_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,value,-1,-1,-1,-1,-1);
}

/**
 * Fine Tone中value4值改变
 * @brief Favorite::on_hSlider_4_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_4_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,value,-1,-1,-1,-1);
}

/**
 * Fine Tone中value5值改变
 * @brief Favorite::on_hSlider_5_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_5_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,-1,value,-1,-1,-1);
}

/**
 * Fine Tone中value6值改变
 * @brief Favorite::on_hSlider_6_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_6_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,-1,-1,value,-1,-1);
}

/**
 * Fine Tone中value7值改变
 * @brief Favorite::on_hSlider_7_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_7_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,-1,-1,-1,value,-1);
}

/**
 * Fine Tone中value8值改变
 * @brief Favorite::on_hSlider_8_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_8_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,-1,-1,-1,-1,value);
}

void Favorite::on_saveFineTone_favorite_clicked()
{
    //qDebug() << "";
}

//----------------------finetone 9979--------------------------------
void Favorite::on_hSlider_1_favorite_9979_valueChanged(int value)
{
    ui->finetone1Value_9979->setText(QString::number(value));

    set9979Finetone();
}

void Favorite::on_hSlider_2_favorite_9979_valueChanged(int value)
{
    ui->finetone2Value_9979->setText(QString::number(value));

    set9979Finetone();
}

void Favorite::on_hSlider_3_favorite_9979_valueChanged(int value)
{
    ui->finetone3Value_9979->setText(QString::number(value));

    set9979Finetone();
}

void Favorite::on_cBox_SHPorSHD_9979_toggled(bool checked)
{
    qDebug() << "SHP_SHD_9979" << checked;
    set9979Finetone();
}

void Favorite::set9979Finetone()
{
    unsigned int finetone1, finetone2, finetone3, SHPorSHD;

    finetone1 = ui->hSlider_1_favorite_9979->value();
    finetone2 = ui->hSlider_2_favorite_9979->value();
    finetone3 = ui->hSlider_3_favorite_9979->value();
    if(ui->cBox_SHPorSHD_9979->isChecked())
        SHPorSHD = 1; // view SHP
    else
        SHPorSHD = 0;  // view SHD
    libqhyccd->SetQHYCCDFineTone(camhandle, SHPorSHD, finetone1, finetone2, finetone3);
    //SetQHYCCDFineTone(camhandle, SHPorSHD, finetone1, finetone2, finetone3);
    qDebug() << "set finetone" << SHPorSHD << finetone1 << finetone2 << finetone3;
}

//--------------------------------------------------------------------

void Favorite::setFineToneEnable(bool enable)
{
    //ui->gBox_fineTone->setEnabled(enable);
    ui->tabWidget_finetone->setVisible(enable);
}

void Favorite::setMotorHeatingEnable(bool enable)
{
    //ui->gBox_shutterMotor->setEnabled(enable);
    ui->gBox_shutterMotor->setVisible(enable);
}

void Favorite::camera_connected()
{
    //usbtraffic init
    ui->label_usbtraffic->setVisible(ix.canUsbtraffic);
    ui->spinBox_usbtraffic->setVisible(ix.canUsbtraffic);
    if(ix.canUsbtraffic)
    {
        ui->spinBox_usbtraffic->setMaximum(ix.usbtrafficMax);
        ui->spinBox_usbtraffic->setMinimum(ix.usbtrafficMin);
        ui->spinBox_usbtraffic->setSingleStep(ix.usbtrafficStep);
        ui->spinBox_usbtraffic->setValue(ix.usbtraffic);
    }

    //trig-in
    ui->cBox_trigIn->setVisible(ix.canTriger);

    //20200512lyl GPSon
    ui->cBox_GPS->setVisible(ix.canGPS);
    ui->comboBox_OSD->setVisible(ix.canGPS);

    //fineton
    this->setFineToneEnable(ix.canFineTone);

    //motorHeating
    this->setMotorHeatingEnable(ix.canMotorHeating);

    //tec protect
    ui->cBox_TEC->setVisible(ix.canTecOverProtect);

    //calmp
    ui->cBox_signalClamp->setVisible(ix.canSignalClamp);

    //calibrate FPN
    ui->grpBox_calibrateFPN->setVisible(ix.canCalibrateFPN);

    //chip temp
    ui->grpBox_chipTemp->setVisible(ix.canChipTemp);

    //slowest download
    ui->cBox_slowDownload->setVisible(ix.canSlowestDownload);

    //Overscan Calibration
    ui->spinBox_calConstant->setValue(ix.calConstant);

    //20201127 lyl SensorChamberCyclePUMP
    ui->grpBox_SensorChamberCycle_PUMP->setVisible(ix.canContolSensorChamberCyclePUMP);
}

void Favorite::resetUI()
{
    ui->retranslateUi(this);
}

void Favorite::on_spinBox_calConstant_valueChanged(int arg1)
{
    ix.calConstant = arg1;
    iniFileParams.calConstant = ix.calConstant;
}
//20200512lyl GPSon
void Favorite::on_cBox_GPS_clicked(bool checked)
{
    ix.isGPSon=checked;
}
//20200512lyl OSD
void Favorite::on_comboBox_OSD_currentIndexChanged(int index)
{
    unsigned int ret;
    if(libqhyccd->EnableQHYCCDImageOSD){
        ret = libqhyccd->EnableQHYCCDImageOSD(camhandle,index);//ret = EnableQHYCCDImageOSD(camhandle,index);
        if(ret != QHYCCD_SUCCESS)
            qCritical("EnableQHYCCDImageOSD: failed");
        else
            qDebug() << "EnableQHYCCDImageOSD success"<<index;
    }
   else{
    qCritical("EnableQHYCCDImageOSD: have no this function !");
    }
//    try{
//        //ret = EnableQHYCCDImageOSD(camhandle,index);
//        ret = libqhyccd->EnableQHYCCDImageOSD(camhandle,index);
//        if(ret != QHYCCD_SUCCESS)
//            qCritical("EnableQHYCCDImageOSD: failed");
//        else
//            qDebug() << "EnableQHYCCDImageOSD success"<<index;
//    }
//    catch (QException e) {
//    qCritical("EnableQHYCCDImageOSD: have no this function !");
//}
}
