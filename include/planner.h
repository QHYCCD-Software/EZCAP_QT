#ifndef PLANNER_H
#define PLANNER_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class Planner;
}

class Planner : public QDialog
{
    Q_OBJECT

public:
    explicit Planner(QWidget *parent = 0);
    ~Planner();

    /*检查文件路径是否正确*/
    bool CheckFilePath(QString path);
    /*写fit文件*/
    void FITwrite(QString namez,int ImageWidth,int ImageHeight,unsigned char *Buf);

    QString getStatus_planner();
    void setStatus_planner(QString qstr);


    void saveASPLN(QString name);
    void loadPLN(QString name);

signals:

public slots:
    /*开始/停止任务表*/
    void on_pBtn_start_planner_clicked();
    void on_pBtn_forceStop_planner_clicked();

private slots:
    /*table 相关按钮及功能的槽*/
    void on_pBtn_loadTable_clicked();
    void on_pBtn_saveTable_clicked();
    void on_tableWidget_planner_cellClicked(int row, int column);
    void on_cBox_colorWheel_planner_clicked();
    void on_pBtn_addRow_clicked();

    /*选择文件路径*/
    void on_pBtn_folder_planner_clicked();

    /*修改文件名*/
    void changeFileName(QString str);
    void on_pBtn_ngc_clicked();
    void on_pBtn_m_clicked();
    void on_pBtn_ic_clicked();
    void on_pBtn_clear_clicked();
    void on_pBtn_1_planner_clicked();
    void on_pBtn_2_planner_clicked();
    void on_pBtn_3_planner_clicked();
    void on_pBtn_4_planner_clicked();
    void on_pBtn_5_planner_clicked();
    void on_pBtn_6_planner_clicked();
    void on_pBtn_7_planner_clicked();
    void on_pBtn_8_planner_clicked();
    void on_pBtn_9_planner_clicked();
    void on_pBtn_0_planner_clicked();
    void on_pBtn_lParentheses_planner_clicked();
    void on_pBtn_rParentheses_planner_clicked();

    /*高速模式设置*/
    void on_cBox_highReadSpeed_planner_clicked();
    /*暗场模式*/
    void on_cBox_darkFrame_planner_clicked();
    /*颜色条被点击*/
    void colorBar1_planner_clicked();

    void resetUI();//recieve the changeLanguage signal

    void errorInfo_showed(QString title,QString errorInfo);//show the error info

    void camera_connected();

    void baisImages_finished();

    void setRowColor(int row, QColor color);

    void cfwSetup_updated();

protected:
    void mousePressEvent(QMouseEvent *e);

public:
    Ui::Planner *ui;

    int colorSign;//记录当前颜色条颜色  1 yellow, 2 green, 3 red, 4 blue
    bool firstRowIsBais;

};

extern Planner *planner_dialog;

#endif // PLANNER_H
