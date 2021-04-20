#include <QVector>

#ifndef MYSTRUCT_H
#define MYSTRUCT_H

//define camera status
#define Camera_Idle      0
#define Camera_Waiting   1
#define Camera_Exposing  2
#define Camera_Reading   3
#define Camera_Download  4
#define Camera_Error     5

//get frame status
#define GetSingleFrame_Failed   2
#define GetSingleFrame_Success  1
#define GetSingleFrame_Waiting  0

//define camera work mode
#define WorkMode_Preview   1
#define WorkMode_Focus     2
#define WorkMode_Capture   3

//define auto stretch mode
#define StretchMode_NoiseFloor          0
#define StretchMode_BackGroundLevel     1
#define StretchMode_3timesBackGround    2
#define StretchMode_10timesBackGround   3
#define StretchMode_MaxRange            4
#define StretchMode_OverScanX256        5
#define StretchMode_OverScanX128        6
#define StretchMode_OverScanX64         7
#define StretchMode_OverScanX32         8
#define StretchMode_OverScanX16         9
#define StretchMode_OverScanX8          10

//define CFW status
#define CFW_Moving   1
#define CFW_Idle     0

//define cross/grid/circle status
#define Cross_Enabled    1
#define Cross_Disabled   0
#define Grid_Enabled     1
#define Grid_Disabled    0
#define Circle_Enabled   1
#define Circle_Disabled  0

//define FitHeader status
#define FitHeader_Add   0
#define FitHeader_Set   1

//define Planner status
#define PlannerStatus_Start  1
#define PlannerStatus_Stop   0
#define PlannerStatus_Done   2

//define Zoom mode
#define Zoom_Auto            0
#define Zoom_SpecifyScaling  1

//define cooler mode
#define Cooler_Off           0
#define Cooler_Manual        1
#define Cooler_Auto          2

//define CFW waiting time  (ms)
#define CFW_WAITING_TIME  28000

//前置声明
class QString;

struct IX
{
    //---CCD info
    QString CamID;
    QString CamModel;
    //camera maximum iamge size
    unsigned int ccdMaxImgW;
    unsigned int ccdMaxImgH;
    //ccd chip size
    double ccdChipW;
    double ccdChipH;
    //ccd pixel size
    double ccdPixelW;
    double ccdPixelH;

    //current image size bpp, channels
    unsigned int imageX;
    unsigned int imageY;
    unsigned int imgBpp;
    unsigned int imgChannels;

    unsigned int lastImageX;
    unsigned int lastImageY;
    unsigned char *lastImgData;
    unsigned char *ImgData;//存放从相机获取到的16位图像数据
    unsigned char *OutputData8;//存放转换后的8位图像数据

    unsigned char *dispIplImgData24;//存放//opencv 8bit 3channels图像数据
    unsigned char *dispIplImgData8; //opencv 8bit 1channels图像数据空间

    //effective area (just used for Capture mode)
    unsigned short onlyStartX;
    unsigned short onlyStartY;
    unsigned short onlySizeX;
    unsigned short onlySizeY;
    //overscan area (just used for Capture mode)
    unsigned short OverScanStartX;
    unsigned short OverScanStartY;
    unsigned short OverScanSizeX;
    unsigned short OverScanSizeY;
    unsigned short calConstant;

    //binning
    unsigned short binx;
    unsigned short lastbinx;
    unsigned short biny;
    unsigned short lastbiny;
    unsigned short maxBinx;
    unsigned short maxBiny;
    bool canbin11;
    bool canbin22;
    bool canbin33;
    bool canbin44;

    //exposure setting   [ms]
    double Exptime;
    double LastExptime;
    //gain setting
    bool canGain;
    int gainMax;
    int gainMin;
    int gainStep;
    int gain;
    int lastGain;
    //offset setting
    bool canOffset;
    int offsetMax;
    int offsetMin;
    int offsetStep;
    int offset;
    int lastOffset;
    //speed setting
    bool canHighSpeed;
    int DownloadSpeed;
    int LastDownloadSpeed;
    //bits mode
    bool can16Bits;
    unsigned int bits;
    unsigned int lastBits;
    //usbtraffic
    bool canUsbtraffic;
    int usbtrafficMax;
    int usbtrafficMin;
    int usbtrafficStep;
    int usbtraffic;
    int lastUsbtraffic;

    //shutter
    bool canMechanicalShutter;
    int MechanicalShutterMode;
    int LastMechanicalShutterMode;    
    //color
    bool canColor;
    int bayermatrix;//彩色相机bayer矩阵格式
    bool isCvtColor;

    //cooler setting
    bool canCooler;
    int coolerMode; //the cooler control mode, 0 disalble; 1:manual; 2:auto control
    double nowTemp; //当前监控到的温度
    double targetTemp;
    int targetVoltage;
    int nowVoltage;//当前电压
    double nowPWM;//当前功率
    //20200318
    bool canHumidity;
    double nowHumidity;//当前湿度
    bool canPressure;
    double nowPressure ;//当前压力

    //FineTone
    bool canFineTone;//是否支持FineTone
    //MotorHeating
    bool canMotorHeating;//是否支持MotorHeating

    //CFW
    bool canFilterWheel;//是否支持ColorWheel
    char dstCfwPos;//色轮目标位置
    char curCfwPos[64];// 色轮当前位置
    int CFWStatus;// 0:闲置/运行完成， 1：正在运行    
    int CFWSlotsNum;
    QVector<QString> filterNames_2;

    //Triger
    bool canTriger; //外触发支持
    bool trigerInOrOut;
    //TECProtect
    bool canTecOverProtect;
    bool tecPretect;
    //CLAMP
    bool canSignalClamp;
    bool clamp;
    //Calibrate FPN
    bool canCalibrateFPN;
    bool isCalibrateFrame;//是否正在进行帧校准
    //Slowest Download
    bool canSlowestDownload;
    bool slowestDowload;
    //chip temperature
    bool canChipTemp;
    //20200512lyl GPSon
    bool canGPS;
    bool isGPSon;
    //Fireware version
    QString driverVer;
    //20200303lyl FPGA version
    QString FPGAVer;
    QString FPGAVer1;


    //-----------------------
    bool isConnected;  //camea is connected or not
    int cameraState;  //camera status ,one of the following status : 0 CameraIdle; 1 CameraWaiting; 2 CameraExposing; 3 CameraReading; 4 CameraDownload ; 5 CameraError

    int workMode;    //camer workmode     1：preview，2：focus，3：capture
    int lastWorkMode;

    int imageReady;  // 1 - get frame success; 2 - get frame failed; 0 - wait for downlaod frame

    int crossBtnState; //cross status, 0：hide cross, 1: show cross
    int gridBtnState; //grid status, 0：hide grid, 1: show grid
    int circleBtnState; //circle status, 0：hide circle, 1: show circle

    int fitHeadEditState;//fit头编辑状态 0: add; 1:set

    int plannerState; //planner status, one of the following status： 2 Done/Idle;  1 Start; 0 Stop

    bool ForceStop;//标记强制停止曝光
    bool onLiveMode; //a flag indicating whether EZCAP is working on PreviewLive/FocusLive mode

    //bool isOverScanCalibrated;
    bool ignoreOverScan;
    bool calibrateOverScan;

    unsigned long Histogram[256];//用于直方图像素值对照表
    unsigned char LUT_1[256][3];//伪彩色转换对照表
    unsigned char StretchLUT[65536];

    int autoStretchMode;  //auto stretch mode:
    int StretchStep;//灰度拉伸调节幅度

    QString lang;   //current language

    int zoomMode;  //缩放模式， 0-自动缩放； 1-指定缩放比例

    QString dateOBS;
    //20200220 lyl Add ReadMode Dialog
    unsigned int currentReadMode;   //current readmode
    unsigned int NumberOfReadModes; //number of readmodes
    char currentReadModeName[50];   //QString current ReadMode Name
    QStringList ReadModeList;       //保存readmode名称列表
    //20200512 lyl OSD
    QStringList OSDList;       //保存OSD名称列表
    //20201127 lyl SensorChamberCyclePUMP
    bool canContolSensorChamberCyclePUMP;
};

struct FOCUSINFO
{
    unsigned char MaxPixel;
    unsigned char MinPixel;
    unsigned char DeltaPixel;
    unsigned char CenterX;
    unsigned char CenterY;

    unsigned char FWHM_X1;
    unsigned char FWHM_X2;
    unsigned char FWHM_Y1;
    unsigned char FWHM_Y2;
    unsigned char FWHM_ResultX;
    unsigned char FWHM_ResultY;
    unsigned char FWHM_Result;
    unsigned char width;
    unsigned char height;
    unsigned char Row[1000];   // x1 x2
    unsigned char Column[1000]; // y1 y2
};

struct INIFILEPARAM
{
    bool iniFileExist;

    QString lang;
    int gain;
    int offset;
    bool tecPretect;
    bool slowestDowload;
    bool clamp;
    unsigned short bPos_Preview;
    unsigned short wPos_Preview;
    unsigned short bPos_Focus;
    unsigned short wPos_Focus;
    unsigned short bPos_Capture;
    unsigned short wPos_Capture;
    int autoStretchMode;
    bool ignoreOverScan;
    bool calibrateOverScan;
    unsigned short calConstant;
    int CFWSlotsNum;
    QVector<QString> filterNames_2;

    bool testGuider;
    bool autoConnect;
    bool oldSDK;
};

extern struct IX ix;
extern struct FOCUSINFO FocusInfo;
extern struct INIFILEPARAM iniFileParams;

#endif // MYSTRUCT_H
