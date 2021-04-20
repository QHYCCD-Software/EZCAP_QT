#ifndef TEMPCONTROL_H
#define TEMPCONTROL_H

#include <QDialog>

namespace Ui {
class TempControl;
}

class TempControl : public QDialog
{
    Q_OBJECT

public:
    explicit TempControl(QWidget *parent = 0);
    TempControl(QWidget *parent, const bool showPID);
    ~TempControl();

private slots:
    void on_vSlider_temp_tempCotrol_valueChanged(int value);

    void on_vSlider_power_tempControl_valueChanged(int value);

    void on_vSlider_Humidity_tempCotrol_valueChanged(int value);

    void resetUI();//recieve the changeLanguage signal

    void on_rBtn_TECOFF_toggled(bool checked);

    void on_rBtn_manuel_toggled(bool checked);

    void on_rBtn_autoControl_toggled(bool checked);



    void on_vSlider_Press_tempCotrol_valueChanged(int value);

public:
    Ui::TempControl *ui;
};

extern class TempControl *tempControl_dialog;

#endif // TEMPCONTROL_H
