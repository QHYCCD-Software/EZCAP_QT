#ifndef MANAGEMENTMENU_H
#define MANAGEMENTMENU_H

#include <QWidget>
#include <QMenu>

namespace Ui {
class ManagementMenu;
}

class ManagementMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ManagementMenu(QWidget *parent = 0);
    ~ManagementMenu();

    Ui::ManagementMenu *ui;

    QMenu *cmenu_captureExp;//capture曝光滑动条 右键菜单

private slots:
    void on_head_preview_clicked(bool checked);

    void on_head_focus_clicked(bool checked);

    void on_head_capture_clicked(bool checked);

    void on_head_hist_clicked(bool checked);

    void on_head_screenView_clicked(bool checked);

    void on_hSlider_gain_preview_valueChanged(int value);

    void on_hSlider_offset_preview_valueChanged(int value);

    void on_hSlider_exposure_preview_valueChanged(int value);

    void on_hSlider_gain_focus_valueChanged(int value);

    void on_hSlider_offset_focus_valueChanged(int value);

    void on_hSlider_exposure_focus_valueChanged(int value);

    void on_hSlider_gain_capture_valueChanged(int value);

    void on_hSlider_offset_capture_valueChanged(int value);

    void on_hSlider_exposure_capture_valueChanged(int value);

    void on_pBtn_expUnit_capture_clicked();

    void on_hSlider_exposure_capture_customContextMenuRequested(const QPoint &pos);

    void setCaptureExp1s();
    void setCaptureExp5s();
    void setCaptureExp10s();
    void setCaptureExp30s();
    void setCaptureExp60s();
    void setCaptureExp120s();
    void setCaptureExp180s();
    void setCaptureExp240s();
    void setCaptureExp5min();
    void setCaptureExp10min();
    void setCaptureExp15min();
    void setCaptureExp30min();
    void setCaptureExp0s();

    void on_checkBox_highSpeed_toggled(bool checked);

    void on_hSlider_wPos_valueChanged(int value);

    void on_hSlider_bPos_valueChanged(int value);

    void on_pBtn_coarse_clicked();

    void on_cBox_autoStretchList_currentIndexChanged(int index);

    void resetUI();//recieve the changeLanguage signal

    void camera_connected();
    void camera_disconnected();

    void on_bin1x1_toggled(bool checked);

    void on_bin2x2_toggled(bool checked);

    void on_bin3x3_toggled(bool checked);

    void on_bin4x4_toggled(bool checked);

signals:
    void switchWorkMode(int workmode);

private:

};

extern ManagementMenu *managerMenu;

#endif // MANAGEMENTMENU_H
