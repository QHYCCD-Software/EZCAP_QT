#include "tempControl.h"
#include "ui_tempControl.h"
#include "ezCap.h"

TempControl *tempControl_dialog;
extern struct IX ix;

TempControl::TempControl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TempControl)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//fixed size of window
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar
    /*//单选按钮win10显示不明显测试
    ui->rBtn_manuel->setStyleSheet("QRadioButton::indicator {width:15px;height:15px;border-radius:7px}"
                                      "QRadioButton::indicator:checked {background-color:green;}"
                                      "QRadioButton::indicator:unchecked {background-color:red;}"
                                      );
    ui->rBtn_autoControl->setStyleSheet("QRadioButton::indicator {width:15px;height:15px;border-radius:7px}"
                                      "QRadioButton::indicator:checked {background-color:green;}"
                                      "QRadioButton::indicator:unchecked {background-color:red;}"
                                      );*/
}

TempControl::TempControl(QWidget *parent, const bool showPID) :
    QDialog(parent),
    ui(new Ui::TempControl)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//fixed size of window
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    //set the default value of temp slider and power slider
    int tempValue = ui->vSlider_temp_tempCotrol->value()-50;
    ui->label_temp_tempControl->setText(QString::number(tempValue));
    double powerValue = (ui->vSlider_power_tempControl->value()) * 100.0 / 255.0;
    ui->label_power_tempControl->setText((QString::number(powerValue, 'f', 1)) + "%");
    //set the default value of humidity slider
    double RHValue = (ui->vSlider_Humidity_tempCotrol->value());
    ui->label_Humidity_tempControl->setText((QString::number(RHValue, 'f', 1)) + "%");
    //set the default value of press slider
    double PressValue = (ui->vSlider_Press_tempCotrol->value())*10.0;
    ui->label_Press_tempControl->setText((QString::number(PressValue, 'f', 1)) + "mbar");

    //cooler worked with manual mode in default
    ui->vSlider_Humidity_tempCotrol->setEnabled(false);
    ui->vSlider_temp_tempCotrol->setEnabled(false);
    ui->vSlider_power_tempControl->setEnabled(true);
    ui->vSlider_power_tempControl->setStyleSheet("QSlider::groove:vertical{ \
                                                        height: 12px; \
                                                        left: 5px; \
                                                        right: 5px; \
                                                      } \
                                        QSlider::handle:vertical{ \
                                                        border-radius: 20px; \
                                                        width:  40px; \
                                                        height: 40px; \
                                                        margin-top: -10px; \
                                                        margin-left: -10px; \
                                                        margin-bottom: -20px; \
                                                        margin-right: -10px; \
                                                        } \
                                        QSlider::sub-page:vertical{border-image: url(:/qslider.png);}");
    ui->vSlider_Press_tempCotrol->setEnabled(false);/*border-image: url(:/qslider_bg.png);*///border-image:url(:/qslider_btn.png);
    ix.coolerMode = 1;

    //-----------------init the image show in label_image_tempControl中---------------------
    //if not init, it will occur error when draw the temp curve
    QImage img_tempControl = QPixmap(":/image/black.bmp").toImage();
    mainWidget->DrawGridBox(&img_tempControl);
    ui->label_image_tempControl->setPixmap(QPixmap::fromImage(img_tempControl));

    ui->label_P->setVisible(showPID);
    ui->label_I->setVisible(showPID);
    ui->label_D->setVisible(showPID);
    ui->doubleSpinBox_P->setVisible(showPID);
    ui->doubleSpinBox_I->setVisible(showPID);
    ui->doubleSpinBox_D->setVisible(showPID);
    if(showPID)
    {
        this->setFixedHeight(292);
    }
    else
    {
        this->setFixedHeight(252);
    }
}

TempControl::~TempControl()
{
    delete ui;
}

/**
 * 响应temp slider值改变
 * @brief TempControl::on_vSlider_temp_tempCotrol_valueChanged
 * @param value
 */
void TempControl::on_vSlider_temp_tempCotrol_valueChanged(int value)
{
    ui->label_temp_tempControl->setText(QString::number(value-50));
}

/**
 * 响应power slider值改变
 * @brief TempControl::on_vSlider_power_tempControl_valueChanged
 * @param value
 */
void TempControl::on_vSlider_power_tempControl_valueChanged(int value)
{
    ui->label_power_tempControl->setText(QString("%1%").arg(QString::number(value * 100 / 255.0, 'f', 1)));
}

/**
 * 响应Humidity slider值改变
 * @brief TempControl::on_vSlider_Humidity_tempControl_valueChanged
 * @param value
 */
void TempControl::on_vSlider_Humidity_tempCotrol_valueChanged(int value)
{
    ui->label_Humidity_tempControl->setText(QString("%1%").arg(QString::number(value,'f', 1)));
}

void TempControl::resetUI()
{
    ui->retranslateUi(this);
}

void TempControl::on_rBtn_TECOFF_toggled(bool checked)
{
    if(checked)
    {
        ix.coolerMode = 0; // cooler disabled
        ui->vSlider_power_tempControl->setEnabled(false);
        ui->vSlider_temp_tempCotrol->setEnabled(false);
    }
}

void TempControl::on_rBtn_manuel_toggled(bool checked)
{
    if(checked)
    {
        ix.coolerMode = 1; // cooler worked in manual mode
        ui->vSlider_temp_tempCotrol->setEnabled(false);
        ui->vSlider_power_tempControl->setEnabled(true);
    }
}

void TempControl::on_rBtn_autoControl_toggled(bool checked)
{
    if(checked)
    {
        ix.coolerMode = 2; // auto control temp
        ui->vSlider_power_tempControl->setEnabled(false);
        ui->vSlider_temp_tempCotrol->setEnabled(true);
    }
}

void TempControl::on_vSlider_Press_tempCotrol_valueChanged(int value)
{
    ui->label_Press_tempControl->setText(QString::number(value*10.0));
}
