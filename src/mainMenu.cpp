#include "mainMenu.h"
#include "ezCap.h"

#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

MainMenu *mainMenuBar;
extern struct IX ix;

MainMenu::MainMenu(QWidget *parent) :
    QMenuBar(parent)
{
    createActions();
    createMenus();

    //依次加入menu或action
    this->addMenu(menuFile);
    this->addMenu(menuCamera);
    this->addMenu(menuPlanner);
    this->addMenu(menuImageProcess);
    this->addMenu(menuCameraSetup);
    this->addMenu(menuTools);
    this->addMenu(menuZoom);
    this->addMenu(menuLanguage);
    this->addMenu(menuHelp);
}

MainMenu::~MainMenu()
{
    if(actOpenFolder)
    {
        delete actOpenFolder;
        actOpenFolder = NULL;
    }
    if(actSaveFIT)
    {
        delete actSaveFIT;
        actSaveFIT = NULL;
    }
    if(actSaveBMP)
    {
        delete actSaveBMP;
        actSaveBMP = NULL;
    }
    if(actSaveJPG)
    {
        delete actSaveJPG;
        actSaveJPG = NULL;
    }
    if(actFitHeaderEditor)
    {
        delete actFitHeaderEditor;
        actFitHeaderEditor = NULL;
    }
    if(actIgnoreOverScanArea)
    {
        delete actIgnoreOverScanArea;
        actIgnoreOverScanArea = NULL;
    }
    if(actCalibrateOverScan)
    {
        delete actCalibrateOverScan;
        actCalibrateOverScan = NULL;
    }
    if(actExit)
    {
        delete actExit;
        actExit = NULL;
    }

    if(actConnect)
    {
        delete actConnect;
        actConnect = NULL;
    }

    if(actShowPlanTable)
    {
        delete actShowPlanTable;
        actShowPlanTable = NULL;
    }

    if(actColorCamera)
    {
        delete actColorCamera;
        actColorCamera = NULL;
    }
    if(actBatchProcess)
    {
        delete actBatchProcess;
        actBatchProcess = NULL;
    }
    if(actNoiseAnalyse)
    {
        delete actNoiseAnalyse;
        actNoiseAnalyse = NULL;
    }

    if(actFavorite)
    {
        delete actFavorite;
        actFavorite = NULL;
    }
    if(actPHDLink)
    {
        delete actPHDLink;
        actPHDLink = NULL;
    }
    if(actTempControl)
    {
        delete actTempControl;
        actTempControl = NULL;
    }


    if(actCaptureDarkFrameTool)
    {
        delete actCaptureDarkFrameTool;
        actCaptureDarkFrameTool = NULL;
    }

    if(actCFWControl)
    {
        delete actCFWControl;
        actCFWControl = NULL;
    }

    if(actFitWindow)
    {
        delete actFitWindow;
        actFitWindow = NULL;
    }    
    if(act0_25X)
    {
        delete act0_25X;
        act0_25X = NULL;
    }
    if(act0_5X)
    {
        delete act0_5X;
        act0_5X = NULL;
    }
    if(act0_75X)
    {
        delete act0_75X;
        act0_75X = NULL;
    }
    if(act1X)
    {
        delete act1X;
        act1X = NULL;
    }
    if(act1_5X)
    {
        delete act1_5X;
        act1_5X = NULL;
    }
    if(act2X)
    {
        delete act2X;
        act2X = NULL;
    }

    if(actEnglish)
    {
        delete actEnglish;
        actEnglish = NULL;
    }
    if(actChinese)
    {
        delete actChinese;
        actChinese = NULL;
    }
    if(actJapanese)
    {
        delete actJapanese;
        actJapanese = NULL;
    }
    if(actFrance)
    {
        delete actFrance;
        actFrance = NULL;
    }
    if(actSpain)
    {
        delete actSpain;
        actSpain = NULL;
    }
    if(actRussia)
    {
        delete actRussia;
        actRussia = NULL;
    }
    if(actGermany)
    {
        delete actGermany;
        actGermany = NULL;
    }

    if(actAbout)
    {
        delete actAbout;
        actAbout = NULL;
    }
    if(actTestMode)
    {
        delete actTestMode;
        actTestMode = NULL;
    }

    if(menuFile)
    {
        delete menuFile;
        menuFile = NULL;
    }

    if(menuCamera)
    {
        delete menuCamera;
        menuCamera = NULL;
    }

    if(menuPlanner)
    {
        delete menuPlanner;
        menuPlanner = NULL;
    }
    if(menuImageProcess)
    {
        delete menuImageProcess;
        menuImageProcess = NULL;
    }
    if(menuCameraSetup)
    {
        delete menuCameraSetup;
        menuCameraSetup = NULL;
    }

    if(menuTools)
    {
        delete menuTools;
        menuTools = NULL;
    }

    if(menuZoom)
    {
        delete menuZoom;
        menuZoom = NULL;
    }
    if(menuLanguage)
    {
        delete menuLanguage;
        menuLanguage = NULL;
    }
    if(menuHelp)
    {
        delete menuHelp;
        menuHelp = NULL;
    }
}

void MainMenu::createActions()
{
    actConnect = new QAction(tr("Connect"),this);
    actConnect->setIconText(tr("Connect"));
    actConnect->setMenuRole(QAction::NoRole);

    actOpenFolder = new QAction(tr("Open Saved Folder"),this);
    actSaveFIT = new QAction(tr("Save FIT"), this);
    actSaveBMP = new QAction(tr("Save BMP"), this);
    actSaveJPG = new QAction(tr("Save JPG"), this);
    actFitHeaderEditor = new QAction(tr("FIT Header Editor"), this);
    actIgnoreOverScanArea = new QAction(tr("Ignore Overscan Area"), this);
    actIgnoreOverScanArea->setCheckable(true);    
    actCalibrateOverScan = new QAction(tr("Overscan Calibration"), this);
    actCalibrateOverScan->setCheckable(true);
    actExit = new QAction(tr("Exit"), this);
    actExit->setMenuRole(QAction::NoRole);

    actShowPlanTable = new QAction(tr("Show Plan Table"), this);

    actColorCamera = new QAction(tr("Color Camera"), this);
    actColorCamera->setCheckable(true);
    actBatchProcess = new QAction(tr("Batch Process"), this);
    actBatchProcess->setEnabled(false);
    actBatchProcess->setCheckable(true);
    actNoiseAnalyse = new QAction(tr("Noise Analyse"), this);
    actNoiseAnalyse->setCheckable(true);

    actFavorite = new QAction(tr("Favorite"), this);
    actPHDLink = new QAction(tr("PHD Link"), this);
    actCaptureDarkFrameTool = new QAction(tr("DarkFrameTool"),this);
    actTempControl = new QAction(tr("Temp Control"), this);
    actCFWControl = new QAction(tr("Filter Wheel Control"), this);

    act1X = new QAction("1X", this);
    act1X->setCheckable(true);
    actFitWindow = new QAction(tr("Fit Window"), this);
    actFitWindow->setCheckable(true);
    act0_75X = new QAction("0.75X", this);
    act0_75X->setCheckable(true);
    act0_5X = new QAction("0.5X", this);
    act0_5X->setCheckable(true);
    act0_25X = new QAction("0.25X", this);
    act0_25X->setCheckable(true);
    act1_5X = new QAction("1.5X", this);
    act1_5X->setCheckable(true);
    act2X = new QAction("2X", this);
    act2X->setCheckable(true);

    actEnglish = new QAction(tr("English"), this);
    actEnglish->setCheckable(true);
    actChinese = new QAction(QString::fromUtf8("简体中文"), this);
    actChinese->setCheckable(true);
    actJapanese = new QAction(QString::fromUtf8("日本語"), this);
    actJapanese->setCheckable(true);
    actFrance = new QAction(tr("France"), this);
    actFrance->setCheckable(true);
    actSpain = new QAction(tr("Spain"), this);
    actSpain->setCheckable(true);
    actRussia = new QAction(tr("Russia"), this);
    actRussia->setCheckable(true);
    actGermany = new QAction(tr("Germany"), this);
    actGermany->setCheckable(true);

    actAbout = new QAction(tr("About"), this);
    actAbout->setMenuRole(QAction::NoRole);
    actTestMode = new QAction(tr("Test Mode"), this);
    actTestMode->setCheckable(true);
    actTestGuid = new QAction(tr("Test Guid"), this);
    actTestGuid->setCheckable(true);
    actDebug = new QAction(tr("Debug"), this);
    actDebug->setCheckable(true);

}

void MainMenu::createMenus()
{
    /*File菜单*/
    menuFile = new QMenu(tr("File"));
    menuFile->addAction(actOpenFolder);
    menuFile->addAction(actSaveFIT);
    menuFile->addAction(actSaveBMP);
    menuFile->addAction(actSaveJPG);
    menuFile->addSeparator();
    menuFile->addAction(actFitHeaderEditor);
    menuFile->addAction(actIgnoreOverScanArea);
    menuFile->addAction(actCalibrateOverScan);
    menuFile->addSeparator();
    menuFile->addAction(actExit);
    //connect menu
    menuCamera = new QMenu(tr("Camera"),this);
    menuCamera->addAction(actConnect);
    actConnect->setShortcut(QKeySequence("Alt+C"));
    /*planner菜单*/
    menuPlanner = new QMenu(tr("Planner"));
    menuPlanner->addAction(actShowPlanTable);
    /*Image process菜单*/
    menuImageProcess = new QMenu(tr("Image Process"));
    menuImageProcess->addAction(actColorCamera);
    menuImageProcess->addAction(actBatchProcess);
    menuImageProcess->addAction(actNoiseAnalyse);
    /*camera setup菜单*/
    menuCameraSetup = new QMenu(tr("Camera Setup"));
    menuCameraSetup->addAction(actFavorite);
    menuCameraSetup->addAction(actPHDLink);
    menuCameraSetup->addAction(actTempControl);
    menuCameraSetup->addAction(actCFWControl);

    /*Tools*/
    menuTools = new QMenu(tr("Tools"));
    menuTools->addAction(actCaptureDarkFrameTool);

    /*zoom菜单*/
    menuZoom = new QMenu(tr("Zoom"));
    menuZoom->addAction(act1X);
    menuZoom->addAction(actFitWindow);
    menuZoom->addAction(act0_75X);
    menuZoom->addAction(act0_5X);
    menuZoom->addAction(act0_25X);
    menuZoom->addAction(act1_5X);
    menuZoom->addAction(act2X);
    /*language菜单*/
    menuLanguage = new QMenu(tr("Language"));
    menuLanguage->addAction(actEnglish);
    menuLanguage->addAction(actChinese);
    menuLanguage->addAction(actJapanese);
    menuLanguage->addAction(actFrance);
    menuLanguage->addAction(actSpain);
    menuLanguage->addAction(actRussia);
    menuLanguage->addAction(actGermany);
    /*help菜单*/
    menuHelp = new QMenu(tr("Help"));
    menuHelp->addAction(actAbout);
    menuHelp->addAction(actTestMode);
    menuHelp->addAction(actTestGuid);
    menuHelp->addAction(actDebug);
    actTestMode->setShortcut(QKeySequence("Alt+T"));

    //设置zoom子菜单项互斥
    QActionGroup *zoomGrp = new QActionGroup(menuZoom);
    zoomGrp->addAction(act0_5X);
    zoomGrp->addAction(act0_25X);
    zoomGrp->addAction(act0_75X);
    zoomGrp->addAction(act1X);
    zoomGrp->addAction(act1_5X);
    zoomGrp->addAction(act2X);
    zoomGrp->addAction(actFitWindow);
    zoomGrp->setExclusive(true);
    act1X->setChecked(true);
    //设置language子菜单项互斥
    QActionGroup *languageGrp = new QActionGroup(menuLanguage);
    languageGrp->addAction(actEnglish);    
    languageGrp->addAction(actChinese);
    languageGrp->addAction(actJapanese);
    languageGrp->addAction(actFrance);
    languageGrp->addAction(actGermany);
    languageGrp->addAction(actRussia);
    languageGrp->addAction(actSpain);
    languageGrp->setExclusive(true);
    actEnglish->setChecked(true);
}

void MainMenu::resetUI()
{
    /*主菜单menu或action*/
    menuFile->setTitle(tr("File"));

    if(ix.isConnected)
    {
        actConnect->setText(tr("Disconnect"));
    }
    else
    {
        actConnect->setText(tr("Connect"));
    }
    menuCamera->setTitle(tr("Camera"));

    menuPlanner->setTitle(tr("Planner"));
    menuImageProcess->setTitle(tr("Image Process"));
    menuCameraSetup->setTitle(tr("Camera Setup"));
    menuTools->setTitle(tr("Tools"));
    menuZoom->setTitle(tr("Zoom"));
    menuLanguage->setTitle(tr("Language"));
    menuHelp->setTitle(tr("Help"));

    /*子菜单menu以及action*/
    actOpenFolder->setText(tr("Open Saved Folder"));
    actSaveFIT->setText(tr("Save FIT"));
    actSaveBMP->setText(tr("Save BMP"));
    actSaveJPG->setText(tr("Save JPG"));
    actFitHeaderEditor->setText(tr("FIT Header Editor"));
    actIgnoreOverScanArea->setText(tr("Ignore Overscan Area"));
    actCalibrateOverScan->setText(tr("Overscan Calibration"));
    actExit->setText(tr("Exit"));

    actShowPlanTable->setText(tr("Show Plan Table"));

    actColorCamera->setText(tr("Color Camera"));
    actBatchProcess->setText(tr("Batch Process"));
    actNoiseAnalyse->setText(tr("Noise Analyse"));

    actFavorite->setText(tr("Favorite"));
    actPHDLink->setText(tr("PHD Link"));
    actTempControl->setText(tr("Temp Control"));
    //actColorWheelSetting->setText(tr("Color Wheel Setting"));
    actCFWControl->setText(tr("Filter Wheel Control"));

    actCaptureDarkFrameTool->setText(tr("DarkFrameTool"));

    actFitWindow->setText(tr("Fit Window"));
    actAbout->setText(tr("About"));
}

void MainMenu::camera_connected()
{
    this->actCFWControl->setVisible(ix.canFilterWheel);
    this->actTempControl->setVisible(ix.canCooler);

    this->menuPlanner->setEnabled(true);
    if(ix.canFilterWheel)
        this->actCFWControl->setEnabled(true);
    this->actCaptureDarkFrameTool->setEnabled(true);
    this->actFavorite->setEnabled(true);

    //修改actionConnect控件为Disconnect
    this->actConnect->setText(tr("Disconnect"));
    actConnect->setShortcut(QKeySequence("Alt+D"));

    //保存图像功能是否可用
    this->actSaveFIT->setEnabled(false);
    this->actSaveBMP->setEnabled(false);
    this->actSaveJPG->setEnabled(false);
}

void MainMenu::camera_disconnected()
{
    this->menuPlanner->setEnabled(false);
    this->menuZoom->setEnabled(false);
    this->menuImageProcess->setEnabled(false);
    this->actConnect->setText(tr("Connect"));
    actConnect->setShortcut(QKeySequence("Alt+C"));
    this->actCFWControl->setEnabled(false);
    this->actFavorite->setEnabled(false);
    this->actOpenFolder->setEnabled(false);
    this->actCalibrateOverScan->setEnabled(false);
    this->actIgnoreOverScanArea->setEnabled(false);
    this->actCaptureDarkFrameTool->setEnabled(false);
}
