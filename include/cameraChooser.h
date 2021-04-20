#ifndef CAMERACHOOSER_H
#define CAMERACHOOSER_H


#include <QDialog>

//-----前置声明-----
class EZCAP;
//-----------------

namespace Ui {
class CameraChooser;
}

class CameraChooser : public QDialog
{
    Q_OBJECT
public:
    explicit CameraChooser(QWidget *parent = 0);
    ~CameraChooser();

private slots:
    void on_coBox_cameraChooser_currentIndexChanged(const QString &arg1);

    void on_cancelBtn_cameraChooser_clicked();

    void on_okBtn_cameraChooser_clicked();

    void on_logo_cameraChooser_clicked();

    void resetUI();

    void on_comboBox_readmode_currentIndexChanged(int index);

signals:
    void connect_camera();

public:
    Ui::CameraChooser *ui;

};

extern class CameraChooser *cameraChooser;//define global class object

#endif // CAMERACHOOSER_H
