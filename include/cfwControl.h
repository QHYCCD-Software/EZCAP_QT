#ifndef CFWCONTROL_H
#define CFWCONTROL_H

#include <QDialog>

namespace Ui {
class CFWControl;
}

class CFWControl : public QDialog
{
    Q_OBJECT

public:
    explicit CFWControl(QWidget *parent = 0);
    ~CFWControl();

    void loadFilterSetup();
    void startCFWProgressBar();


private slots:
    void on_pushButton_setupCFW_clicked();

    void on_comboBox_cfwPos_currentIndexChanged(int index);

    void filterNames_updated();

    void curCFWPos_changed(int dstIndex);

    void camera_connected();

signals:
    void changeCFWPosition();
    void endCFWProgress();

private:
    Ui::CFWControl *ui;

    bool filterLoaded;
};

extern class CFWControl *cfwControl_dialog;
#endif // CFWCONTROL_H
