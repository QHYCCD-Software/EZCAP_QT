#ifndef EZCAP_H
#define EZCAP_H

#include <QMainWindow>
#include <QThread>
#include <QEvent>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QVBoxLayout>
#include <QTranslator>
#include <myStruct.h>
#include <qhyccdstruct.h>
#include "disktools.h"
#ifdef WIN32
#include <windows.h>
//#endif
//#ifdef LINUX
//#include <sys/stat.h>
#else
#include <unistd.h>
#endif

class BorderLayout;

class QImage;
class QScrollArea;
class QScrollBar;
class QLabel;
class QMenu;

class MenuForm;
class DownloadPreThread;
class DownloadCapThread;
class DownloadFocThread;

//-----------执行计划任务线程类-------
class ExecutePlanTable : public QThread
{
    Q_OBJECT

protected:
    void run();

signals:
    void changeRowColor(int row, QColor color);
    void startCaptureImage();
    //void showCFWProBar();
    void showErrorInfo(QString,QString);
    void finish_baisImages();

    void changeCurCFWPos(int dstIndex);

};

//-----------发送滤镜轮控制命令-------
class ExecuteCFWOrder : public QThread
{
    Q_OBJECT

protected:
    void run();

public:
};

//------------查询色轮状态线程-----------

//---------------------------

namespace Ui {
class EZCAP;
}
/**
 * @brief The EZCAP class
 */
class EZCAP : public QMainWindow
{
    Q_OBJECT

public:
    explicit EZCAP(QWidget *parent = 0);
    ~EZCAP();

    QString EZCAP_VER;
    QString EZCAP_VER_SHORT;
    QString RELEASE_TIME;
    static const bool TESTED_PID;
    //static const int CFW_WAITING_TIME;

    bool getParamsFromCamera();//连接相机后，设置界面控件初始状态
    void scaleImage(double factor);//图像显示规模改变（zoom中设置）

    /*IplImage转QImage*/
    QImage *IplImageToQImage(const IplImage *iplImage);
    /*QImage转IplImage*/
    IplImage *QImageToIplImage(const QImage *qImage);
    /*显示capture图像*/
    void displayCaptureImage(int x,int y);
    void displayCaptureImage_Ex(int x, int y, unsigned char *dataBuf);
    /*显示screenview图像*/
    void displayScreenViewImage(int boxWidth,int boxHeight,int boxX,int boxY);
    void screenViewBoxResize();
    /*显示直方图*/
    void displayHistogramImage(int x,int y,unsigned char *buf);
    /*显示preview图像*/
    void displayPreviewImage(int x,int y,int boxWidth,int boxHeight,int boxX,int boxY);
    void displayPreviewImage_Ex(int x, int y, int boxW, int boxH, int boxCx, int boxCy, unsigned char *dataBuf);
    /*显示focus图像*/
    void displayFocusImage(int x,int y);
    void displayFocusImage_Ex(int x, int y, unsigned char *dataBuf);
    /*显示FocusAssistant区域的图像*/
    void displayFocusAssistantImage(IplImage *image);
    /*获取Focus信息值*/
    void FWHMFocus(IplImage *Img,FOCUSINFO &FocusInfo);
    /*在QImage上绘制网格线*/
    void DrawGridBox(QImage *img);
    /*伪彩色转换*/
    void FalseColorConvert(IplImage *inputImg,IplImage *outputImg);
    /*加载伪彩色图片*/
    void LoadFalseColor(QString LUTMAP);
    /*检查路径是否安全*/
    bool CheckWritePath(QString filename);
    /*保存界面设置*/
    void saveParasAsIni();
    /*加载界面设置*/
    bool loadParasFromIni();

    void languageChanged();

    void setStretchLUT(unsigned short W, unsigned short B);


    void getOverScanBlack(unsigned char *Buf, int x, int y);
    void getImageInfo(unsigned char *Buf,int ImageWidth,int ImageHeight, int startX,int startY,int sizeX,int sizeY, float &std,float &rms,double &max,double &min);
    int calibrateOverscan(unsigned char* inbuf, unsigned char*outbuf, int ImgW, int ImgH, int OSStartX, int OSStartY, int OSSizeX, int OSSizeY);
    void ImageAnalyze(IplImage *Img,int x,int y);
    QString AutoFileName();
    //------温度控制定时器------------
    void startTimerTemp();
    void stopTimerTemp();
    //-------色轮状态控制定时器-------
    void startCFWTimer();
    void stopCFWTimer();
    //-------dither状态查询定时器 启动控制----
    void startDitherTimer();
    void stopDitherTimer();
    //-------循环泵定时器------
    void startPumpTimer();
    void stopPumpTimer();


    void updateWindowsTitle();
    void pnpEventFunc();
private slots:
    /*File 菜单中的槽*/
    void saveAsFIT();
    void saveAsBMP();
    void saveAsJPG();
    void openFolder();
    void showFITHeaderEditor();
    void ignoreOverScanAreaClicked(bool checked);
    void calibrateOverScanClicked(bool checked);
    void exitMainWindow();
    /*connect action 槽*/
    void showCameraChooser();
    /*planner 菜单中的槽*/
    void showPlanTable();
    /*camera setup 菜单中的槽*/
    void showFavoriteSetting();
    void showPHDLink();
    void showCaptureDarkFrameTool();
    void showTempControl();
    void cfwPositionChanged();
    void showCFWControl();
    /*help 菜单中的槽*/
    void showAbout();
    /*zoom 菜单中的槽*/
    void zoomFitWindow();
    void zoom0_25X();
    void zoom0_5X();
    void zoom0_75X();
    void zoom1X();
    void zoom1_5X();
    void zoom2X();

    //preview/focus/capture当前活动页改变响应槽函数
    void currentWorkingModeChanged(int workmode);

    //preview tab页的槽函数
    void mgrMenu_pBtn_preview_clicked();
    void mgrMenu_pBtn_live_preview_clicked();
    void mgrMenu_pBtn_cross_clicked();
    void mgrMenu_pBtn_grid_clicked();
    void mgrMenu_pBtn_circle_clicked();

    //focus tab页的槽函数
    void mgrMenu_pBtn_focus_clicked();
    void mgrMenu_pBtn_live_focus_clicked();

    //focus assistant页槽函数
    void on_pBtn_linear_clicked();
    void on_pBtn_thermal_clicked();
    void on_pBtn_false_clicked();
    void on_pBtn_invert_clicked();
    void focusAssistantImageDblClick();

    //capture tab页的槽函数
    void mgrMenu_pBtn_capture_clicked();
    void mgrMenu_pBtn_stop_clicked();
    void mgrMenu_hSlider_bPos_sliderReleased();
    void mgrMenu_hSlider_wPos_sliderReleased();
    void mgrMenu_pBtn_stretchMinusB_clicked();
    void mgrMenu_pBtn_stretchPlusB_clicked();
    void mgrMenu_pBtn_stretchMinusW_clicked();
    void mgrMenu_pBtn_stretchPlusW_clicked();
    void mgrMenu_pBtn_auto_histogram_clicked();

    void favorite_pBtn_calibrateFrame_clicked();
    void favorite_pBtn_getRealTemp_clicked();
    void favorite_pBtn_controlSensorChamberCyclePUMP_clicked();

    /*screenview 鼠标左击响应槽*/
    void screenViewAreaMouseDown(int posX, int posY);
    
    /*点击图像区域*/
    void displayedImageMouseDown(int posX, int posY);
    /*图像显示区域label_imgShow 右键菜单*/
    void on_label_ImgShow_customContextMenuRequested(const QPoint &pos);

    void goFocusCenter();
    //void doTempControl();
    //定时器响应槽
    void tempTimer_timeout();
    void cfwTimer_timeout();
    void ditherTimer_timeout();
    void PumpTimer_timeout();
    //切换显示语言
    void changeToEnglish();
    void changeToChinese();
    void changeToJapanese();
    void activeTestMode();
    void activeLive();
    void switchDebug();
    void switchTestGuid();

    void hScrollBarValueChanged(int value);
    void vScrollBarValueChanged(int value);

    void camera_connected();
    void camera_disconnected();

signals:
    //改变显示语言信号
    void changeLanguage();
    void connect_camera();
    void disconnect_camera();

    void change_fitHeaderInfo();

protected:
    void closeEvent(QCloseEvent * event );

    void resizeEvent(QResizeEvent *);

    bool eventFilter(QObject *target, QEvent *event);

    void mouseMoveEvent(QMouseEvent *e);


public:
    Ui::EZCAP *ui;

    QTranslator *translator;

    BorderLayout *mainLayout;
    QVBoxLayout *managerLayout;

    //----------定义线程对象-------------
    ExecutePlanTable *exePlanTable;
    DownloadPreThread *downloadPre;
    DownloadCapThread *downloadCap;
    DownloadFocThread *downloadFoc;

   //-----------滤镜轮调用API改成阻塞型，因此需要放入线程执行-------------
    ExecuteCFWOrder *runCFWOrder;

    //主管理面板的滚动条区域对象
    QScrollArea *scrollArea_ImgShow;//显示图像的滚动条区域对象

    QImage *qImg_capture;
    QImage *qImg_preview;
    QImage *qImg_focus;
    QLabel *statusLabel_imgSize;//显示当前图像分辨率的label对象
    QLabel *statusLabel_mousePos;//显示当前鼠标位置坐标的label对象
    QLabel *statusLabel_rgb;//显示当前位置图像rgb值的对象
    QLabel *statusLabel_Temp;//显示当前相机温度
    QLabel *statusLabel_RH;//显示当前密封腔内的湿度
    QLabel *statusLabel_PRESS;//显示当前压力
    QLabel *statusLabel_msg;//显示提示信息
    QLabel *statusLabel_SDKmsg;//显示提示信息
    QMenu *cmenu_captureExp;//capture曝光滑动条 右键菜单
    QMenu *cmenu_imgArea;//图像显示区域右键菜单

    MenuForm *menuForm;
    QTimer *tempTimer;//温度控制定时器
    QTimer *cfwTimer;//色轮状态查询定时器
    QTimer *ditherTimer; //PHD2 dither状态查询定时器
    QTimer *PumpTimer;//循环泵开关定时器
    int pumpcount;

    //IplImage *Img;//用于存储整体的8BIT图像，显示
    IplImage *captureImg;
    IplImage *ScreenViewImg;//用于显示整体屏幕
    double scaleFactor; //缩放比例

    IplImage *ImgRAW;
    IplImage *ImgColor;

    //---Focus
    unsigned short FocusCenterX_Pre;  //the focus center point in Preview image
    unsigned short FocusCenterY_Pre;

    int focusAreaStartX;   //focus area
    int focusAreaStartY;
    int focusAreaSizeX;
    int focusAreaSizeY;

    //---FocusAssistant
    int ZoomFocus_X;  //focus center point in FocusAssistantImage
    int ZoomFocus_Y;
    bool FocusZoomMode;
    int FocusCurveX;
    int fwhm_x;//focus assistant中fwhm图的起始点坐标
    int fwhm_y;
    int peak_x;//focus assistant中peak图的起始点坐标
    int peak_y;

    //-----stretch
    unsigned short OverScanRMS; //overscan black value, used for Auto hist.

    unsigned short Preview_WPOS;
    unsigned short Preview_BPOS;
    unsigned short Focus_WPOS;
    unsigned short Focus_BPOS;
    unsigned short Capture_WPOS;
    unsigned short Capture_BPOS;

    //----temperature control
    bool Flag_Timer; // 用于温度控制，每秒一次，然后分频为2秒，其中第一秒为取数，第二秒为设置
    bool Flag_Timer_2; //温度控制   Timer分频 ，每4秒一次
    int CurveX;
    int posX_tempImg;//温度曲线图的起始点坐标
    int posY_tempImg;
    int posX_tempImg_RH;//湿度曲线图起始点坐标
    int posY_tempImg_RH;
    int posX_tempImg_Press;//压力曲线图起始点坐标
    int posY_tempImg_Press;

    //-------rectange area size and center point in the screen view------
    int viewBoxCX;
    int viewBoxCY;
    int viewBoxW;
    int viewBoxH;

    bool isSettleDone; //标记dither功能中，Settle是否完成

    //save
    QString lastSavedPath;

    int hScrollBarValue_Pre;
    int vScrollBarValue_Pre;
    int hScrollBarValue_Cap;
    int vScrollBarValue_Cap;
    bool needCalcScrollBarValue;  //标志是否需要重新计算滚动条位置， 第一次点击拍摄或者切换binning模式时需要重新计算滚动条位置
    bool noImgInWorkMode;  //标志当前工作模式下是否已拍摄了图片，用于避免从preview拍摄图像后，切换到capture点击拍摄，未拍摄完成时进行灰度拉伸会刷新显示preview中拍摄的图片。

    QStringList devList; //list to hold on the id of the cameras    
    //QString filetemp;//20201125lyl温度湿度压力数据保存
};

extern class EZCAP *mainWidget;//define global class object

#endif // EZCAP_H
