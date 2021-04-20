#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMenuBar>

class MainMenu : public QMenuBar
{
    Q_OBJECT
public:
    explicit MainMenu(QWidget *parent = 0);
    ~MainMenu();

    /*创建菜单action*/
    void createActions();
    /*创建菜单menu*/
    void createMenus();

    /*主菜单menu或action*/
    QMenu *menuFile;

    QAction *actConnect;
    QMenu *menuCamera;

    QMenu *menuPlanner;
    QMenu *menuImageProcess;
    QMenu *menuCameraSetup;
    QMenu *menuTools;
    QMenu *menuZoom;
    QMenu *menuLanguage;
    QMenu *menuHelp;

    /*子菜单menu以及action*/
    QAction *actOpenFolder;
    QAction *actSaveFIT;
    QAction *actSaveBMP;
    QAction *actSaveJPG;
    QAction *actFitHeaderEditor;
    QAction *actIgnoreOverScanArea;
    QAction *actCalibrateOverScan;
    QAction *actExit;
    QAction *actShowPlanTable;
    QAction *actColorCamera;
    QAction *actBatchProcess;
    QAction *actNoiseAnalyse;
    QAction *actFavorite;
    QAction *actTempControl;
    QAction *actCFWControl;
    QAction *actPHDLink;
    QAction *actFitWindow;
    QAction *act0_25X;
    QAction *act0_5X;
    QAction *act0_75X;
    QAction *act1X;
    QAction *act1_5X;
    QAction *act2X;
    QAction *actEnglish;
    QAction *actChinese;
    QAction *actJapanese;
    QAction *actFrance;
    QAction *actSpain;
    QAction *actRussia;
    QAction *actGermany;
    QAction *actAbout;
    QAction *actTestMode;
    QAction *actDebug;
    QAction *actTestGuid;
    QAction *actCaptureDarkFrameTool;

signals:

private slots:
    void resetUI();//recieve the changeLanguage signal

    void camera_connected();
    void camera_disconnected();

protected:


};

extern class MainMenu *mainMenuBar;

#endif // MAINMENU_H
