#ifndef FAVORITE_H
#define FAVORITE_H

#include <QDialog>

namespace Ui {
class Favorite;
}

class Favorite : public QDialog
{
    Q_OBJECT

public:
    explicit Favorite(QWidget *parent = 0);
    ~Favorite();

    /*设置motor heating 可控*/
    void setMotorHeatingEnable(bool enable);
    /*设置fine tone 可控*/
    void setFineToneEnable(bool enable);

    void set9979Finetone();

private slots:
    void setFineToneValue(int value1, int value2, int value3, int value4, int value5, int value6, int value7, int value8);

    void on_hSlider_1_favorite_valueChanged(int value);

    void on_hSlider_2_favorite_valueChanged(int value);

    void on_hSlider_3_favorite_valueChanged(int value);

    void on_hSlider_4_favorite_valueChanged(int value);

    void on_hSlider_5_favorite_valueChanged(int value);

    void on_hSlider_6_favorite_valueChanged(int value);

    void on_hSlider_7_favorite_valueChanged(int value);

    void on_hSlider_8_favorite_valueChanged(int value);

    void resetUI();//recieve the changeLanguage signal

    void on_saveFineTone_favorite_clicked();

    void on_hSlider_1_favorite_9979_valueChanged(int value);

    void on_hSlider_2_favorite_9979_valueChanged(int value);

    void on_hSlider_3_favorite_9979_valueChanged(int value);

    void on_cBox_SHPorSHD_9979_toggled(bool checked);

    void on_cBox_trigIn_clicked(bool checked);

    void on_spinBox_usbtraffic_valueChanged(int arg1);

    void camera_connected();

    void on_spinBox_calConstant_valueChanged(int arg1);

    void on_cBox_GPS_clicked(bool checked);

    void on_comboBox_OSD_currentIndexChanged(int index);

public:
    Ui::Favorite *ui;
};

extern class Favorite *favorite_dialog;//定义全局类对象，供主界面类中使用

#endif // FAVORITE_H
