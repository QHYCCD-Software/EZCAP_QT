#include "ezCap.h"
#include "ui_ezCap.h"
#include "mainMenu.h"
#include "borderLayout.h"
#include "planner.h"
#include "ui_planner.h"
#include "favorite.h"
#include "ui_favorite.h"
#include "tempControl.h"
#include "ui_tempControl.h"
#include "phdLink.h"
#include "darkframetool.h"
#include "ui_phdLink.h"
#include "about.h"
#include "fitHeader.h"
#include "ui_fitHeader.h"
#include "cameraChooser.h"
#include "ui_cameraChooser.h"
#include "managementMenu.h"
#include "ui_managementMenu.h"
#include "downloadPreThread.h"
#include "downloadCapThread.h"
#include "downloadFocThread.h"
#include "cfwControl.h"
#include "cfwSetup.h"
//20200220 lyl Add ReadMode Dialog
#include "readmode.h"
#include "ui_readmode.h"

//#include "qhyccdStatus.h"
#include "include/dllqhyccd.h"

#include "qthread.h"

#include <QImage>
#include <qfiledialog.h>
#include <QPainter>
#include <QtCore>
#include <QScrollArea>
#include <QScrollBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMenu>
#include <QDesktopServices>
#include <QException>

EZCAP *mainWidget;
const bool EZCAP::TESTED_PID = false;

struct IX ix;
struct FOCUSINFO FocusInfo;
struct INIFILEPARAM iniFileParams;

CvFont QHYFont;

qhyccd_handle *camhandle;

char camid[64];
bool show_disconnect_confirm_box = true;
uint32_t pnp_counter =0;
uint32_t transfer_error_counter =0;


void EZCAP::updateWindowsTitle() {
    uint8_t superSpeed ='-';
    uint8_t camStatus = '-';
    uint32_t sdk_build_version='-';
    qDebug() << "connected" + ix.isConnected;
    sdk_build_version = libqhyccd->GetQHYCCDSDKBuildVersion();
    if(ix.isConnected){
        qDebug() << "====>GET SPEED";
        superSpeed = libqhyccd->GetCameraIsSuperSpeedFromID(camid);
        superSpeed = superSpeed + 2 +48;//48=0(ascii code) In SDK 0=usb2.0 1=usb3.0 , so speed+=2
        camStatus = libqhyccd->GetCameraStatusFromID(camid)+48;//48=0(ascii code)
    }else{
        ix.CamID = "-";
        ix.FPGAVer = "-";
        ix.FPGAVer1 = "-";
        ix.driverVer = "-";
        statusLabel_Temp->clear();
        statusLabel_RH->clear();
        statusLabel_PRESS->clear();
    }
    this->setWindowTitle(tr("EZCAP[") + EZCAP_VER + "] Cam-ID: [" + ix.CamID +"]   FPGA.A:[" + ix.FPGAVer + "]  "  +"FPGA.B:[" + ix.FPGAVer1 + "] FirmWare:[" + ix.driverVer +"] USB["+QString(superSpeed)+"] Conn[" +QString(camStatus)+"] Build[" + QString::number(sdk_build_version) +"]");
}

void pnpEventExFunc(){
    qDebug() << ".....pnpEventExFunc......";
    pnp_counter++;
    mainWidget->statusLabel_SDKmsg->setText("PNP: " + QString::number(pnp_counter) + "Err: " + QString::number(transfer_error_counter));
    mainWidget->ui->plainTextEdit_debug->appendPlainText(QString("pnpEvent  [%1]").arg(mainWidget->statusLabel_SDKmsg->text()));
}
void transferEventErrorFunc(){
    qDebug() << ".....transferEventErrorFunc......";
    transfer_error_counter++;
    mainWidget->statusLabel_SDKmsg->setText("PNP: " + QString::number(pnp_counter) + "Err: " + QString::number(transfer_error_counter));
    mainWidget->ui->plainTextEdit_debug->appendPlainText(QString("Err  [%1]").arg(mainWidget->statusLabel_SDKmsg->text()));
}
void pnp_Event_In_Func(char *id){
	qDebug() << ".....pnp_Event_In_Func......";
    qDebug( "Cam +  [%s]", id);
    mainWidget->ui->plainTextEdit_debug->appendPlainText(QString("Cam +  [%1]").arg(id));

    QString eventID = QString(id);
    if(ix.isConnected){
        qDebug( "Cam In [%s] , But Cam[%s] already connected ", id, ix.CamID.toStdString().c_str());
        return ;
    }
    if(iniFileParams.autoConnect){
        mainMenuBar->actConnect->trigger();
    }
}
void pnp_Event_Out_Func(char *id){
    qDebug( "Cam -  [%s]", id);
    mainWidget->ui->plainTextEdit_debug->appendPlainText(QString("Cam -  [%1]").arg(id));
    QString eventID = QString(id);
    if(eventID.compare(ix.CamID) != 0){
        qDebug( "Cam Out Compare false  [%s] [%s]", id, ix.CamID.toStdString().c_str());
        return ;
    }
    if(managerMenu->ui->pBtn_live_preview->isEnabled()){
        managerMenu->ui->pBtn_live_preview->click();
        ix.cameraState = Camera_Idle; //todo need to extract a disconnect function and enable the preview button
        #ifdef WIN32
                    Sleep(200);
        #else
                    usleep(200);
        #endif
    }
    show_disconnect_confirm_box = false;
    mainMenuBar->actConnect->trigger(); //Do disconnect
//    show_disconnect_confirm_box = true;
}
void data_Event_Single_Func(char *id, uint8_t *imageData){
    qDebug( "img +Single  [%s]", id);
}
void data_Event_Live_Func(char *id, uint8_t *imageData){
    qDebug( "img +Live  [%s]", id);
}


/**
 * @brief EZCAP::EZCAP
 * @param parent
 */
EZCAP::EZCAP(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EZCAP)
{
    qDebug() << "EZCAP    EZCAP    START-----------";
    //define soft version
    //date in macro is like Nov 21 2020,but in QT, it use local time, mean 十一月 21 2020,  That why we have to use Qlocale
    QLocale macro_locale(QLocale::English, QLocale::UnitedStates);
    qDebug() << "macro date before convert = " + QString(__DATE__);

    QDateTime macro_Date = macro_locale.toDateTime(__DATE__, "MMM dd yyyy");
    QDateTime macro_Date_short = macro_locale.toDateTime(__DATE__, "MMM  d yyyy");
    EZCAP_VER =macro_Date.toString("yyyy-MM-dd");
    EZCAP_VER_SHORT =macro_Date_short.toString("yyyy-MM-dd");
    if(EZCAP_VER_SHORT.length()>0){EZCAP_VER = EZCAP_VER_SHORT;}
    RELEASE_TIME = EZCAP_VER + QStringLiteral("   ") + QStringLiteral(__TIME__);

    ui->setupUi(this);
    libqhyccd =new dllqhyccd();
    //---------------------create the sub-window dialog class---------------------------------------------------
    about_dialog = new About(this,EZCAP_VER,RELEASE_TIME);//pass the version info when create the dialog
    favorite_dialog = new Favorite(this);
    fitHeader_dialog = new FitHeader(this);
    planner_dialog = new Planner(this);
    tempControl_dialog = new TempControl(this, TESTED_PID);
    phdLink_dialog = new PHDLink(this);
    darkFrameTool_dialog = new DarkFrameTool(this);
    cameraChooser = new CameraChooser(this);//need to passed by the father point
    managerMenu = new ManagementMenu(this);
    mainMenuBar = new MainMenu(0);
    cfwControl_dialog = new CFWControl(this);
    cfwSetup_dialog = new CFWSetup(cfwControl_dialog);

    //----------------------------main menu setting-------------------------------------------------------------
    this->setMenuBar(mainMenuBar);
    mainMenuBar->raise();//mainmenu show on the top,suit for Mac os

    //-------------------------manager Menu mainlayout-----------------------------------------------------------
    managerLayout = new QVBoxLayout();
    managerLayout->setMargin(0);
    managerLayout->setSpacing(0);
    managerLayout->setSizeConstraint(QLayout::SetFixedSize);
    managerLayout->addWidget(managerMenu);
    ui->scrollAreaWidgetContents_2->setLayout(managerLayout);
    ui->scrollArea_manager->setWidget(ui->scrollAreaWidgetContents_2);//managerMenu contains the ui

    //------------------add the scroll in the image showing area -------------------------------------------------
    ui->label_ImgShow->setBackgroundRole(QPalette::Base);
    ui->label_ImgShow->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label_ImgShow->setScaledContents(true);//设置QLabel自动缩放,既:显示图像大小自动调整为Qlabel大小
    scrollArea_ImgShow = new QScrollArea(this);
    scrollArea_ImgShow->setObjectName("scrollArea_ImgShow");//设置objectName，用于qss中设置其显示风格
    scrollArea_ImgShow->setBackgroundRole(QPalette::Dark);
    scrollArea_ImgShow->setWidget(ui->label_ImgShow);

    //---------------------状态栏显示区域----------------------------------------------------------
    statusLabel_imgSize = new QLabel(this);//用于显示图像分辨率
    statusLabel_imgSize->setFrameShape(QFrame::NoFrame);
    statusLabel_imgSize->setFixedSize(75,18);
    statusLabel_mousePos = new QLabel(this);//用于显示鼠标指向的图片位置
    statusLabel_mousePos->setFrameShape(QFrame::NoFrame);
    statusLabel_mousePos->setFixedSize(75,18);
    statusLabel_rgb = new QLabel(this);//用于显示鼠标所在位置的rbg值
    statusLabel_rgb->setFrameShape(QFrame::NoFrame);
    statusLabel_rgb->setFixedSize(155,18);
    statusLabel_Temp = new QLabel(this);//显示温度
    statusLabel_Temp->setFrameShape(QFrame::NoFrame);
    statusLabel_Temp->setFixedSize(75, 18);
    statusLabel_RH = new QLabel(this);
    statusLabel_RH->setFixedSize(60, 18);
    statusLabel_PRESS =new QLabel(this);
    statusLabel_PRESS->setFixedSize(100,18);
    statusLabel_msg = new QLabel(this);//显示提示信息
    statusLabel_SDKmsg = new QLabel(this);//显示提示信息
    ui->statusBar->addWidget(statusLabel_imgSize);
    ui->statusBar->addWidget(statusLabel_mousePos);
    ui->statusBar->addWidget(statusLabel_rgb);
    ui->statusBar->addWidget(statusLabel_Temp);
    ui->statusBar->addWidget(statusLabel_RH);
    ui->statusBar->addWidget(statusLabel_PRESS);
    ui->statusBar->addWidget(statusLabel_msg);
    ui->statusBar->addWidget(statusLabel_SDKmsg);

    //----------------------整体borderlayout布局-------------------------------------------------
    mainLayout = new BorderLayout();
    mainLayout->setSpacing(1);
    mainLayout->addWidget(ui->scrollArea_manager, BorderLayout::West);
    mainLayout->addWidget(scrollArea_ImgShow, BorderLayout::Center);
    mainLayout->addWidget(mainMenuBar, BorderLayout::North);//test menubar in Mac os
    mainLayout->addWidget(ui->statusBar, BorderLayout::South);
    ui->centralWidget->setLayout(mainLayout);

    //-------------------init members-----------------------------------------------------------
    tempTimer = new QTimer();//温度 计时器
    tempTimer->setInterval(1000);//设置定时器时间间隔1s

    cfwTimer = new QTimer();// 色轮状态查询  定时器
    cfwTimer->setInterval(1000);

    ditherTimer = new QTimer(); //Dither 状态查询定时器
    ditherTimer->setInterval(500);
    isSettleDone = false; //settle 默认状态

    PumpTimer = new QTimer();//循环泵 计时器
    PumpTimer->setInterval(1000);

    //---------------------关联信号与槽-----------------------------------------
    connect(scrollArea_ImgShow->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(hScrollBarValueChanged(int)));
    connect(scrollArea_ImgShow->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(vScrollBarValueChanged(int)));

    connect(mainMenuBar->actOpenFolder, SIGNAL(triggered()), this, SLOT(openFolder()));
    connect(mainMenuBar->actSaveFIT, SIGNAL(triggered()), this, SLOT(saveAsFIT()));
    connect(mainMenuBar->actSaveBMP, SIGNAL(triggered()), this, SLOT(saveAsBMP()));
    connect(mainMenuBar->actSaveJPG, SIGNAL(triggered()), this, SLOT(saveAsJPG()));
    connect(mainMenuBar->actFitHeaderEditor, SIGNAL(triggered()), this, SLOT(showFITHeaderEditor()));
    connect(mainMenuBar->actIgnoreOverScanArea, SIGNAL(triggered(bool)), this, SLOT(ignoreOverScanAreaClicked(bool)));
    connect(mainMenuBar->actCalibrateOverScan, SIGNAL(triggered(bool)), this, SLOT(calibrateOverScanClicked(bool)));
    connect(mainMenuBar->actExit, SIGNAL(triggered()), this, SLOT(exitMainWindow()));
    connect(mainMenuBar->actConnect, SIGNAL(triggered()), this, SLOT(showCameraChooser()));
    connect(mainMenuBar->actShowPlanTable, SIGNAL(triggered()), this, SLOT(showPlanTable()));
    connect(mainMenuBar->actFavorite, SIGNAL(triggered()), this, SLOT(showFavoriteSetting()));
    connect(mainMenuBar->actPHDLink, SIGNAL(triggered()), this, SLOT(showPHDLink()));
    connect(mainMenuBar->actTempControl, SIGNAL(triggered()), this, SLOT(showTempControl()));
    connect(mainMenuBar->actAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(mainMenuBar->actTestMode, SIGNAL(triggered()), this, SLOT(activeTestMode()));
    connect(mainMenuBar->actDebug, SIGNAL(triggered()), this, SLOT(switchDebug()));
    connect(mainMenuBar->actTestGuid, SIGNAL(triggered()), this, SLOT(switchTestGuid()));
    connect(mainMenuBar->actCFWControl, SIGNAL(triggered(bool)), this, SLOT(showCFWControl()));

    connect(mainMenuBar->actCaptureDarkFrameTool, SIGNAL(triggered()),this, SLOT(showCaptureDarkFrameTool()));

    connect(mainMenuBar->actEnglish, SIGNAL(triggered()), this, SLOT(changeToEnglish()));
    connect(mainMenuBar->actChinese, SIGNAL(triggered()), this, SLOT(changeToChinese()));
    connect(mainMenuBar->actJapanese, SIGNAL(triggered()), this, SLOT(changeToJapanese()));
    connect(this, SIGNAL(changeLanguage()), about_dialog, SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), cameraChooser, SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), favorite_dialog, SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), fitHeader_dialog, SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), mainMenuBar, SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), managerMenu, SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), planner_dialog, SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), tempControl_dialog, SLOT(resetUI()));
    //20200220 lyl Add ReadMode Dialog
    //connect(this, SIGNAL(changeLanguage()), readMode, SLOT(resetUI()));

    connect(cfwControl_dialog, SIGNAL(changeCFWPosition()), this, SLOT(cfwPositionChanged()));
    connect(cfwControl_dialog, SIGNAL(endCFWProgress()), this->cfwTimer, SLOT(stop()));
    connect(cfwSetup_dialog, SIGNAL(updateFilterNames()), cfwControl_dialog, SLOT(filterNames_updated()));
    connect(cfwSetup_dialog, SIGNAL(updateFilterNames()), planner_dialog, SLOT(cfwSetup_updated()));

    connect(mainMenuBar->actFitWindow, SIGNAL(triggered()), this, SLOT(zoomFitWindow()));
    connect(mainMenuBar->act0_25X, SIGNAL(triggered()), this, SLOT(zoom0_25X()));
    connect(mainMenuBar->act0_5X, SIGNAL(triggered()), this, SLOT(zoom0_5X()));
    connect(mainMenuBar->act0_75X, SIGNAL(triggered()), this, SLOT(zoom0_75X()));
    connect(mainMenuBar->act1X, SIGNAL(triggered()), this, SLOT(zoom1X()));
    connect(mainMenuBar->act1_5X, SIGNAL(triggered()), this, SLOT(zoom1_5X()));
    connect(mainMenuBar->act2X, SIGNAL(triggered()), this, SLOT(zoom2X()));

    connect(managerMenu->ui->pBtn_cross, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_cross_clicked()));
    connect(managerMenu->ui->pBtn_grid, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_grid_clicked()));
    connect(managerMenu->ui->pBtn_circle, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_circle_clicked()));
    connect(managerMenu->ui->pBtn_preview, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_preview_clicked()));
    connect(managerMenu->ui->pBtn_live_preview, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_live_preview_clicked()));
    connect(managerMenu->ui->pBtn_focus, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_focus_clicked()));
    connect(managerMenu->ui->pBtn_live_focus, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_live_focus_clicked()));
    connect(managerMenu->ui->pBtn_capture, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_capture_clicked()));
    connect(managerMenu->ui->pBtn_stop, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_stop_clicked()));
    connect(managerMenu->ui->hSlider_bPos, SIGNAL(sliderReleased()), this, SLOT(mgrMenu_hSlider_bPos_sliderReleased()));
    connect(managerMenu->ui->hSlider_wPos, SIGNAL(sliderReleased()), this, SLOT(mgrMenu_hSlider_wPos_sliderReleased()));
    connect(managerMenu->ui->pBtn_stretchMinusB, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_stretchMinusB_clicked()));
    connect(managerMenu->ui->pBtn_stretchPlusB, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_stretchPlusB_clicked()));
    connect(managerMenu->ui->pBtn_stretchMinusW, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_stretchMinusW_clicked()));
    connect(managerMenu->ui->pBtn_stretchPlusW, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_stretchPlusW_clicked()));
    connect(managerMenu->ui->pBtn_auto_histogram, SIGNAL(clicked()), this, SLOT(mgrMenu_pBtn_auto_histogram_clicked()));
    connect(managerMenu, SIGNAL(switchWorkMode(int)), this, SLOT(currentWorkingModeChanged(int)));

    connect(cameraChooser, SIGNAL(connect_camera()), mainMenuBar, SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), managerMenu, SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), planner_dialog, SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), this, SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), cfwControl_dialog, SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), favorite_dialog, SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), fitHeader_dialog, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()), managerMenu, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()), mainMenuBar, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()), planner_dialog, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()), this, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()), cfwControl_dialog, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()), favorite_dialog, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()), fitHeader_dialog, SLOT(camera_connected()));
    connect(this, SIGNAL(disconnect_camera()), mainMenuBar, SLOT(camera_disconnected()));
    connect(this, SIGNAL(disconnect_camera()), managerMenu, SLOT(camera_disconnected()));
    connect(this, SIGNAL(disconnect_camera()), this, SLOT(camera_disconnected()));

    connect(this, SIGNAL(change_fitHeaderInfo()), fitHeader_dialog, SLOT(fitHeaderInfo_changed()));

    connect(favorite_dialog->ui->pBtn_calibrateFrame, SIGNAL(clicked()), this, SLOT(favorite_pBtn_calibrateFrame_clicked()));
    connect(favorite_dialog->ui->pBtn_getRealTemp, SIGNAL(clicked()), this, SLOT(favorite_pBtn_getRealTemp_clicked()));
    connect(favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP, SIGNAL(clicked()), this, SLOT(favorite_pBtn_controlSensorChamberCyclePUMP_clicked()));

    //--------------------关联plannerdialog中forceStop信号到capture中的stop---------------------------------------- 
    connect(planner_dialog->ui->pBtn_forceStop_planner, SIGNAL(clicked()), managerMenu->ui->pBtn_stop, SLOT(click()));

    //-------------------关联定时器信号---------------------------------------------------------------------	
    connect(tempTimer, SIGNAL(timeout()), this, SLOT(tempTimer_timeout()));
    connect(cfwTimer, SIGNAL(timeout()), this, SLOT(cfwTimer_timeout()));
    connect(ditherTimer, SIGNAL(timeout()), this, SLOT(ditherTimer_timeout()));
    connect(PumpTimer, SIGNAL(timeout()), this, SLOT(PumpTimer_timeout()));



    //--------------------init-------------------------------------------------
    //-------------------------------------------------------------------------
    //初始化preview，focus,capture “收起/展开功能”关闭
    managerMenu->ui->head_preview->setCheckable(false);
    managerMenu->ui->head_focus->setCheckable(false);
    managerMenu->ui->head_capture->setCheckable(false);

    mainMenuBar->menuPlanner->setEnabled(false);
    mainMenuBar->actCFWControl->setEnabled(false);

    mainMenuBar->menuZoom->setEnabled(false);
    mainMenuBar->actSaveBMP->setEnabled(false);
    mainMenuBar->actSaveFIT->setEnabled(false);
    mainMenuBar->actSaveJPG->setEnabled(false);
    mainMenuBar->actCalibrateOverScan->setEnabled(false);
    mainMenuBar->actIgnoreOverScanArea->setEnabled(false);
    mainMenuBar->actOpenFolder->setEnabled(false);
    mainMenuBar->actCaptureDarkFrameTool->setEnabled(false);
    mainMenuBar->actFavorite->setEnabled(false);//true

    //初始FocusAssistant panel不显示
    ui->widgetFocusAssistant->setVisible(false);
    //初始显示在focusAssistant的图像
    QImage img_focusAssistant = QPixmap(":/image/black.bmp").toImage();
    DrawGridBox(&img_focusAssistant);//绘制表格线
    ui->image2_focusAssistant->setPixmap(QPixmap::fromImage(img_focusAssistant));
    ui->image3_focusAssistant->setPixmap(QPixmap::fromImage(img_focusAssistant));

    //初始化screenview显示的图像
    managerMenu->ui->img_screenView->setPixmap(QPixmap(":/image/black.bmp"));

    //初始伪彩色图片
    LoadFalseColor("Linear.bmp");

    //为控件添加事件过滤注册（对于控件事件过滤，必须添加注册，否则不能生效）
    ui->label_ImgShow->installEventFilter(this);
    managerMenu->ui->img_screenView->installEventFilter(this);
    ui->image1_focusAssistant->installEventFilter(this);

    //字体结构初始化
    cvInitFont(&QHYFont,CV_FONT_HERSHEY_SIMPLEX,0.5,0.5,0,1,8);

    qImg_capture = NULL;
    qImg_preview = NULL;
    qImg_focus = NULL;
    captureImg = NULL;// capture image, used to save
    ImgRAW = NULL;
    ImgColor = NULL;
    ScreenViewImg = cvCreateImage(cvSize(196,128), IPL_DEPTH_8U, 3);

    translator = NULL;//init translator object is null.
    cmenu_captureExp = NULL;
    cmenu_imgArea = NULL;

    downloadPre = NULL;
    downloadFoc = NULL;
    downloadCap = NULL;
    exePlanTable = NULL;
    runCFWOrder = NULL;
    runCFWOrder = new ExecuteCFWOrder();

    fwhm_x = 0;//focus assistant中fwhm图的起始点坐标
    fwhm_y = 0;
    peak_x = 0;//focus assistant中peak图的起始点坐标
    peak_y = 0;

    //ini file parameters init
    iniFileParams.iniFileExist = false;
    iniFileParams.lang = "EN";
    iniFileParams.gain = 0;
    iniFileParams.offset = 0;
    iniFileParams.tecPretect = false;
    iniFileParams.slowestDowload = false;
    iniFileParams.clamp = false;
    iniFileParams.bPos_Preview = 0;
    iniFileParams.wPos_Preview = 65535;
    iniFileParams.bPos_Focus = 0;
    iniFileParams.wPos_Focus = 65535;
    iniFileParams.bPos_Capture = 0;
    iniFileParams.wPos_Capture = 65535;
    iniFileParams.autoStretchMode = 0;
    iniFileParams.ignoreOverScan = false;
    iniFileParams.calibrateOverScan = false;
    iniFileParams.calConstant = 1000;
    iniFileParams.CFWSlotsNum = 0;

    //初始化ix
    ix.lang = "EN";
    ix.isConnected = false;
    ix.cameraState = Camera_Idle;
    ix.imageReady = GetSingleFrame_Waiting;
    ix.workMode = 0;//workMode: 1 preview  2 focus 3 capture
    ix.lastWorkMode = -1;
    ix.ForceStop = false;
    ix.onLiveMode = false;
    ix.plannerState = PlannerStatus_Done;
    ix.fitHeadEditState = FitHeader_Set;
    ix.isCalibrateFrame = false;
    ix.ignoreOverScan = false;
    ix.calibrateOverScan = false;
    ix.calConstant = 1000;

    ix.ccdMaxImgW = 1000;
    ix.ccdMaxImgH = 1000;

    ix.binx = 1;
    ix.biny = 1;
    ix.maxBinx = 1;
    ix.maxBiny = 1;
    ix.canbin11 = true;
    ix.canbin22 = false;
    ix.canbin33 = false;
    ix.canbin44 = false;
    ix.CamID = "QHYCCD Cameras";
    ix.CamModel = "QHYCCD Cameras";

    ix.Exptime = 1;
    ix.LastExptime = 0;
    ix.DownloadSpeed = 0;
    ix.LastDownloadSpeed = -1;
    ix.can16Bits = false;
    ix.bits = 8;
    ix.canUsbtraffic = false;
    ix.usbtraffic = 0;
    ix.lastUsbtraffic = -1;
    //20200512lyl GPSon
    ix.canGPS =false;
    ix.isGPSon=false;

    ix.imageX = 1000;
    ix.imageY = 1000;
    ix.imgBpp = 16;
    ix.imgChannels = 1;
    ix.lastImageX = 1000;
    ix.lastImageY = 1000;
    ix.lastImgData = NULL;
    ix.ImgData = NULL;//初始化存放图像数据数组
    ix.OutputData8 = NULL;//初始化存放转换后的8位图像数据

    ix.dispIplImgData24 = NULL;
    ix.dispIplImgData8 = nullptr;

    ix.canCooler = false;
    ix.nowTemp = 0;
    ix.targetTemp = 100.0;
    ix.nowPWM = 2;//128;
    //20200318
    ix.canHumidity=false;
    ix.nowHumidity=0;//当前湿度
    ix.canPressure=false;
    ix.nowPressure =0;//当前压力

    ix.canColor = false;
    ix.isCvtColor = false;

    ix.canFilterWheel = false;
    ix.dstCfwPos = '0';//初始CFW目标位置为第0孔
    ix.CFWStatus = CFW_Idle;
    ix.CFWSlotsNum = 0;

    ix.canMechanicalShutter = false;
    ix.MechanicalShutterMode = 0;
    ix.LastMechanicalShutterMode = 0;

    ix.StretchStep = 256;
    ix.autoStretchMode = 0;

    ix.zoomMode = Zoom_SpecifyScaling;

    //20200220 lyl Add ReadMode Dialog
    ix.NumberOfReadModes=1;
    ix.currentReadMode=0;

    scaleFactor = 1.0;
    //20201127 lyl SensorChamberCyclePUMP
    ix.canContolSensorChamberCyclePUMP = false;
    //---EZCAP params init

    hScrollBarValue_Pre = 0;
    vScrollBarValue_Pre = 0;
    hScrollBarValue_Cap = 0;
    vScrollBarValue_Cap = 0;
    needCalcScrollBarValue = true;

    noImgInWorkMode = true;

    OverScanRMS = 0;

    Flag_Timer = 0;//初始化温控 Timer  每秒一次，然后分频为2秒，其中第一秒为取数，第二秒为设置
    Flag_Timer_2 = 0;//初始化温控 Timer分频 ，每4秒一次
    CurveX = 0;
    posX_tempImg = 0;
    posY_tempImg = 0;
    posX_tempImg_RH = 0;
    posY_tempImg_RH = 0;
    posX_tempImg_Press = 0;
    posY_tempImg_Press = 0;

    Preview_WPOS = 65535;//初始化W B位置值
    Preview_BPOS = 0;
    Focus_WPOS = 65535;
    Focus_BPOS = 0;
    Capture_WPOS = 65535;
    Capture_BPOS = 0;

    FocusCenterX_Pre = 300;//初始化focus中心坐标
    FocusCenterY_Pre = 300;
    focusAreaStartX = 0;
    focusAreaStartY = 200;
    focusAreaSizeX = 800;
    focusAreaSizeY = 200;
    ZoomFocus_X = 0;
    ZoomFocus_Y = 0;
    FocusZoomMode = 0;
    FocusCurveX = 0;

    viewBoxCX = 98;
    viewBoxCY = 64;
    viewBoxW = 100;
    viewBoxH = 100;

    // load parameters from IniFile
    if(loadParasFromIni())
    {
        iniFileParams.iniFileExist = true;

        ix.lang = iniFileParams.lang;
        Preview_BPOS = iniFileParams.bPos_Preview;
        Preview_WPOS = iniFileParams.wPos_Preview;
        Focus_BPOS = iniFileParams.bPos_Focus;
        Focus_WPOS = iniFileParams.wPos_Focus;
        Capture_BPOS = iniFileParams.bPos_Capture;
        Capture_WPOS = iniFileParams.wPos_Capture;
        ix.autoStretchMode = iniFileParams.autoStretchMode;
        ix.ignoreOverScan = iniFileParams.ignoreOverScan;
        ix.calibrateOverScan = iniFileParams.calibrateOverScan;
        ix.calConstant = iniFileParams.calConstant;
    }
    else
    {
        iniFileParams.iniFileExist = false;
    }
    mainMenuBar->actTestGuid->setChecked(iniFileParams.testGuider);
    mainMenuBar->actTestMode->setChecked(iniFileParams.autoConnect);
    // switch to current language.
    languageChanged();
    //init lang menu
    if(ix.lang.compare("CN") == 0)
        mainMenuBar->actChinese->setChecked(true);
    else if(ix.lang.compare("JP") == 0)
        mainMenuBar->actJapanese->setChecked(true);
    else
        mainMenuBar->actEnglish->setChecked(true);

    //load the FIT Header data form CSV file
    QString path_csv = QApplication::applicationDirPath() + "/" + "FITHEADER.csv";
    path_csv = QDir::toNativeSeparators(path_csv);
    fitHeader_dialog->loadCSV(path_csv);
    //20201125lyl温度湿度压力数据保存
//    QString filenameTEMP = QFileDialog::getSaveFileName(this,tr("Save"),"",tr("temp Files(*.txt)")); //选择路径txt   xlsx
//    if(filenameTEMP.isEmpty())
//    {
//        qDebug() << "save Excel Files: fileName is empty, can not save";
//    }
//    else
//    {
//        filetemp=filenameTEMP;
//        if(QFile::exists(filenameTEMP))
//        {
//            qDebug() << "exists  Files: " << filenameTEMP;
//            QFile::remove(filenameTEMP);
//        }
//        qDebug() << "save Excel Files: " << filenameTEMP;
//        QFile fileTEMP(filenameTEMP);
//        fileTEMP.open(QIODevice::Text | QIODevice::WriteOnly);
//        QTextStream out(&fileTEMP);
//        out<<qSetFieldWidth(5)<<left<<"Temp \t Humidity \t Press \t \n";
//        fileTEMP.close();
//    }
    if(iniFileParams.oldSDK){
        qDebug() << "Skip Event when use Old SDK";
        return;
    }
    qDebug() << "regist event pnp_Event_In_Func";
    libqhyccd->RegisterPnpEventOut(pnp_Event_Out_Func);
    libqhyccd->RegisterPnpEventIn(pnp_Event_In_Func);
    qDebug() << "regist event pnp_Event_Out_Func";
    qDebug() << "regist event data_Event_Single_Func";
    libqhyccd->RegisterDataEventSingle(data_Event_Single_Func);
    qDebug() << "regist event data_Event_Live_Func";
    libqhyccd->RegisterDataEventLive(data_Event_Live_Func);
    qDebug() << "regist event end.";
    libqhyccd->RegisterPnpEvent(pnpEventExFunc);
    libqhyccd->RegisterTransferEventError(transferEventErrorFunc);
    updateWindowsTitle();
    ui->plainTextEdit_debug->hide();
}


EZCAP::~EZCAP()
{
    if(libqhyccd)
    {
        delete libqhyccd;
         libqhyccd=NULL;
    }
    if(mainMenuBar)
    {
        delete mainMenuBar;
        mainMenuBar = NULL;
    }
    if(scrollArea_ImgShow)
    {
        delete scrollArea_ImgShow;
        scrollArea_ImgShow = NULL;
    }
    if(about_dialog)
    {
        delete about_dialog;
        about_dialog = NULL;
    }
    if(favorite_dialog)
    {
        delete favorite_dialog;
        favorite_dialog = NULL;
    }
    if(phdLink_dialog)
    {
        delete phdLink_dialog;
        phdLink_dialog = NULL;
    }

    if(darkFrameTool_dialog)
    {
        delete darkFrameTool_dialog;
        darkFrameTool_dialog = NULL;
    }

    if(fitHeader_dialog)
    {
        delete fitHeader_dialog;
        fitHeader_dialog = NULL;
    }
    if(planner_dialog)
    {
        delete planner_dialog;
        planner_dialog = NULL;
    }
    if(tempControl_dialog)
    {
        delete tempControl_dialog;
        tempControl_dialog = NULL;
    }
    if(cameraChooser)
    {
        delete cameraChooser;
        cameraChooser = NULL;
    }

    //20200220 lyl Add ReadMode Dialog
    if(readMode)
    {
        delete readMode;
        readMode = NULL;
    }

    if(statusLabel_imgSize)
    {
        delete statusLabel_imgSize;
        statusLabel_imgSize = NULL;
    }
    if(statusLabel_mousePos)
    {
        delete statusLabel_mousePos;
        statusLabel_mousePos = NULL;
    }
    if(statusLabel_rgb)
    {
        delete statusLabel_rgb;
        statusLabel_rgb = NULL;
    }
    if(statusLabel_Temp)
    {
        delete statusLabel_Temp;
        statusLabel_Temp = NULL;
    }

    if(statusLabel_RH)
    {
        delete statusLabel_RH;
        statusLabel_RH = NULL;
    }
    if(statusLabel_PRESS)
    {
        delete statusLabel_PRESS;
        statusLabel_PRESS = NULL;
    }
    if(statusLabel_msg)
    {
        delete statusLabel_msg;
        statusLabel_msg = NULL;
    }
    if(statusLabel_SDKmsg)
    {
        delete statusLabel_SDKmsg;
        statusLabel_SDKmsg = NULL;
    }

    if(tempTimer)
    {
        delete tempTimer;
        tempTimer = NULL;
    }
    if(cfwTimer)
    {
        delete cfwTimer;
        cfwTimer = NULL;
    }
    if(PumpTimer)
    {
        delete PumpTimer;
        PumpTimer = NULL;
    }

    if(managerLayout)
    {
        delete managerLayout;
        managerLayout = NULL;
    }
    if(mainLayout)
    {
        delete mainLayout;
        mainLayout = NULL;
    }

    if(ix.lastImgData)
    {
        delete ix.lastImgData;
        ix.lastImgData = NULL;
    }
    if(ix.ImgData)
    {
        delete ix.ImgData;
        ix.ImgData = NULL;
    }
    if(ix.OutputData8)
    {
        delete ix.OutputData8;
        ix.OutputData8 = NULL;
    }
    if(ix.dispIplImgData24)
    {
        delete ix.dispIplImgData24;
        ix.dispIplImgData24 = NULL;
    }
    if(ix.dispIplImgData8)
    {
        delete ix.dispIplImgData8;
        ix.dispIplImgData8 = NULL;
    }

    if(captureImg)
    {
        cvReleaseImage(&captureImg);
    }

    if(runCFWOrder)
    {
        delete(runCFWOrder);
        runCFWOrder = NULL;
    }

    qDebug() << "resources deleted.";

    delete ui;

}

//根据相机初始参数设置界面中控件是否可用     相机打开成功后，设置界面初值
bool EZCAP::getParamsFromCamera()
{
    qDebug()<<"EZCAP   getParamsFromCamera() start~";
    unsigned int ret;
    unsigned char buf[32];
    double max,min,step;
    ret = libqhyccd->GetQHYCCDFWVersion(camhandle,buf); //ret = GetQHYCCDFWVersion(camhandle,buf);
    if(ret == QHYCCD_ERROR)
    {
        ix.driverVer = "-";
    }
    else
    {
        //可正确显示2010年~2025的相机驱动版本
        unsigned char year = buf[0] >> 4;
        if(year <= 9)
            ix.driverVer = QString::number(year + 0x10,10) + "-" + QString::number((buf[0]&~0xf0),10) + "-" + QString::number(buf[1], 10);
        else
            ix.driverVer = QString::number(year,10) + "-" + QString::number((buf[0]&~0xf0),10) + "-" + QString::number(buf[1], 10);
    }
    //20200303lyl FPGAVersion
    if(libqhyccd->GetQHYCCDFPGAVersion)
    {
        ret=libqhyccd->GetQHYCCDFPGAVersion(camhandle,0,buf);//ret = GetQHYCCDFPGAVersion(camhandle,0,buf);
        if(ret == QHYCCD_ERROR)
        {
            ix.FPGAVer= "-";
        }
        else
        {
            ix.FPGAVer = QString::number(buf[0],10)+ "-" + QString::number(buf[1],10)+ "-" + QString::number(buf[2],10) + "-" + QString::number(buf[3],10);
        }
        ret=libqhyccd->GetQHYCCDFPGAVersion(camhandle,1,buf);//ret = GetQHYCCDFPGAVersion(camhandle,1,buf);//获得第二个FPGAversion
        if(ret == QHYCCD_ERROR)
        {
            ix.FPGAVer1= "-";
        }
        else
        {
            ix.FPGAVer1 = QString::number(buf[0],10)+ "-" + QString::number(buf[1],10)+ "-" + QString::number(buf[2],10) + "-" + QString::number(buf[3],10);
        }
    }
    else{//QString exception
        qCritical("GetQHYCCDFPGAVersion: have no this function !");
    }
//    try
//    {
//        ret=libqhyccd->GetQHYCCDFPGAVersion(camhandle,0,buf);//ret = GetQHYCCDFPGAVersion(camhandle,0,buf);
//        if(ret == QHYCCD_ERROR)
//        {
//            ix.FPGAVer= "Unknown";
//        }
//        else
//        {
//            ix.FPGAVer = QString::number(buf[0],10)+ "-" + QString::number(buf[1],10)+ "-" + QString::number(buf[2],10) + "-" + QString::number(buf[3],10);
//        }
//        ret=libqhyccd->GetQHYCCDFPGAVersion(camhandle,1,buf);//ret = GetQHYCCDFPGAVersion(camhandle,1,buf);//获得第二个FPGAversion
//        if(ret == QHYCCD_ERROR)
//        {
//            ix.FPGAVer1= "Unknown";
//        }
//        else
//        {
//            ix.FPGAVer1 = QString::number(buf[0],10)+ "-" + QString::number(buf[1],10)+ "-" + QString::number(buf[2],10) + "-" + QString::number(buf[3],10);
//        }
//    }
//    catch (QException e) {//QString exception
//        qCritical("GetQHYCCDFPGAVersion: have no this function !");
//    }
    ret = libqhyccd->GetQHYCCDChipInfo(camhandle, &ix.ccdChipW, &ix.ccdChipH, &ix.ccdMaxImgW, &ix.ccdMaxImgH, &ix.ccdPixelW, &ix.ccdPixelH, &ix.bits);//ret = GetQHYCCDChipInfo(camhandle, &ix.ccdChipW, &ix.ccdChipH, &ix.ccdMaxImgW, &ix.ccdMaxImgH, &ix.ccdPixelW, &ix.ccdPixelH, &ix.bits);
    if(ret == QHYCCD_SUCCESS)
    {
        qDebug() << "EZCAP   GetQHYCCDChipInfo: ccd image size " << ix.ccdMaxImgW << ix.ccdMaxImgH << ix.ccdPixelW << ix.ccdPixelH;
    }
    else
    {
        qCritical("GetQHYCCDChipInfo: failed");
    }
    //---can support color setting
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_COLOR); //ret = IsQHYCCDControlAvailable(camhandle,CAM_COLOR);
    if(ret == QHYCCD_ERROR)
    {
        ix.canColor = false;
    }
    else
    {
        ix.bayermatrix = ret;
        ix.canColor = true;
    }
    qDebug() << "EZCAP   IsQHYCCDControlAvailable CAM_COLOR" << ix.canColor;
    //---check can set offset setting or not
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_OFFSET);//ret = IsQHYCCDControlAvailable(camhandle,CONTROL_OFFSET);
    if(ret == QHYCCD_SUCCESS)
        ix.canOffset = true;
    else
        ix.canOffset = false;
    qDebug() << "EZCAP   IsQHYCCDControlAvailable CONTROL_OFFSET" << ix.canOffset;
    if(ix.canOffset)
    {
        qDebug() << "EZCAP   IsQHYCCDControlAvailable CONTROL_OFFSET";  //<< min<<max<<step;
        ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle,CONTROL_OFFSET,&min,&max,&step);//ret = GetQHYCCDParamMinMaxStep(camhandle,CONTROL_OFFSET,&min,&max,&step);
        qDebug() << "EZCAP   IsQHYCCDControlAvailable CONTROL_OFFSET";  // << min<<max<<step;
        if(ret == QHYCCD_SUCCESS)
        {//qDebug() << "EZCAP   IsQHYCCDControlAvailable CONTROL_OFFSET" << min<<max<<step;
            ix.offsetMax = max;
            ix.offsetMin = min;
            ix.offsetStep = step;
        }
        if(iniFileParams.iniFileExist)
        {
            ix.offset = iniFileParams.offset;
        }
        else
        {
            ix.offset = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_OFFSET);    //ix.offset = GetQHYCCDParam(camhandle, CONTROL_OFFSET);
            qDebug() << "EZCAP   GetQHYCCDParam CONTROL_OFFSET" << ix.offset;
        }
    }

    //---can support gain or not
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_GAIN); //ret = IsQHYCCDControlAvailable(camhandle,CONTROL_GAIN);
    if(ret == QHYCCD_SUCCESS)
        ix.canGain = true;
    else
        ix.canGain = false;
    qDebug() << "EZCAP   IsQHYCCDControlAvailable CONTROL_GAIN" << ix.canGain;
    //check gain range
    if(ix.canGain)
    {
        ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle,CONTROL_GAIN,&min,&max,&step); //ret = GetQHYCCDParamMinMaxStep(camhandle,CONTROL_GAIN,&min,&max,&step);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.gainMax = max;
            ix.gainMin = min;
            ix.gainStep = step;
        }
        if(iniFileParams.iniFileExist)
        {
            ix.gain = iniFileParams.gain;
        }
        else
        {
            ix.gain = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_GAIN);   //ix.gain = GetQHYCCDParam(camhandle, CONTROL_GAIN);
            qDebug() << "EZCAP   GetQHYCCDParam CONTROL_GAIN" << ix.gain;
        }
    }
    //---can support binning mode
    ret =libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN1X1MODE); //ret = IsQHYCCDControlAvailable(camhandle,CAM_BIN1X1MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canbin11 = true;
        ix.maxBinx = 1;
        ix.maxBiny = 1;
    }
    else
    {
        ix.canbin11 = false;
    }
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN2X2MODE); //ret = IsQHYCCDControlAvailable(camhandle,CAM_BIN2X2MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canbin22 = true;
        ix.maxBinx = 2;
        ix.maxBiny = 2;
    }
    else
    {
        ix.canbin22 = false;
    }
    ret =libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN3X3MODE);  //ret = IsQHYCCDControlAvailable(camhandle,CAM_BIN3X3MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canbin33 = true;
        ix.maxBinx = 3;
        ix.maxBiny = 3;
    }
    else
    {
        ix.canbin33 = false;
    }
    ret =libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN4X4MODE);   //ret = IsQHYCCDControlAvailable(camhandle,CAM_BIN4X4MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canbin44 = true;
        ix.maxBinx = 4;
        ix.maxBiny = 4;
    }
    else
    {
        ix.canbin44 = false;
    }
    qDebug() << "IsQHYCCDControlAvailable CAM_BIN1X1MODE" << ix.canbin11;
    qDebug() << "IsQHYCCDControlAvailable CAM_BIN2X2MODE" << ix.canbin22;
    qDebug() << "IsQHYCCDControlAvailable CAM_BIN3X3MODE" << ix.canbin33;
    qDebug() << "IsQHYCCDControlAvailable CAM_BIN4X4MODE" << ix.canbin44;
    qDebug() << "Camera Maximum Binning" << ix.maxBinx << ix.maxBiny;

    //---can support highspeed setting
    ret =libqhyccd-> IsQHYCCDControlAvailable(camhandle,CONTROL_SPEED); //ret = IsQHYCCDControlAvailable(camhandle,CONTROL_SPEED);
    if(ret == QHYCCD_SUCCESS)
        ix.canHighSpeed = true;
    else
        ix.canHighSpeed = false;
    qDebug() << "IsQHYCCDControlAvailable CONTROL_SPEED" << ix.canHighSpeed;

    //---can support usbtraffic
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_USBTRAFFIC);   //ret = IsQHYCCDControlAvailable(camhandle, CONTROL_USBTRAFFIC);
    if(ret == QHYCCD_SUCCESS)
        ix.canUsbtraffic = true;
    else
        ix.canUsbtraffic = false;
    qDebug() << "IsQHYCCDControlAvailable CONTROL_USBTRAFFIC" << ix.canUsbtraffic;
    //check usbtraffic range
    if(ix.canUsbtraffic)
    {
        ret =libqhyccd-> GetQHYCCDParamMinMaxStep(camhandle,CONTROL_USBTRAFFIC,&min,&max,&step);  //ret = GetQHYCCDParamMinMaxStep(camhandle,CONTROL_USBTRAFFIC,&min,&max,&step);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.usbtrafficMax = max;
            ix.usbtrafficMin = min;
            ix.usbtrafficStep = step;
            ix.usbtraffic = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC);
            //ix.usbtraffic = GetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC);
            qDebug() << "GetQHYCCDParam CONTROL_USBTRAFFIC" << ix.usbtraffic;
        }
    }
    //---can support GPS  20200512lyl
    try{
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_GPS);
        //ret = IsQHYCCDControlAvailable(camhandle, CAM_GPS);
        if(ret == QHYCCD_SUCCESS)
            ix.canGPS = true;
        else
            ix.canGPS = false;
        qDebug() << "IsQHYCCDControlAvailable CAM_GPS" << ix.canGPS;
    }
    catch (QException e) {
        qCritical("IsQHYCCDControlAvailable  CAM_GPS: have no this function !");
    }
    //---can support OSD  20200512lyl
    if(ix.canGPS)
    {    //列表填充
        ix.OSDList.clear();
        ix.OSDList.append("Disable OSD");
        ix.OSDList.append("HardwareFrameCounter");
        ix.OSDList.append("GPS Data");
        favorite_dialog->ui->comboBox_OSD->clear();
        favorite_dialog->ui->comboBox_OSD->addItems(ix.OSDList);
    }
    //----set transferbit------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_TRANSFERBIT);
    //ret = IsQHYCCDControlAvailable(camhandle, CONTROL_TRANSFERBIT);
    if(ret == QHYCCD_SUCCESS)
    {
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_16BITS);
        //ret = IsQHYCCDControlAvailable(camhandle, CAM_16BITS);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.can16Bits = true;
            ix.bits = 16;
        }
        else
        {
            ix.can16Bits = false;
            ix.bits = 8;
        }
        qDebug() << "IsQHYCCDControlAvailable CAM_16BITS" << ix.can16Bits;
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_TRANSFERBIT, ix.bits);
        //ret = SetQHYCCDParam(camhandle, CONTROL_TRANSFERBIT, ix.bits);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.lastBits = ix.bits;
            qDebug() << "SetQHYCCDParam CONTROL_TRANSFERBIT" << ix.bits;
        }
    }

    //---can support cfw control
    ret =libqhyccd-> IsQHYCCDControlAvailable(camhandle,CONTROL_CFWPORT);
    //ret = IsQHYCCDControlAvailable(camhandle,CONTROL_CFWPORT);
    if(ret == QHYCCD_SUCCESS)
        ix.canFilterWheel = true;
    else
         ix.canFilterWheel = false;
    qDebug() << "IsQHYCCDControlAvailable CONTROL_CFWPORT" << ix.canFilterWheel;
    if(ix.canFilterWheel)
    {
        if(!iniFileParams.iniFileExist)
        {
            //if inifile does not exist...
            ret = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_CFWSLOTSNUM);
            //ret = GetQHYCCDParam(camhandle, CONTROL_CFWSLOTSNUM);
            if(ret == QHYCCD_ERROR)
                iniFileParams.CFWSlotsNum = 9;
            else
                iniFileParams.CFWSlotsNum = ret;

            iniFileParams.filterNames_2.clear();
            QString str;
            for(int i=0; i<iniFileParams.CFWSlotsNum; i++)
            {
                str = QString("Pos %1").arg(i+1);
                iniFileParams.filterNames_2.append(str);
            }
        }
        qDebug() << "Number of Filters: " << iniFileParams.CFWSlotsNum;

        ix.CFWSlotsNum = iniFileParams.CFWSlotsNum;
        ix.filterNames_2.clear();
        for(int i=0; i<ix.CFWSlotsNum; i++)
        {
            ix.filterNames_2.append(iniFileParams.filterNames_2.at(i));
        }
    }

    //---can support cooler
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_COOLER);
    //ret = IsQHYCCDControlAvailable(camhandle,CONTROL_COOLER);
    if(ret == QHYCCD_SUCCESS)
        ix.canCooler = true;
    else
        ix.canCooler = false;
    qDebug() << "IsQHYCCDControlAvailable CONTROL_COOLER" << ix.canCooler;

    //---can support humidity 20200318
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_HUMIDITY);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_HUMIDITY);
    if(ret == QHYCCD_SUCCESS)
        ix.canHumidity = true;
    else
        ix.canHumidity = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_HUMIDITY" << ix.canHumidity;

    //---can support pressure 20200318
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_PRESSURE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_PRESSURE);
    if(ret == QHYCCD_SUCCESS)
        ix.canPressure = true;
    else
        ix.canPressure = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_PRESSURE" << ix.canPressure;

    //---mechanical shutter
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_MECHANICALSHUTTER);
    //ret = IsQHYCCDControlAvailable(camhandle, CAM_MECHANICALSHUTTER);
    if(ret == QHYCCD_SUCCESS)
        ix.canMechanicalShutter = true;
    else
        ix.canMechanicalShutter = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_MECHANICALSHUTTER" << ix.canMechanicalShutter;

    //---trigger
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_TRIGER_INTERFACE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_TRIGER_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
        ix.canTriger = true;
    else
        ix.canTriger = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_TRIGER_INTERFACE" << ix.canTriger;

    //---fineTone
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_FINETONE_INTERFACE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_FINETONE_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
        ix.canFineTone = true;
    else
        ix.canFineTone = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_FINETONE_INTERFACE" << ix.canFineTone;

    //---motor heating
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_SHUTTERMOTORHEATING_INTERFACE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_SHUTTERMOTORHEATING_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
        ix.canMotorHeating = true;
    else
        ix.canMotorHeating = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_SHUTTERMOTORHEATING_INTERFACE" << ix.canMotorHeating;

    //---TEC Over protect
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_TECOVERPROTECT_INTERFACE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_TECOVERPROTECT_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
        ix.canTecOverProtect = true;
    else
        ix.canTecOverProtect = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_TECOVERPROTECT_INTERFACE" << ix.canTecOverProtect;
    if(ix.canTecOverProtect && iniFileParams.iniFileExist)
    {
        ix.tecPretect = iniFileParams.tecPretect;
    }

    //---Clamp
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_SINGNALCLAMP_INTERFACE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_SINGNALCLAMP_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
        ix.canSignalClamp = true;
    else
        ix.canSignalClamp = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_SINGNALCLAMP_INTERFACE" << ix.canSignalClamp;
    if(ix.canSignalClamp && iniFileParams.iniFileExist)
    {
        ix.clamp = iniFileParams.clamp;
    }

    //---calibrate FPN
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_CALIBRATEFPN_INTERFACE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_CALIBRATEFPN_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
        ix.canCalibrateFPN = true;
    else
        ix.canCalibrateFPN = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_CALIBRATEFPN_INTERFACE" << ix.canCalibrateFPN;

    //---chip temperature
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_CHIPTEMPERATURESENSOR_INTERFACE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_CHIPTEMPERATURESENSOR_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
        ix.canChipTemp = true;
    else
        ix.canChipTemp = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_CHIPTEMPERATURESENSOR_INTERFACE" << ix.canChipTemp;

    //---slowest download
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_USBREADOUTSLOWEST_INTERFACE);
    //ret = IsQHYCCDControlAvailable(camhandle,CAM_USBREADOUTSLOWEST_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
        ix.canSlowestDownload = true;
    else
        ix.canSlowestDownload = false;
    qDebug() << "IsQHYCCDControlAvailable CAM_USBREADOUTSLOWEST_INTERFACE" << ix.canSlowestDownload;
    if(ix.canSlowestDownload && iniFileParams.iniFileExist)
    {
        ix.slowestDowload = iniFileParams.slowestDowload;
    }
    //20201127 lyl SensorChamberCyclePUMP
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_SensorChamberCycle_PUMP);
    if(ret == QHYCCD_SUCCESS)
        ix.canContolSensorChamberCyclePUMP = true;
    else
        ix.canContolSensorChamberCyclePUMP = false;

    int len = 0;
    //len = GetQHYCCDMemLength(camhandle);//获取最大所需内存空间大小
    len = libqhyccd->GetQHYCCDMemLength(camhandle);
    if(len <= 0)
    {
        qCritical() << "GetMaxFrameLength() returned 0";
        QMessageBox::critical(this,tr("Warning"),tr("Can not get max frame length,Please contact the developer!"),QMessageBox::Ok);
        return false;
    }

    if(ix.lastImgData)
    {
        delete ix.lastImgData;
        ix.lastImgData = NULL;
    }
    if(ix.ImgData)
    {
        delete ix.ImgData;
        ix.ImgData = NULL;
    }
    if(ix.OutputData8)
    {
        delete ix.OutputData8;
        ix.OutputData8 = NULL;
    }
    if(ix.dispIplImgData24)
    {
        delete ix.dispIplImgData24;
        ix.dispIplImgData24 = NULL;
    }
    if(ix.dispIplImgData8)
    {
        delete ix.dispIplImgData8;
        ix.dispIplImgData8 = NULL;
    }

    if(ix.lastImgData)
    {
        delete ix.lastImgData;
        ix.lastImgData = NULL;
    }
    if(ix.ImgData)
    {
        delete ix.ImgData;
        ix.ImgData = NULL;
    }
    if(ix.OutputData8)
    {
        delete ix.OutputData8;
        ix.OutputData8 = NULL;
    }
    if(ix.dispIplImgData24)
    {
        delete ix.dispIplImgData24;
        ix.dispIplImgData24 = NULL;
    }
    if(ix.dispIplImgData8)
    {
        delete ix.dispIplImgData8;
        ix.dispIplImgData8 = NULL;
    }
    ix.lastImgData = new unsigned char[ix.ccdMaxImgW * ix.ccdMaxImgH * 2];  //分配内存 记录上一张图像数据，用户经常需要在拍摄过程中查看图像
    ix.ImgData = new unsigned char[ix.ccdMaxImgW * ix.ccdMaxImgH * 2];//分配内存
    ix.OutputData8 = new unsigned char[ix.ccdMaxImgW * ix.ccdMaxImgH];
    //分配opecv 8bit 3channels图像数据空间，由于opencv图像按4字节对齐特效，需分配已对齐图像数据所需的大小。
    ix.dispIplImgData24 = new unsigned char[(ix.ccdMaxImgW+3)/4*4 * ix.ccdMaxImgH * 3];
    //分配opecv 8bit 1channels图像数据空间，由于opencv图像按4字节对齐特效，需分配已对齐图像数据所需的大小。
    ix.dispIplImgData8 = new unsigned char[(ix.ccdMaxImgW+3)/4*4 * ix.ccdMaxImgH];

    if(qImg_capture)
    {
        delete qImg_capture;
        qImg_capture = NULL;
    }
    return true;
}

void EZCAP::camera_connected()
{
    //显示版本，驱动等信息
    if(ix.FPGAVer =="-" && ix.FPGAVer1 =="-")
    {
        this->setWindowTitle(tr("APPVERSION:") + EZCAP_VER + "  "+"FWVersion:" + ix.driverVer + "  " + "CameraID:" + ix.CamID);
    }
    else if(ix.FPGAVer =="-" && ix.FPGAVer1 !="-")
    {
        this->setWindowTitle(tr("APPVERSION:") + EZCAP_VER + "  "+ "FPGAVersion1:" + ix.FPGAVer1 + "  "  + "FWVersion:" + ix.driverVer + "  " + "CameraID:" + ix.CamID);
    }
    else if(ix.FPGAVer !="-" && ix.FPGAVer1 =="-")
    {
        this->setWindowTitle(tr("APPVERSION:") + EZCAP_VER + "  "+ "FPGAVersion:" + ix.FPGAVer + "  "  + "FWVersion:" + ix.driverVer + "  " + "CameraID:" + ix.CamID);
    }
    else {
        this->setWindowTitle(tr("APPVERSION:") + EZCAP_VER + "  "+ "FPGAVersion:" + ix.FPGAVer + "  "  +"FPGAVersion1:" + ix.FPGAVer1 + "  "  + "FWVersion:" + ix.driverVer + "  " + "CameraID:" + ix.CamID);
    }

    updateWindowsTitle();
    if(cameraChooser->isVisible())
        cameraChooser->close();

    ix.cameraState = Camera_Idle;
    ix.imageReady = GetSingleFrame_Waiting;
    ix.ForceStop = false;
    ix.onLiveMode = false;
    ix.plannerState = PlannerStatus_Done;

    if(ix.canCooler)
    {
        this->startTimerTemp();//支持制冷控制, 则启动温控定时器
    }
}

void EZCAP::camera_disconnected()
{
    if(planner_dialog->isVisible())
        planner_dialog->close();

    if(cameraChooser->isVisible())
        cameraChooser->close();
    if(ix.canCooler)
    {
        this->stopTimerTemp();//stop cooler control Timer
    }
    //20200323lyl断开连接preview，focus,capture 功能关闭
    managerMenu->ui->head_preview->setCheckable(false);
    managerMenu->ui->head_focus->setCheckable(false);
    managerMenu->ui->head_capture->setCheckable(false);
    mainMenuBar->menuPlanner->setEnabled(false);
    mainMenuBar->actCFWControl->setEnabled(false);
    mainMenuBar->menuZoom->setEnabled(false);
    mainMenuBar->actSaveBMP->setEnabled(false);
    mainMenuBar->actSaveFIT->setEnabled(false);
    mainMenuBar->actSaveJPG->setEnabled(false);
    mainMenuBar->actCalibrateOverScan->setEnabled(false);
    mainMenuBar->actIgnoreOverScanArea->setEnabled(false);
    mainMenuBar->actOpenFolder->setEnabled(false);
    mainMenuBar->actCaptureDarkFrameTool->setEnabled(false);
    mainMenuBar->actFavorite->setEnabled(false);
     mainWidget->updateWindowsTitle();
}

//------------------------------------------------------------------------------------
//------------------translate language------------------------------------------------

void EZCAP::languageChanged()
{
    bool isLoaded = false;

    if(translator)
    {
        qApp->removeTranslator(translator);
        delete translator;
        translator = NULL;
    }
    translator = new QTranslator(this);

    if(ix.lang.compare("EN") == 0)
        isLoaded = translator->load(":/language/lan_en_us.qm");
    else if(ix.lang.compare("CN") == 0)
        isLoaded = translator->load(":/language/lan_zh_cn.qm");
    else if(ix.lang.compare("JP") == 0)
        isLoaded = translator->load(":/language/lan_ja_jp.qm");
    else
        isLoaded = translator->load(":/language/lan_en_us.qm");

    if(isLoaded)
    {
        qApp->installTranslator(translator);
        ui->retranslateUi(this);
        emit changeLanguage();//emit the signal to retranslate the ui of each dialog
    }
    else
    {
        qCritical() << "translator loaded failure, can not change language";
    }
}

void EZCAP::changeToEnglish()
{
    ix.lang = "EN";
    iniFileParams.lang = "EN";

    languageChanged();
}

void EZCAP::changeToChinese()
{
    ix.lang = "CN";
    iniFileParams.lang = "CN";

    languageChanged();
}

void EZCAP::changeToJapanese()
{
    ix.lang = "JP";
    iniFileParams.lang = "JP";

    languageChanged();
}

void EZCAP::activeLive()
{
    qDebug("----------- active live----");
        unsigned int ret = QHYCCD_ERROR;
        ret = libqhyccd->BeginQHYCCDLive(camhandle);
        qCritical("--- BeginQHYCCDLive return %d", ret);
}
void EZCAP::switchDebug()
{
    qDebug("----------- switch Debug ----");
    libqhyccd->EnableQHYCCDMessage(mainMenuBar->actDebug->isChecked());
    ui->plainTextEdit_debug->setVisible(mainMenuBar->actDebug->isChecked());
    ui->plainTextEdit_debug->appendPlainText("debug changed ");
}
void EZCAP::switchTestGuid()
{
    qDebug("----------- switch Test Guid ----");
    iniFileParams.testGuider =mainMenuBar->actTestGuid->isChecked();
}
void EZCAP::activeTestMode()
{
    qDebug("-----------switch Test  ----" );
    iniFileParams.autoConnect = mainMenuBar->actTestMode->isChecked();
}

//*******************************************************************
//        菜单栏操作
//*******************************************************************
/**
 * @brief EZCAP::saveParasAsIni
 */
void EZCAP::saveParasAsIni()
{
    //Qt中使用QSettings类读写ini文件
    //QSettings构造函数的第一个参数是ini文件的路径,第二个参数表示针对ini文件,第三个参数可以缺省
    QString path_ezcap = QCoreApplication::applicationDirPath() + "/" + "EZCAP.ini";
    path_ezcap = QDir::toNativeSeparators(path_ezcap);
    QSettings *iniWrite = new QSettings(path_ezcap, QSettings::IniFormat);

    iniWrite->beginGroup("LANGUAGE");
    iniWrite->setValue("lang", iniFileParams.lang);
    iniWrite->endGroup();

    iniWrite->beginGroup("CameraSetting");
    iniWrite->setValue("Gain", iniFileParams.gain);
    iniWrite->setValue("Offset", iniFileParams.offset);
    iniWrite->endGroup();

    iniWrite->beginGroup("AdvancedSetting");
    iniWrite->setValue("TecProtect", iniFileParams.tecPretect);
    iniWrite->setValue("Slowest", iniFileParams.slowestDowload);
    iniWrite->setValue("Clamp", iniFileParams.clamp);
    iniWrite->endGroup();

    iniWrite->beginGroup("StretchSetting");
    iniWrite->setValue("Preview_BPos", iniFileParams.bPos_Preview);
    iniWrite->setValue("Preview_WPos", iniFileParams.wPos_Preview);
    iniWrite->setValue("Focus_BPos", iniFileParams.bPos_Focus);
    iniWrite->setValue("Focus_WPos", iniFileParams.wPos_Focus);
    iniWrite->setValue("Capture_BPos", iniFileParams.bPos_Capture);
    iniWrite->setValue("Capture_WPos", iniFileParams.wPos_Capture);
    iniWrite->setValue("AutoStretchMode", iniFileParams.autoStretchMode);
    iniWrite->endGroup();

    iniWrite->beginGroup("ImageSetting");
    iniWrite->setValue("IgnoreOverScanArea", iniFileParams.ignoreOverScan);
    iniWrite->setValue("OverScanCalibrate", iniFileParams.calibrateOverScan);
    iniWrite->setValue("CalConstant", iniFileParams.calConstant);
    iniWrite->endGroup();

    iniWrite->beginGroup("FilterNames");
    iniWrite->setValue("SlotsNum", iniFileParams.CFWSlotsNum);
    QString key;
    for(int i=0; i<iniFileParams.filterNames_2.count(); i++)
    {
        key = QString("Slots%1").arg(i+1);
        iniWrite->setValue(key, iniFileParams.filterNames_2.at(i));
    }

    iniWrite->endGroup();

    //写入完成后删除指针
    delete iniWrite;
}

/**
 * @brief EZCAP::loadParasFromIni
 */
bool EZCAP::loadParasFromIni()
{
    bool ret = false;

    //读取EZCAP.ini中的内容
    QString path_ezcap = QCoreApplication::applicationDirPath() + "/" + "EZCAP.ini";
    path_ezcap = QDir::toNativeSeparators(path_ezcap);

    QFile *settingFile = new QFile(path_ezcap);
    if(settingFile->exists())
    {
        QSettings *iniRead = new QSettings(path_ezcap, QSettings::IniFormat);

        iniRead->beginGroup("LANGUAGE");
        iniFileParams.lang = iniRead->value("lang", "EN").toString();
        iniRead->endGroup();

        iniRead->beginGroup("CameraSetting");
        iniFileParams.gain = iniRead->value("Gain", 0).toInt();
        iniFileParams.offset = iniRead->value("Offset", 0).toInt();
        iniRead->endGroup();

        iniRead->beginGroup("AdvancedSetting");
        iniFileParams.tecPretect = iniRead->value("TecProtect", false).toBool();
        iniFileParams.slowestDowload = iniRead->value("Slowest", false).toBool();
        iniFileParams.clamp = iniRead->value("Clamp", false).toBool();
        iniRead->endGroup();

        iniRead->beginGroup("StretchSetting");
        iniFileParams.bPos_Preview = iniRead->value("Preview_BPos", 0).toInt();
        iniFileParams.wPos_Preview = iniRead->value("Preview_WPos", 65535).toInt();
        iniFileParams.bPos_Focus = iniRead->value("Focus_BPos", 0).toInt();
        iniFileParams.wPos_Focus = iniRead->value("Focus_WPos", 65535).toInt();
        iniFileParams.bPos_Capture = iniRead->value("Capture_BPos", 0).toInt();
        iniFileParams.wPos_Capture = iniRead->value("Capture_WPos", 65535).toInt();
        iniFileParams.autoStretchMode = iniRead->value("AutoStretchMode", 0).toInt();
        iniRead->endGroup();

        iniRead->beginGroup("ImageSetting");
        iniFileParams.ignoreOverScan = iniRead->value("IgnoreOverScanArea", false).toBool();
        iniFileParams.calibrateOverScan = iniRead->value("OverScanCalibrate", false).toBool();
        iniFileParams.calConstant = iniRead->value("CalConstant", 1000).toUInt();
        iniRead->endGroup();

        iniRead->beginGroup("FilterNames");
        iniFileParams.CFWSlotsNum = iniRead->value("SlotsNum", 0).toInt();
        QString key;
        QString defStr;
        for(int i=0; i<iniFileParams.CFWSlotsNum; i++)
        {
            key = QString("Slots%1").arg(i+1);
            defStr = QString("Pos %1").arg(i+1);
            iniFileParams.filterNames_2.append(iniRead->value(key, defStr).toString());

        }

        iniRead->endGroup();

        iniRead->beginGroup("Factory");
        iniFileParams.testGuider = iniRead->value("TestGuider", false).toBool();
        iniFileParams.autoConnect = iniRead->value("AutoConnect", false).toBool();
        iniFileParams.oldSDK = iniRead->value("oldSDK", false).toBool();
        iniRead->endGroup();

        delete iniRead;


        ret = true;
    }

    delete settingFile;

    return ret;
}

/**
 * @brief EZCAP::CheckWritePath
 * @param filename
 * @return
 */
bool EZCAP::CheckWritePath(QString filename)
{
    //尝试写一个文件，测试文件路径是否安全
    bool ret;
    QString TestName;
    TestName = filename + ".QHY";

    //判断文件是否存在
    if(QFile::exists(TestName))
    {
        QFile::remove(TestName);
    }

    fitHeader_dialog->FITwrite_Common(TestName,10,10,ix.ImgData);

    if(QFile::exists(TestName))
    {
        QFile::remove(TestName);        
        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}

/**
 * @brief EZCAP::exitMainWindow   
 */
void EZCAP::exitMainWindow()
{
    //点击exit菜单项,关闭主窗体
    this->close();
}

/**
 * @brief EZCAP::closeEvent   
 * @param event
 */
void EZCAP::closeEvent( QCloseEvent * event )
{
    //（注意：正常关闭软件时，需要关闭相机，如果当前有拍摄任务，强制停止曝光后，立马CloseQHYCCD(关闭相机)会报错，
    //估计尚未从相机中读完所有图像数据，关闭相机导致句柄异常，
    //  若添加循环等待 则会导致界面卡死，故而暂时采用以下机制。）
    qDebug() << "------------------Close EZCAP------------------";

    int ret;
    bool acceptCloseEvent = true;

    if(ix.cameraState != Camera_Idle)
    {
        QMessageBox::StandardButton choiceBtn;
        choiceBtn = QMessageBox::question(NULL,tr("Close EZCAP"),
                                   tr("Warning: A task is running,Are you sure wanted to exit?"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(choiceBtn == QMessageBox::Yes)
        {
            acceptCloseEvent = true;
        }
        else
        {
            acceptCloseEvent = false;
        }

    }

    if(acceptCloseEvent)
    {
        isSettleDone = true; //退出可能存在的Dither等待循环

        stopTimerTemp(); //stop temperature control Timer

        if(ix.CFWStatus == CFW_Moving)
        {
            ix.CFWStatus = CFW_Idle;  //stop CFW timer
            stopCFWTimer();
        }
        if(ix.plannerState == PlannerStatus_Start)
        {
            ix.plannerState = PlannerStatus_Stop;
        }

        if(camhandle)
        {
            if(ix.cameraState != Camera_Idle)
            {
                //ret = CancelQHYCCDExposingAndReadout(camhandle);//停止曝光
                ret = libqhyccd->CancelQHYCCDExposingAndReadout(camhandle);
                if(ret != QHYCCD_SUCCESS)
                    qCritical("CancelQHYCCDExposingAndReadout: failed");
                else
                    qDebug("CancelQHYCCDExposingAndReadout success!");
            }
            ix.ForceStop = true;  //stop exposing.
            ix.onLiveMode = false;  //stop the exist PreviewLive/FocusLive
            ix.cameraState = Camera_Idle;

            ret = libqhyccd->CloseQHYCCD(camhandle);
            //ret = CloseQHYCCD(camhandle);
            if(ret != QHYCCD_SUCCESS)
                qCritical("CloseQHYCCD: failed");
            else
                qDebug() <<"CloseQHYCCD success";
        }
        ret = libqhyccd->ReleaseQHYCCDResource();
        //ret = ReleaseQHYCCDResource();
        if(ret != QHYCCD_SUCCESS)
            qCritical("ReleaseQHYCCDResource: failed");
        else
            qDebug() << "ReleaseQHYCCDResource success";

        saveParasAsIni();//保存界面设置参数

        //保存FitHeader列表信息
        QString path_csv = QApplication::applicationDirPath() + "/" + "FITHEADER.csv";
        path_csv = QDir::toNativeSeparators(path_csv);
        fitHeader_dialog->saveAsCSV(path_csv);

        //
        planner_dialog->close();
        fitHeader_dialog->close();
        about_dialog->close();
        favorite_dialog->close();
        tempControl_dialog->close();
        phdLink_dialog->close();

        event->accept();
    }
    else
    {
        //取消关闭操作
        event->ignore();
    }
}

/**
 * @brief EZCAP::showCameraChooser  
 */
void EZCAP::showCameraChooser()
{
    unsigned int ret = QHYCCD_ERROR;
    int num = 0;
    QString tempId = "";

    if(ix.isConnected)
    {
        bool acceptDisEvent = true;
        if((ix.cameraState != Camera_Idle || ix.plannerState != PlannerStatus_Done) && show_disconnect_confirm_box)
        {
            QMessageBox::StandardButton choiceBtn;
            choiceBtn = QMessageBox::question(NULL,tr("Disconnect Camera"),
                                              tr("Warning: A task is running,Are you sure you want to disconnect?"),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if(choiceBtn == QMessageBox::Yes)
            {
                acceptDisEvent = true;
            }
            else
            {
                acceptDisEvent = false;
            }
        }

        if(acceptDisEvent)
        {
            qDebug() << "disconnect camera" << endl;

////            this->setWindowTitle(tr("APPVERSION:") + EZCAP_VER);
//            //reset the camera params, so that it will be set again when re-connected.
            ix.lastbinx = 0;
            ix.lastbiny = 0;
            ix.LastDownloadSpeed = -1;
            ix.LastExptime = -1;
            ix.lastGain = -1;
            ix.lastOffset = -1;
            ix.LastMechanicalShutterMode = -1;
            ix.workMode = -1;
            //20200302 lyl 大分辨率相机preview后disconnect，重连小分辨率异常问题解决。
            ix.lastWorkMode=-1;

            ix.isConnected = false;
            isSettleDone = true; //退出可能存在的Dither等待循环

            if(ix.CFWStatus == CFW_Moving)
            {
                stopCFWTimer();
                ix.CFWStatus = CFW_Idle;  //stop CFW timer
            }
            if(ix.plannerState == PlannerStatus_Start)
            {
                ix.plannerState = PlannerStatus_Stop;
            }

            //camera has connected
            if(camhandle)
            {
                if(ix.cameraState != Camera_Idle)
                {
                    //ret = CancelQHYCCDExposingAndReadout(camhandle);
                    ret = libqhyccd->CancelQHYCCDExposingAndReadout(camhandle);
                    if(ret == QHYCCD_SUCCESS)
                        qDebug("CancelQHYCCDExposingAndReadout success!");
                    else
                        qCritical() << "CancelQHYCCDExposingAndReadout failure";

                    while(ix.imageReady == GetSingleFrame_Waiting)
                    {
                        QCoreApplication::processEvents();
                    }
                    ix.imageReady = GetSingleFrame_Failed;
                }
                ix.ForceStop = true;  //stop exposing.
                ix.onLiveMode = false;  //stop the exist PreviewLive/FocusLive
                ix.cameraState = Camera_Idle;
                ret = libqhyccd->CloseQHYCCD(camhandle);
//                ret = CloseQHYCCD(camhandle);
                if(ret != QHYCCD_SUCCESS)
                {
                    qCritical("CloseQHYCCD: failed");
                }
                else
                {
                    camhandle = NULL;
                    qDebug() << "EZCAP  |  CloseQHYCCD success";
                }
            }
            qDebug() << ix.isConnected;
            emit disconnect_camera();
        }
    }
    else
    {
        devList.clear();
        //num = ScanQHYCCD();
        num = libqhyccd->ScanQHYCCD();
        if(num > 0)
        {
            for(int i=0; i<num; i++)
            {                
                ret = libqhyccd->GetQHYCCDId(i, camid);//ret = GetQHYCCDId(i, camid);
                if(ret == QHYCCD_SUCCESS)
                {
                    tempId = QString(QLatin1String(camid));
                    if(tempId.contains("POLEMASTER") == false)
                    {
                        devList.append(tempId);
                    }
                }
                else
                {
                    qCritical() << QString("GetQHYCCDId index[%1] failure").arg(i);
                }
            }
        }
        if(devList.count() > 0)
        {
            if(devList.count() > 1)//test
            {
                cameraChooser->ui->coBox_cameraChooser->clear();
                cameraChooser->ui->coBox_cameraChooser->addItems(devList);
                cameraChooser->setWindowModality(Qt::ApplicationModal);//设置父窗口不可控
                qDebug() << "EZCAP   Show CameraChooser";
                cameraChooser->show();//show cameraChooser window//cameraChooser->ui->coBox_cameraChooser->focus
            }
            else
            {
                tempId = devList.at(0);
                QByteArray pstr = tempId.toLatin1();
                memset(camid,'\0',64);
                memcpy(camid,pstr.data(),pstr.size());
                camhandle = libqhyccd->OpenQHYCCD(camid); //camhandle = OpenQHYCCD(camid);
                if(camhandle != NULL)
                {
                    qDebug() << "EZCAP  OpenQHYCCD success";
                    ix.isConnected = true;
                    ix.CamID = QString(camid);
                    ix.CamModel = ix.CamID.left(ix.CamID.lastIndexOf('-'));
                    //20200220 lyl Add ReadMode Dialog
                    ix.ReadModeList.clear();//清空列表
//                    try{
//                        qDebug() << "EZCAP  GetQHYCCDNumberOfReadModes begain";
//                        ret=libqhyccd->GetQHYCCDNumberOfReadModes(camhandle,&ix.NumberOfReadModes);       //ret=GetQHYCCDNumberOfReadModes(camhandle,&ix.NumberOfReadModes);
//                    }
//                    catch(QException e)
//                    {
//                        qCritical("GetQHYCCDNumberOfReadModes: have no this function !");
//                    }
                    if(libqhyccd->GetQHYCCDNumberOfReadModes){
                        qDebug() << "EZCAP  GetQHYCCDNumberOfReadModes begain";
                        ret=libqhyccd->GetQHYCCDNumberOfReadModes(camhandle,&ix.NumberOfReadModes);  //ret=GetQHYCCDNumberOfReadModes(camhandle,&ix.NumberOfReadModes);
                        if(iniFileParams.autoConnect){
                            qDebug() << "EZCAP  always select first mode under factory mode";
                            ix.NumberOfReadModes=1;
                        }
                        if(iniFileParams.testGuider){
                            //Do guider test            0: EAST RA+   3: WEST RA-   1: NORTH DEC+   2: SOUTH DEC- \n
                            qDebug() << "EZCAP   Do guider test3";
                            for (int guidCount = 0; guidCount < 11; ++guidCount) {
                                libqhyccd->ControlQHYCCDGuide(camhandle, guidCount%4, 250);
//                                sleep(200);
                                #ifdef WIN32
                                    Sleep(200);
                                #else
                                    sleep(200);
                                #endif
                            }
                        }
                        if(ix.NumberOfReadModes>1 &&  ret == QHYCCD_SUCCESS)//20200318排除只有一种readmode模式
                        {
                            readMode = new ReadMode(this);
                            qDebug() << "EZCAP  GetQHYCCDNumberOfReadModes:"<<ix.NumberOfReadModes;
                            for(int i=0; i<ix.NumberOfReadModes; i++)
                            {
                                if (libqhyccd->GetQHYCCDReadModeName(camhandle, i, ix.currentReadModeName) == 0)//if (GetQHYCCDReadModeName(camhandle, i, ix.currentReadModeName) == 0)
                                    ix.ReadModeList.append(ix.currentReadModeName);//列表填充
                            }
                            //20200220 lyl Add ReadMode 可在此处增加对话框显示，选定readmode之后进行SetQHYCCDReadMode()
                            readMode->ui->comboBox_readmode->clear();
                            readMode->ui->comboBox_readmode->addItems(ix.ReadModeList);
                            readMode->setWindowModality(Qt::ApplicationModal);//设置父窗口不可控
                            qDebug() << "EZCAP  Show readMode";//模态对话框//readMode->setModal(true);//readMode->show();
                            readMode->exec();
                        }
                        else if(ix.NumberOfReadModes<=1 &&  ret == QHYCCD_SUCCESS)//20200318仅有一种readmode模式
                        {
                            ret = libqhyccd->SetQHYCCDReadMode(camhandle, ix.currentReadMode);//ret = SetQHYCCDReadMode(camhandle, ix.currentReadMode);
                            if(ret != QHYCCD_SUCCESS)
                                qCritical("SetQHYCCDReadMode: failed");
                            else
                                qDebug() << "EZCAP   SetQHYCCDReadMode success"<<ix.currentReadMode;
                        }
                        else
                            qDebug() << "EZCAP   GetQHYCCDNumberOfReadModes failed";
                    }
                    else {
                        qCritical("GetQHYCCDNumberOfReadModes: have no this function !");
                    }

//                    if(ix.NumberOfReadModes>1 &&  ret == QHYCCD_SUCCESS)//20200318排除只有一种readmode模式
//                    {
//                        readMode = new ReadMode(this);
//                        qDebug() << "EZCAP  GetQHYCCDNumberOfReadModes:"<<ix.NumberOfReadModes;
//                        for(int i=0; i<ix.NumberOfReadModes; i++)
//                        {
//                            if (libqhyccd->GetQHYCCDReadModeName(camhandle, i, ix.currentReadModeName) == 0)//if (GetQHYCCDReadModeName(camhandle, i, ix.currentReadModeName) == 0)
//                                ix.ReadModeList.append(ix.currentReadModeName);//列表填充
//                        }
//                        //20200220 lyl Add ReadMode 可在此处增加对话框显示，选定readmode之后进行SetQHYCCDReadMode()
//                        readMode->ui->comboBox_readmode->clear();
//                        readMode->ui->comboBox_readmode->addItems(ix.ReadModeList);
//                        readMode->setWindowModality(Qt::ApplicationModal);//设置父窗口不可控
//                        qDebug() << "EZCAP  Show readMode";//模态对话框//readMode->setModal(true);//readMode->show();
//                        readMode->exec();
//                    }
//                    else if(ix.NumberOfReadModes<=1 &&  ret == QHYCCD_SUCCESS)//20200318仅有一种readmode模式
//                    {
//                        ret = libqhyccd->SetQHYCCDReadMode(camhandle, ix.currentReadMode);//ret = SetQHYCCDReadMode(camhandle, ix.currentReadMode);
//                        if(ret != QHYCCD_SUCCESS)
//                            qCritical("SetQHYCCDReadMode: failed");
//                        else
//                            qDebug() << "EZCAP   SetQHYCCDReadMode success"<<ix.currentReadMode;
//                    }
//                    else
//                        qDebug() << "EZCAP   GetQHYCCDNumberOfReadModes failed";
                    ret = libqhyccd->SetQHYCCDStreamMode(camhandle, 0);           //ret = SetQHYCCDStreamMode(camhandle, 0);
                    if(ret != QHYCCD_SUCCESS)
                        qCritical("SetQHYCCDStreamMode: failed");
                    else
                        qDebug() << "EZCAP   SetQHYCCDStreamMode success"<<ret;
                    ret = libqhyccd->InitQHYCCD(camhandle);//ret = InitQHYCCD(camhandle);
                    qDebug() << "EZCAP   InitQHYCCD () end"<<ret;
                    if(ret != QHYCCD_SUCCESS)
                    {
                        qCritical("InitQHYCCD: failed");
                        QMessageBox::critical(this,tr("Error"),tr("Camera initialization failed!"),QMessageBox::Ok);
                    }
                    else
                    {
                        qDebug() << "EZCAP   InitQHYCCD success";
                        if(!this->getParamsFromCamera()){
                            qCritical() << "getParamsFromCamera Error";
                            return;
                        }//设置界面控件初始状态
                        emit connect_camera();
                        if(iniFileParams.autoConnect){
                            managerMenu->ui->head_capture->click();
                            managerMenu->ui->pBtn_capture->click();
                        }
                    }

                }
                else
                {
                    qCritical("OpenQHYCCD: %s falied", camid);
                    QMessageBox::critical(this,tr("Warning"),tr("Camera connection failed!"),QMessageBox::Ok);
                }
            }
        }
        else
        {
            QMessageBox::critical(this,tr("Warning"),tr("Not Found QHYCCD Devices!"),QMessageBox::Ok);
        }
    }
}

/**
 * @brief EZCAP::showPlanTable  
 */
void EZCAP::showPlanTable()
{
    qDebug() << "EZCAP   show planner table";
    planner_dialog->show();
}

/**
 * @brief EZCAP::showFavoriteSetting  
 */
void EZCAP::showFavoriteSetting()
{
    qDebug() << "EZCAP   show favorite settting";
    favorite_dialog->show();
}

/**
 * @brief EZCAP::showTempControl  
 */
void EZCAP::showTempControl()
{
    qDebug() << "EZCAP   show tempControl";
    tempControl_dialog->show();
}

/**
 * @brief EZCAP::showCFWControl
 */
void EZCAP::showCFWControl()
{
    qDebug() << "EZCAP    show CFW Control panel";
    cfwControl_dialog->show();
}

/**
 * @brief EZCAP::showPHDLink
 */
void EZCAP::showPHDLink()
{
    qDebug() << "EZCAP    show PHD Link";
    phdLink_dialog->show();
}


void EZCAP::showCaptureDarkFrameTool()
{
    qDebug() << "EZCAP    show Capture Dark Frame Tool";

    QMessageBox::critical(this,tr("Warning"),tr("Dark frame required,Please cover the camera OR Chose the right filter wheel hole!"),QMessageBox::Ok);

    darkFrameTool_dialog->show();
}

/**
 * @brief EZCAP::showAbout 
 */
void EZCAP::showAbout()
{
    qDebug() << "show about";
    about_dialog->show();
}

/**
 * @brief EZCAP::showFITHeaderEditor 
 */
void EZCAP::showFITHeaderEditor()
{
    qDebug() << "show fitHeader table";
    fitHeader_dialog->show();
}

void EZCAP::ignoreOverScanAreaClicked(bool checked)
{
    ix.ignoreOverScan = checked;
    iniFileParams.ignoreOverScan = ix.ignoreOverScan;

    if(ix.workMode == WorkMode_Capture && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {
#if 0
        displayCaptureImage(ix.imageX,ix.imageY);
#else
        displayCaptureImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
#endif
        displayScreenViewImage(100,100,100,100);
    }
}

void EZCAP::calibrateOverScanClicked(bool checked)
{
    ix.calibrateOverScan = checked;
    iniFileParams.calibrateOverScan = ix.calibrateOverScan;
}

/**
 * @brief EZCAP::saveAsFIT 
 */
void EZCAP::saveAsFIT()
{
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save"),
        "",
        tr("Image Files(*.fit)")); //选择路径
    if(filename.isEmpty())
    {
        qCritical() << "saveAsFIT: fileName is empty, can not save";
        return;
    }
    else
    {
        QFileInfo fi(filename);
        lastSavedPath = fi.absolutePath(); //record the iamge saved path

        if(!CheckWritePath(filename))
        {
            QMessageBox::critical(this,tr("Warning"),tr("The selection folder's name has problem and can not be saved there. Please change another folder!"),QMessageBox::Ok);
        }
        else
        {
            //如果是preview模式，则用标准FIT保存函数，忽略掉ignoreOverScanArea的影响
            if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == WorkMode_Preview && ix.imageReady == GetSingleFrame_Success)
            {
                fitHeader_dialog->FITwrite_Common(filename, ix.imageX, ix.imageY, ix.lastImgData);
                qDebug() << "Preview image saveAsFIT " << filename;
            }
            else if(ix.workMode == WorkMode_Focus && ix.lastWorkMode == WorkMode_Focus && ix.imageReady == GetSingleFrame_Success)
            {
                fitHeader_dialog->FITwrite_Common(filename, ix.imageX, ix.imageY, ix.lastImgData);
                qDebug() << "Focus image saveAsFIT " << filename;
            }
            else if(ix.workMode == WorkMode_Capture && ix.lastWorkMode == WorkMode_Capture && ix.imageReady == GetSingleFrame_Success)
            {
                fitHeader_dialog->FITWrite(filename, ix.lastImgData);
                qDebug() << "Capture image saveAsFIT " << filename;
            }
            mainMenuBar->actOpenFolder->setEnabled(true);
        }
    }
}

/**
 * @brief EZCAP::saveAsBMP  
 */
void EZCAP::saveAsBMP()
{
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save"),
        "",
        tr("Image Files(*.bmp)")); 
    if(filename.isEmpty())
    {
        qCritical() << "saveAsBMP: fileName is empty,can not save";
        return;
    }
    else
    {
        QFileInfo fi(filename);
        lastSavedPath = fi.absolutePath(); //record the iamge saved path

        if(ix.workMode == WorkMode_Preview)
        {
            //string holds the text converted to Unicode, 支持中文路径
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
            cvSaveImage(encodedString.constData(), captureImg);
            qDebug() << "Preview image saveAsBMP " << filename;
        }
        else if(ix.workMode == WorkMode_Focus)
        {
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
            cvSaveImage(encodedString.constData(), captureImg);
            qDebug() << "Focus image saveAsBMP " << filename;
        }
        else if(ix.workMode == WorkMode_Capture)
        {
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
            cvSaveImage(encodedString.constData(), captureImg);
            qDebug() << "Capture image saveAsBMP " << filename;
        }
        mainMenuBar->actOpenFolder->setEnabled(true);
    }
}

/**
 * @brief EZCAP::saveAsJPG 
 */
void EZCAP::saveAsJPG()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save"),
        "",
        tr("Image Files(*.jpg)")); 
    if(filename.isEmpty())
    {
        qCritical() << "saveAsJPG: fileName is empty,can not save";
        return;
    }
    else
    {
        QFileInfo fi(filename);
        lastSavedPath = fi.absolutePath(); //record the iamge saved path

        if(ix.workMode == WorkMode_Preview)
        {
            //string holds the text converted to Unicode, 支持中文路径
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
            cvSaveImage(encodedString.constData(), captureImg);
            qDebug() << "Preview image saveAsJPG " << filename;
        }
        else if(ix.workMode == WorkMode_Focus)
        {
            //string holds the text converted to Unicode, 支持中文路径
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
            cvSaveImage(encodedString.constData(), captureImg);
            qDebug() << "Focus image saveAsJPG " << filename;
        }
        else if(ix.workMode == WorkMode_Capture)
        {
            //string holds the text converted to Unicode, 支持中文路径
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
            cvSaveImage(encodedString.constData(), captureImg);
            qDebug() << "Capture image saveAsJPG " << filename;
        }
        mainMenuBar->actOpenFolder->setEnabled(true);
    }
}

/**
 * @brief EZCAP::openFolder
 */
void EZCAP::openFolder()
{
    if(lastSavedPath.isEmpty())
    {
        qCritical() << "openFolder: empty path";
    }
    else
    {
        qDebug() << "openFolder(last saved foder)" << lastSavedPath;

        lastSavedPath = QDir::toNativeSeparators(lastSavedPath);
        QUrl url = QUrl::fromLocalFile(lastSavedPath);
        QDesktopServices::openUrl(url); //open folder
    }
}

//-----------------------------------------------------------------

void EZCAP::cfwPositionChanged()
{
    if(!ix.isConnected){
        qDebug() << QString("Trigger cfwPositionChangeed But not connected  (char code) %1").arg(ix.dstCfwPos);
        return;
    }
    qDebug() << QString("move CFW to: (char code) %1").arg(ix.dstCfwPos);
    ix.CFWStatus = CFW_Moving;

    runCFWOrder->start(); //start thread to set the CFW posiion

    this->startCFWTimer();//开启色轮状态查询  定时器
    cfwControl_dialog->startCFWProgressBar(); //开启进度条

    runCFWOrder->wait();
}

/**
 * @brief EZCAP::scaleImage  改变图像显示比例
 * @param factor
 */
void EZCAP::scaleImage(double factor)
{
    scaleFactor = factor;
    scrollArea_ImgShow->setWidgetResizable(false);
    ui->label_ImgShow->resize(ix.imageX*factor, ix.imageY*factor);

    if(ix.workMode == WorkMode_Capture && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {
        screenViewBoxResize();
    }
}

/**
 * @brief EZCAP::zoomFitWindow  
 */
void EZCAP::zoomFitWindow()
{
    if(ix.zoomMode != Zoom_Auto)
    {
        qDebug() << "zoom auto mode";
        ix.zoomMode = Zoom_Auto;
        scrollArea_ImgShow->setWidgetResizable(true);
    }
}

/**
 * @brief EZCAP::zoom0_25X   
 */
void EZCAP::zoom0_25X()
{
    if(scaleFactor != 0.25 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        qDebug() << "zoom 25%";
        scaleFactor = 0.25;
        ix.zoomMode = Zoom_SpecifyScaling;
        scaleImage(scaleFactor);
    }
}

/**
 * @brief EZCAP::zoom0_5X    
 */
void EZCAP::zoom0_5X()
{
    if(scaleFactor != 0.5 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        qDebug() << "zoom 50%";
        scaleFactor = 0.5;
        ix.zoomMode = Zoom_SpecifyScaling;
        scaleImage(scaleFactor);
    }
}

/**
 * @brief EZCAP::zoom0_75X     
 */
void EZCAP::zoom0_75X()
{
    if(scaleFactor != 0.75 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        qDebug() << "zoom 75%";
        scaleFactor = 0.75;
        ix.zoomMode = Zoom_SpecifyScaling;
        scaleImage(scaleFactor);
    }
}
/**
 * @brief EZCAP::zoom1X   
 */
void EZCAP::zoom1X()
{
    if(scaleFactor != 1.0 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        qDebug() << "zoom 100%";
        scaleFactor = 1.0;
        ix.zoomMode = Zoom_SpecifyScaling;
        scaleImage(scaleFactor);
    }
}

/**
 * @brief EZCAP::zoom1_5X   
 */
void EZCAP::zoom1_5X()
{
    if(scaleFactor != 1.5 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        qDebug() << "zoom 150%";
        scaleFactor = 1.5;
        ix.zoomMode = Zoom_SpecifyScaling;
        scaleImage(scaleFactor);
    }
}

/**
 * @brief EZCAP::zoom2X    
 */
void EZCAP::zoom2X()
{
    if(scaleFactor != 2.0 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        qDebug() << "zoom 200%";
        scaleFactor = 2.0;
        ix.zoomMode = Zoom_SpecifyScaling;
        scaleImage(scaleFactor);
    }
}

//-------------------------------------------------------------------
void EZCAP::hScrollBarValueChanged(int value)
{
    if(ix.workMode == WorkMode_Preview)
    {
        hScrollBarValue_Pre = value;
    }
    else if(ix.workMode == WorkMode_Capture)
    {
        hScrollBarValue_Cap = value;
    }
}

void EZCAP::vScrollBarValueChanged(int value)
{
    if(ix.workMode == WorkMode_Preview)
    {
        vScrollBarValue_Pre = value;
    }
    else if(ix.workMode == WorkMode_Capture)
    {
        vScrollBarValue_Cap = value;
    }
}

//--------------------------------------------------------------------------------
void EZCAP::favorite_pBtn_calibrateFrame_clicked()
{
    if(ix.isConnected && ix.workMode == WorkMode_Capture && ix.isCalibrateFrame == false)
    {
        qDebug() << "start calibrate frame...";
        ix.isCalibrateFrame = true;
        favorite_dialog->ui->pBtn_calibrateFrame->setStyleSheet("border-image: url(:/image/buttonDown.bmp);");
        managerMenu->ui->pBtn_capture->setEnabled(false);
        //camera has connected and it worked with capture mode
        libqhyccd->QHYCCDI2CTwoWrite(camhandle,0x30BA,0x000b);
        libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN,0.0);
        //QHYCCDI2CTwoWrite(camhandle,0x30BA,0x000b);
        //SetQHYCCDParam(camhandle,CONTROL_GAIN,0.0);
#ifdef WIN32
        ::Sleep(10);
#else
        ::usleep(10000);
#endif
        //SetQHYCCDParam(camhandle,CONTROL_GAIN,1.0);
        libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN,1.0);
#ifdef WIN32
        ::Sleep(10);
#else
        ::usleep(10000);
#endif
        libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN,2.0);
        //SetQHYCCDParam(camhandle,CONTROL_GAIN,2.0);
#ifdef WIN32
        ::Sleep(10);
#else
        ::usleep(10000);
#endif
        libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN,3.0);
        //SetQHYCCDParam(camhandle,CONTROL_GAIN,3.0);
#ifdef WIN32
        ::Sleep(10);
#else
        ::usleep(10000);
#endif
        libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN,4.0);
        //SetQHYCCDParam(camhandle,CONTROL_GAIN,4.0);
#ifdef WIN32
        ::Sleep(10);
#else
        ::usleep(10000);
#endif
        //SetQHYCCDParam(camhandle,CONTROL_GAIN,ix.gain);
        libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN,ix.gain);
        //wait for the calibrate frame finished.
        QTime dieTime= QTime::currentTime().addSecs(3);
        while( QTime::currentTime() < dieTime )
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
        libqhyccd->QHYCCDI2CTwoWrite(camhandle,0x30BA,0x000a);
        //QHYCCDI2CTwoWrite(camhandle,0x30BA,0x000a);
        managerMenu->ui->pBtn_capture->setEnabled(true);
        favorite_dialog->ui->pBtn_calibrateFrame->setStyleSheet("border-image: url(:/image/button.bmp);");
        ix.isCalibrateFrame = false;
        qDebug() << "calibrate frame done.";
    }
}

void EZCAP::favorite_pBtn_getRealTemp_clicked()
{
    unsigned short temp70,temp55,altemp;
    double slope,T0;
    double temp;
    char chartemp[32];
    //QHYCCDI2CTwoWrite(camhandle,0x30b4,0x11);
    //temp70 = QHYCCDI2CTwoRead(camhandle,0x30c6);
    //temp55 = QHYCCDI2CTwoRead(camhandle,0x30c8);
    //altemp = QHYCCDI2CTwoRead(camhandle,0x30b2);
    libqhyccd->QHYCCDI2CTwoWrite(camhandle,0x30b4,0x11);
    temp70 = libqhyccd->QHYCCDI2CTwoRead(camhandle,0x30c6);
    temp55 = libqhyccd->QHYCCDI2CTwoRead(camhandle,0x30c8);
    altemp = libqhyccd->QHYCCDI2CTwoRead(camhandle,0x30b2);
    if(temp70 == temp55)
    {
        favorite_dialog->ui->pBtn_getRealTemp->setText(QString("error"));
    }
    else
    {
        slope = 15.0 / (temp70 - temp55);
        T0 = 70.0 - temp70*slope;

        temp = slope * altemp + T0;
        sprintf(chartemp,"%.2f",temp);
        favorite_dialog->ui->pBtn_getRealTemp->setText(QString(chartemp));
    }
}
//20201127 lyl SensorChamberCyclePUMP
void EZCAP::favorite_pBtn_controlSensorChamberCyclePUMP_clicked()
{
    if(favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->isChecked()){
        libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SensorChamberCycle_PUMP,1);//open
        this->startPumpTimer();
    }
    else {
        libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SensorChamberCycle_PUMP,0);//close
    }
}

//--------------------------------------------------------------------------------

void EZCAP::setStretchLUT(unsigned short W, unsigned short B)
{
    double ratio;
    uint32_t pixel;

    ratio = double((W - B)) / 256.0;
    if(ratio == 0)
        ratio = 1;

    for(int i=0; i < 65536; i++)
    {
        if(i > B)
        {
            pixel = (i - B) / ratio;
            if(pixel > 255)
                pixel = 255;
        }
        else
        {
            pixel = 0;
        }

        ix.StretchLUT[i] = pixel;
    }
}

//********************************************************************************
//                              preview tab页操作
//********************************************************************************
/**
 * @brief EZCAP::mgrMenu_pBtn_cross_clicked  
 */
void EZCAP::mgrMenu_pBtn_cross_clicked()
{
    if (ix.crossBtnState == Cross_Disabled)
    {
        managerMenu->ui->pBtn_cross->setText("+");
        ix.crossBtnState = Cross_Enabled;
    }
    else if(ix.crossBtnState == Cross_Enabled)
    {
        managerMenu->ui->pBtn_cross->setText(tr("Cross"));
        ix.crossBtnState = Cross_Disabled;
    }

    if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {   //if already had image, redisplay it
#if 0
        displayPreviewImage(ix.imageX ,ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
#else
        displayPreviewImage_Ex(ix.lastImageX, ix.lastImageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.lastImgData);
#endif
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_grid_clicked   
 */
void EZCAP::mgrMenu_pBtn_grid_clicked()
{
    if (ix.gridBtnState == Grid_Disabled)
    {
        managerMenu->ui->pBtn_grid->setText(QString::fromUtf8("▓▓"));
        ix.gridBtnState = Grid_Enabled;
    }
    else if(ix.gridBtnState == Grid_Enabled)
    {
        managerMenu->ui->pBtn_grid->setText(tr("Grid"));
        ix.gridBtnState = Grid_Disabled;
    }

    if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {
#if 0
        displayPreviewImage(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
#else
        displayPreviewImage_Ex(ix.lastImageX, ix.lastImageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.lastImgData);
#endif
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_circle_clicked
 */
void EZCAP::mgrMenu_pBtn_circle_clicked()
{
    if (ix.circleBtnState == Circle_Disabled)
    {
        managerMenu->ui->pBtn_circle->setText(QString::fromUtf8("◎"));
        ix.circleBtnState = Circle_Enabled;
    }
    else if(ix.circleBtnState == Circle_Enabled)
    {
        managerMenu->ui->pBtn_circle->setText(tr("Circle"));
        ix.circleBtnState = Circle_Disabled;
    }

    if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {
#if 0
        displayPreviewImage(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
#else
        displayPreviewImage_Ex(ix.lastImageX, ix.lastImageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.lastImgData);
#endif
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_preview_clicked  
 */
void EZCAP::mgrMenu_pBtn_preview_clicked()
{
    unsigned int ret;
    QTime prev_time = QTime::currentTime();
    int prev_betweenTime;

    managerMenu->ui->pBtn_live_preview->setEnabled(false);
    managerMenu->ui->pBtn_preview->setChecked(true);
    managerMenu->ui->head_capture->setCheckable(false);
    managerMenu->ui->head_focus->setCheckable(false);

    memset(ix.ImgData, 0, ix.ccdMaxImgW * ix.ccdMaxImgH * 2);
    memset(ix.OutputData8, 0, ix.ccdMaxImgW * ix.ccdMaxImgH);
    memset(ix.dispIplImgData24, 0, (ix.ccdMaxImgW+3)/4*4 * ix.ccdMaxImgH * 3);

    if(ix.cameraState == Camera_Idle)
    {
        qDebug() << "----preview start----";

        ix.cameraState = Camera_Waiting;
        ix.imageReady = GetSingleFrame_Waiting;

        if(ix.Exptime != ix.LastExptime)
        {
            //ret = SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.LastExptime = ix.Exptime;
                qDebug() << "SetQHYCCDParam: CONTROL_EXPOSURE" << ix.Exptime << "ms";
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_EXPOSURE failure";
            }
        }

        if(ix.canUsbtraffic && ix.lastUsbtraffic != ix.usbtraffic)
        {
            //ret = SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.lastUsbtraffic = ix.usbtraffic;
                qDebug() << QString("SetQHYCCDParam CONTROL_USBTRAFFIC %1").arg(ix.usbtraffic);
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_USBTRAFFIC failure";
            }
        }

        if(ix.canHighSpeed && ix.LastDownloadSpeed != ix.DownloadSpeed)
        {
            //ret = SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed); //set speed
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.LastDownloadSpeed = ix.DownloadSpeed;
                qDebug() << "SetQHYCCDParam: CONTROL_SPEED" << ix.DownloadSpeed;
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_SPEED failure";
            }
        }

        if(ix.lastbinx != ix.binx || ix.lastbiny != ix.biny || ix.lastWorkMode != ix.workMode)
        {
            //ret = SetQHYCCDBinMode(camhandle, ix.binx, ix.biny);
            ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.binx, ix.biny);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.lastbinx = ix.binx;
                ix.lastbiny = ix.biny;
                qDebug() << "SetQHYCCDBinMode:" << ix.binx << ix.biny;
            }
            else
            {
                qCritical() << "SetQHYCCDBinMode failure";
            }
            ret = libqhyccd->SetQHYCCDResolution(camhandle, 0, 0, ix.ccdMaxImgW / ix.binx, ix.ccdMaxImgH / ix.biny);
            //ret = SetQHYCCDResolution(camhandle, 0, 0, ix.ccdMaxImgW / ix.binx, ix.ccdMaxImgH / ix.biny);
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() << "SetQHYCCDResolution:0, 0," << ix.ccdMaxImgW / ix.binx << ix.ccdMaxImgH / ix.biny;
            }
            else
            {
                qCritical() << "SetQHYCCDResolution failure";
            }

            ix.lastWorkMode = ix.workMode;
        }

        ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳

        //进度条清0
        managerMenu->ui->proBar_preview->setValue(0);
        managerMenu->ui->proBar_previewTime->setValue(0);

        ix.cameraState = Camera_Exposing;
        prev_time.start();
        //ret = ExpQHYCCDSingleFrame(camhandle); //start exposure...
        ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            qDebug() <<"ExpQHYCCDSingleFrame success, wait...";
        #ifdef WIN32
            Sleep(200);
        #else
            usleep(200000);
        #endif

        }
        else if(ret == QHYCCD_READ_DIRECTLY)
        {
            qDebug() << "ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!";
        }
        else
        {
            qCritical("ExpQHYCCDSingleFrame failure");
        }

        prev_betweenTime = prev_time.elapsed();//返回从上次start()或restart()开始以来的时间差，单位ms
        while (prev_betweenTime < ix.Exptime)
        {
            if((prev_betweenTime + 1000) >= ix.Exptime)
            {
                break;
            }
            prev_betweenTime = prev_time.elapsed();
            managerMenu->ui->proBar_previewTime->setValue(prev_betweenTime*100/ix.Exptime);
            QApplication::processEvents();//防止长时间导致界面假死
#ifdef WIN32
            Sleep(1);
#else
            usleep(1000);
#endif
        }
        managerMenu->ui->proBar_previewTime->setValue(100);

        ix.cameraState = Camera_Reading;
        downloadPre = new DownloadPreThread(this);
        connect(downloadPre, SIGNAL(finished()), downloadPre, SLOT(deleteLater()));
        downloadPre->start();

        if(ix.canCooler)
        {
            mainWidget->stopTimerTemp();//停止温控定时器
        }

        while(ix.imageReady == GetSingleFrame_Waiting)
        {
            //处理下载进度条
            managerMenu->ui->proBar_preview->setValue(libqhyccd->GetQHYCCDReadingProgress(camhandle));
            //managerMenu->ui->proBar_preview->setValue(GetQHYCCDReadingProgress(camhandle));
#ifdef WIN32
            Sleep(1);
#else
            usleep(1000);
#endif
            QApplication::processEvents();//响应界面操作，防止界面假死
        }

        managerMenu->ui->proBar_preview->setValue(100);

        if(ix.canCooler)
        {
            mainWidget->startTimerTemp();//开启温控定时器
        }

        if(ix.imageReady == GetSingleFrame_Success)
        {
            emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息
            //display preview image
            qDebug() << "display image..." << ix.imageX << ix.imageY;
#if 0
            displayPreviewImage(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
#else
            displayPreviewImage_Ex(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.ImgData);
#endif

            //display the histogram image
            qDebug() << "display histogram...";
            displayHistogramImage(ix.imageX, ix.imageY, ix.ImgData);

            //get overscan balck value, used for auto histogram
            getOverScanBlack(ix.ImgData, ix.imageX, ix.imageY);

            noImgInWorkMode = false;
        }

        qDebug() << "----preview end----";
        managerMenu->ui->pBtn_preview->setChecked(false);
        managerMenu->ui->pBtn_live_preview->setEnabled(true);
        managerMenu->ui->head_capture->setCheckable(true);
        managerMenu->ui->head_focus->setCheckable(true);

        ix.cameraState = Camera_Idle;

        mainMenuBar->actSaveBMP->setEnabled(true);//设置保存图像菜单项可用
        mainMenuBar->actSaveFIT->setEnabled(true);
        mainMenuBar->actSaveJPG->setEnabled(true);

    }

}

/**
 * @brief EZCAP::mgrMenu_pBtn_live_preview_clicked     
 */
void EZCAP::mgrMenu_pBtn_live_preview_clicked()
{
    ix.onLiveMode = !ix.onLiveMode;

    if(ix.cameraState == Camera_Idle && ix.onLiveMode)
    {
        unsigned int ret;
        QTime sTime = QTime::currentTime();

        managerMenu->ui->pBtn_live_preview->setChecked(true);//设置live按钮处于按下状态
        managerMenu->ui->pBtn_preview->setEnabled(false);//禁用preview按钮
        managerMenu->ui->head_capture->setCheckable(false);
        managerMenu->ui->head_focus->setCheckable(false);

        memset(ix.ImgData,0,ix.ccdMaxImgW * ix.ccdMaxImgH * 2);
        memset(ix.OutputData8,0,ix.ccdMaxImgW * ix.ccdMaxImgH);        
        memset(ix.dispIplImgData24, 0, (ix.ccdMaxImgW+3)/4*4 * ix.ccdMaxImgH * 3);

        qDebug() << "----begin preview live----";

        ix.cameraState = Camera_Waiting;

        while(ix.onLiveMode)
        {
            ix.imageReady = GetSingleFrame_Waiting;
            if(ix.Exptime != ix.LastExptime)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
                //ret = SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
                if(ret == QHYCCD_SUCCESS)
                {
                    qDebug() << "SetQHYCCDParam: CONTROL_EXPOSURE, [ms] " << ix.Exptime;
                    ix.LastExptime = ix.Exptime;
                }
                else
                {
                    qCritical() << "SetQHYCCDParam: CONTROL_EXPOSURE failure";
                }
            }

            if(ix.canUsbtraffic && ix.lastUsbtraffic != ix.usbtraffic)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
                //ret = SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.lastUsbtraffic = ix.usbtraffic;
                    qDebug() << QString("SetQHYCCDParam CONTROL_USBTRAFFIC %1").arg(ix.usbtraffic);
                }
                else
                {
                    qCritical() << "SetQHYCCDParam: CONTROL_USBTRAFFIC failure";
                }
            }

            if(ix.canHighSpeed && ix.LastDownloadSpeed != ix.DownloadSpeed)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed); //set speed
                //ret = SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.LastDownloadSpeed = ix.DownloadSpeed;
                    qDebug() << "SetQHYCCDParam: CONTROL_SPEED" << ix.DownloadSpeed;
                }
                else
                {
                    qCritical() << "SetQHYCCDParam: CONTROL_SPEED failure";
                }
            }

            if(ix.lastbinx != ix.binx || ix.lastbiny != ix.biny || ix.lastWorkMode != ix.workMode)
            {
                ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.binx, ix.biny);
                //ret = SetQHYCCDBinMode(camhandle, ix.binx, ix.biny);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.lastbinx = ix.binx;
                    ix.lastbiny = ix.biny;
                    qDebug() << "SetQHYCCDBinMode:" << ix.binx << ix.biny;
                }
                else
                {
                    qCritical("SetQHYCCDBinMode failure");
                }
                ret = libqhyccd->SetQHYCCDResolution(camhandle, 0, 0, ix.ccdMaxImgW / ix.binx, ix.ccdMaxImgH / ix.biny);
                //ret = SetQHYCCDResolution(camhandle, 0, 0, ix.ccdMaxImgW / ix.binx, ix.ccdMaxImgH / ix.biny);
                if(ret == QHYCCD_SUCCESS)
                {
                    qDebug() << "SetQHYCCDResolution:0, 0," << ix.ccdMaxImgW / ix.binx << ix.ccdMaxImgH / ix.biny;
                }
                else
                {
                    qCritical() << "SetQHYCCDResolution failure";
                }

                ix.lastWorkMode = ix.workMode;
            }

            ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳

            ix.cameraState = Camera_Exposing;
            sTime.start();
            //ret = ExpQHYCCDSingleFrame(camhandle); //start exposure...
            ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() <<"ExpQHYCCDSingleFrame success, wait...";
            #ifdef WIN32
                Sleep(200);
            #else
                usleep(200000);
            #endif

            }
            else if(ret == QHYCCD_READ_DIRECTLY)
            {
                qDebug() << "ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!";
            }
            else
            {
                qCritical("ExpQHYCCDSingleFrame failed");
            }

            //进度条复位
            managerMenu->ui->proBar_previewTime->setValue(0);
            managerMenu->ui->proBar_preview->setValue(0);

            int prev_betweenTime = sTime.elapsed();//返回从上次start()或restart()开始以来的时间差，单位ms
            while (prev_betweenTime < ix.Exptime)
            {
                if((prev_betweenTime + 1000) >= ix.Exptime)
                {
                    break;
                }
                prev_betweenTime = sTime.elapsed();
                managerMenu->ui->proBar_previewTime->setValue(prev_betweenTime*100/ix.Exptime);
                QApplication::processEvents();//防止长时间导致界面假死
#ifdef WIN32
                Sleep(1);
#else
                usleep(1000);
#endif
            }
            managerMenu->ui->proBar_previewTime->setValue(100);

            //开启线程，获取图像数据
            qDebug() << "downloading preview frame";
            ix.cameraState = Camera_Reading;
            downloadPre = new DownloadPreThread(this);
            connect(downloadPre, SIGNAL(finished()), downloadPre, SLOT(deleteLater()));
            downloadPre->start();

            if(ix.canCooler)
            {
                mainWidget->stopTimerTemp();//停止温控定时器
            }

            while(ix.imageReady == GetSingleFrame_Waiting)
            {
                //处理下载进度条
                //managerMenu->ui->proBar_preview->setValue(GetQHYCCDReadingProgress(camhandle));
                managerMenu->ui->proBar_preview->setValue(libqhyccd->GetQHYCCDReadingProgress(camhandle));
            #ifdef WIN32
                Sleep(1);
            #else
                usleep(1000);
            #endif
                QApplication::processEvents();//响应界面操作，防止界面假死
            }
            managerMenu->ui->proBar_preview->setValue(100);

            if(ix.canCooler)
            {
                mainWidget->startTimerTemp();//开启温控定时器
            }

            if(ix.imageReady == GetSingleFrame_Success)
            {
                emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息
                //显示图像
                qDebug() << "display image...";
#if 0
                displayPreviewImage(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
#else
                displayPreviewImage_Ex(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.ImgData);
#endif

                //显示直方图
                qDebug() << "display histogram...";
                displayHistogramImage(ix.imageX, ix.imageY, ix.ImgData);

                //get the overscan balck value, used for auto histogram
                getOverScanBlack(ix.ImgData, ix.imageX, ix.imageY);

                noImgInWorkMode = false;
            }

            QApplication::processEvents();//响应界面是否发送了停止信号
        }
        qDebug() << "----preivew live end----";

        ix.cameraState = Camera_Idle;

        managerMenu->ui->pBtn_live_preview->setChecked(false);
        managerMenu->ui->pBtn_preview->setEnabled(true);
        managerMenu->ui->head_capture->setCheckable(true);
        managerMenu->ui->head_focus->setCheckable(true);

        mainMenuBar->actSaveBMP->setEnabled(true);//设置保存图像菜单项可用
        mainMenuBar->actSaveFIT->setEnabled(true);
        mainMenuBar->actSaveJPG->setEnabled(true);

    }
}

/**
 * @brief EZCAP::displayPreviewImage      
 * @param x
 * @param y
 * @param boxWidth
 * @param boxHeight
 * @param boxX
 * @param boxY
 */
void EZCAP::displayPreviewImage(int x, int y, int boxWidth, int boxHeight, int boxX, int boxY)
{
    unsigned char pixel;
    long s,k,n;
    IplImage *Img = NULL;

    Img = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
    Img->imageData = (char*)ix.dispIplImgData24;

    if(captureImg)
    {
        cvReleaseImage(&captureImg);
        captureImg = NULL;
    }
    captureImg = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 3);

    //---convert 16bit data to 8bit data
    s = 0;
    k = 0;
    for(int i = 0; i < y; i++)
    {
        for(int j = 0; j < x; j++)
        {
            // look up the Stretch LUT table to convert current 16bit data to 8bit
            pixel = ix.StretchLUT[ix.lastImgData[s]+ix.lastImgData[s+1]*256];
            ix.OutputData8[k] = pixel;
            s += 2;
            k += 1;
        }
    }

    //-----copy 8bit data to opencv 8bit 3channels IplImage
    n=0;
    k=0;
    for(int j=0; j<y; j++)
    {
        for(int i=0; i<x; i++)
        {
            ix.dispIplImgData24[n] = ix.OutputData8[k];
            ix.dispIplImgData24[n+1] = ix.OutputData8[k];
            ix.dispIplImgData24[n+2] = ix.OutputData8[k];
            n += 3;
            k += 1;
        }
        n += Img->widthStep - Img->nChannels*Img->width;
    }


    //export to the Screen View image
    cvResize(Img,ScreenViewImg,1);

    //add image shape
    cvRectangle(Img, cvPoint(boxX-boxWidth/2,boxY-boxHeight/2), cvPoint(boxX+boxWidth/2,boxY+boxHeight/2), CV_RGB(255,0,0), 1, 8, 0);
    cvLine(Img, cvPoint(boxX-3,boxY), cvPoint(boxX+3,boxY), CV_RGB(255,0,0), 1, 8, 0);
    cvLine(Img, cvPoint(boxX,boxY-3), cvPoint(boxX,boxY+3), CV_RGB(255,0,0), 1, 8, 0);

    cvPutText(Img, "Focus Area", cvPoint(boxX-boxWidth/2,boxY-boxHeight/2-3), &QHYFont, CV_RGB(255,0,0));

    if (managerMenu->ui->pBtn_cross->text() == "+")
    {
        cvLine(Img, cvPoint(0,Img->height/2), cvPoint(Img->width/2-4 , Img->height/2), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(Img->width/2+4,Img->height/2), cvPoint(Img->width, Img->height/2), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(Img->width /2,0), cvPoint(Img->width/2,Img->height/2-4), CV_RGB(0,255,0),1, 8, 0 );
        cvLine(Img, cvPoint(Img->width /2,Img->height/2+4), cvPoint(Img->width/2,Img->height), CV_RGB(0,255,0), 1, 8, 0);
    }

    if (managerMenu->ui->pBtn_grid->text() == QString::fromUtf8("▓▓"))
    {
        cvLine(Img, cvPoint(0,Img->height/4), cvPoint(Img->width,Img->height/4), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(0,Img->height/2), cvPoint(Img->width,Img->height/2), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(0,Img->height*3/4), cvPoint(Img->width,Img->height*3/4), CV_RGB(0,255,0), 1, 8, 0);

        cvLine(Img, cvPoint(Img->width/4,0), cvPoint(Img->width/4,Img->height), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(Img->width/2,0), cvPoint(Img->width/2,Img->height), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(Img->width*3/4,0), cvPoint(Img->width*3/4,Img->height), CV_RGB(0,255,0), 1, 8, 0);
    }

    if (managerMenu->ui->pBtn_circle->text() == QString::fromUtf8("◎"))
    {
        cvCircle(Img, cvPoint(Img->width /2,Img->height /2), Img->height /2, CV_RGB(255,255,0), 1, 8, 0);
        cvCircle(Img, cvPoint(Img->width /2,Img->height /2), Img->height /6, CV_RGB(255,255,0), 1, 8, 0);
        cvCircle(Img, cvPoint(Img->width /2,Img->height /2), 10, CV_RGB(200,200,0), 1, 8, 0);
    }

    //display image size into statusBar
    QString str1 = QString::number(x) + "x" + QString::number(y);
    statusLabel_imgSize->setText(str1);

    //show the image in label
    if(qImg_preview)
    {
        delete qImg_preview;
        qImg_preview = NULL;
    }
    cvCvtColor(Img,Img,CV_BGR2RGB);//iplimage BGR to image RGB

    cvCopy(Img,captureImg);//拷贝Img数据到captureImg中，记录当前preview图像，用于保存时使用

    qImg_preview = this->IplImageToQImage(Img);
    if(qImg_preview)
    {
        scrollArea_ImgShow->setWidgetResizable(false);
        ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_preview));
        ui->label_ImgShow->adjustSize();
    }

    cvReleaseImage(&Img);

}

void EZCAP::displayPreviewImage_Ex(int x, int y, int boxW, int boxH, int boxCx, int boxCy, unsigned char *dataBuf)
{
    unsigned char pixel;
    long s,k,n;
    IplImage *Img = NULL;

    Img = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
    Img->imageData = (char*)ix.dispIplImgData24;

    if(captureImg)
    {
        cvReleaseImage(&captureImg);
        captureImg = NULL;
    }
    captureImg = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 3);

    //---convert 16bit data to 8bit data
    s = 0;
    k = 0;
    for(int i = 0; i < y; i++)
    {
        for(int j = 0; j < x; j++)
        {
            // look up the Stretch LUT table to convert current 16bit data to 8bit
            pixel = ix.StretchLUT[dataBuf[s]+dataBuf[s+1]*256];
            ix.OutputData8[k] = pixel;
            s += 2;
            k += 1;
        }
    }

    //-----copy 8bit data to opencv 8bit 3channels IplImage
    n=0;
    k=0;
    for(int j=0; j<y; j++)
    {
        for(int i=0; i<x; i++)
        {
            ix.dispIplImgData24[n] = ix.OutputData8[k];
            ix.dispIplImgData24[n+1] = ix.OutputData8[k];
            ix.dispIplImgData24[n+2] = ix.OutputData8[k];
            n += 3;
            k += 1;
        }
        n += Img->widthStep - Img->nChannels*Img->width;
    }


    //export to the Screen View image
    cvResize(Img,ScreenViewImg,1);

    //add image shape
    cvRectangle(Img, cvPoint(boxCx-boxW/2,boxCy-boxH/2), cvPoint(boxCx+boxW/2,boxCy+boxH/2), CV_RGB(255,0,0), 1, 8, 0);
    cvLine(Img, cvPoint(boxCx-3,boxCy), cvPoint(boxCx+3,boxCy), CV_RGB(255,0,0), 1, 8, 0);
    cvLine(Img, cvPoint(boxCx,boxCy-3), cvPoint(boxCx,boxCy+3), CV_RGB(255,0,0), 1, 8, 0);

    cvPutText(Img, "Focus Area", cvPoint(boxCx-boxW/2,boxCy-boxH/2-3), &QHYFont, CV_RGB(255,0,0));

    if (managerMenu->ui->pBtn_cross->text() == "+")
    {
        cvLine(Img, cvPoint(0,Img->height/2), cvPoint(Img->width/2-4 , Img->height/2), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(Img->width/2+4,Img->height/2), cvPoint(Img->width, Img->height/2), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(Img->width /2,0), cvPoint(Img->width/2,Img->height/2-4), CV_RGB(0,255,0),1, 8, 0 );
        cvLine(Img, cvPoint(Img->width /2,Img->height/2+4), cvPoint(Img->width/2,Img->height), CV_RGB(0,255,0), 1, 8, 0);
    }

    if (managerMenu->ui->pBtn_grid->text() == QString::fromUtf8("▓▓"))
    {
        cvLine(Img, cvPoint(0,Img->height/4), cvPoint(Img->width,Img->height/4), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(0,Img->height/2), cvPoint(Img->width,Img->height/2), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(0,Img->height*3/4), cvPoint(Img->width,Img->height*3/4), CV_RGB(0,255,0), 1, 8, 0);

        cvLine(Img, cvPoint(Img->width/4,0), cvPoint(Img->width/4,Img->height), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(Img->width/2,0), cvPoint(Img->width/2,Img->height), CV_RGB(0,255,0), 1, 8, 0);
        cvLine(Img, cvPoint(Img->width*3/4,0), cvPoint(Img->width*3/4,Img->height), CV_RGB(0,255,0), 1, 8, 0);
    }

    if (managerMenu->ui->pBtn_circle->text() == QString::fromUtf8("◎"))
    {
        cvCircle(Img, cvPoint(Img->width /2,Img->height /2), Img->height /2, CV_RGB(255,255,0), 1, 8, 0);
        cvCircle(Img, cvPoint(Img->width /2,Img->height /2), Img->height /6, CV_RGB(255,255,0), 1, 8, 0);
        cvCircle(Img, cvPoint(Img->width /2,Img->height /2), 10, CV_RGB(200,200,0), 1, 8, 0);
    }

    //display image size into statusBar
    QString str1 = QString::number(x) + "x" + QString::number(y);
    statusLabel_imgSize->setText(str1);

    //show the image in label
    if(qImg_preview)
    {
        delete qImg_preview;
        qImg_preview = NULL;
    }
    cvCvtColor(Img,Img,CV_BGR2RGB);//iplimage BGR to image RGB

    cvCopy(Img,captureImg);//拷贝Img数据到captureImg中，记录当前preview图像，用于保存时使用

    qImg_preview = this->IplImageToQImage(Img);
    if(qImg_preview)
    {
        scrollArea_ImgShow->setWidgetResizable(false);
        ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_preview));
        ui->label_ImgShow->adjustSize();
    }

    cvReleaseImage(&Img);
}

/**
 * @brief getOverScanBlack
 * @param Buf
 * @param x
 * @param y
 */
void EZCAP::getOverScanBlack(unsigned char *Buf, int x,int y)
{
    float std,rms;
    double max,min;    
    int ret;
    unsigned int pxOB,pyOB,sxOB,syOB;//存放GetOverScanArea获取的数据

    pxOB = pyOB = sxOB = syOB = 0;
    //调用SDK，获取overscan区域
    ret = libqhyccd->GetQHYCCDOverScanArea(camhandle,&pxOB, &pyOB, &sxOB, &syOB);
    //ret = GetQHYCCDOverScanArea(camhandle,&pxOB, &pyOB, &sxOB, &syOB);
    if(ret == QHYCCD_SUCCESS)
        qDebug() << "getOverScanBlack: OverScanArean" << pxOB << pyOB << sxOB << syOB;
    else
        qCritical() << "getOverScanBlack Error: GetOverScanArea failed,maybe the sdk not support";

    getImageInfo(Buf, x, y, pxOB, pyOB, sxOB, syOB, std, rms, max, min);

    OverScanRMS = rms;

    qDebug() << "getOverScanBlack: OverScanRMS=" << OverScanRMS;
}
//------------------------------------------------------------------------------------

//*********************************************************************************************************
//                             focus tab页操作
//*********************************************************************************************************
/**
 * @brief EZCAP::mgrMenu_pBtn_focus_clicked   
 */
void EZCAP::mgrMenu_pBtn_focus_clicked()
{
    unsigned int ret;

    managerMenu->ui->pBtn_focus->setChecked(true);//设置处于按下状态
    managerMenu->ui->pBtn_live_focus->setEnabled(false);//禁用focus tab中live按钮
    managerMenu->ui->head_preview->setCheckable(false);
    managerMenu->ui->head_capture->setCheckable(false);

    memset(ix.ImgData,0,ix.ccdMaxImgW * ix.ccdMaxImgH * 2);
    memset(ix.OutputData8,0,ix.ccdMaxImgW * ix.ccdMaxImgH);
    memset(ix.dispIplImgData24, 0, (ix.ccdMaxImgW+3)/4*4 * ix.ccdMaxImgH * 3);

    if(ix.cameraState == Camera_Idle)
    {
        qDebug() << "----focus start----";

        ix.cameraState = Camera_Waiting;
        ix.imageReady = GetSingleFrame_Waiting;

        if(ix.Exptime != ix.LastExptime)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
            //ret = SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.LastExptime = ix.Exptime;
                qDebug() << "SetQHYCCDParam: CONTROL_EXPOSURE, [ms] " << ix.Exptime;
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_EXPOSURE failure";
            }
        }

        if(ix.canUsbtraffic && ix.lastUsbtraffic != ix.usbtraffic)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
            //ret = SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.lastUsbtraffic = ix.usbtraffic;
                qDebug() << QString("SetQHYCCDParam CONTROL_USBTRAFFIC %1").arg(ix.usbtraffic);
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_USBTRAFFIC failure";
            }
        }

        if(ix.canHighSpeed && ix.LastDownloadSpeed != ix.DownloadSpeed)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed);
            //ret = SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.LastDownloadSpeed = ix.DownloadSpeed;
                qDebug() << "SetQHYCCDParam: CONTROL_SPEED " << ix.DownloadSpeed;
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_SPEED failure";
            }
        }

        if(ix.lastbinx != ix.binx || ix.lastbiny != ix.biny || ix.lastWorkMode != ix.workMode)
        {
            ret = libqhyccd->SetQHYCCDBinMode(camhandle, 1, 1);
            //ret = SetQHYCCDBinMode(camhandle, 1, 1);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.lastbinx = ix.binx;
                ix.lastbiny = ix.biny;
                qDebug() << "SetQHYCCDBinMode" << ix.binx << ix.biny;
            }
            else
            {
                qCritical() << "SetQHYCCDBinMode failure";
            }

            //set focus area position
            focusAreaStartX = 0;
            focusAreaStartY = FocusCenterY_Pre * ix.maxBiny - 100;
            focusAreaSizeX = ix.ccdMaxImgW;
            focusAreaSizeY = 200;
            if(focusAreaStartX < 0) focusAreaSizeX = 0;
            if(focusAreaStartY < 0) focusAreaStartY = 0;
            if(focusAreaSizeY > (int)ix.ccdMaxImgH ) focusAreaSizeY = (int)ix.ccdMaxImgH;
            if(focusAreaStartY + focusAreaSizeY > (int)ix.ccdMaxImgH) focusAreaStartY = (int)ix.ccdMaxImgH - focusAreaSizeY;
            ret = libqhyccd->SetQHYCCDResolution(camhandle, focusAreaStartX, focusAreaStartY, focusAreaSizeX, focusAreaSizeY);
            //ret = SetQHYCCDResolution(camhandle, focusAreaStartX, focusAreaStartY, focusAreaSizeX, focusAreaSizeY);
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() << "SetQHYCCDResolution " << focusAreaStartX << focusAreaStartY << focusAreaSizeX << focusAreaSizeY;
            }
            else
            {
                qCritical() << "SetQHYCCDResolution failure";
            }

            ix.lastWorkMode = ix.workMode;
        }

        ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳

        ix.cameraState = Camera_Exposing;
        //ret = ExpQHYCCDSingleFrame(camhandle); //start exposure...
        ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            qDebug() <<"ExpQHYCCDSingleFrame success, wait...";
        #ifdef WIN32
            Sleep(200);
        #else
            usleep(200000);
        #endif
        }
        else if(ret == QHYCCD_READ_DIRECTLY)
        {
            qDebug() << "ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!";
        }
        else
        {
            qCritical("ExpQHYCCDSingleFrame failed");
        }

        //获取图像
        qDebug() << "donwnloading focus frame";
        ix.cameraState = Camera_Reading;
        downloadFoc = new DownloadFocThread(this);
        connect(downloadFoc, SIGNAL(finished()), downloadFoc, SLOT(deleteLater()));
        downloadFoc->start();

        while(ix.imageReady == GetSingleFrame_Waiting)
        {
            QApplication::processEvents();//响应界面操作，防止界面假死
        }

        if(ix.imageReady == GetSingleFrame_Success)
        {
            emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息
            //显示图像
            qDebug() << "display image..." << ix.imageX << ix.imageY;
#if 0
            displayFocusImage(ix.imageX ,ix.imageY);
#else
            displayFocusImage_Ex(ix.imageX, ix.imageY, ix.ImgData);
#endif

            //显示直方图
            qDebug() << "display histogram...";
            displayHistogramImage(ix.imageX, ix.imageY, ix.ImgData);

            //获取overscan balck值
            qDebug() << "clibrate black level with overscan area";
            getOverScanBlack(ix.ImgData, ix.imageX, ix.imageY);

            noImgInWorkMode = false;
        }
        qDebug() << "----focus end----";

        ix.cameraState = Camera_Idle;

        managerMenu->ui->pBtn_focus->setChecked(false);
        managerMenu->ui->pBtn_live_focus->setEnabled(true);
        managerMenu->ui->head_preview->setCheckable(true);
        managerMenu->ui->head_capture->setCheckable(true);

        mainMenuBar->actSaveBMP->setEnabled(true);//设置保存图像菜单项是否可用
        mainMenuBar->actSaveFIT->setEnabled(false);
        mainMenuBar->actSaveJPG->setEnabled(true);
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_live_focus_clicked   
 */
void EZCAP::mgrMenu_pBtn_live_focus_clicked()
{
    ix.onLiveMode = !ix.onLiveMode;

    if(ix.cameraState == Camera_Idle && ix.onLiveMode)
    {
        unsigned int ret;

        managerMenu->ui->pBtn_live_focus->setChecked(true);//设置live按钮处于按下状态
        managerMenu->ui->pBtn_focus->setEnabled(false);//禁用focus按钮
        managerMenu->ui->head_preview->setCheckable(false);
        managerMenu->ui->head_capture->setCheckable(false);

        memset(ix.ImgData,0,ix.ccdMaxImgW * ix.ccdMaxImgH * 2);
        memset(ix.OutputData8,0,ix.ccdMaxImgW * ix.ccdMaxImgH);
        memset(ix.dispIplImgData24, 0, (ix.ccdMaxImgW+3)/4*4 * ix.ccdMaxImgH * 3);

        qDebug() <<"----focus live start----";

        ix.cameraState = Camera_Waiting;

        while(ix.onLiveMode)
        {
            ix.imageReady = GetSingleFrame_Waiting;
            if(ix.Exptime != ix.LastExptime)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
                //ret = SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.LastExptime = ix.Exptime;
                    qDebug() << "SetQHYCCDParam: CONTROL_EXPOSURE, [ms] " << ix.Exptime;
                }
                else
                {
                    qCritical() << "SetQHYCCDParam: CONTROL_EXPOSURE failure";
                }
            }

            if(ix.canUsbtraffic && ix.lastUsbtraffic != ix.usbtraffic)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
                //ret = SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.lastUsbtraffic = ix.usbtraffic;
                    qDebug() << QString("SetQHYCCDParam CONTROL_USBTRAFFIC %1").arg(ix.usbtraffic);
                }
                else
                {
                    qCritical() << "SetQHYCCDParam: CONTROL_USBTRAFFIC failure";
                }
            }

            if(ix.canHighSpeed && ix.LastDownloadSpeed != ix.DownloadSpeed)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed);
                //ret = SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.LastDownloadSpeed = ix.DownloadSpeed;
                    qDebug() << "SetQHYCCDParam: CONTROL_SPEED " << ix.DownloadSpeed;
                }
                else
                {
                    qCritical() << "SetQHYCCDParam: CONTROL_SPEED failure";
                }
            }

            if(ix.lastbinx != ix.binx || ix.lastbiny != ix.biny || ix.lastWorkMode != ix.workMode)
            {
                //ret = SetQHYCCDBinMode(camhandle, ix.binx, ix.biny);
                ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.binx, ix.biny);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.lastbinx = ix.binx;
                    ix.lastbiny = ix.biny;
                    qDebug() << "SetQHYCCDBinMode" << ix.binx << ix.biny;
                }
                else
                {
                    qCritical() << "SetQHYCCDBinMode failure";
                }

                //set focus area position
                focusAreaStartX = 0;
                focusAreaStartY = FocusCenterY_Pre * ix.maxBiny - 100;
                focusAreaSizeX = ix.ccdMaxImgW;
                focusAreaSizeY = 200;
                if(focusAreaStartX < 0) focusAreaSizeX = 0;
                if(focusAreaStartY < 0) focusAreaStartY = 0;
                if(focusAreaSizeY > (int)ix.ccdMaxImgH) focusAreaSizeY = (int)ix.ccdMaxImgH;
                if(focusAreaStartY + focusAreaSizeY > (int)ix.ccdMaxImgH) focusAreaStartY = (int)ix.ccdMaxImgH - focusAreaSizeY;
                ret = libqhyccd->SetQHYCCDResolution(camhandle, focusAreaStartX, focusAreaStartY, focusAreaSizeX, focusAreaSizeY);
                //ret = SetQHYCCDResolution(camhandle, focusAreaStartX, focusAreaStartY, focusAreaSizeX, focusAreaSizeY);
                if(ret == QHYCCD_SUCCESS)
                {
                    qDebug() << "SetQHYCCDResolution..." << focusAreaStartX << focusAreaStartY << focusAreaSizeX << focusAreaSizeY;
                }
                else
                {
                    qCritical() << "SetQHYCCDResolution failure";
                }

                ix.lastWorkMode = ix.workMode;
            }

            ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳

            ix.cameraState = Camera_Exposing;
            ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
            //ret = ExpQHYCCDSingleFrame(camhandle); //start exposure...
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() <<"ExpQHYCCDSingleFrame success, wait...";
            #ifdef WIN32
                Sleep(200);
            #else
                usleep(200000);
            #endif
            }
            else if(ret == QHYCCD_READ_DIRECTLY)
            {
                qDebug() << "ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!";
            }
            else
            {
                qCritical("ExpQHYCCDSingleFrame failed");
            }

            ix.cameraState = Camera_Reading;
            //开启线程，获取图像数据
            downloadFoc = new DownloadFocThread(this);
            connect(downloadFoc, SIGNAL(finished()), downloadFoc, SLOT(deleteLater()));
            downloadFoc->start();

            while(ix.imageReady == GetSingleFrame_Waiting)
            {
                //....
                QApplication::processEvents();//响应界面操作，防止界面假死
            }

            if(ix.imageReady == GetSingleFrame_Success)
            {
                emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息

                //显示图像
#if 0
                displayFocusImage(ix.imageX ,ix.imageY);
#else
                displayFocusImage_Ex(ix.imageX, ix.imageY, ix.ImgData);
#endif
                //显示直方图
                displayHistogramImage(ix.imageX, ix.imageY, ix.ImgData);
                //获取overscan balck值
                getOverScanBlack(ix.ImgData, ix.imageX, ix.imageY);

                noImgInWorkMode = false;
            }

            QApplication::processEvents();//响应界面是否发送了停止信号

        }
        qDebug() << "----focus live end----";

        ix.cameraState = Camera_Idle;

        //live结束 还原focus和live按钮状态
        managerMenu->ui->pBtn_live_focus->setChecked(false);
        managerMenu->ui->pBtn_focus->setEnabled(true);
        managerMenu->ui->head_preview->setCheckable(true);//设置preivew tab有效
        managerMenu->ui->head_capture->setCheckable(true);//设置capture tab有效

        mainMenuBar->actSaveBMP->setEnabled(true);//设置保存图像菜单项可用
        mainMenuBar->actSaveFIT->setEnabled(false);
        mainMenuBar->actSaveJPG->setEnabled(true);
    }
}

/**
 * @brief EZCAP::displayFocusImage   
 * @param x
 * @param y
 * @param boxWidth
 * @param boxHeight
 * @param boxX
 * @param boxY
 */
void EZCAP::displayFocusImage(int x, int y)
{
    unsigned char pixel;
    long s, k, n;
    int SubX0;
    IplImage *srcImg;
    IplImage *focAreaImg;
    int focusAreaW;
    int focusAreaH;

    if(x < 800)
       focusAreaW = x;
    else
        focusAreaW = 800;
    focusAreaH = 200;

    //srcImg = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 3);
    srcImg = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
    srcImg->imageData = (char*)ix.dispIplImgData24;
    focAreaImg = cvCreateImage(cvSize(focusAreaW, focusAreaH), IPL_DEPTH_8U, 3);

    if(captureImg)
    {
        cvReleaseImage(&captureImg);
        captureImg = NULL;
    }
    captureImg = cvCreateImage(cvSize(focusAreaW, focusAreaH), IPL_DEPTH_8U, 3);

    //convert 16bit to 8bit
    s = 0;
    k = 0;
    for(int i = 0; i < y; i++)
    {
        for(int j = 0; j < x; j++)
        {
            // look up the Stretch LUT table to convert current 16bit data to 8bit
            pixel = ix.StretchLUT[ix.lastImgData[s]+ix.lastImgData[s+1]*256];
            ix.OutputData8[k] = pixel;
            s += 2;
            k += 1;
        }
    }

    //copy 8bit data to opencv 8bit 3channels IplImage
    n=0;
    k=0;
    for(int j=0; j<y; j++)
    {
        for(int i=0; i<x; i++)
        {
            ix.dispIplImgData24[n] = ix.OutputData8[k];
            ix.dispIplImgData24[n+1] = ix.OutputData8[k];
            ix.dispIplImgData24[n+2] = ix.OutputData8[k];
            n += 3;
            k += 1;
        }
        n += srcImg->widthStep - srcImg->nChannels*srcImg->width;
    }


    SubX0 = FocusCenterX_Pre * ix.maxBinx - focusAreaW / 2;
    if(SubX0 > x - focusAreaW)
    {
        SubX0 = x - focusAreaW;
    }
    if(SubX0 < 0)
    {
        SubX0 = 0;
    }

    cvSetImageROI(srcImg, cvRect(SubX0, 0, focusAreaW, focusAreaH));
    cvCopy(srcImg, focAreaImg, NULL);
    cvResetImageROI(srcImg);

    //qDebug() << "display the focus assistant image";
    displayFocusAssistantImage(focAreaImg);

    //display image size into statusBar
    QString str1 = QString::number(focusAreaW) + "x" + QString::number(focusAreaH);
    statusLabel_imgSize->setText(str1);

    //显示到qlabel上
    cvCvtColor(focAreaImg,focAreaImg,CV_BGR2RGB);//iplimage BGR to image RGB

    cvCopy(focAreaImg, captureImg);//store current focus image, using to save as bmp, jpg or fit

    qImg_focus = this->IplImageToQImage(focAreaImg);
    if(qImg_focus)
    {
        ui->label_ImgShow->setMaximumHeight(focusAreaH);
        ui->label_ImgShow->setMaximumWidth(focusAreaW);
        scrollArea_ImgShow->setWidgetResizable(false);
        ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_focus));
        ui->label_ImgShow->adjustSize();
    }

    cvReleaseImage(&srcImg);
    cvReleaseImage(&focAreaImg);

}

void EZCAP::displayFocusImage_Ex(int x, int y, unsigned char *dataBuf)
{
    unsigned char pixel;
    long s, k, n;
    int SubX0;
    IplImage *srcImg = NULL;
    IplImage *focAreaImg = NULL;
    int focusAreaW;
    int focusAreaH;

    if(x < 800)
       focusAreaW = x;
    else
        focusAreaW = 800;
    focusAreaH = 200;

    //srcImg = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 3);
    srcImg = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
    srcImg->imageData = (char*)ix.dispIplImgData24;
    focAreaImg = cvCreateImage(cvSize(focusAreaW, focusAreaH), IPL_DEPTH_8U, 3);

    if(captureImg)
    {
        cvReleaseImage(&captureImg);
        captureImg = NULL;
    }
    captureImg = cvCreateImage(cvSize(focusAreaW, focusAreaH), IPL_DEPTH_8U, 3);

    //convert 16bit to 8bit
    s = 0;
    k = 0;
    for(int i = 0; i < y; i++)
    {
        for(int j = 0; j < x; j++)
        {
            // look up the Stretch LUT table to convert current 16bit data to 8bit
            pixel = ix.StretchLUT[dataBuf[s]+dataBuf[s+1]*256];
            ix.OutputData8[k] = pixel;
            s += 2;
            k += 1;
        }
    }

    //copy 8bit data to opencv 8bit 3channels IplImage
    n=0;
    k=0;
    for(int j=0; j<y; j++)
    {
        for(int i=0; i<x; i++)
        {
            ix.dispIplImgData24[n] = ix.OutputData8[k];
            ix.dispIplImgData24[n+1] = ix.OutputData8[k];
            ix.dispIplImgData24[n+2] = ix.OutputData8[k];
            n += 3;
            k += 1;
        }
        n += srcImg->widthStep - srcImg->nChannels*srcImg->width;
    }


    SubX0 = FocusCenterX_Pre * ix.maxBinx - focusAreaW / 2;
    if(SubX0 > x - focusAreaW)
    {
        SubX0 = x - focusAreaW;
    }
    if(SubX0 < 0)
    {
        SubX0 = 0;
    }

    cvSetImageROI(srcImg, cvRect(SubX0, 0, focusAreaW, focusAreaH));
    cvCopy(srcImg, focAreaImg, NULL);
    cvResetImageROI(srcImg);

    //qDebug() << "display the focus assistant image";
    displayFocusAssistantImage(focAreaImg);

    //display image size into statusBar
    QString str1 = QString::number(focusAreaW) + "x" + QString::number(focusAreaH);
    statusLabel_imgSize->setText(str1);

    //显示到qlabel上
    cvCvtColor(focAreaImg,focAreaImg,CV_BGR2RGB);//iplimage BGR to image RGB

    cvCopy(focAreaImg, captureImg);//store current focus image, using to save as bmp, jpg or fit

    qImg_focus = this->IplImageToQImage(focAreaImg);
    if(qImg_focus)
    {
        ui->label_ImgShow->setMaximumHeight(focusAreaH);
        ui->label_ImgShow->setMaximumWidth(focusAreaW);
        scrollArea_ImgShow->setWidgetResizable(false);
        ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_focus));
        ui->label_ImgShow->adjustSize();
    }

    cvReleaseImage(&srcImg);
    cvReleaseImage(&focAreaImg);
}
//----------------------------------------------------------------------------------------------------------
//                                     FocusAssistant
//----------------------------------------------------------------------------------------------------------
/**
 * @brief EZCAP::displayFocusAssistantImage  
 * @param image
 */
void EZCAP::displayFocusAssistantImage(IplImage *image)
{
    int pixel;

    IplImage *ZoomImg;
    IplImage *GuideBIGImg ;

    ZoomImg = cvCreateImage(cvSize(200,200), IPL_DEPTH_8U, 3 );
    GuideBIGImg = cvCreateImage(cvSize(200,200), IPL_DEPTH_8U, 1 );

    if (ZoomFocus_X < 20)
        ZoomFocus_X = 20;
    if (ZoomFocus_Y < 20)
        ZoomFocus_Y = 20;

    cvSetImageROI(image, cvRect(ZoomFocus_X -20, ZoomFocus_Y -20, 40, 40));

    if (FocusZoomMode == 0)
        cvResize(image, ZoomImg, 1);
    else
        cvResize(image, ZoomImg, 0);

    cvResetImageROI(image);

    cvCvtColor(ZoomImg,GuideBIGImg,CV_RGB2GRAY);

    FWHMFocus(GuideBIGImg,FocusInfo);//获取Focus信息

    FalseColorConvert(ZoomImg,ZoomImg);//伪彩色变换

    //draw FWHM curve
    QImage image2_focusAssistant = ui->image2_focusAssistant->pixmap()->toImage();
    QPainter painter2(&image2_focusAssistant); //为这个QImage构造一个QPainter
    painter2.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
    QPen pen2 = painter2.pen();
    pen2.setWidth(2);
    pen2.setColor(QColor(255, 0, 0));
    painter2.setPen(pen2);//重设画笔
    painter2.drawLine(fwhm_x, fwhm_y, FocusCurveX, 128 - FocusInfo.FWHM_Result);//画线

    ui->image2_focusAssistant->setPixmap(QPixmap::fromImage(image2_focusAssistant));//刷新显示
    ui->fwhm_value_focusAssistant->setText(QString::number(FocusInfo.FWHM_Result));//显示fwhm值

    //draw PEAK curve
    QImage image3_focusAssistant = ui->image3_focusAssistant->pixmap()->toImage();
    QPainter painter3(&image3_focusAssistant);//为这个QImage构造一个QPainter
    painter3.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
    QPen pen3 = painter3.pen();
    pen3.setColor(QColor(0, 0, 255));
    painter3.setPen(pen3);//重设画笔
    painter3.drawLine(peak_x, peak_y, FocusCurveX, (255-FocusInfo.DeltaPixel)/2);//画线

    ui->image3_focusAssistant->setPixmap(QPixmap::fromImage(image3_focusAssistant));//刷新显示
    ui->peak_value_focusAssistant->setText(QString::number(FocusInfo.DeltaPixel));//显示peak值

/*
    char fwhm[64];
    char itsty[64];

    sprintf(fwhm,"FWHM %7d",FocusInfo.FWHM_Result);
    sprintf(itsty,"Intensity %d",FocusInfo.DeltaPixel);

    SendTwoLine2QHYCCDInterCamOled(camhandle,fwhm,itsty);
*/
    //当前终点坐标为下一次的起始点坐标
    fwhm_x = FocusCurveX;
    fwhm_y = 128 - FocusInfo.FWHM_Result;
    peak_x = FocusCurveX;
    peak_y = (255 - FocusInfo.DeltaPixel)/2;

    FocusCurveX++;

    if (FocusCurveX > 300)
    {
        FocusCurveX = 0;
        //初始化表格线
        DrawGridBox(&image2_focusAssistant);
        DrawGridBox(&image3_focusAssistant);
        //初始化起始点坐标
        fwhm_x = 0;
        fwhm_y = 0;
        peak_x = 0;
        peak_y = 0;

        pen2.setColor(QColor(255,0,0));
        pen2.setWidth(2);
        painter2.setPen(pen2);//重设画笔

        pen3.setColor(QColor(0,0,255));
        pen3.setWidth(2);
        painter3.setPen(pen3);
    }


    QImage screenImg_focus = managerMenu->ui->img_screenView->pixmap()->toImage();
    QPainter painter_screenFocus(&screenImg_focus); //为这个QImage构造一个QPainter
    painter_screenFocus.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。

    QBrush brush_screenFocus = painter_screenFocus.brush();
    brush_screenFocus.setStyle(Qt::SolidPattern);
    brush_screenFocus.setColor(QColor(0,0,0));
    painter_screenFocus.setBrush(brush_screenFocus);//设置画刷

    QPen pen_screenFocus = painter_screenFocus.pen();
    pen_screenFocus.setColor(QColor(0,255,0));
    painter_screenFocus.setPen(pen_screenFocus);//设置画笔

    painter_screenFocus.drawRect(0,0,200,200);//绘制矩形

    int x1 = 0;//设置起点坐标
    int y1 = 0;
    //画直线
    for (int i = 0; i < 200; i++)
    {
        pixel = FocusInfo.Row[i];
        if (pixel > 192)
            pixel = 192;

        painter_screenFocus.drawLine(x1,y1,i,pixel);
        //s=s+3;
        x1 = i;
        y1 = pixel;
    }

    brush_screenFocus.setStyle(Qt::SolidPattern);
    brush_screenFocus.setColor(QColor(0,0,0));
    painter_screenFocus.setBrush(brush_screenFocus);//设置画刷
    pen_screenFocus.setColor(QColor(255,0,0));
    painter_screenFocus.setPen(pen_screenFocus);//设置画笔
    //设置起点坐标
    x1 = 0;
    y1 = FocusInfo.MinPixel;
    painter_screenFocus.drawLine(x1,y1,200,FocusInfo.MinPixel);//绘制直线

    pen_screenFocus.setColor(QColor(255,255,0));
    painter_screenFocus.setPen(pen_screenFocus);//设置画笔
    //设置起点坐标
    x1 = FocusInfo.CenterX;
    y1 = 0;
    painter_screenFocus.drawLine(x1,y1,FocusInfo.CenterX,192);//绘制直线

    pen_screenFocus.setColor(QColor(255,0,0));
    painter_screenFocus.setPen(pen_screenFocus);
    //设置起点坐标
    x1 = FocusInfo.FWHM_X1;
    y1 = 0;
    painter_screenFocus.drawLine(x1,y1,FocusInfo.FWHM_X1,192);
    //设置起点坐标
    x1 = FocusInfo.FWHM_X2;
    y1 = 0;
    painter_screenFocus.drawLine(x1,y1,FocusInfo.FWHM_X2,192);

    managerMenu->ui->img_screenView->setPixmap(QPixmap::fromImage(screenImg_focus));//刷新screenview图像显示

    //绘制圆圈
    cvCircle(ZoomImg, cvPoint(FocusInfo.CenterX, FocusInfo.CenterY), 5, CV_RGB(255,0,0), 1, 8, 0);

    //显示到qlabel上
    cvCvtColor(ZoomImg, ZoomImg, CV_BGR2RGB);//iplimage BGR杞琿image RGB
    qImg_focus = this->IplImageToQImage(ZoomImg);
    if(qImg_focus)
    {
        ui->image1_focusAssistant->setPixmap(QPixmap::fromImage(*qImg_focus));
    }

    cvReleaseImage(&ZoomImg);
    cvReleaseImage(&GuideBIGImg);

}

/**
 * @brief EZCAP::FWHMFocus   
 * @param Img
 * @param FocusInfo
 */
void EZCAP::FWHMFocus(IplImage *Img,FOCUSINFO &FocusInfo)
{
    //input image should be 1 channel image, and 8bit
    IplImage *MaxColumn ,*MaxRow;

    MaxColumn = cvCreateImage(cvSize(1,Img->height), IPL_DEPTH_8U, 1);
    MaxRow = cvCreateImage(cvSize(Img->width,1), IPL_DEPTH_8U, 1);

    double minPixel,maxPixel;
    CvPoint  maxLocation;

    cvMinMaxLoc(Img, &minPixel, &maxPixel, 0, &maxLocation, NULL);

    FocusInfo.MaxPixel = maxPixel;
    FocusInfo.MinPixel = minPixel;
    FocusInfo.DeltaPixel = maxPixel - minPixel;

    FocusInfo.width = Img->width ;
    FocusInfo.height = Img->height;

    FocusInfo.CenterX = maxLocation.x;
    FocusInfo.CenterY = maxLocation.y;

    cvSetImageROI(Img, cvRect(maxLocation.x, 0, 1, Img->height));
    cvCopy(Img, MaxColumn, NULL);
    cvResetImageROI(Img);

    cvSetImageROI(Img, cvRect(0, maxLocation.y, Img->width ,1));
    cvCopy(Img, MaxRow, NULL);
    cvResetImageROI(Img);

    for(int i=0; i < Img->height ; i++)
    {
        FocusInfo.Column[i] = MaxColumn->imageData[i*4];
    }

    for(int i=0; i < Img->height ; i++)
    {
        FocusInfo.Row[i] = MaxRow->imageData[i];
    }

    int FWHM_Level;    //FWHM is the half intensity
    FWHM_Level = (maxPixel-minPixel)/2 + minPixel;

    int FWHM;

    FWHM = maxLocation.y;
    while(FocusInfo.Column[FWHM] > FWHM_Level)
    {
        FWHM--;
        if(FWHM < 0)
            return;
    }
    FocusInfo.FWHM_Y1 = FWHM;

    FWHM = maxLocation.y;
    while(FocusInfo.Column[FWHM] > FWHM_Level)
    {
        FWHM++;
        if(FWHM > Img->height)
            return;
    }
    FocusInfo.FWHM_Y2 = FWHM;

    FocusInfo.FWHM_ResultY = FocusInfo.FWHM_Y2 - FocusInfo.FWHM_Y1 ;

    FWHM = maxLocation.x;
    while(FocusInfo.Row[FWHM] > FWHM_Level)
    {
        FWHM--;
        if(FWHM < 0)
            return;
    }
    FocusInfo.FWHM_X1 = FWHM;

    FWHM = maxLocation.x;
    while(FocusInfo.Row[FWHM] > FWHM_Level)
    {
        FWHM++;
        if(FWHM > Img->width)
            return;
    }
    FocusInfo.FWHM_X2 = FWHM;

    FocusInfo.FWHM_ResultX = FocusInfo.FWHM_X2 - FocusInfo.FWHM_X1 ;

    double R,R1,R2;

    R1 = FocusInfo.FWHM_ResultX ;
    R2 = FocusInfo.FWHM_ResultY ;
    R = sqrt(R1*R1+R2*R2)/1.414 ;
    FocusInfo.FWHM_Result = (unsigned char)R;

    cvReleaseImage(&MaxRow);
    cvReleaseImage(&MaxColumn);
}

/**
 * @brief EZCAP::DrawGridBox  
 * @param img
 */
void EZCAP::DrawGridBox(QImage *img)
{
    int x,y;
    x = img->width();
    y = img->height();

    QPainter painter(img); //create a painter for the QImage
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。

    //改变画刷
    QBrush brush = painter.brush();
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(QColor(10,10,10));
    painter.setBrush(brush);
    //改变画笔
    QPen pen = painter.pen();
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(4);
    pen.setColor(QColor(128,128,128));
    painter.setPen(pen);
    painter.drawRect(0,0,x,y);//绘制矩形

    pen.setWidth(1);
    pen.setColor(QColor(64,64,64));
    painter.setPen(pen);
    //画细分线
    for(int i=0; i<y; i=i+y/4)
    {
        painter.drawLine(0,i,x,i);
    }

    for(int i=0; i<x; i=i+x/6)
    {
        painter.drawLine(i,0,i,y);
    }

}

/**
 * @brief EZCAP::FalseColorConvert 
 * @param inputImg
 * @param outputImg
 */
void EZCAP::FalseColorConvert(IplImage *inputImg,IplImage *outputImg)
{
    // 使用三通道LUT对图像进行伪彩色处理
    //需要先定义全局变量  unsigned char LUT1[256][3];
    //必须保证输入和输出图像尺寸相同
    int height,width;
    height = inputImg->height ;
    width = inputImg->width ;

    if(inputImg->nChannels == 1)
    {
        //输入为单色，输出为彩色
        for(int y=0; y<height; y++)
        {
            unsigned char *ptrInput = (unsigned char*)(inputImg->imageData + y * inputImg->widthStep);
            unsigned char *ptrOutput = (unsigned char*)(outputImg->imageData + y * outputImg->widthStep);
            for(int x=0; x<width; x++)
            {
                ptrOutput[3*x] = ix.LUT_1[ptrInput[x]][0];
                ptrOutput[3*x+1] = ix.LUT_1[ptrInput[x]][1];
                ptrOutput[3*x+2] = ix.LUT_1[ptrInput[x]][2];
            }
        }
    }
    else
    {
        for(int y=0; y<height; y++)
        {
            unsigned char *ptrInput = (unsigned char*)(inputImg->imageData + y * inputImg->widthStep);
            unsigned char *ptrOutput = (unsigned char*)(outputImg->imageData + y * outputImg->widthStep);
            for(int x=0; x<width; x++)
            {
                ptrOutput[3*x] = ix.LUT_1[ptrInput[3*x]][0];
                ptrOutput[3*x+1] = ix.LUT_1[ptrInput[3*x+1]][1];
                ptrOutput[3*x+2] = ix.LUT_1[ptrInput[3*x+2]][2];
            }
        }
    }
}

/**
 * @brief EZCAP::LoadFalseColor  
 * @param LUTMAP
 */
void EZCAP::LoadFalseColor(QString LUTMAP)
{
    int i;
    long vr,vg,vb;
    double vcolor;

    if (LUTMAP == "Linear")
    {
        ui->falseColor_focusAssistant->setPixmap(QPixmap(":image/Linear.bmp"));
        for (i = 0; i<256; i++)
        {
            ix.LUT_1[i][0]=i;
            ix.LUT_1[i][1]=i;
            ix.LUT_1[i][2]=i;
        }
    }
    else
    {
        QPixmap pixmap_falseColor(":/image/"+LUTMAP);
        ui->falseColor_focusAssistant->setPixmap(pixmap_falseColor);//reflash the falsecolor image

        for (i = 0; i<256; i++)
        {
            //qpixmap convert to qimage, and convert an RGB to BGR
            QImage imgTemp = pixmap_falseColor.toImage().rgbSwapped();

            vcolor = imgTemp.pixel(i,3);

            vr = fmod(fmod(vcolor,65536),256);
            vg = fmod((vcolor-vr)/256,256);
            vb = fmod((vcolor-vr-256*vg)/65536,256);

            ix.LUT_1[i][0]=vb;
            ix.LUT_1[i][1]=vg;
            ix.LUT_1[i][2]=vr;
        }
    }
}

/**
 * @brief EZCAP::on_pBtn_linear_clicked  
 */
void EZCAP::on_pBtn_linear_clicked()
{
    LoadFalseColor("Linear.bmp");
}

/**
 * @brief EZCAP::on_pBtn_thermal_clicked  
 */
void EZCAP::on_pBtn_thermal_clicked()
{
    LoadFalseColor("ThermalMAP.bmp");
}

/**
 * @brief EZCAP::on_pBtn_false_clicked   
 */
void EZCAP::on_pBtn_false_clicked()
{
    LoadFalseColor("FalseColor.bmp");
}

/**
 * @brief EZCAP::on_pBtn_invert_clicked   
 */
void EZCAP::on_pBtn_invert_clicked()
{
    LoadFalseColor("NegativeFilm.bmp");
}
//--------------------------------------------------------------------------------------

//***************************************************************************************
//                                  capture tab页操作
//***************************************************************************************

/**
 * @brief EZCAP::mgrMenu_pBtn_stop_clicked        
 */
void EZCAP::mgrMenu_pBtn_stop_clicked()
{
    //传输数据过程中 stop无效 仍需等待数据传输完成
    if(ix.cameraState != Camera_Idle)
    {
        isSettleDone = true; //停止可能存在的Dither 等待循环

        //qDebug() << "CancelQHYCCDExposing...";
        //停止曝光
        uint32_t ret = libqhyccd->CancelQHYCCDExposingAndReadout(camhandle);
        //uint32_t ret = CancelQHYCCDExposingAndReadout(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            qDebug() << "CancelQHYCCDExposingAndReadout";
        }
        else
        {
            qCritical() << "CancelQHYCCDExposingAndReadout failed";
        }

        ix.ForceStop = true;//曝光停止，退出循环
        ix.cameraState = Camera_Idle;
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_capture_clicked       
 */
void EZCAP::mgrMenu_pBtn_capture_clicked()
{    
    unsigned int ret = QHYCCD_ERROR;
    unsigned int px,py,sx,sy;//存放GetEffectiveArea中获取的数据
    unsigned int pxOB,pyOB,sxOB,syOB;//存放GetOverScanArea获取的数据
    py = sx = sy = 0;
    pxOB = pyOB = sxOB = syOB = 0;

    QTime sTime = QTime::currentTime();
    int betweenTime;

    managerMenu->ui->pBtn_capture->setChecked(true);
    managerMenu->ui->head_focus->setCheckable(false);//禁用focus tab
    managerMenu->ui->head_preview->setCheckable(false);//禁用preview tab

    memset(ix.ImgData, 0, ix.ccdMaxImgW * ix.ccdMaxImgH * 2);
    memset(ix.OutputData8, 0, ix.ccdMaxImgW * ix.ccdMaxImgH);
    memset(ix.dispIplImgData24, 0, (ix.ccdMaxImgW+3)/4*4 * ix.ccdMaxImgH * 3);
    memset(ix.dispIplImgData8, 0, (ix.ccdMaxImgW+3)/4*4 * ix.ccdMaxImgH);

    if(ix.cameraState == Camera_Idle)
    {
        qDebug()<<"----start capture----";

        ix.cameraState = Camera_Waiting;
        ix.imageReady = GetSingleFrame_Waiting;


//        if(phdLink_dialog->IsDitherEnabled())
//        {
//            int dither = phdLink_dialog->GetDitherLevel();
//            double settle = phdLink_dialog->GetSettleRange();

//            qDebug() <<"Dither Settle:" << dither << settle;

//            startDitherTimer();
//            ret = libqhyccd->ControlPHD2Dither(dither, settle);
//            //ret = ControlPHD2Dither(dither, settle);    //Control PHD2 Dither
//            if(ret == 0)
//            {
//                statusLabel_msg->setText(tr("Dither: Waiting..."));

//                isSettleDone = false;
//                while(!isSettleDone)
//                {
//#ifdef WIN32
//                    Sleep(1);
//#else
//                    usleep(1000);
//#endif
//                    QApplication::processEvents();//防止长时间导致界面假死
//                }
//            }
//            stopDitherTimer();
//        }

        //set camera parameters...
        //------------------------
        if(ix.canMechanicalShutter && (ix.MechanicalShutterMode != ix.LastMechanicalShutterMode))
        {
            ret = libqhyccd->ControlQHYCCDShutter(camhandle, ix.MechanicalShutterMode);
            //ret = ControlQHYCCDShutter(camhandle, ix.MechanicalShutterMode);//control shutter open or close
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() << "ControlQHYCCDShutter." << ix.MechanicalShutterMode;
                ix.LastMechanicalShutterMode = ix.MechanicalShutterMode;
            }
            else
            {
                qCritical() << "ControlQHYCCDShutter failure!";
            }
        }

        if(ix.Exptime != ix.LastExptime)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
            //ret = SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.Exptime * 1000);
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() << "SetQHYCCDParam: CONTROL_EXPOSURE, [ms]" << ix.Exptime;
                ix.LastExptime = ix.Exptime;
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_EXPOSURE failure";
            }
        }

        if(ix.canUsbtraffic && ix.lastUsbtraffic != ix.usbtraffic)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
            //ret = SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.usbtraffic);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.lastUsbtraffic = ix.usbtraffic;
                qDebug() << QString("SetQHYCCDParam CONTROL_USBTRAFFIC %1").arg(ix.usbtraffic);
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_USBTRAFFIC failure";
            }
        }

        if(ix.canHighSpeed && ix.DownloadSpeed != ix.LastDownloadSpeed)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed);
            //ret = SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.DownloadSpeed);
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() << "SetQHYCCDParam: CONTROL_SPEED" << ix.DownloadSpeed;
                ix.LastDownloadSpeed = ix.DownloadSpeed;
            }
            else
            {
                qCritical() << "SetQHYCCDParam: CONTROL_SPEED failure";
            }
        }

        if(ix.lastbinx != ix.binx || ix.lastbiny != ix.biny || ix.lastWorkMode != ix.workMode)
        {
            ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.binx, ix.biny);
            //ret = SetQHYCCDBinMode(camhandle, ix.binx, ix.biny);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.lastbinx = ix.binx;
                ix.lastbiny = ix.biny;
                qDebug() << "SetQHYCCDBinMode " << ix.binx << ix.biny;

                needCalcScrollBarValue = true;
            }
            else
            {
                qCritical() << "SetQHYCCDBinMode failure";
            }
            ret = libqhyccd->SetQHYCCDResolution(camhandle, 0, 0, ix.ccdMaxImgW/ix.binx, ix.ccdMaxImgH/ix.biny);
            //ret = SetQHYCCDResolution(camhandle, 0, 0, ix.ccdMaxImgW/ix.binx, ix.ccdMaxImgH/ix.biny);
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() << "SetQHYCCDResolution 0 0" << ix.ccdMaxImgW / ix.binx << ix.ccdMaxImgH / ix.biny;
            }
            else
            {
                qCritical() << "SetQHYCCDResolution failure";
            }

            ix.lastWorkMode = ix.workMode;
        }

        //fit header info...
        ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳
        //ix.dateOBS = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

        //Triger function
        if(ix.canTriger)
        {
            ret = libqhyccd->SetQHYCCDTrigerFunction(camhandle, ix.trigerInOrOut);
            //ret = SetQHYCCDTrigerFunction(camhandle, ix.trigerInOrOut);
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() << "SetQHYCCDTrigerFunction" << ix.trigerInOrOut;
            }
            else
            {
                qCritical() << "SetQHYCCDTrigerFunction failure";
            }
        }
        //20200512lyl GPSon
        if(ix.isGPSon)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CAM_GPS, ix.isGPSon);
            //ret = SetQHYCCDParam(camhandle,CAM_GPS, ix.isGPSon);
            if(ret == QHYCCD_SUCCESS)
            {
                qDebug() << "SetQHYCCDParam CAM_GPS" << ix.isGPSon;
            }
            else
            {
                qCritical() << "SetQHYCCDParam CAM_GPS failure";
            }
        }

        statusLabel_msg->setText(tr("Exposuring..."));
        //sign up camera is exposuring...
        ix.cameraState = Camera_Exposing;
        sTime.start();
        ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
        //ret = ExpQHYCCDSingleFrame(camhandle);//start exposure...
        if(ret == QHYCCD_ERROR)
        {
            qCritical("ExpQHYCCDSingleFrame failed");
        }
        else if(ret == QHYCCD_READ_DIRECTLY)
        {
            qDebug() << "ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!";
        }
        else
        {
            qDebug() << "ExpQHYCCDSingleFrame success, wait...";
#ifdef WIN32
            Sleep(200);
#else
            usleep(200000);
#endif
        }

        //---progress for Exposing
        managerMenu->ui->proBar_captureTime->setValue(0);
        managerMenu->ui->proBar_capture->setValue(0);

        betweenTime = sTime.elapsed();//返回从上次start()或restart()开始以来的时间差，单位ms
        while (betweenTime < ix.Exptime && !ix.ForceStop)
        {
            if((betweenTime + 1000) >= ix.Exptime)
            {
                break;
            }
            betweenTime = sTime.elapsed();
            managerMenu->ui->proBar_captureTime->setValue(betweenTime*100/ix.Exptime);
#ifdef WIN32
            Sleep(1);
#else
            usleep(1000);
#endif
            QApplication::processEvents();//防止长时间导致界面假死
        }
        managerMenu->ui->proBar_captureTime->setValue(100);


        if(ix.canCooler)
        {
            mainWidget->stopTimerTemp();//停止温控定时器
        }

        if(!ix.ForceStop)
        {
            //开启线程，从usb donload图像数据
            qDebug() << "downloading captured frame";
            ix.cameraState = Camera_Reading;

            downloadCap = new DownloadCapThread(this);
            connect(downloadCap, SIGNAL(finished()), downloadCap, SLOT(deleteLater()));
            downloadCap->start();

            //while(ix.cameraState == Camera_Reading)
            while(ix.imageReady == GetSingleFrame_Waiting && ix.cameraState != Camera_Idle)
            {
                //处理下载进度条
                //....
                managerMenu->ui->proBar_capture->setValue(libqhyccd->GetQHYCCDReadingProgress(camhandle));
                //managerMenu->ui->proBar_capture->setValue(GetQHYCCDReadingProgress(camhandle));
#ifdef WIN32
                Sleep(1);
#else
                usleep(1000);
#endif
                statusLabel_msg->setText(tr("Downloading..."));
                QApplication::processEvents();//响应界面操作，防止界面假死
            }
            managerMenu->ui->proBar_capture->setValue(100);
        }
        else
        {
            ix.imageReady = GetSingleFrame_Failed;
        }
        ix.ForceStop = false; //reset ForceStop flag after GetQHYCCDSingleFrame returned, otherwise can not wait in next time
#if 0
//miniCam5系列看帧序号的东西
        unsigned char value[8];
        value[0] = (ix.ImgData[0] & 0xf0) >> 4;
        value[1] = (ix.ImgData[1] & 0xf0) >> 4;
        value[2] = (ix.ImgData[2] & 0xf0) >> 4;
        value[3] = (ix.ImgData[3] & 0xf0) >> 4;
        value[4] = (ix.ImgData[4] & 0xf0) >> 4;
        value[5] = (ix.ImgData[5] & 0xf0) >> 4;
        value[6] = (ix.ImgData[6] & 0xf0) >> 4;
        value[7] = (ix.ImgData[7] & 0xf0) >> 4;
        char str[16];
        sprintf(str,"%x%x%x%x%x%x%x%x",value[6],value[7],value[4],value[5],value[2],value[3],value[0],value[1]);
        this->setWindowTitle(tr("FrameNum=") + QString(str));
#endif

        if(ix.canCooler)
        {
            mainWidget->startTimerTemp();//开启温控定时器
        }


        statusLabel_msg->setText(tr("IDLE"));
        ix.cameraState = Camera_Idle;

        //---------调用SDK，获取overscan区域,用于噪声分析---------------------
        ret =libqhyccd->GetQHYCCDOverScanArea(camhandle,&pxOB, &pyOB, &sxOB, &syOB);
       // ret = GetQHYCCDOverScanArea(camhandle,&pxOB, &pyOB, &sxOB, &syOB);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.OverScanStartX = pxOB;
            ix.OverScanStartY = pyOB;
            ix.OverScanSizeX = sxOB;
            ix.OverScanSizeY = syOB;
            qDebug() << "GetQHYCCDOverScanArea" << ix.OverScanStartX << ix.OverScanStartY << ix.OverScanSizeX << ix.OverScanSizeY;
        }
        else
        {
            qCritical() << "GetOverScanArea failed,maybe the sdk not support";
        }

        //---------------------获取有效区域----------------------------------
        //ret = GetQHYCCDEffectiveArea(camhandle,&px, &py, &sx, &sy);
        ret = libqhyccd->GetQHYCCDEffectiveArea(camhandle,&px, &py, &sx, &sy);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.onlyStartX = px;
            ix.onlyStartY = py;
            ix.onlySizeX = sx;
            ix.onlySizeY = sy;
            qDebug() << "GetQHYCCDEffectiveArea" << ix.onlyStartX << ix.onlyStartY << ix.onlySizeX << ix.onlySizeY;
        }
        else
        {
            qCritical() << "GetEffectiveArea failed,maybe the sdk not support";
        }

        if(ix.imageReady == GetSingleFrame_Success)
        {
            emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息

            //---------------------overscan calibrate--------------------------
            if(ix.calibrateOverScan)
            {
#if 0
                calibrateOverscan(ix.lastImgData, ix.lastImgData, ix.imageX, ix.imageY, ix.OverScanStartX, ix.OverScanStartY, ix.OverScanSizeX, ix.OverScanSizeY);
#else
                calibrateOverscan(ix.ImgData, ix.lastImgData, ix.imageX, ix.imageY, ix.OverScanStartX, ix.OverScanStartY, ix.OverScanSizeX, ix.OverScanSizeY);
#endif
            }

            //------------------------------------------------------------------
            qDebug() << "display image..." << ix.imageX << ix.imageY << scaleFactor;
#if 0
            displayCaptureImage(ix.imageX, ix.imageY);
#else
            //由于可能进行黑电平校正，校正后数据存储在lastImgData中，所以这里传入lastImgData数据用于显示
            displayCaptureImage_Ex(ix.imageX, ix.imageY, ix.lastImgData);
#endif

            //set viewBox size with current imagex and imagey
            if(ix.zoomMode == Zoom_Auto)
            {
                viewBoxW = 196;
                viewBoxH = 128;
            }
            else
            {
                viewBoxW = 196 * scrollArea_ImgShow->width() / (ix.imageX * scaleFactor);
                viewBoxH = 128 * scrollArea_ImgShow->height() / (ix.imageY * scaleFactor);
            }

            qDebug() << "display screen view...";
            displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);

            if(needCalcScrollBarValue)
            {
                //set scrollbar value for ScrollArea_ImgShow
                scrollArea_ImgShow->horizontalScrollBar()->setValue((viewBoxCX - viewBoxW/2) * scrollArea_ImgShow->width() / viewBoxW);
                scrollArea_ImgShow->verticalScrollBar()->setValue((viewBoxCY - viewBoxH/2) * scrollArea_ImgShow->height() / viewBoxH);
                needCalcScrollBarValue = false;
            }

            qDebug() << "display histogram...";
            displayHistogramImage(ix.imageX, ix.imageY, ix.lastImgData);

            //获取OB level
            getOverScanBlack(ix.lastImgData, ix.imageX, ix.imageY);

            noImgInWorkMode = false;
        }

        qDebug()<<"----capture done----";
        //------------------------------------------------------------------
        managerMenu->ui->pBtn_capture->setChecked(false);
        managerMenu->ui->head_focus->setCheckable(true);
        managerMenu->ui->head_preview->setCheckable(true);

        mainMenuBar->actSaveBMP->setEnabled(true);
        mainMenuBar->actSaveFIT->setEnabled(true);
        mainMenuBar->actSaveJPG->setEnabled(true);
        if(iniFileParams.autoConnect){
            managerMenu->ui->pBtn_auto_histogram->click();
        }
    }

}

/**
 * @brief EZCAP::displayCaptureImage  
 * @param x
 * @param y
 */
void EZCAP::displayCaptureImage(int x, int y)
{
    if(ix.canColor)
        ix.isCvtColor = mainMenuBar->actColorCamera->isChecked();
    else
        ix.isCvtColor = false;

    if(ImgRAW != NULL)
    {
        if((ImgRAW->width != x) || (ImgRAW->height != y))
        {
            if(ImgRAW != NULL)
            {
                cvReleaseImage(&ImgRAW);
                ImgRAW = NULL;
            }
            ImgRAW = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 1 );
        }
    }
    else
    {
        ImgRAW = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 1 );
    }
    ImgRAW->imageData = (char*)ix.dispIplImgData8; //指定opencv图像数据

    if(ImgColor != NULL)
    {
        if((ImgColor->width != x) || (ImgColor->height != y))
        {
            if(ImgColor != NULL)
            {
                cvReleaseImage(&ImgColor);
                ImgColor = NULL;
            }
            ImgColor  = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
        }
    }
    else
    {
        ImgColor  = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
    }
    ImgColor->imageData = (char *)ix.dispIplImgData24;

    if(captureImg != NULL)
    {
        if((captureImg->width != x) || (captureImg->height != y))
        {
            if(captureImg != NULL)
            {
                cvReleaseImage(&captureImg);
                captureImg = NULL;
            }
            captureImg  = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);

        }
    }
    else
    {
        captureImg  = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
    }
    captureImg->imageData = (char *)ix.dispIplImgData24;

   /*
    float tmpstd1,tmprms1,tmpstd2,tmprms2;
    double tmpmax1,tmpmin1,tmpmax2,tmpmin2;

    getImageInfo(ix.ImgData,ix.imageX,ix.imageY,0,0,ix.imageX,200,tmpstd1,tmprms1,tmpmax1,tmpmin1);
    getImageInfo(ix.ImgData,ix.imageX,ix.imageY,0,300,ix.imageX,100,tmpstd2,tmprms2,tmpmax2,tmpmin2);

    qDebug() << "tmprms1" << tmprms1;
    qDebug() << "tmprms2" << tmprms2;
    if(tmprms1 - tmprms2 >= 2000)
    {
        Beep(500,5000);
    }
    */


    // look up the Stretch LUT table to convert current 16bit data to 8bit
    uint32_t s=0,k=0;
    for(unsigned int j=0; j<ix.imageY; j++)
    {
        for(unsigned int i=0; i<ix.imageX; i++)
        {
            ix.OutputData8[k] = ix.StretchLUT[ix.lastImgData[s]+ix.lastImgData[s+1]*256];
            k += 1;
            s += 2;
        }
    }

    //由于opencv分配内存按4字节对齐，因此对于width不是4的整数倍的图像，直接显示存在问题，需要做下处理以避免其自动补齐内存操作的影响。
    if(ImgRAW->nChannels * ImgRAW->width == ImgRAW->widthStep)
    {
        //width是4的倍数，无需补齐数据，则直接拷贝
        memcpy(ix.dispIplImgData8, ix.OutputData8, ImgRAW->imageSize);
    }
    else
    {
        //width不是4的倍数，需要补齐，逐行拷贝跳过widthStep的影响。
        for(unsigned int i=0; i<ix.imageY; i++)
            memcpy(ix.dispIplImgData8 + ImgRAW->widthStep*i, ix.OutputData8 + ix.imageX*i, ix.imageX);
    }


    if(ix.binx == 1 && ix.biny == 1)
    {
        if(ix.bayermatrix == BAYER_BG && ix.isCvtColor)
        {
            cvCvtColor(ImgRAW,ImgColor,CV_BayerBG2BGR_VNG);
        }
        else if(ix.bayermatrix == BAYER_GB && ix.isCvtColor)
        {
            cvCvtColor(ImgRAW,ImgColor,CV_BayerGB2BGR_VNG);
        }
        else if(ix.bayermatrix == BAYER_GR && ix.isCvtColor)
        {
            cvCvtColor(ImgRAW,ImgColor,CV_BayerGR2BGR_VNG);
        }
        else if(ix.bayermatrix == BAYER_RG && ix.isCvtColor)
        {
            cvCvtColor(ImgRAW,ImgColor,CV_BayerRG2BGR_VNG);
        }
        else
        {
            cvCvtColor(ImgRAW,ImgColor,CV_GRAY2RGB);
        }
    }
    else
    {
        if (!ix.isCvtColor)
            cvCvtColor(ImgRAW,ImgColor,CV_GRAY2RGB);
        else
            cvCvtColor(ImgRAW,ImgColor,CV_BayerRG2RGB);

    }

    //显示图像噪声分析
    if(mainMenuBar->actNoiseAnalyse->isChecked())
    {
        ImageAnalyze(ImgColor,x,y);
    }

    //显示有效图像保存范围
    if(ix.ignoreOverScan)
    {
        cvRectangle(ImgColor, cvPoint(ix.onlyStartX, ix.onlyStartY), cvPoint(ix.onlySizeX+ix.onlyStartX, ix.onlySizeY+ix.onlyStartY),
                    CV_RGB(0,255,0),1, 8, 0 );
    }

    //状态栏显示图像分辨率
    QString str1 = QString::number(x) + "x" + QString::number(y);
    statusLabel_imgSize->setText(str1);

    cvResize(ImgColor,ScreenViewImg,3);//输出图像给全屏小窗口（缩略图）

    cvCopy(ImgColor, captureImg);//store current capture image, using for save as..

    if(qImg_capture == NULL)
    {
        qDebug() << "qImg_capture == NULL";
        qImg_capture = this->IplImageToQImage(ImgColor);        
    }
    else if(qImg_capture->width() != (int)ix.imageX || qImg_capture->height() != (int)ix.imageY)
    {
        qDebug() << "qImg_capture size changed";
        qImg_capture = this->IplImageToQImage(ImgColor);
    }

    if(qImg_capture)
    {
        if(ix.zoomMode == Zoom_Auto)
        {
            scrollArea_ImgShow->setWidgetResizable(true);
            ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_capture));
        }
        else
        {
            scrollArea_ImgShow->setWidgetResizable(false);
            ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_capture));
            ui->label_ImgShow->adjustSize();
            ui->label_ImgShow->resize(ix.imageX*scaleFactor, ix.imageY*scaleFactor);
        }
    }

}

void EZCAP::displayCaptureImage_Ex(int x, int y, unsigned char *dataBuf)
{
    //QTime t0 = QTime::currentTime();

    if(ix.canColor)
        ix.isCvtColor = mainMenuBar->actColorCamera->isChecked();
    else
        ix.isCvtColor = false;

    if(ImgRAW != NULL)
    {
        if((ImgRAW->width != x) || (ImgRAW->height != y))
        {
            if(ImgRAW != NULL)
            {
                cvReleaseImage(&ImgRAW);
                ImgRAW = NULL;
            }
            ImgRAW = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 1 );
        }
    }
    else
    {
        ImgRAW = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 1 );
    }
    ImgRAW->imageData = (char*)ix.dispIplImgData8; //指定opencv图像数据

    if(ImgColor != NULL)
    {
        if((ImgColor->width != x) || (ImgColor->height != y))
        {
            if(ImgColor != NULL)
            {
                cvReleaseImage(&ImgColor);
                ImgColor = NULL;
            }
            ImgColor  = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
        }
    }
    else
    {
        ImgColor  = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
    }
    ImgColor->imageData = (char *)ix.dispIplImgData24;

    if(captureImg != NULL)
    {
        if((captureImg->width != x) || (captureImg->height != y))
        {
            if(captureImg != NULL)
            {
                cvReleaseImage(&captureImg);
                captureImg = NULL;
            }
            captureImg  = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);

        }
    }
    else
    {
        captureImg  = cvCreateImageHeader(cvSize(x,y), IPL_DEPTH_8U, 3);
    }
    captureImg->imageData = (char *)ix.dispIplImgData24;

   /*
    float tmpstd1,tmprms1,tmpstd2,tmprms2;
    double tmpmax1,tmpmin1,tmpmax2,tmpmin2;

    getImageInfo(ix.ImgData,ix.imageX,ix.imageY,0,0,ix.imageX,200,tmpstd1,tmprms1,tmpmax1,tmpmin1);
    getImageInfo(ix.ImgData,ix.imageX,ix.imageY,0,300,ix.imageX,100,tmpstd2,tmprms2,tmpmax2,tmpmin2);

    qDebug() << "tmprms1" << tmprms1;
    qDebug() << "tmprms2" << tmprms2;
    if(tmprms1 - tmprms2 >= 2000)
    {
        Beep(500,5000);
    }
    */

    //QTime t1 = QTime::currentTime();
    // look up the Stretch LUT table to convert current 16bit data to 8bit
    uint32_t s=0,k=0;
    for(int j=0; j<y; j++)
    {
        for(int i=0; i<x; i++)
        {
            ix.OutputData8[k] = ix.StretchLUT[dataBuf[s]+dataBuf[s+1]*256];
            k += 1;
            s += 2;
        }
    }
    //qDebug() << "stretch B W cost time" << t1.elapsed();

    //QTime t2 = QTime::currentTime();
    //由于opencv分配内存按4字节对齐，因此对于width不是4的整数倍的图像，直接显示存在问题，需要做下处理以避免其自动补齐内存操作的影响。
    if(ImgRAW->nChannels * ImgRAW->width == ImgRAW->widthStep)
    {
        //width是4的倍数，无需补齐数据，则直接拷贝
        memcpy(ix.dispIplImgData8, ix.OutputData8, ImgRAW->imageSize);
    }
    else
    {
        //width不是4的倍数，需要补齐，逐行拷贝跳过widthStep的影响。
        for(int i=0; i<y; i++)
            memcpy(ix.dispIplImgData8 + ImgRAW->widthStep*i, ix.OutputData8 + x*i, x);
    }
    //qDebug() << "opencv memcpy cost time" << t2.elapsed();

    if(ix.binx == 1 && ix.biny == 1)
    {
        if(ix.bayermatrix == BAYER_BG && ix.isCvtColor)
        {
            cvCvtColor(ImgRAW,ImgColor,CV_BayerBG2BGR_VNG);
        }
        else if(ix.bayermatrix == BAYER_GB && ix.isCvtColor)
        {
            cvCvtColor(ImgRAW,ImgColor,CV_BayerGB2BGR_VNG);
        }
        else if(ix.bayermatrix == BAYER_GR && ix.isCvtColor)
        {
            cvCvtColor(ImgRAW,ImgColor,CV_BayerGR2BGR_VNG);
        }
        else if(ix.bayermatrix == BAYER_RG && ix.isCvtColor)
        {
            cvCvtColor(ImgRAW,ImgColor,CV_BayerRG2BGR_VNG);
        }
        else
        {
            cvCvtColor(ImgRAW,ImgColor,CV_GRAY2RGB);
        }
    }
    else
    {
        if (!ix.isCvtColor)
            cvCvtColor(ImgRAW,ImgColor,CV_GRAY2RGB);
        else
            cvCvtColor(ImgRAW,ImgColor,CV_BayerRG2RGB);

    }

    //显示图像噪声分析
    if(mainMenuBar->actNoiseAnalyse->isChecked())
    {
        ImageAnalyze(ImgColor,x,y);
    }

    //显示有效图像保存范围
    if(ix.ignoreOverScan)
    {
        cvRectangle(ImgColor, cvPoint(ix.onlyStartX, ix.onlyStartY), cvPoint(ix.onlySizeX+ix.onlyStartX, ix.onlySizeY+ix.onlyStartY),
                    CV_RGB(0,255,0),1, 8, 0 );
    }

    //状态栏显示图像分辨率
    QString str1 = QString::number(x) + "x" + QString::number(y);
    statusLabel_imgSize->setText(str1);

    cvResize(ImgColor,ScreenViewImg,3);//输出图像给全屏小窗口（缩略图）

    cvCopy(ImgColor, captureImg);//store current capture image, using for save as..

    if(qImg_capture == NULL)
    {
        qDebug() << "qImg_capture == NULL";
        qImg_capture = this->IplImageToQImage(ImgColor);
    }
    else if(qImg_capture->width() != (int)ix.imageX || qImg_capture->height() != (int)ix.imageY)
    {
        qDebug() << "qImg_capture size changed";
        qImg_capture = this->IplImageToQImage(ImgColor);
    }

    if(qImg_capture)
    {
        //QTime t3 = QTime::currentTime();
        if(ix.zoomMode == Zoom_Auto)
        {
            scrollArea_ImgShow->setWidgetResizable(true);
            ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_capture));
        }
        else
        {
            scrollArea_ImgShow->setWidgetResizable(false);
            ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_capture));
            ui->label_ImgShow->adjustSize();
            ui->label_ImgShow->resize(ix.imageX*scaleFactor, ix.imageY*scaleFactor);
        }
        //qDebug() << "draw QImage cost time" << t3.elapsed();
    }
    //qDebug() << "displayCaptueImage cost time" << t0.elapsed();

}

/**
 * @brief EZCAP::calibrateOverscan
 * @param inbuf
 * @param outbuf
 * @param ImgW
 * @param ImgH
 * @param OSStartX
 * @param OSStartY
 * @param OSSizeX
 * @param OSSizeY
 * @return
 */
int EZCAP::calibrateOverscan(unsigned char* inbuf, unsigned char*outbuf, int ImgW, int ImgH, int OSStartX, int OSStartY, int OSSizeX, int OSSizeY)
{
    CvScalar RMS;
    unsigned short rms;
    IplImage *srcImg;
    IplImage *roi;
    int ret = -1;

    if(inbuf)
    {
        if(((OSStartX + OSSizeX) > ImgW) || ((OSStartY + OSSizeY) > ImgH))
        {
            qCritical() << "calibrateOverscan Error: Out of image size...";
            ret = -1;
        }
        else if(OSSizeX == 0 || OSSizeY == 0)
        {
            qWarning() << "calibrateOverscan Warning: The size of Overscan Area is Zero.";
            ret = 0;
        }
        else
        {
            srcImg = cvCreateImage(cvSize(ImgW,ImgH), IPL_DEPTH_16U, 1 );
            roi = cvCreateImage(cvSize(OSSizeX,OSSizeY), IPL_DEPTH_16U, 1);

            memcpy(srcImg->imageData, inbuf, srcImg->imageSize);

            cvSetImageROI(srcImg, cvRect(OSStartX, OSStartY, OSSizeX, OSSizeY));
            cvCopy(srcImg, roi);
            cvResetImageROI(srcImg);

            RMS = cvAvg(roi);

            rms = RMS.val[0];
            if(rms < 1000)
            {
                QMessageBox::warning(this,tr("Warning"),tr("Offset is too low,Please increase the offset!"),QMessageBox::Ok);
            }

            if(rms < ix.calConstant)
            {
                RMS.val[0] = ix.calConstant - rms;
                cvAddS(srcImg, RMS, srcImg);
            }
            else
            {
                RMS.val[0] = rms - ix.calConstant;
                cvSubS(srcImg, RMS, srcImg);//subtraction the black value.
            }
            memcpy(outbuf, srcImg->imageData, srcImg->imageSize);

            ret = 1;

            cvReleaseImage(&srcImg);
            cvReleaseImage(&roi);
        }
    }

    return ret;
}
//------------------------------------------------------------------------------

//******************************************************************************
//                         histogram页中操作
//******************************************************************************
/**
 * @brief EZCAP::mgrMenu_hSlider_bPos_sliderReleased   
 */
void EZCAP::mgrMenu_hSlider_bPos_sliderReleased()
{
    if(ix.workMode == WorkMode_Preview)
    {
        Preview_BPOS = managerMenu->ui->hSlider_bPos->value();
        iniFileParams.bPos_Preview = Preview_BPOS;
        setStretchLUT(Preview_WPOS, Preview_BPOS);  //set the stretch LUT value

        if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
        {
            qDebug() << "display preview image with B " << Preview_BPOS;
#if 0
            displayPreviewImage(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
#else
            displayPreviewImage_Ex(ix.lastImageX, ix.lastImageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.lastImgData);
#endif
        }
    }
    else if(ix.workMode == WorkMode_Focus)
    {
        Focus_BPOS = managerMenu->ui->hSlider_bPos->value();
        iniFileParams.bPos_Focus = Focus_BPOS;
        setStretchLUT(Focus_WPOS, Focus_BPOS);  //set the stretch LUT value

        if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
        {
            qDebug() << "display focus image with B " << Focus_BPOS;
#if 0
            displayFocusImage(ix.imageX, ix.imageY);
#else
            displayFocusImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
#endif
        }
    }
    else if(ix.workMode == WorkMode_Capture)
    {
        Capture_BPOS = managerMenu->ui->hSlider_bPos->value();
        //qDebug() << "current B value" << Capture_BPOS;
        iniFileParams.bPos_Capture = Capture_BPOS;
        setStretchLUT(Capture_WPOS, Capture_BPOS);  //set the stretch LUT value

        if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
        {
            qDebug() << "display capture image with B " << Capture_BPOS;
#if 0
            displayCaptureImage(ix.imageX, ix.imageY);
#else
            displayCaptureImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
#endif
            displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
        }
    }

}

/**
 * @brief EZCAP::mgrMenu_hSlider_wPos_sliderReleased  
 */
void EZCAP::mgrMenu_hSlider_wPos_sliderReleased()
{
    if(ix.workMode == WorkMode_Preview)
    {
        Preview_WPOS = managerMenu->ui->hSlider_wPos->value();
        iniFileParams.wPos_Preview = Preview_WPOS;
        setStretchLUT(Preview_WPOS, Preview_BPOS);  //set the stretch LUT value

        if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
        {
            qDebug() << "display preview image with W " << Preview_WPOS;
#if 0
            displayPreviewImage(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
#else
            displayPreviewImage_Ex(ix.lastImageX, ix.lastImageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.lastImgData);
#endif
        }
    }
    else if(ix.workMode == WorkMode_Focus)
    {
        Focus_WPOS = managerMenu->ui->hSlider_wPos->value();
        iniFileParams.wPos_Focus = Focus_WPOS;
        setStretchLUT(Focus_WPOS, Focus_BPOS);  //set the stretch LUT value

        if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
        {
            qDebug() << "display focus image with W " << Focus_WPOS;
#if 0
            displayFocusImage(ix.imageX, ix.imageY);
#else
            displayFocusImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
#endif
        }
    }
    else if(ix.workMode == WorkMode_Capture)
    {
        Capture_WPOS = managerMenu->ui->hSlider_wPos->value();
        iniFileParams.wPos_Capture = Capture_WPOS;
        setStretchLUT(Capture_WPOS, Capture_BPOS);  //set the stretch LUT value

        if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
        {
            qDebug() << QString("display capture image with B %1 W %2").arg(Capture_BPOS).arg(Capture_WPOS);
#if 0
            displayCaptureImage(ix.imageX,ix.imageY);

#else
            displayCaptureImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
#endif
            displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
        }
    }

}
/**
 * @brief EZCAP::mgrMenu_pBtn_stretchMinusB_clicked   
 */
void EZCAP::mgrMenu_pBtn_stretchMinusB_clicked()
{
    int curPos = managerMenu->ui->hSlider_bPos->value();
    managerMenu->ui->hSlider_bPos->setValue(curPos - ix.StretchStep);

    mgrMenu_hSlider_bPos_sliderReleased();
}
/**
 * @brief EZCAP::mgrMenu_pBtn_stretchPlusB_clicked  
 */
void EZCAP::mgrMenu_pBtn_stretchPlusB_clicked()
{
    managerMenu->ui->hSlider_bPos->setValue(managerMenu->ui->hSlider_bPos->value() + ix.StretchStep);
    mgrMenu_hSlider_bPos_sliderReleased();
}
/**
 * @brief EZCAP::mgrMenu_pBtn_stretchMinusW_clicked   
 */
void EZCAP::mgrMenu_pBtn_stretchMinusW_clicked()
{
    managerMenu->ui->hSlider_wPos->setValue(managerMenu->ui->hSlider_wPos->value() - ix.StretchStep);
    mgrMenu_hSlider_wPos_sliderReleased();
}
/**
 * @brief EZCAP::mgrMenu_pBtn_stretchPlusW_clicked   
 */
void EZCAP::mgrMenu_pBtn_stretchPlusW_clicked()
{
    managerMenu->ui->hSlider_wPos->setValue(managerMenu->ui->hSlider_wPos->value() + ix.StretchStep);
    mgrMenu_hSlider_wPos_sliderReleased();
}


void HistInfo(uint32_t x,uint32_t y,uint8_t *InBuf,uint8_t *outBuf)
{
    uint32_t s,k;
    uint32_t i;
    uint32_t Histogram[256];
    uint32_t pixel;
    uint32_t maxHist;

    IplImage *histImg,*histResizeImg;
    IplImage *histColorImg, *histColorBigImg;

    histImg          = cvCreateImage(cvSize(256,100), IPL_DEPTH_8U, 3 );
    histResizeImg    = cvCreateImage(cvSize(192,130), IPL_DEPTH_8U, 3 );
    histColorImg     = cvCreateImage(cvSize(256,10), IPL_DEPTH_8U, 3 );
    histColorBigImg  = cvCreateImage(cvSize(1000,10), IPL_DEPTH_8U, 3 );

    cvSet(histImg,CV_RGB(0,0,0),NULL);
    s=x * y ;
    for (i=0;i<256;i++) {Histogram[i]=0;}
    k=1;

    while(s)
    {
        pixel=InBuf[k];
        Histogram[pixel]++;
        k=k+2;
        s--;
    }


    maxHist=Histogram[0];
    for (uint32_t i=1; i < 255; i++)
    {
        if (Histogram[i]>maxHist)
        {
            maxHist=Histogram[i];
        }
    }

    if (maxHist==0)   maxHist=1;

    for (i=0;i<256;i++)
    {
        cvLine(histImg, cvPoint(i,100), cvPoint(i,100-Histogram[i]*256/maxHist),CV_RGB(255,0,0),1,8, 0 );
    }

    for (i = 0; i < 256; i++)
    {
        cvLine(histColorImg,cvPoint(i,0),cvPoint(i,10),CV_RGB(Histogram[i]*256/maxHist,Histogram[i]*256/maxHist,Histogram[i]*256/maxHist),1,8,0);
    }

    cvResize(histColorImg,histColorBigImg,2);
    cvResize(histImg,histResizeImg,2);

    memcpy(outBuf,histResizeImg->imageData,histResizeImg->imageSize);

    cvReleaseImage(&histImg);
    cvReleaseImage(&histResizeImg);
    cvReleaseImage(&histColorImg);
    cvReleaseImage(&histColorBigImg);
}

/**
 * @brief EZCAP::displayHistogramImage 
 * @param x
 * @param y
 * @param buf
 */
void EZCAP::displayHistogramImage(int x, int y, unsigned char *buf)
{
    unsigned int pixel;
    long s,k;

    IplImage *histImg = cvCreateImage(cvSize(192,130), IPL_DEPTH_8U, 3 );
    unsigned char *outBuf = (unsigned char*)malloc(35000000);
    if(outBuf)
    {
        //获取直方图数据    
        HistInfo(x,y,buf,outBuf);
        histImg->imageData = (char*)outBuf;

        //根据buf中数据，保存相应数据到histogram中，用于之后的自动拉伸功能
        s = x * y ;
        for(int i=0; i<256; i++)
        {
            ix.Histogram[i] = 0;//初始化Histogram数组
        }
        k=1;
        //给histogram数组赋值
        while(s)
        {
            pixel = buf[k];
            ix.Histogram[pixel]++;
            k = k+2;
            s--;
        }

        //opencv默认彩色模式BRG,  QImage默认为RGB,故先进行颜色转换，防止IplImage转QImage后红色变蓝色，蓝色变红色
        cvCvtColor(histImg,histImg,CV_BGR2RGB);
        QImage *histgramQImg = IplImageToQImage(histImg);

        //显示histogram
        managerMenu->ui->img_hist->setPixmap(QPixmap::fromImage(*histgramQImg));

        free(outBuf);
        outBuf = NULL;
        cvReleaseImage(&histImg);
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_auto_histogram_clicked  
 */
void EZCAP::mgrMenu_pBtn_auto_histogram_clicked()
{
    //输入HIST信息，然后进行分析，获得自动stretch的数据，进行autostretch
    unsigned long maxHist;
    unsigned char maxHistIndex;
    int W_POS = 1, B_POS = 0;

    //求最大值
    maxHist = ix.Histogram[0];
    maxHistIndex = 0;
    for(int i = 1; i < 255; i++)
    {
        if(ix.Histogram[i]>maxHist)
        {
            maxHist = ix.Histogram[i];
            //ix.maxHistPosition = i;
            maxHistIndex = i;
        }
    }

    //设置自动拉伸的W B值
    switch(ix.autoStretchMode)
    {
        case StretchMode_NoiseFloor:
        {
            if(maxHistIndex < 1)
                maxHistIndex = 1;
            else if(maxHistIndex > 254)
                maxHistIndex = 254;

            B_POS = 256 * (maxHistIndex - 1);
            W_POS = 256 * (maxHistIndex + 1);
        }
        break;
        case StretchMode_BackGroundLevel:
        {
            if(maxHistIndex < 4)
                maxHistIndex = 4;
            else if(maxHistIndex > 245)
                maxHistIndex = 245;

            B_POS = 256 * (maxHistIndex - 3);
            W_POS = 256 * (maxHistIndex + 10);
        }
        break;
        case StretchMode_3timesBackGround:
        {
            if(maxHistIndex < 11)
                maxHistIndex = 11;
            else if(maxHistIndex > 215)
                maxHistIndex = 215;

            B_POS = 256 * (maxHistIndex - 10);
            W_POS = 256 * (maxHistIndex + 30);
        }
        break;
        case StretchMode_10timesBackGround:
        {
            if(maxHistIndex < 31)
                maxHistIndex = 31;
            else if(maxHistIndex > 200)
                maxHistIndex = 200;

            B_POS= 256 * (maxHistIndex - 30);
            W_POS= 256 * (maxHistIndex + 50);
        }
        break;
        case StretchMode_MaxRange:
        {
            B_POS = 0;
            W_POS = 65535;
        }
        break;
        case StretchMode_OverScanX256:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 256;
        }
        break;
        case StretchMode_OverScanX128:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 512;
        }
        break;
        case StretchMode_OverScanX64:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 1028;
        }
        break;
        case StretchMode_OverScanX32:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 2048;
        }
        break;
        case StretchMode_OverScanX16:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 4096;
        }
        break;
        case StretchMode_OverScanX8:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 8192;
        }
        break;

    }
    //越界处理
    if(W_POS > 65535)
        W_POS = 65535;
    if(W_POS < 0)
        W_POS = 0;
    if(B_POS > 65535)
        B_POS = 65535;
    if(B_POS < 0)
        B_POS = 0;

    managerMenu->ui->hSlider_bPos->setValue(B_POS);
    managerMenu->ui->hSlider_wPos->setValue(W_POS);
    mgrMenu_hSlider_bPos_sliderReleased();
    mgrMenu_hSlider_wPos_sliderReleased();

}
//------------------------------------------------------------------------------

//******************************************************************************
//                         screenview页中操作
//******************************************************************************
/**
 * @brief EZCAP::displayScreenViewImage  
 * @param boxWidth
 * @param boxHeight
 * @param boxX  
 * @param boxY  
 */
void EZCAP::displayScreenViewImage(int boxWidth, int boxHeight, int boxX, int boxY)
{
    IplImage *screenTempImg;//由于需要在原始数据上画图，为了避免损坏原始数据，因此需要用临时的来显示
    int x,y;

    x=196;
    y=128;
    screenTempImg = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 3);
    cvCopy(ScreenViewImg, screenTempImg,NULL );

    cvRectangle(screenTempImg, cvPoint(boxX-boxWidth/2, boxY-boxHeight/2), cvPoint(boxX+boxWidth/2, boxY+boxHeight/2), CV_RGB(0,0,255), 1, 8, 0 );

    //iplimage转qimage
    QImage *screenQImg = IplImageToQImage(screenTempImg);

    //ScreenView中显示图片预览
    managerMenu->ui->img_screenView->setPixmap(QPixmap::fromImage(*screenQImg));

    cvReleaseImage(&screenTempImg);
}
//------------------------------------------------------------------------------

//*************************************************************************************
//            事件过滤器以及相关事件
//*************************************************************************************
/**
 * @brief EZCAP::displayedImageMouseDown             
 * @param posX
 * @param posY
 */
void EZCAP::displayedImageMouseDown(int posX, int posY)
{
    if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == WorkMode_Preview && ix.imageReady == GetSingleFrame_Success)
    {
#if 0
        if (ix.imageX != 0)
        {
            if(posX < 100)
                posX = 100;
            if(posX > (int)(ix.imageX - 100))
                posX = ix.imageX - 100;
            if(posY < 50)
                posY = 50;
            if(posY > (int)(ix.imageY - 50))
                posY = ix.imageY - 50;

            FocusCenterX_Pre = posX;
            FocusCenterY_Pre = posY;

            this->displayPreviewImage(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
        }
#else
        if (ix.lastImageX > 0)
        {
            if(posX < 100)
                posX = 100;
            if(posX > (int)(ix.lastImageX - 100))
                posX = ix.lastImageX - 100;
            if(posY < 50)
                posY = 50;
            if(posY > (int)(ix.lastImageY - 50))
                posY = ix.lastImageY - 50;

            FocusCenterX_Pre = posX;
            FocusCenterY_Pre = posY;

            displayPreviewImage_Ex(ix.lastImageX, ix.lastImageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.lastImgData);
        }
#endif
    }
    else if(ix.workMode == WorkMode_Focus && ix.lastWorkMode == WorkMode_Focus && ix.imageReady == GetSingleFrame_Success)
    {
#if 0
        if (ix.imageX != 0)
        {
            if (posX < 800 && posY < 200)
            {
                ZoomFocus_X = posX;
                ZoomFocus_Y = posY;
            }

            this->displayFocusImage(ix.imageX, ix.imageY);
        }
#else
        if (ix.lastImageX > 0)
        {
            if (posX < 800 && posY < 200)
            {
                ZoomFocus_X = posX;
                ZoomFocus_Y = posY;
            }

            displayFocusImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
        }
#endif
    }

}

/**
 * @brief EZCAP::screenViewAreaMouseDown           
 * @param posX
 * @param posY
 */
void EZCAP::screenViewAreaMouseDown(int posX, int posY)
{
    if(ix.workMode == WorkMode_Capture)
    {
        viewBoxCX = posX;
        viewBoxCY = posY;

        screenViewBoxResize();
    }
}

void EZCAP::screenViewBoxResize()
{
    if(ix.workMode == WorkMode_Capture)
    {
        viewBoxW = 196 * scrollArea_ImgShow->width() / ui->label_ImgShow->width();
        viewBoxH = 128 * scrollArea_ImgShow->height() / ui->label_ImgShow->height();

        displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);

        scrollArea_ImgShow->horizontalScrollBar()->setValue((viewBoxCX - viewBoxW/2) * ui->label_ImgShow->width() / 196);
        scrollArea_ImgShow->verticalScrollBar()->setValue((viewBoxCY - viewBoxH/2) * ui->label_ImgShow->height() / 128);
    }
}

void EZCAP::resizeEvent(QResizeEvent *)
{
    screenViewBoxResize(); // window resize, then screenview box redraw...
}

/**
 * @brief EZCAP::focusAssistantImageDblClick       
 */
void EZCAP::focusAssistantImageDblClick()
{
    FocusZoomMode = !FocusZoomMode;
#if 0
    displayFocusImage(ix.imageX, ix.imageY);//刷新focus图像显示
#else
    displayFocusImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
#endif
}

/**
 * @brief EZCAP::eventFilter           
 * @param target
 * @param event
 * @return
 */
bool EZCAP::eventFilter(QObject *target, QEvent *event)
{
    if (target == ui->label_ImgShow)
    {
        //鼠标移动进入label_ImgShow区域，设置鼠标跟踪有效
        if(event->type() == QEvent::Enter)
        {
            ui->label_ImgShow->setMouseTracking(true);
            scrollArea_ImgShow->setMouseTracking(true);
            ui->centralWidget->setMouseTracking(true);
            this->setMouseTracking(true);
        }
        //鼠标离开进入label_ImgShow区域，设置鼠标跟踪无效
        if(event->type() == QEvent::Leave)
        {
            ui->label_ImgShow->setMouseTracking(false);
            scrollArea_ImgShow->setMouseTracking(false);
            ui->centralWidget->setMouseTracking(false);
            this->setMouseTracking(false);

            this->statusLabel_mousePos->setText("");
            this->statusLabel_rgb->setText("");
        }
        //鼠标点击label_ImgShow区域
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
            // check whether it's left button pressed
            if (pMouseEvent->button() == Qt::LeftButton || pMouseEvent->button() == Qt::RightButton)
            {
                displayedImageMouseDown(pMouseEvent->x(),pMouseEvent->y());//适用preview和focus模式

                return true;
            }
        }
    }

    if(target == managerMenu->ui->img_screenView)
    {   //点击screenvieww图像区域
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
            // check whether it's left button pressed
            if (pMouseEvent->button() == Qt::LeftButton)
            {
                //响应screenview图像被点击操作
                screenViewAreaMouseDown(pMouseEvent->x(),pMouseEvent->y());

                return true; //一定要返回true，如果 不想别的object也能接收到这个event
            }
        }
    }

    if(target == ui->image1_focusAssistant)
    {
        //双击focusassistant区域图像
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            focusAssistantImageDblClick();

            return true;
        }
    }

    return QMainWindow::eventFilter(target,event);
}

/**
 * @brief EZCAP::mouseMoveEvent 
 * @param e
 */
void EZCAP::mouseMoveEvent(QMouseEvent *e)
{
    if(ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {
        //label_ImgShow控件的父控件scrollArea_ImgShow
        int x,y;//当前焦点位于图像上的坐标
        if(ix.zoomMode == Zoom_Auto)//mainMenuBar->actFitWindow->isChecked())
        {
            x = (e->x() - scrollArea_ImgShow->pos().x() - ui->label_ImgShow->pos().x()) * ix.imageX / ui->label_ImgShow->width();
            y = (e->y() - scrollArea_ImgShow->pos().y() - ui->label_ImgShow->pos().y()) * ix.imageY / ui->label_ImgShow->height();
        }
        else
        {
            x = (e->x() - scrollArea_ImgShow->pos().x() - ui->label_ImgShow->pos().x())/scaleFactor;
            y = (e->y() - scrollArea_ImgShow->pos().y() - ui->label_ImgShow->pos().y())/scaleFactor;
        }
         //显示坐标
        QString str2 = "("+QString::number(x)+","+QString::number(y)+")";
        this->statusLabel_mousePos->setText(str2);

        //计算灰度值或RGB值，把存储的8为图像数据转成opencv RGB彩色,在转成Qimage图像，最后根据QImage计算
        QString rgbStr = "";
        IplImage *tempImg1 = NULL;
        IplImage *tempImg2 = NULL;

        tempImg1 = cvCreateImage(cvSize(ix.imageX,ix.imageY), IPL_DEPTH_8U, 1);
        tempImg2 = cvCreateImage(cvSize(ix.imageX,ix.imageY), IPL_DEPTH_8U, 3);
        tempImg1->imageData = (char*)ix.OutputData8;
        cvCvtColor(tempImg1,tempImg2,CV_GRAY2RGB);//灰转彩 （RGB为QImage默认颜色模式）

        //转qimage
        QImage *tempQimage = IplImageToQImage(tempImg2);

        if(ix.isCvtColor && ix.workMode == WorkMode_Capture)
        {
            //calcute R G B value
            QRgb qrgb = tempQimage->pixel(x,y);
            int red = qRed(qrgb);
            int green = qGreen(qrgb);
            int blue = qBlue(qrgb);
            double i = (red + green + blue)/3;
            rgbStr = tr("i:") + QString::number(i) + "  " +
                     tr("r:") + QString::number(red) + "," +
                     tr("g:") + QString::number(green) + "," +
                     tr("b:") + QString::number(blue);
        }
        else
        {
            //calcute gray value
            int igray = qGray(tempQimage->pixel(x,y));
            rgbStr = tr("i:") + QString::number(igray) + "  " +
                     tr("r:") + QString::number(igray) + "," +
                     tr("g:") + QString::number(igray) + "," +
                     tr("b:") + QString::number(igray);
        }

        if(tempQimage)
        {
            delete tempQimage;
            tempQimage = NULL;
        }
        cvReleaseImage(&tempImg1);//释放内存
        cvReleaseImage(&tempImg2);

        //显示RGB值
        this->statusLabel_rgb->setText(rgbStr);

    }
}
//-------------------------------------------------------------------------------------

//*************************************************************************************
// 函数IplImageToQImage        IplImage转QImage
//*************************************************************************************
QImage *EZCAP::IplImageToQImage(const IplImage *iplImage)
{
#if 1
    QImage *image = NULL;

    uchar *imgData;
    switch(iplImage->depth)
    {
        case IPL_DEPTH_8U:
        {
            imgData=(uchar *)iplImage->imageData;

            if(iplImage->nChannels == 1)
                image = new QImage(imgData,iplImage->width,iplImage->height, iplImage->widthStep, QImage::Format_Indexed8);
            else if(iplImage->nChannels == 3)
                image = new QImage(imgData,iplImage->width,iplImage->height, iplImage->widthStep, QImage::Format_RGB888);
            else
                qDebug() << "IplImageToQImage: image format is not supported : depth=8U and channels=" << QString::number(iplImage->nChannels);

        }
        break;
        default:
            qDebug() << "IplImageToQImage: image format is not supported";
    }

    return image;
#else
    QImage *qImage;

        int w = iplImage->width;
        int h = iplImage->height;

        qImage = new QImage(w, h, QImage::Format_RGB32);

        int x, y;
        for(x = 0; x < iplImage->width; ++x)
        {
            for(y = 0; y < iplImage->height; ++y)
            {
                CvScalar color = cvGet2D(iplImage, y, x);

                int r = color.val[2];
                int g = color.val[1];
                int b = color.val[0];

                qImage->setPixel(x, y, qRgb(r,g,b));
            }
        }

        return qImage;
#endif
}
//-------------------------------------------------------------------------------------

//*************************************************************************************
// 函数 QImageToIplImage       QImage转IplImage
//*************************************************************************************
IplImage *EZCAP::QImageToIplImage(const QImage *qImage)
{
    int width = qImage->width();
    int height = qImage->height();
    CvSize Size;
    Size.height = height;
    Size.width = width;
    IplImage *IplImageBuffer = cvCreateImage(Size, IPL_DEPTH_16U, 1);
    char *charTemp = (char *) IplImageBuffer->imageData;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int index = y * width + x;
            charTemp[index] = (unsigned char) qGray(qImage->pixel(x, y));
        }
    }
    return IplImageBuffer;
}
//-------------------------------------------------------------------------------------

//*************************************************************************************
//  函数 currentWorkingModeChanged       改变tabWidget的当前活动页
//*************************************************************************************
/**
 * @brief EZCAP::currentWorkingModeChanged  
 * @param index
 */
void EZCAP::currentWorkingModeChanged(int workmode)
{
    switch(workmode)
    {
    case WorkMode_Preview:
        {
            qDebug() << "set layout for Preview work mode";

            ui->widgetFocusAssistant->setParent(NULL);//设置focusAssistant父控件为空
            delete scrollArea_ImgShow->layout();//清除布局
            ui->label_ImgShow->setMaximumHeight(16777215);//重设label_imgShow最大宽高
            ui->label_ImgShow->setMaximumWidth(16777215);
            scrollArea_ImgShow->setWidget(ui->label_ImgShow);//重新布局
            ui->widgetFocusAssistant->setVisible(false);

            managerMenu->ui->hSlider_bPos->setValue(Preview_BPOS);
            managerMenu->ui->hSlider_wPos->setValue(Preview_WPOS);
            setStretchLUT(Preview_WPOS, Preview_BPOS);  //set the stretch LUT value

            mainMenuBar->menuImageProcess->setEnabled(false);
            mainMenuBar->menuZoom->setEnabled(false);
            mainMenuBar->actIgnoreOverScanArea->setEnabled(false);
            mainMenuBar->actCalibrateOverScan->setEnabled(false);
            mainMenuBar->actSaveBMP->setEnabled(false);
            mainMenuBar->actSaveFIT->setEnabled(false);
            mainMenuBar->actSaveJPG->setEnabled(false);
            if(ix.canColor)
                mainMenuBar->actColorCamera->setEnabled(false);

            if(ix.Exptime > 10000)
                ix.Exptime = 10000;  // if exposure > 10000ms in Preview mode, then exposure=10000ms
            managerMenu->ui->hSlider_exposure_preview->setValue(ix.Exptime);
            if(ix.canGain)
            {
                managerMenu->ui->hSlider_gain_preview->setValue(ix.gain);
                unsigned int ret = QHYCCD_ERROR;
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN, ix.gain);
                //ret = SetQHYCCDParam(camhandle,CONTROL_GAIN, ix.gain);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.lastGain = ix.gain;
                    qDebug() << "SetQHYCCDParam CONTROL_GAIN" << ix.gain;
                }
                else
                {
                    qCritical() << "SetQHYCCDParam CONTROL_GAIN failure";
                }
            }
            if(ix.canOffset)
            {
                managerMenu->ui->hSlider_offset_preview->setValue(ix.offset);
                unsigned int ret = QHYCCD_ERROR;
                ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.offset);
                //ret = SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.offset);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.lastOffset = ix.offset;
                    qDebug() << "SetQHYCCDParam CONTROL_OFFSET " << ix.offset;
                }
                else
                {
                    qCritical() << "SetQHYCCDParam CONTROL_OFFSET failure";
                }
            }


            //---set params
            ix.onLiveMode = false;
            ix.binx = ix.maxBinx;
            ix.biny = ix.maxBiny;
            ix.DownloadSpeed = 1;
            ix.zoomMode = Zoom_SpecifyScaling;
            scaleFactor = 1;  //指定preview模式下缩放比例为1，否则mousemove中计算的坐标会不对

            if(ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
            {
                scrollArea_ImgShow->setWidgetResizable(false);
                //qDebug() << "show preview image" << ix.imageX << ix.imageY << scaleFactor;
                //ui->label_ImgShow->resize(ix.imageX * scaleFactor, ix.imageY * scaleFactor); //自适应图片大小
#if 0
                displayPreviewImage(ix.imageX, ix.imageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre);
#else
                displayPreviewImage_Ex(ix.lastImageX, ix.lastImageY, 200, 100, FocusCenterX_Pre, FocusCenterY_Pre, ix.lastImgData);
#endif

                //set scrollbar value for ScrollArea_ImgShow
                scrollArea_ImgShow->horizontalScrollBar()->setValue(hScrollBarValue_Pre);
                scrollArea_ImgShow->verticalScrollBar()->setValue(vScrollBarValue_Pre);

                displayHistogramImage(ix.imageX, ix.imageY, ix.lastImgData);
                noImgInWorkMode = false;
            }
            else
            {
                scrollArea_ImgShow->setWidgetResizable(true);//自适应父控件大小
                ui->label_ImgShow->setPixmap(QPixmap(":/image/balck.bmp"));
                managerMenu->ui->img_hist->setPixmap(QPixmap(":/image/balck.bmp"));
                noImgInWorkMode = true;
            }

            managerMenu->ui->img_screenView->setPixmap(QPixmap(":/image/black.bmp"));
        }
        break;
    case WorkMode_Focus:
        {
            qDebug() << "set layout for Focus work mode";

            ui->label_ImgShow->setMaximumHeight(200);//设置最大宽高，以实现固定图像显示区域大小效果
            //add focusAssistant panel, re-layout
            QVBoxLayout *layout1 = new QVBoxLayout(scrollArea_ImgShow);
            layout1->addWidget(ui->label_ImgShow);
            layout1->addWidget(ui->widgetFocusAssistant);
            scrollArea_ImgShow->setLayout(layout1);
            ui->widgetFocusAssistant->setVisible(true);

            managerMenu->ui->hSlider_bPos->setValue(Focus_BPOS);  //set stretch W B
            managerMenu->ui->hSlider_wPos->setValue(Focus_WPOS);
            setStretchLUT(Focus_WPOS, Focus_BPOS);  //set the stretch LUT value

            mainMenuBar->menuImageProcess->setEnabled(false);
            mainMenuBar->menuZoom->setEnabled(false);
            mainMenuBar->actIgnoreOverScanArea->setEnabled(false);
            mainMenuBar->actCalibrateOverScan->setEnabled(false);
            mainMenuBar->actSaveBMP->setEnabled(false);
            mainMenuBar->actSaveFIT->setEnabled(false);
            mainMenuBar->actSaveJPG->setEnabled(false);
            if(ix.canColor)
                mainMenuBar->actColorCamera->setEnabled(false);

            if(ix.Exptime > 3600)
                ix.Exptime = 3600;
            managerMenu->ui->hSlider_exposure_focus->setValue(ix.Exptime);
            if(ix.canGain)
                managerMenu->ui->hSlider_gain_focus->setValue(ix.gain);
            if(ix.canOffset)
                managerMenu->ui->hSlider_offset_focus->setValue(ix.offset);

            //---set params
            ix.onLiveMode = false;
            ix.binx = 1;
            ix.biny = 1;
            ix.DownloadSpeed = 1;
            ix.zoomMode = Zoom_SpecifyScaling;
            scaleFactor = 1;  //指定focus模式下缩放比例为1，否则mousemove中计算的坐标会不对

            if(ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
            {
                if(ix.imageX < 800 && ix.imageX > 0)
                    ui->label_ImgShow->setMaximumWidth(ix.imageX);
                else
                    ui->label_ImgShow->setMaximumWidth(800);

#if 0
                displayFocusImage(ix.imageX, ix.imageY);
#else
                displayFocusImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
#endif
                displayHistogramImage(ix.imageX, ix.imageY, ix.lastImgData);
                noImgInWorkMode = false;
            }
            else
            {
                ui->label_ImgShow->setMaximumWidth(800);
                ui->label_ImgShow->setPixmap(QPixmap(":/image/balck.bmp"));  //if no focus image, set it be black
                managerMenu->ui->img_hist->setPixmap(QPixmap(":/image/balck.bmp"));
                managerMenu->ui->img_screenView->setPixmap(QPixmap(":/image/black.bmp"));
                noImgInWorkMode = true;
            }
        }
        break;
    case WorkMode_Capture:
        {
            qDebug() << "set layout for Capture work mode";

            ui->widgetFocusAssistant->setParent(NULL);//设置focusAssistant父控件为空
            delete scrollArea_ImgShow->layout();
            ui->label_ImgShow->setMaximumHeight(16777215);//重设label_imgShow最大宽高
            ui->label_ImgShow->setMaximumWidth(16777215);
            scrollArea_ImgShow->setWidget(ui->label_ImgShow);//重新布局
            ui->widgetFocusAssistant->setVisible(false); //hide focusAssistant

            managerMenu->ui->pBtn_expUnit_capture->setText(QString("ms"));
            managerMenu->ui->hSlider_exposure_capture->setValue(ix.Exptime);
            if(ix.canGain)
                managerMenu->ui->hSlider_gain_capture->setValue(ix.gain);
            if(ix.canOffset)
                managerMenu->ui->hSlider_offset_capture->setValue(ix.offset);

            qDebug() << QString("set stretch value with previous setting B:%1 W:%2").arg(Capture_BPOS).arg(Capture_WPOS);
            managerMenu->ui->hSlider_bPos->setValue(Capture_BPOS); //set stretch B W
            managerMenu->ui->hSlider_wPos->setValue(Capture_WPOS);
            setStretchLUT(Capture_WPOS, Capture_BPOS);

            mainMenuBar->menuImageProcess->setEnabled(true);
            mainMenuBar->menuZoom->setEnabled(true);
            mainMenuBar->actIgnoreOverScanArea->setEnabled(true);
            mainMenuBar->actIgnoreOverScanArea->setChecked(ix.ignoreOverScan);
            mainMenuBar->actCalibrateOverScan->setEnabled(true);
            mainMenuBar->actCalibrateOverScan->setChecked(ix.calibrateOverScan);
            mainMenuBar->actSaveBMP->setEnabled(false);
            mainMenuBar->actSaveFIT->setEnabled(false);
            mainMenuBar->actSaveJPG->setEnabled(false);
            if(ix.canColor)
                mainMenuBar->actColorCamera->setEnabled(true);

            ix.onLiveMode = false;
            if(ix.canbin44 && managerMenu->ui->bin4x4->isChecked())
            {
                ix.binx = 4;
                ix.biny = 4;
            }
            else if(ix.canbin33 && managerMenu->ui->bin3x3->isChecked())
            {
                ix.binx = 3;
                ix.biny = 3;
            }
            else if(ix.canbin22 && managerMenu->ui->bin2x2->isChecked())
            {
                ix.binx = 2;
                ix.biny = 2;
            }
            else
            {
                ix.binx = 1;
                ix.biny = 1;
            }
            if(ix.canHighSpeed && managerMenu->ui->checkBox_highSpeed->isChecked())
                ix.DownloadSpeed = 1;
            else
                ix.DownloadSpeed = 0;

            if(mainMenuBar->actFitWindow->isChecked())
            {
                ix.zoomMode = Zoom_Auto;
            }
            else
            {
                ix.zoomMode = Zoom_SpecifyScaling;
                if(mainMenuBar->act0_25X->isChecked())
                    scaleFactor = 0.25;
                else if(mainMenuBar->act0_5X->isChecked())
                    scaleFactor = 0.5;
                else if(mainMenuBar->act0_75X->isChecked())
                    scaleFactor = 0.75;
                else if(mainMenuBar->act1_5X->isChecked())
                    scaleFactor = 1.5;
                else if(mainMenuBar->act2X->isChecked())
                    scaleFactor = 2.0;
                else
                    scaleFactor = 1.0;
            }

            if(ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
            {
#if 0
                displayCaptureImage(ix.imageX, ix.imageY);
#else
                displayCaptureImage_Ex(ix.lastImageX, ix.lastImageY, ix.lastImgData);
#endif
                displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);

                //set scrollbar value for ScrollArea_ImgShow
                scrollArea_ImgShow->horizontalScrollBar()->setValue(hScrollBarValue_Cap);
                scrollArea_ImgShow->verticalScrollBar()->setValue(vScrollBarValue_Cap);

                displayHistogramImage(ix.imageX, ix.imageY, ix.lastImgData);
                noImgInWorkMode = false;
            }
            else
            {
                scrollArea_ImgShow->setWidgetResizable(true);//label自适应父控件大小
                ui->label_ImgShow->setPixmap(QPixmap(":/image/balck.bmp"));
                managerMenu->ui->img_hist->setPixmap(QPixmap(":/image/balck.bmp"));
                managerMenu->ui->img_screenView->setPixmap(QPixmap(":/image/black.bmp"));
                noImgInWorkMode = true;
            }
        }
        break;
    }
}

//-------------------------------------------------------------------------------------

//*************************************************************************************
//         图像显示区域操作
//*************************************************************************************
/**
 * @brief EZCAP::on_label_ImgShow_customContextMenuRequested  
 * @param pos
 */
void EZCAP::on_label_ImgShow_customContextMenuRequested(const QPoint &pos)
{
    if(ix.lastWorkMode == ix.workMode && ix.imageReady)
    {
        if(cmenu_imgArea)//保证同时只存在一个menu，及时释放内存
        {
            delete cmenu_imgArea;
            cmenu_imgArea = NULL;
        }
        cmenu_imgArea = new QMenu(ui->label_ImgShow);  //popMenu

        QAction *actGoFocusCenter = cmenu_imgArea->addAction(tr("Go Focus Center"));
        QAction *actSaveFit = cmenu_imgArea->addAction(tr("Save FIT"));
        QAction *actSaveBmp = cmenu_imgArea->addAction(tr("Save BMP"));
        QAction *actSaveJpg = cmenu_imgArea->addAction(tr("Save JPG"));
        QAction *actOpenSavedPath = cmenu_imgArea->addAction(tr("Open Last Saved Folder"));

        connect(actGoFocusCenter, SIGNAL(triggered()), this, SLOT(goFocusCenter()));
        connect(actSaveFit, SIGNAL(triggered()), this, SLOT(saveAsFIT()));
        connect(actSaveBmp, SIGNAL(triggered()), this, SLOT(saveAsBMP()));
        connect(actSaveJpg, SIGNAL(triggered()), this, SLOT(saveAsJPG()));
        connect(actOpenSavedPath, SIGNAL(triggered()), this, SLOT(openFolder()));

        qDebug() << "cursor" << pos.x() << pos.y();
        cmenu_imgArea->exec(QCursor::pos());//在当前鼠标位置显示
    }
}

/**
 * @brief EZCAP::goFocusCenter   
 */
void EZCAP::goFocusCenter()
{
    //跳转到focus页
    managerMenu->ui->head_focus->click();
}

//-------------------------------------------------------------------------------------

/**
 * @brief EZCAP::getImageInfo 
 * @param Buf
 * @param ImageWidth
 * @param ImageHeight
 * @param startX
 * @param startY
 * @param sizeX
 * @param sizeY
 * @param std
 * @param rms
 * @param max
 * @param min
 */
void EZCAP::getImageInfo(unsigned char *Buf,int ImageWidth,int ImageHeight, int startX,int startY,int sizeX,int sizeY, float &std,float &rms,double &max,double &min)
{
    //输入16位图像尺寸和图像缓冲区，以及局部图像开始位置和长宽，输出方差，平均值，最大值，最小值
    CvScalar RMS,STD;
    IplImage *FitImg;

    if(sizeX > 0 && sizeY > 0)
    {
        FitImg = cvCreateImage(cvSize(ImageWidth,ImageHeight), IPL_DEPTH_16U, 1);
        FitImg->imageData = (char*)Buf;

        if(startX + sizeX > ImageWidth || startY + sizeY > ImageHeight)
        {
            qCritical() << "getImageInfo Error: Out of image size. can not roi...";
            rms = 0;
            std = 0;
        }
        else
        {
            cvSetImageROI(FitImg, cvRect(startX,startY,sizeX,sizeY));
            cvAvgSdv(FitImg, &RMS, &STD, NULL);
            cvMinMaxLoc(FitImg, &min, &max, 0, 0, NULL );
            cvResetImageROI(FitImg);

            rms=RMS.val[0];
            std=STD.val[0];
        }
        cvReleaseImage(&FitImg);
    }
    else
    {
        qWarning() << "getImageInfo: sizeX = sizeY = 0";
        rms = 0;
        std = 0;
        min = 0;
        max = 0;
    }
}

/**
 * @brief EZCAP::ImageAnalyze 
 * @param Img
 * @param x
 * @param y
 */
void EZCAP::ImageAnalyze(IplImage *Img,int x,int y)
{
    //从IMGDATA区获得原始数据进行分析。函数输入的IMG仅用于画框和显示
    int OX,Y;
    float std,rms;
    double max,min;

    OX=x;
    Y=y;

    QString std2str,rms2str,max2str,min2str;

    for (int i=1; i < 9; i=i+3)
    {
        for (int j = 1; j < 9; j=j+3)
        {
            getImageInfo(ix.ImgData,x,y,OX*i/9,Y*j/9,OX/9,Y/9,std,rms,max,min);

            cvRectangle(Img, cvPoint(OX*i/9,Y*j/9), cvPoint(OX*i/9+OX/9,Y*j/9+Y/9),CV_RGB(255,0,0),1,8,0);

            cvPutText(Img, (QString("STD:")+std2str.setNum(std,'f',1)).toStdString().c_str(),  cvPoint(OX*i/9,Y*j/9+Y/9+20), &QHYFont, CV_RGB(255,0,0) );
            cvPutText(Img, (QString("RMS:")+rms2str.setNum(rms,'f',1)).toStdString().c_str(),  cvPoint(OX*i/9,Y*j/9+Y/9+35), &QHYFont, CV_RGB(255,0,0) );
            cvPutText(Img, (QString("MAX:")+max2str.setNum(max,'f',1)).toStdString().c_str(),  cvPoint(OX*i/9,Y*j/9+Y/9+50), &QHYFont, CV_RGB(255,0,0) );
            cvPutText(Img, (QString("MIN:")+min2str.setNum(min,'f',1)).toStdString().c_str(),  cvPoint(OX*i/9,Y*j/9+Y/9+65), &QHYFont, CV_RGB(255,0,0) );
        }
    }

    getImageInfo(ix.ImgData,x,y,ix.OverScanStartX,ix.OverScanStartY,ix.OverScanSizeX,ix.OverScanSizeY,std,rms,max,min);

    cvRectangle(Img, cvPoint(ix.OverScanStartX ,ix.OverScanStartY),
            cvPoint(ix.OverScanStartX+ix.OverScanSizeX,ix.OverScanStartY+ix.OverScanSizeY),CV_RGB(0,255,0),1,8,0);

    cvPutText(Img, (QString("OverScan Area")).toStdString().c_str(),
              cvPoint(ix.OverScanStartX-150,ix.OverScanStartY+5), &QHYFont, CV_RGB(0,255,0) );
    cvPutText(Img, (QString("STD:")+std2str.setNum(std,'f',1)).toStdString().c_str(),
              cvPoint(ix.OverScanStartX-150,ix.OverScanStartY+20), &QHYFont, CV_RGB(0,255,0) );
    cvPutText(Img, (QString("RMS:")+rms2str.setNum(rms,'f',1)).toStdString().c_str(),
              cvPoint(ix.OverScanStartX-150,ix.OverScanStartY+35), &QHYFont, CV_RGB(0,255,0) );
    cvPutText(Img, (QString("MAX:")+max2str.setNum(max,'f',1)).toStdString().c_str(),
              cvPoint(ix.OverScanStartX-150,ix.OverScanStartY+50), &QHYFont, CV_RGB(0,255,0) );
    cvPutText(Img, (QString("MIN:")+min2str.setNum(min,'f',1)).toStdString().c_str(),
              cvPoint(ix.OverScanStartX-150,ix.OverScanStartY+65), &QHYFont, CV_RGB(0,255,0) );

    OverScanRMS = rms;

}

/**
 * @brief EZCAP::AutoFileName  
 * @return
 */
QString EZCAP::AutoFileName()
{
    int H,M,S,MS;
    QString TimeStamp;

    QTime timeForStamp = QTime::currentTime();
    H = timeForStamp.hour();//获取小时
    M = timeForStamp.minute();//获取分钟
    S = timeForStamp.second();//获取秒
    MS = timeForStamp.msec();//获取毫秒
    TimeStamp = QString::number(H)+"-"+ QString::number(M)+"-"+QString::number(S)+"-"+QString::number(MS);

    return TimeStamp;
}
//-------------------------------------------------------------------------------------

//******************************************************************************************************************************
// *     线程ExecutePlanTable          执行计划任务表
// *****************************************************************************************************************************
IplImage *imgsum,*imgcur,*imgavg,*imgresult,*imgblack;
bool darkframeflag = false;

void ExecutePlanTable::run()
{
    //-----------------处理连续拍摄事件---------------
    if(ix.plannerState == PlannerStatus_Start)
    {
        planner_dialog->ui->status1_planner->setText(tr("Capture..."));

        //---highspeed setting
        if(ix.canHighSpeed)
        {
            managerMenu->ui->checkBox_highSpeed->setChecked(planner_dialog->ui->cBox_highReadSpeed_planner->isChecked());
        }

        //读取Script.ini中的内容
        QString path_script;
        bool c_enable;//该项任务是否执行
        int c_exp;    //该项任务设置的曝光时间
        int c_cfwPos;  //   色轮位置
        int c_repeat; //   重复次数
        int c_bin;    //   bin模式
        int c_delay;  //   等待时间
        QString namez;
        int c_loop;  //任务循环次数
        int c_total; //计划表中任务总条目数
        int c_gain;//增益 注意这个增益是百分比
        bool c_avg_enable;//该项任务执行后，是否平均输出（仅仅输出平均后的项目）
        bool c_subblack_enable;//是否减去暗场，有个前提，不含FPN噪声的暗场必须存在
        bool c_subbias_enable;//是否减去bias场

        QString tempStr = "";

        char biasfilename[1024];
        char blackfilename[1024];

        path_script = QCoreApplication::applicationDirPath() + "/" + "Script.ini";
        path_script = QDir::toNativeSeparators(path_script); //converted '/'. on windows return \   else return /

        QSettings *configIniRead = new QSettings(path_script, QSettings::IniFormat);

        //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
        configIniRead->beginGroup("Total");
        c_total = configIniRead->value("TaskNum", 0).toInt();
        configIniRead->endGroup();

        configIniRead->beginGroup("Loop");
        c_loop = configIniRead->value("LoopNum", 0).toInt();
        configIniRead->endGroup();

        for(int LP = 0; LP < c_loop; LP++)
        {
            for(int iTask = 1; iTask <= c_total; iTask++)
            {
                configIniRead->beginGroup("CAP" + QString::number(iTask));
                c_enable = configIniRead->value("Enabled", false).toBool();
                c_bin = configIniRead->value("BIN", 1).toInt();
                c_exp = configIniRead->value("EXP", 1).toInt();
                c_repeat = configIniRead->value("Repeat", 1).toInt();
                c_cfwPos = configIniRead->value("CFW", 0).toInt();
                c_delay = configIniRead->value("Delay", 0).toInt();
                c_gain = configIniRead->value("GAIN", 0).toInt();
                c_avg_enable = configIniRead->value("AVGEnabled", false).toBool();
                c_subblack_enable = configIniRead->value("SubBlackEnabled", false).toBool();
                c_subbias_enable = configIniRead->value("SubBiasEnabled", false).toBool();
                configIniRead->endGroup();

                memset(biasfilename,'\0',1024);
                sprintf(biasfilename,"./Bias/%s.fit",camid);

                memset(blackfilename,'\0',1024);
                sprintf(blackfilename,"./Black/%s-BIN%d-EXP%d-GAIN%d.fit",camid,c_bin,c_exp,c_gain);

                if(c_enable)
                {
                    //修改任务表格中iTask任务背景色，以表示其正在执行
                    emit changeRowColor(iTask - 1, QColor(100,100,0));

                    //-----------------------------color filter wheel setting-----------------------------
                    if(c_cfwPos != 0)
                    {
                        if(ix.canFilterWheel)
                        {
                            emit changeCurCFWPos(c_cfwPos - 1);
                        }
                        else
                        {
                            //send signal to show the error info
                            emit showErrorInfo(tr("Forced To Stop Planner"),tr("Camera not support CFW, cannot run CFW to ")+ QString::number(c_cfwPos));

                            //reset the row color
                            emit changeRowColor(iTask - 1, QColor(80, 50, 0));

                            //set the value to the loop end
                            LP = c_loop;
                            iTask = c_total + 1;

                            break;
                        }
                    }

                    //-----------------------------gain setting---------------------------------
                    if(ix.canGain)
                        managerMenu->ui->hSlider_gain_capture->setValue(c_gain);

                    //-----------------------------bin mode setting-----------------------------
                    if(c_bin == 1 && ix.canbin11)
                        managerMenu->ui->bin1x1->setChecked(true);
                    else if(c_bin == 2 && ix.canbin22)
                        managerMenu->ui->bin2x2->setChecked(true);
                    else if(c_bin == 3 && ix.canbin33)
                        managerMenu->ui->bin3x3->setChecked(true);
                    else if(c_bin == 4 && ix.canbin44)
                        managerMenu->ui->bin4x4->setChecked(true);
                    else
                    {   //send signal to show the error info
                        emit showErrorInfo(tr("Forced To Stop Planner"),tr("Camera Not Support Binning ") + QString::number(c_bin)+"*"+QString::number(c_bin) + tr(", Planner will be Forced To Stop!"));

                        //reset the row color
                        emit changeRowColor(iTask - 1, QColor(80, 50, 0));

                        //set the value to the loop end
                        LP = c_loop;
                        iTask = c_total + 1;

                        break;
                    }

                    //-----------------------------exposure time setting-----------------------------
                    if (c_exp > 1000)
                    {
                        managerMenu->ui->pBtn_expUnit_capture->setText("s");
                        managerMenu->ui->hSlider_exposure_capture->setValue(c_exp/1000);
                    }
                    else
                    {
                        managerMenu->ui->pBtn_expUnit_capture->setText("ms");
                        managerMenu->ui->hSlider_exposure_capture->setValue(c_exp);
                    }

                    //-----------------------------start repeat capture and save image-----------------------------
                    for (int iRP = 0; iRP < c_repeat; iRP++)
                    {
                        tempStr = "CAP" + QString::number(iTask) + " " + QString::number(iRP + 1) + " of " + QString::number(c_repeat) + " Loop:" + QString::number(LP+1);
                        planner_dialog->ui->status1_planner->setText(tempStr);

#ifdef Q_OS_WIN32
                        QString savePath;
                        if(planner_dialog->ui->pBtn_folder_planner->text().compare("Folder") == 0 )
                        {
                            savePath = QCoreApplication::applicationDirPath();
                        }
                        else
                        {
                            savePath = planner_dialog->ui->pBtn_folder_planner->text();
                        }
                        DiskTools *dt = new DiskTools;
                        quint64 freeSpace = dt->getDiskFreeSpace(savePath);
                        //qDebug() << "Free:" << freeSpace << "MB";
                        planner_dialog->ui->status2_planner->setText(tr("Free:")+QString::number(freeSpace)+"MB");
#endif

                        //发送doCapture信号，执行Capture 获取图像
                        emit startCaptureImage();

                        if (c_loop > 1)
                        {
                            namez = configIniRead->value("/ScriptName/FileName").toString().trimmed() + "-" + QString::number(iTask) + "-" +
                                    QString::number(managerMenu->ui->hSlider_exposure_capture->value()) + managerMenu->ui->pBtn_expUnit_capture->text() +
                                    "-" + QString::number(iRP + 1) + "-CFW" + QString::number(c_cfwPos) + "-Loop" + QString::number(LP) + ".fit";
                        }
                        else
                        {
                            namez = configIniRead->value("/ScriptName/FileName").toString().trimmed() + "-" + QString::number(iTask) + "-" +
                                    QString::number(managerMenu->ui->hSlider_exposure_capture->value()) + managerMenu->ui->pBtn_expUnit_capture->text() +
                                    "-" + QString::number(iRP + 1) + "-CFW" + QString::number(c_cfwPos) + ".fit";
                        }


                        if(QFile::exists(biasfilename) && c_subbias_enable)
                        {
                            qDebug() << "enable sub bias frame function" << endl;

                            if(imgavg == NULL)
                            {
                                imgavg = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_16U,1);
                                cvZero(imgavg);
                                fitHeader_dialog->FITRead(biasfilename,ix.imageX * ix.imageY,(unsigned char*)imgavg->imageData);
                            }

                            if(imgcur == NULL)
                            {
                                imgcur = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_16U,1);
                                cvZero(imgcur);
                            }

                            if(imgresult == NULL)
                            {
                                imgresult = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_16U,1);
                                cvZero(imgresult);
                            }

                            memcpy(imgcur->imageData,ix.ImgData,imgcur->imageSize);
                            cvSub(imgcur,imgavg,imgresult,NULL);
                            memcpy(ix.ImgData,imgresult->imageData,imgresult->imageSize);
                        }

                        if(QFile::exists(blackfilename) && c_subblack_enable)
                        {
                            qDebug() << "enable sub balck frame function" << endl;

                            if(imgblack == NULL)
                            {
                                imgblack = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_16U,1);
                                cvZero(imgblack);
                            }

                            if(imgcur == NULL)
                            {
                                imgcur = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_16U,1);
                                cvZero(imgcur);
                            }

                            if(imgresult == NULL)
                            {
                                imgresult = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_16U,1);
                                cvZero(imgresult);
                            }

                            fitHeader_dialog->FITRead(blackfilename,ix.imageX * ix.imageY,(unsigned char*)imgblack->imageData);

                            memcpy(imgcur->imageData,ix.ImgData,imgcur->imageSize);
                            cvSub(imgcur,imgblack,imgresult,NULL);
                            memcpy(ix.ImgData,imgresult->imageData,imgresult->imageSize);
                        }


                        //this will need a flag in panel for turning on OR turning off this function
                        if(c_avg_enable)
                        {
                            qDebug() << "enable average function" << endl;
                            if(imgcur == NULL)
                            {
                                imgcur = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_16U,1);
                                cvZero(imgcur);
                            }

                            if(imgsum == NULL)
                            {
                                imgsum = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_32F,1);
                                cvZero(imgsum);
                            }

                            if(imgavg == NULL)
                            {
                                imgavg = cvCreateImage(cvSize(ix.imageX,ix.imageY),IPL_DEPTH_16U,1);
                                cvZero(imgavg);
                            }

                            memcpy(imgcur->imageData,ix.ImgData,imgcur->imageSize);

                            cvAcc(imgcur,imgsum);

                            if((iRP + 1) == c_repeat)
                            {
                                cvConvertScale(imgsum,imgavg,1.0/c_repeat);                            

                                if(iTask == 1)
                                {
                                    fitHeader_dialog->FITWrite(biasfilename, (unsigned char *)imgavg->imageData);
                                }
                                else if(iTask > 1 && darkframeflag)
                                {
                                    fitHeader_dialog->FITWrite(blackfilename, (unsigned char *)imgavg->imageData);
                                }
                                else
                                {
                                    //sava Image as fit
                                    fitHeader_dialog->FITWrite(namez, ix.ImgData);
                                }
                            }
                        }
                        else
                        {
                            //sava Image as fit
                            fitHeader_dialog->FITWrite(namez, ix.ImgData);
                        }

                        //---------------if click ForceStop------------------
                        if(ix.plannerState == PlannerStatus_Stop)
                        {   //ForceSTOP,  break out repeat capture loop
                            iRP = c_repeat;
                            c_delay = 0;
                            qDebug() << "force stop planner...";
                        }

                        //----------------Delay setting----------------------
                        if(c_delay > 0)
                            QThread::msleep(c_delay*1000);

                    }

                    //task row1 finished, reset the row color
                    emit changeRowColor(iTask - 1, QColor(80, 50, 0));

                    if(ix.plannerState == PlannerStatus_Stop)
                    {   //ForceSTOP,  break out tasks loop
                        iTask = c_total + 1;
                        LP = c_loop;
                    }

                }

                if(planner_dialog->firstRowIsBais && iTask == 1)
                {
                    emit finish_baisImages();
                }

            }
        }

        //读入入完成后删除指针
        delete configIniRead;

        ix.plannerState = PlannerStatus_Done;
        planner_dialog->ui->status1_planner->setText(tr("Done"));
        darkframeflag = false;
        qDebug() << "Plane Done";

    }
}
//-------------------------------------------------------------------------------------

/**
 * @brief ExecuteCFWOrder::run
 */
void ExecuteCFWOrder::run()
{
    //int ret = SendOrder2QHYCCDCFW(camhandle, &ix.dstCfwPos, 1);//set the CFW posiion
    int ret = libqhyccd->SendOrder2QHYCCDCFW(camhandle, &ix.dstCfwPos, 1);
    if(ret != QHYCCD_SUCCESS)
        qCritical() << "SendOrder2QHYCCDCFW failure";
    else
        qDebug() << "SendOrder2QHYCCDCFW success";
}

//-------------------------------------------------------------------------------------------------------------------
//                      温度控制
//-------------------------------------------------------------------------------------------------------------------
double DegreeToR(double degree)
{

#define SQR3(x) ((x)*(x)*(x))
#define SQRT3(x) (exp(log(x)/3))

        if (degree<-50) degree=-50;
        if (degree>50)  degree=50;

        double x,y;
        double R;
        double T;

        double A=0.002679;
        double B=0.000291;
        double C=4.28e-7;

        T=degree+273.15;


        y=(A-1/T)/C;
        x=sqrt( SQR3(B/(3*C))+(y*y)/4);
        R=exp(  SQRT3(x-y/2)-SQRT3(x+y/2));

        return R;
}


double RToDegree(double R)
{
        double 	T;
        double LNR;

        if (R>400) R=400;
        if (R<1) R=1;

        LNR=log(R);
        T=1 / (0.002679+0.000291*LNR + LNR*LNR*LNR*4.28e-7);
        T=T-273.15;
        return T;
}

double DegreeTomV(double degree)
{
        double V;
        double R;

        R=DegreeToR(degree);
        V=33000/(R+10)-1625;

        return V;
}

double mVToDegree(double V)
{
        double R;
        double T;

        R=33/(V/1000+1.625)-10;

        T=RToDegree(R);

        return T;
}

void EZCAP::tempTimer_timeout()
{
    if(!ix.isConnected)
    {
        return;
    }
    Flag_Timer = !Flag_Timer;//计时器标识
    if(favorite_dialog->ui->cBox_TEC->isChecked())
        tempControl_dialog->ui->vSlider_power_tempControl->setMaximum(180);
    else
        tempControl_dialog->ui->vSlider_power_tempControl->setMaximum(255);
    QImage img_tempcontrol;
    CurveX++;
    if(CurveX >= 320-22)
    {   //绘制温度曲线  越界处理
        CurveX = 0;
        posX_tempImg = 0;
        posY_tempImg = 0;
        posX_tempImg_RH = 0;
        posY_tempImg_RH = 0;
        posX_tempImg_Press = 0;
        posY_tempImg_Press = 0;

        img_tempcontrol = QPixmap(":/image/black.bmp").toImage();
        mainWidget->DrawGridBox(&img_tempcontrol);
        tempControl_dialog->ui->label_image_tempControl->setPixmap(QPixmap::fromImage(img_tempcontrol));
    }
    else
    {
        img_tempcontrol = tempControl_dialog->ui->label_image_tempControl->pixmap()->toImage();
    }
    QPainter paint_temp(&img_tempcontrol); //为这个QImage构造一个QPainter
    paint_temp.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
    QPen pen_temp = paint_temp.pen();
    QPainter paint_Humidity(&img_tempcontrol); //为这个QImage构造一个QPainter
    paint_Humidity.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
    QPen pen_Humidity = paint_Humidity.pen();
    QPainter paint_Press(&img_tempcontrol); //20200329
    paint_Press.setCompositionMode(QPainter::CompositionMode_SourceIn);
    QPen pen_Press = paint_Press.pen();

    if(Flag_Timer == 1)
    {
        QApplication::processEvents();//防止长时间导致界面假死
        //取数
        //ix.nowTemp = GetQHYCCDParam(camhandle,CONTROL_CURTEMP);//获取温度
        ix.nowTemp = libqhyccd->GetQHYCCDParam(camhandle,CONTROL_CURTEMP);
        qDebug()<<"ix.nowTemp:="<<ix.nowTemp;
        ix.nowVoltage = DegreeTomV(ix.nowTemp);//计算电压值
        if(ix.canHumidity){
            int ret = libqhyccd->GetQHYCCDHumidity(camhandle, &ix.nowHumidity);
            //int ret = GetQHYCCDHumidity(camhandle, &ix.nowHumidity);
            if(ret == QHYCCD_SUCCESS)
            {
                statusLabel_RH->setText(QString("RH:") + QString::number(ix.nowHumidity, 'f', 1) + "%");
                tempControl_dialog->ui->HumidityStatus_tempControl->setText(QString("RH:") +QString::number(ix.nowHumidity, 'f', 1)+"%");
                tempControl_dialog->ui->vSlider_Humidity_tempCotrol->setValue(ix.nowHumidity);
            }
        }
        //20200318
        if(ix.canPressure){
            int ret = libqhyccd->GetQHYCCDPressure(camhandle, &ix.nowPressure);
            //int ret = GetQHYCCDPressure(camhandle, &ix.nowPressure);
            if(ret == QHYCCD_SUCCESS)
            {
                statusLabel_PRESS->setText( QString::number(ix.nowPressure, 'f', 1) + "mbar");//20200329去掉显示QString("PRESS:") +
                tempControl_dialog->ui->PressStatus_tempControl->setText(QString::number(ix.nowPressure) + "mbar");
                tempControl_dialog->ui->vSlider_Press_tempCotrol->setValue(ix.nowPressure/10.0);
            }
        }
            if(1)//ix.workMode == 3 || ix.workMode == 2)
        {
            char temp[16];
            char info[32];
            char time[32];
            char mode[32];
            sprintf(temp,"%.2f",ix.nowTemp);
            sprintf(info,"FWHM%d,PEAK%d",FocusInfo.FWHM_Result,FocusInfo.DeltaPixel);

            sprintf(time,"          ");
#ifdef WIN32
            sprintf(mode,"USB Camera");
#else
            sprintf(mode,"StandAlone");
#endif

#ifdef Q_OS_MAC
            sprintf(mode,"USB Camera");
#endif
            //int ret = SendFourLine2QHYCCDInterCamOled(camhandle,temp,info,time,mode);
            int ret = libqhyccd->SendFourLine2QHYCCDInterCamOled(camhandle,temp,info,time,mode);
            if(ret != QHYCCD_SUCCESS)
            {
                //qDebug() << "SendFourLine2QHYCCDInterCamOled failed";
            }
        }

        statusLabel_Temp->setText(QString("TEMP:") + QString::number(ix.nowTemp, 'f', 1) + QString::fromUtf8("℃"));//显示温度值
        tempControl_dialog->ui->tempStatus_tempControl->setText(QString::number(ix.nowTemp, 'f', 1) + QString::fromUtf8("℃"));
        //tempControl_dialog->ui->PRESSStatus_tempControl->setText(QString::number(ix.nowPressure) + "mbar");//显示电压值20200329更换为压力值
        tempControl_dialog->ui->PWMStatus_tempControl->setText(QString("Power:") +QString::number(ix.nowPWM*100/255, 'f', 1) + "%");//显示功率
        if(ix.coolerMode == Cooler_Manual)
        {   //手动控温模式，设置当前显示温度
            tempControl_dialog->ui->vSlider_temp_tempCotrol->setValue((int)(ix.nowTemp + 50));
        }
        else if(ix.coolerMode == Cooler_Auto)
        {   //自动控温模式，设置当前功率
            tempControl_dialog->ui->vSlider_power_tempControl->setValue(ix.nowPWM);
        }
        //draw the temp curve
        pen_temp.setColor(QColor(255,0,0));//设置画笔颜色红色
        paint_temp.setPen(pen_temp);//set pen of paint
        paint_temp.drawLine(posX_tempImg, posY_tempImg, CurveX,(127-(ix.nowTemp+50)*127.0/90.0));//draw line 电压值画图：(-ix.nowVoltage+200)/15
        //draw the Humidity curve
        if(ix.canHumidity){
        pen_Humidity.setColor(QColor(0,255,255));//0,0,255设置画笔颜色湖蓝色
        paint_temp.setPen(pen_Humidity);//set pen of paint
        paint_temp.drawLine(posX_tempImg_RH, posY_tempImg_RH, CurveX,(127-ix.nowHumidity*127.0/100.0));//draw line ix.nowHumidity
        }
        //draw the Press curve
       if(ix.canPressure){
        pen_Press.setColor(QColor(0,255,0));//设置画笔颜色绿色
        paint_temp.setPen(pen_Press);//set pen of paint
        paint_temp.drawLine(posX_tempImg_Press, posY_tempImg_Press, CurveX,(127-(ix.nowPressure/10.0)*127.0/200.0));
        }
       //20201125lyl温度湿度压力数据保存
//       QFile file(filetemp);
//       file.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append);
//       QTextStream out(&file);
//       out<<qSetFieldWidth(5)<<left<<ix.nowTemp<<" \t "<<ix.nowHumidity<<" \t "<<ix.nowPressure<<" \t ";//<<"Temp \t Humidity \t Press \t";
//       out<<qSetFieldWidth(1)<<'\n';
//       file.close();

        tempControl_dialog->ui->label_image_tempControl->setPixmap(QPixmap::fromImage(img_tempcontrol));//show image in the label
        posX_tempImg = CurveX;
        posY_tempImg = (127-(ix.nowTemp+50)*127.0/90.0);//(-ix.nowVoltage + 200) / 15;
         if(ix.canHumidity){
        posX_tempImg_RH= CurveX;
        posY_tempImg_RH= (127-ix.nowHumidity*127/100.0);//CurveX;ix.nowHumidity
         }
         if(ix.canPressure){
        posX_tempImg_Press= CurveX;
        posY_tempImg_Press=(127-(ix.nowPressure/10.0)*127.0/200.0);
        }
    }
    else
    {   //控制
        uint32_t ret = QHYCCD_ERROR;
        if(ix.coolerMode == Cooler_Off)
        {
            //停止制冷
            ix.nowPWM = 0;
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_MANULPWM, 0);
            //ret = SetQHYCCDParam(camhandle, CONTROL_MANULPWM, 0);
            if(ret != QHYCCD_SUCCESS)
            {
                qCritical() << "SetQHYCCDParam(camhandle,CONTROL_MANULPWM,0) failure";
            }
        }
        else if(ix.coolerMode == Cooler_Manual)
        {
            //开启手动制冷
            ix.nowPWM = tempControl_dialog->ui->vSlider_power_tempControl->value();
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_MANULPWM,ix.nowPWM);
            //ret = SetQHYCCDParam(camhandle,CONTROL_MANULPWM,ix.nowPWM);
            if(ret != QHYCCD_SUCCESS)
            {
                qCritical() << "SetQHYCCDParam CONTROL_MANULPWM" << ix.nowPWM << " failure";
            }
        }
        else if(ix.coolerMode == Cooler_Auto)
        {
            ix.nowPWM =libqhyccd-> GetQHYCCDParam(camhandle,CONTROL_CURPWM);
            //ix.nowPWM = GetQHYCCDParam(camhandle,CONTROL_CURPWM);
            //开始自动温控制冷

            if(this->TESTED_PID)
            {
                //PID参数调试模式下，设置当前PID参数
                double pVal = tempControl_dialog->ui->doubleSpinBox_P->value();
                double iVal = tempControl_dialog->ui->doubleSpinBox_I->value();
                double dVal = tempControl_dialog->ui->doubleSpinBox_D->value();
                libqhyccd->TestQHYCCDPIDParas(camhandle, pVal, iVal, dVal);
                //TestQHYCCDPIDParas(camhandle, pVal, iVal, dVal);
                //qDebug() << "set pid paras";
            }
            ix.targetTemp = tempControl_dialog->ui->vSlider_temp_tempCotrol->value() - 50;
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_COOLER, ix.targetTemp);
            //ret = SetQHYCCDParam(camhandle, CONTROL_COOLER, ix.targetTemp);
            if(ret != QHYCCD_SUCCESS)
            {
                qCritical() << "ControlQHYCCDTemp failure";
            }
            else
            {
                qDebug() << "SetQHYCCDParam Cooler" << ix.targetTemp;
            }
        }
    }
}

void EZCAP::startTimerTemp()
{
    tempTimer->start();
}
void EZCAP::stopTimerTemp()
{
    tempTimer->stop();
}

//----------------色轮状态查询定时器响应--------------------------
void EZCAP::cfwTimer_timeout()
{
    if(ix.isConnected)
    {
        for(int i=0; i< 64; i++)
        {
            ix.curCfwPos[i] = 0;
        }
        int ret = libqhyccd->GetQHYCCDCFWStatus(camhandle, ix.curCfwPos);
        //int ret = GetQHYCCDCFWStatus(camhandle, ix.curCfwPos);
        if(ret == QHYCCD_SUCCESS)
        {
            if(ix.dstCfwPos == ix.curCfwPos[0])
            {
                ix.CFWStatus = CFW_Idle;//CFW运转到位
                stopCFWTimer();
                qDebug() << "CFW moved Done!";
            }
            else
            {
                qDebug() << QString("CFW is moving...target hole %1, current hole %2").arg(QString(ix.dstCfwPos)).arg(QString(ix.curCfwPos[0]));
            }
        }
        QCoreApplication::processEvents();
    }
}

void EZCAP::startCFWTimer()
{
    cfwTimer->start();
}

void EZCAP::stopCFWTimer()
{
    cfwTimer->stop();
}

//-------------------PHD2 Dither功能 状态查询定时器-----------------
void EZCAP::ditherTimer_timeout()
{
    if(phdLink_dialog->IsDitherEnabled())
    {
        char str[1024];
        int ret;
        QString recStr;
        //ret = CheckPHD2Status(str);
        ret = libqhyccd->CheckPHD2Status(str);
        if(ret == 0)
        {
            recStr = QString(QLatin1String(str));
            if(recStr.compare(QString("SettleDone"), recStr) == 0)
            {
                isSettleDone = true;
            }
            qDebug() <<"PHD2 Status:" << recStr;
        }
    }
    else
    {
        isSettleDone = true;  //如果断开PHD2连接，则退出等待循环
    }

    QApplication::processEvents();

}
void EZCAP::startDitherTimer()
{
    ditherTimer->start();
}
void EZCAP::stopDitherTimer()
{
    ditherTimer->stop();
}
void EZCAP::PumpTimer_timeout()
{
    pumpcount+=1;
    if(pumpcount>=5*60)//5*
    {
        stopPumpTimer();
    }
    QApplication::processEvents();
}
void EZCAP::startPumpTimer()
{
    PumpTimer->start();
    pumpcount=0;
    favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->setEnabled(false);
}
void EZCAP::stopPumpTimer()
{
    PumpTimer->stop();
    libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SensorChamberCycle_PUMP,0);
    favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->setEnabled(true);
    favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->setChecked(false);

}
