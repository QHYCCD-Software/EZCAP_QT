#include "managementMenu.h"
#include "ui_managementMenu.h"
#include "ezCap.h"

//#include "qhyccdStatus.h"
#include "include/dllqhyccd.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QDebug>

ManagementMenu *managerMenu;
extern struct IX ix;
extern qhyccd_handle *camhandle;

ManagementMenu::ManagementMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManagementMenu)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    //-------------------preview layout---------------------------
    ui->head_preview->setFixedHeight(18);
    ui->hSlider_gain_preview->setFixedHeight(19);
    ui->hSlider_offset_preview->setFixedHeight(19);
    ui->hSlider_exposure_preview->setFixedHeight(19);
    ui->pBtn_cross->setFixedHeight(17);
    ui->pBtn_grid->setFixedHeight(17);
    ui->pBtn_circle->setFixedHeight(17);
    ui->proBar_preview->setFixedHeight(16);
    ui->proBar_previewTime->setFixedHeight(16);
    ui->pBtn_preview->setFixedHeight(24);
    ui->pBtn_live_preview->setFixedHeight(24);

    ui->head_preview->setFont(QFont("Tahoma", 14, QFont::Bold));
    ui->label_gain_preview->setFont(QFont("Tahoma", 10));
    ui->label_offset_preview->setFont(QFont("Tahoma", 10));
    ui->label_exposure_preview->setFont(QFont("Tahoma", 10));
    ui->pBtn_circle->setFont(QFont("Tahoma", 12));
    ui->pBtn_cross->setFont(QFont("Tahoma", 12));
    ui->pBtn_grid->setFont(QFont("Tahoma", 12));
    ui->pBtn_preview->setFont(QFont("Tahoma", 14, QFont::Bold));
    ui->pBtn_live_preview->setFont(QFont("Tahoma", 14, QFont::Bold));

    QGridLayout *widgetPreviewLayout = new QGridLayout;
    widgetPreviewLayout->setContentsMargins(3,0,3,2);//设置四周间隔
    widgetPreviewLayout->setVerticalSpacing(6);//设置行间隔
    widgetPreviewLayout->addWidget(ui->label_gain_preview,0,0,1,10);
    widgetPreviewLayout->addWidget(ui->label_offset_preview,0,10,1,10);
    widgetPreviewLayout->addWidget(ui->hSlider_gain_preview, 1,0,2,10);
    widgetPreviewLayout->addWidget(ui->hSlider_offset_preview,1,10,2,10);
    widgetPreviewLayout->addWidget(ui->label_exposure_preview,3,0,1,20);
    widgetPreviewLayout->addWidget(ui->hSlider_exposure_preview,4,0,2,20);
    widgetPreviewLayout->addWidget(ui->pBtn_cross,6,0,2,7);
    widgetPreviewLayout->addWidget(ui->pBtn_grid,6,7,2,6);
    widgetPreviewLayout->addWidget(ui->pBtn_circle,6,13,2,7);
    widgetPreviewLayout->addWidget(ui->proBar_previewTime,8,0,2,20);
    widgetPreviewLayout->addWidget(ui->proBar_preview,10,0,2,20);
    widgetPreviewLayout->addWidget(ui->pBtn_preview,12,0,2,10);
    widgetPreviewLayout->addWidget(ui->pBtn_live_preview,12,10,2,10);
    ui->widget_preview->setLayout(widgetPreviewLayout);

    QGridLayout *grpBoxPreviewLayout = new QGridLayout;
    grpBoxPreviewLayout->setMargin(0);
    grpBoxPreviewLayout->addWidget(ui->head_preview, 0, 0, 1, 20);
    grpBoxPreviewLayout->addWidget(ui->widget_preview, 1, 0, 1, 20);
    ui->grpBox_preview->setLayout(grpBoxPreviewLayout);
    ui->grpBox_preview->setFixedWidth(205);

    //----------------focus layout---------------------------------
    ui->head_focus->setFixedHeight(18);
    ui->hSlider_gain_focus->setFixedHeight(19);
    ui->hSlider_offset_focus->setFixedHeight(19);
    ui->hSlider_exposure_focus->setFixedHeight(19);
    ui->pBtn_focus->setFixedHeight(24);
    ui->pBtn_live_focus->setFixedHeight(24);

    ui->head_focus->setFont(QFont("Tahoma", 14, QFont::Bold));
    ui->label_gain_focus->setFont(QFont("Tahoma", 10));
    ui->label_offset_focus->setFont(QFont("Tahoma", 10));
    ui->label_exposure_focus->setFont(QFont("Tahoma", 10));
    ui->pBtn_focus->setFont(QFont("Tahoma", 14, QFont::Bold));
    ui->pBtn_live_focus->setFont(QFont("Tahoma", 14, QFont::Bold));

    QGridLayout *widgetFocusLayout = new QGridLayout;
    widgetFocusLayout->setContentsMargins(3,0,3,2);
    widgetFocusLayout->setVerticalSpacing(6);//设置行间隔
    widgetFocusLayout->addWidget(ui->label_gain_focus,0,0,1,10);
    widgetFocusLayout->addWidget(ui->label_offset_focus,0,10,1,10);
    widgetFocusLayout->addWidget(ui->hSlider_gain_focus,1,0,2,10);
    widgetFocusLayout->addWidget(ui->hSlider_offset_focus,1,10,2,10);
    widgetFocusLayout->addWidget(ui->label_exposure_focus,3,0,1,20);
    widgetFocusLayout->addWidget(ui->hSlider_exposure_focus,4,0,2,20);
    widgetFocusLayout->addWidget(ui->pBtn_focus,6,0,2,10);
    widgetFocusLayout->addWidget(ui->pBtn_live_focus,6,10,2,10);
    ui->widget_focus->setLayout(widgetFocusLayout);

    QGridLayout *grpBoxFocusLayout = new QGridLayout;
    grpBoxFocusLayout->setMargin(0);
    grpBoxFocusLayout->addWidget(ui->head_focus, 0, 0, 1, 20);
    grpBoxFocusLayout->addWidget(ui->widget_focus, 1, 0, 1, 20);
    ui->grpBox_focus->setLayout(grpBoxFocusLayout);
    ui->grpBox_focus->setFixedWidth(205);

    //-----------------capture layout------------------------------
    ui->head_capture->setFixedHeight(18);
    ui->hSlider_gain_capture->setFixedHeight(19);
    ui->hSlider_offset_capture->setFixedHeight(19);
    ui->hSlider_exposure_capture->setFixedHeight(19);
    ui->pBtn_expUnit_capture->setFixedHeight(16);
    ui->proBar_capture->setFixedHeight(16);
    ui->proBar_captureTime->setFixedHeight(16);
    ui->bin1x1->setFixedHeight(16);
    ui->bin2x2->setFixedHeight(16);
    ui->bin3x3->setFixedHeight(16);
    ui->bin4x4->setFixedHeight(16);
    ui->checkBox_highSpeed->setFixedHeight(16);
    ui->pBtn_capture->setFixedHeight(24);
    ui->pBtn_stop->setFixedHeight(24);

    ui->head_capture->setFont(QFont("Tahoma", 14, QFont::Bold));
    ui->label_gain_capture->setFont(QFont("Tahoma", 10));
    ui->label_offset_capture->setFont(QFont("Tahoma", 10));
    ui->label_exposure_capture->setFont(QFont("Tahoma", 10));
    ui->pBtn_expUnit_capture->setFont(QFont("Tahoma", 10));
    ui->pBtn_capture->setFont(QFont("Tahoma", 14, QFont::Bold));
    ui->pBtn_stop->setFont(QFont("Tahoma", 14, QFont::Bold));

    QGridLayout *widgetCapLayout = new QGridLayout;
    widgetCapLayout->setContentsMargins(3,0,3,0);
    widgetCapLayout->setVerticalSpacing(6);//设置行间隔
    widgetCapLayout->addWidget(ui->label_gain_capture,0,0,1,10);
    widgetCapLayout->addWidget(ui->label_offset_capture,0,10,1,10);
    widgetCapLayout->addWidget(ui->hSlider_gain_capture, 1,0,2,10);
    widgetCapLayout->addWidget(ui->hSlider_offset_capture,1,10,2,10);
    widgetCapLayout->addWidget(ui->label_exposure_capture,3,0,1,10);
    widgetCapLayout->addWidget(ui->pBtn_expUnit_capture,3,10,1,6);
    widgetCapLayout->addWidget(ui->hSlider_exposure_capture,4,0,2,20);
    widgetCapLayout->addWidget(ui->proBar_captureTime,6,0,2,20);
    widgetCapLayout->addWidget(ui->proBar_capture,8,0,2,20);
    widgetCapLayout->addWidget(ui->bin1x1,10,0,2,10);
    widgetCapLayout->addWidget(ui->bin2x2,10,10,2,10);
    widgetCapLayout->addWidget(ui->bin3x3,12,0,2,10);
    widgetCapLayout->addWidget(ui->bin4x4,12,10,2,10);
    widgetCapLayout->addWidget(ui->checkBox_highSpeed,14,0,2,20);
    widgetCapLayout->addWidget(ui->pBtn_capture,16,0,2,10);
    widgetCapLayout->addWidget(ui->pBtn_stop,16,10,2,10);
    ui->widget_capture->setLayout(widgetCapLayout);

    QGridLayout *grpBoxCaptureLayout = new QGridLayout;
    grpBoxCaptureLayout->setMargin(0);
    grpBoxCaptureLayout->addWidget(ui->head_capture, 0, 0, 1, 20);
    grpBoxCaptureLayout->addWidget(ui->widget_capture, 1, 0, 1, 20);
    ui->grpBox_capture->setLayout(grpBoxCaptureLayout);
    ui->grpBox_capture->setFixedWidth(205);

    //-----------------Histogram layout--------------------------------
    ui->head_hist->setFixedHeight(18);
    ui->img_hist->setFixedSize(192, 130);
    ui->pBtn_stretchMinusB->setFixedHeight(16);
    ui->pBtn_stretchPlusB->setFixedHeight(16);
    ui->pBtn_coarse->setFixedHeight(16);
    ui->pBtn_stretchMinusW->setFixedHeight(16);
    ui->pBtn_stretchPlusW->setFixedHeight(16);
    ui->pBtn_auto_histogram->setFixedHeight(22);
    ui->cBox_autoStretchList->setFixedHeight(22);

    ui->head_hist->setFont(QFont("Tahoma", 14, QFont::Bold));
    ui->pBtn_auto_histogram->setFont(QFont("Tahoma", 12));
    ui->cBox_autoStretchList->setFont(QFont("Tahoma", 12));
    ui->pBtn_stretchMinusB->setFont(QFont("Tahoma", 12));
    ui->pBtn_stretchMinusW->setFont(QFont("Tahoma", 12));
    ui->pBtn_stretchPlusB->setFont(QFont("Tahoma", 12));
    ui->pBtn_stretchPlusW->setFont(QFont("Tahoma", 12));
    ui->label_B->setFont(QFont("Tahoma", 10));
    ui->label_W->setFont(QFont("Tahoma", 10));
    ui->pBtn_coarse->setFont(QFont("Tahoma", 12));

    QHBoxLayout *histBtnLayout = new QHBoxLayout;
    histBtnLayout->setMargin(0);
    histBtnLayout->setSpacing(11);
    histBtnLayout->addWidget(ui->pBtn_stretchMinusB);
    histBtnLayout->addWidget(ui->pBtn_stretchPlusB);
    histBtnLayout->addWidget(ui->pBtn_coarse);
    histBtnLayout->addWidget(ui->pBtn_stretchMinusW);
    histBtnLayout->addWidget(ui->pBtn_stretchPlusW);

    QVBoxLayout *hSliderWBLayout = new QVBoxLayout;
    hSliderWBLayout->setMargin(0);
    hSliderWBLayout->setSpacing(0);
    hSliderWBLayout->addWidget(ui->hSlider_wPos);
    hSliderWBLayout->addWidget(ui->hSlider_bPos);

    QHBoxLayout *autoBtnsLayout = new QHBoxLayout;
    autoBtnsLayout->setMargin(0);
    autoBtnsLayout->setSpacing(20);
    autoBtnsLayout->addWidget(ui->pBtn_auto_histogram);
    autoBtnsLayout->addWidget(ui->cBox_autoStretchList);

    QVBoxLayout *widgetHistLayout = new QVBoxLayout;
    widgetHistLayout->setSpacing(0);
    widgetHistLayout->setMargin(0);
    widgetHistLayout->addWidget(ui->img_hist);
    widgetHistLayout->addLayout(histBtnLayout);
    widgetHistLayout->addLayout(hSliderWBLayout);
    widgetHistLayout->addLayout(autoBtnsLayout);
    ui->widget_hist->setLayout(widgetHistLayout);

    QGridLayout *grpBoxHistLayout = new QGridLayout;
    grpBoxHistLayout->setMargin(0);
    grpBoxHistLayout->setSpacing(0);
    grpBoxHistLayout->addWidget(ui->head_hist, 0, 0, 1, 20);
    grpBoxHistLayout->addWidget(ui->widget_hist, 1, 0, 1, 20);
    ui->grpBox_hist->setLayout(grpBoxHistLayout);
    ui->grpBox_hist->setFixedWidth(205);

    //------------screenView layout--------------------------------------
    ui->head_screenView->setFixedHeight(18);
    ui->img_screenView->setFixedSize(192,130);

    ui->head_screenView->setFont(QFont("Tahoma", 14));

    QVBoxLayout *widgetScreenLayout = new QVBoxLayout;
    widgetScreenLayout->setContentsMargins(0,2,0,2);//left:0,top:2,right:0,bottom:2
    widgetScreenLayout->setSpacing(0);
    widgetScreenLayout->setSizeConstraint(QLayout::SetFixedSize);
    widgetScreenLayout->addWidget(ui->img_screenView);
    ui->widget_screenView->setLayout(widgetScreenLayout);

    QGridLayout *grpBoxScreenViewLayout = new QGridLayout;
    grpBoxScreenViewLayout->setMargin(0);
    grpBoxScreenViewLayout->setSpacing(0);
    grpBoxScreenViewLayout->addWidget(ui->head_screenView,0,0,1,2);
    grpBoxScreenViewLayout->addWidget(ui->widget_screenView,1,0,1,2);
    ui->grpBox_screenView->setLayout(grpBoxScreenViewLayout);
    ui->grpBox_screenView->setFixedWidth(205);
#else
    //-------------------preview layout---------------------------
    //ui->head_preview->setFixedHeight(15);
    ui->hSlider_gain_preview->setFixedSize(70,25);
    ui->hSlider_offset_preview->setFixedSize(70,25);
    ui->hSlider_exposure_preview->setFixedSize(169,25);
    ui->pBtn_cross->setFixedSize(50,17);
    ui->pBtn_grid->setFixedSize(50,17);
    ui->pBtn_circle->setFixedSize(50,17);
    ui->proBar_preview->setFixedSize(168,12);
    ui->proBar_previewTime->setFixedSize(168,12);
    ui->pBtn_preview->setFixedSize(70,24);
    ui->pBtn_live_preview->setFixedSize(70,24);

    QGridLayout *widgetPreviewLayout = new QGridLayout;
    widgetPreviewLayout->setContentsMargins(10,1,0,2);//设置四周间隔

    widgetPreviewLayout->setHorizontalSpacing(0);//设置列间隔为0
    widgetPreviewLayout->setVerticalSpacing(2);//设置行间隔
    widgetPreviewLayout->addWidget(ui->label_gain_preview,0,0,1,10);
    widgetPreviewLayout->addWidget(ui->label_offset_preview,0,10,1,10);
    widgetPreviewLayout->addWidget(ui->hSlider_gain_preview, 1,0,2,10);
    widgetPreviewLayout->addWidget(ui->hSlider_offset_preview,1,10,2,10);
    widgetPreviewLayout->addWidget(ui->label_exposure_preview,3,0,1,20);
    widgetPreviewLayout->addWidget(ui->hSlider_exposure_preview,4,0,2,20);
    widgetPreviewLayout->addWidget(ui->pBtn_cross,6,0,1,5);
    widgetPreviewLayout->addWidget(ui->pBtn_grid,6,6,1,5);
    widgetPreviewLayout->addWidget(ui->pBtn_circle,6,12,1,6);
    widgetPreviewLayout->addWidget(ui->proBar_previewTime,7,0,2,20);
    widgetPreviewLayout->addWidget(ui->proBar_preview,9,0,2,20);
    widgetPreviewLayout->addWidget(ui->pBtn_preview,11,0,2,10);
    widgetPreviewLayout->addWidget(ui->pBtn_live_preview,11,10,2,10);
    ui->widget_preview->setLayout(widgetPreviewLayout);

    QGridLayout *grpBoxPreviewLayout = new QGridLayout;
    grpBoxPreviewLayout->setMargin(0);
    grpBoxPreviewLayout->setSpacing(0);//设置间隔为0
    grpBoxPreviewLayout->addWidget(ui->head_preview, 0, 0, 1, 20);
    grpBoxPreviewLayout->addWidget(ui->widget_preview, 1, 0, 1, 20);
    ui->grpBox_preview->setLayout(grpBoxPreviewLayout);
    ui->grpBox_preview->setFixedWidth(201);

    //----------------focus layout---------------------------------
    //ui->head_focus->setFixedHeight(15);
    ui->hSlider_gain_focus->setFixedSize(70,25);
    ui->hSlider_offset_focus->setFixedSize(70,25);
    ui->hSlider_exposure_focus->setFixedSize(165,25);
    ui->pBtn_focus->setFixedSize(70,24);
    ui->pBtn_live_focus->setFixedSize(70,24);

    QGridLayout *widgetFocusLayout = new QGridLayout;
    widgetFocusLayout->setContentsMargins(10,1,0,2);
    widgetFocusLayout->setHorizontalSpacing(0);
    widgetFocusLayout->setVerticalSpacing(3);
    widgetFocusLayout->addWidget(ui->label_gain_focus,0,0,1,10);
    widgetFocusLayout->addWidget(ui->label_offset_focus,0,10,1,10);
    widgetFocusLayout->addWidget(ui->hSlider_gain_focus,1,0,2,10);
    widgetFocusLayout->addWidget(ui->hSlider_offset_focus,1,10,2,10);
    widgetFocusLayout->addWidget(ui->label_exposure_focus,3,0,1,20);
    widgetFocusLayout->addWidget(ui->hSlider_exposure_focus,4,0,2,20);
    widgetFocusLayout->addWidget(ui->pBtn_focus,6,0,2,10);
    widgetFocusLayout->addWidget(ui->pBtn_live_focus,6,10,2,10);
    ui->widget_focus->setLayout(widgetFocusLayout);

    QGridLayout *grpBoxFocusLayout = new QGridLayout;
    grpBoxFocusLayout->setMargin(0);
    grpBoxFocusLayout->setSpacing(0);
    grpBoxFocusLayout->addWidget(ui->head_focus, 0, 0, 1, 20);
    grpBoxFocusLayout->addWidget(ui->widget_focus, 1, 0, 1, 20);
    ui->grpBox_focus->setLayout(grpBoxFocusLayout);
    ui->grpBox_focus->setFixedWidth(201);

    //-----------------capture layout------------------------------
    //ui->head_capture->setFixedHeight(25);
    ui->hSlider_gain_capture->setFixedSize(70,25);
    ui->hSlider_offset_capture->setFixedSize(70,25);
    ui->hSlider_exposure_capture->setFixedSize(164,25);
    ui->pBtn_expUnit_capture->setFixedSize(40,16);
    ui->proBar_capture->setFixedSize(164,12);
    ui->proBar_captureTime->setFixedSize(164,12);
    ui->bin1x1->setFixedSize(89,15);
    ui->bin2x2->setFixedSize(89,15);
    ui->bin3x3->setFixedSize(89,15);
    ui->bin4x4->setFixedSize(89,15);
    ui->checkBox_highSpeed->setFixedSize(171,16);
    ui->pBtn_capture->setFixedSize(70,24);
    ui->pBtn_stop->setFixedSize(70,24);

    QGridLayout *widgetCapLayout = new QGridLayout;
    widgetCapLayout->setContentsMargins(10,1,0,2);
    widgetCapLayout->setHorizontalSpacing(0);
    widgetCapLayout->setVerticalSpacing(2);
    widgetCapLayout->addWidget(ui->label_gain_capture,0,0,1,10);
    widgetCapLayout->addWidget(ui->label_offset_capture,0,10,1,10);
    widgetCapLayout->addWidget(ui->hSlider_gain_capture, 1,0,2,10);
    widgetCapLayout->addWidget(ui->hSlider_offset_capture,1,10,2,10);
    widgetCapLayout->addWidget(ui->label_exposure_capture,3,0,1,13);
    widgetCapLayout->addWidget(ui->pBtn_expUnit_capture,3,13,1,6);
    widgetCapLayout->addWidget(ui->hSlider_exposure_capture,4,0,2,20);
    widgetCapLayout->addWidget(ui->proBar_captureTime,6,0,2,20);
    widgetCapLayout->addWidget(ui->proBar_capture,8,0,2,20);
    widgetCapLayout->addWidget(ui->bin1x1,10,0,2,10);
    widgetCapLayout->addWidget(ui->bin2x2,10,10,2,10);
    widgetCapLayout->addWidget(ui->bin3x3,12,0,2,10);
    widgetCapLayout->addWidget(ui->bin4x4,12,10,2,10);
    widgetCapLayout->addWidget(ui->checkBox_highSpeed,14,3,2,17);
    widgetCapLayout->addWidget(ui->pBtn_capture,16,0,2,10);
    widgetCapLayout->addWidget(ui->pBtn_stop,16,10,2,10);
    ui->widget_capture->setLayout(widgetCapLayout);

    QGridLayout *grpBoxCaptureLayout = new QGridLayout;
    grpBoxCaptureLayout->setMargin(0);
    grpBoxCaptureLayout->setSpacing(0);
    grpBoxCaptureLayout->addWidget(ui->head_capture, 0, 0, 1, 20);
    grpBoxCaptureLayout->addWidget(ui->widget_capture, 1, 0, 1, 20);
    ui->grpBox_capture->setLayout(grpBoxCaptureLayout);
    ui->grpBox_capture->setFixedWidth(201);

    //-----------------Histogram layout--------------------------------
    //ui->head_hist->setFixedHeight(15);
    ui->img_hist->setFixedSize(192, 130);
    ui->pBtn_stretchMinusB->setFixedHeight(16);
    ui->pBtn_stretchPlusB->setFixedHeight(16);
    ui->pBtn_coarse->setFixedSize(38,16);
    ui->pBtn_stretchMinusW->setFixedHeight(16);
    ui->pBtn_stretchPlusW->setFixedHeight(16);
    ui->pBtn_auto_histogram->setFixedSize(45,22);
    ui->cBox_autoStretchList->setFixedSize(131,22);
    ui->hSlider_bPos->setFixedHeight(24);
    ui->hSlider_wPos->setFixedHeight(24);

    QHBoxLayout *histBtnLayout = new QHBoxLayout;
    histBtnLayout->setMargin(0);
    histBtnLayout->setSpacing(0);
    histBtnLayout->setSizeConstraint(QLayout::SetFixedSize);
    histBtnLayout->addWidget(ui->pBtn_stretchMinusB);
    histBtnLayout->addWidget(ui->pBtn_stretchPlusB);
    histBtnLayout->addWidget(ui->pBtn_coarse);
    histBtnLayout->addWidget(ui->pBtn_stretchMinusW);
    histBtnLayout->addWidget(ui->pBtn_stretchPlusW);

    QVBoxLayout *hSliderWBLayout = new QVBoxLayout;
    hSliderWBLayout->setMargin(0);
    hSliderWBLayout->setSpacing(0);
    hSliderWBLayout->addWidget(ui->hSlider_wPos);
    hSliderWBLayout->addWidget(ui->hSlider_bPos);

    QHBoxLayout *autoBtnsLayout = new QHBoxLayout;
    autoBtnsLayout->setMargin(0);
    autoBtnsLayout->setSpacing(10);
    autoBtnsLayout->addWidget(ui->pBtn_auto_histogram);
    autoBtnsLayout->addWidget(ui->cBox_autoStretchList);

    QVBoxLayout *widgetHistLayout = new QVBoxLayout;
    widgetHistLayout->setSpacing(0);
    widgetHistLayout->setContentsMargins(4, 0, 4, 4);
    widgetHistLayout->addWidget(ui->img_hist);
    widgetHistLayout->addLayout(histBtnLayout);
    widgetHistLayout->addLayout(hSliderWBLayout);
    widgetHistLayout->addLayout(autoBtnsLayout);
    ui->widget_hist->setLayout(widgetHistLayout);

    QGridLayout *grpBoxHistLayout = new QGridLayout;
    grpBoxHistLayout->setMargin(0);
    grpBoxHistLayout->setSpacing(0);
    grpBoxHistLayout->addWidget(ui->head_hist, 0, 0, 1, 20);
    grpBoxHistLayout->addWidget(ui->widget_hist, 1, 0, 1, 20);
    ui->grpBox_hist->setLayout(grpBoxHistLayout);
    ui->grpBox_hist->setFixedWidth(201);

    //------------screenView layout--------------------------------------
    //ui->head_screenView->setFixedHeight(15);
    ui->img_screenView->setFixedSize(192,130);

    QVBoxLayout *widgetScreenLayout = new QVBoxLayout;
    widgetScreenLayout->setContentsMargins(3, 1, 3, 2);//left:3,top:3,right:3,bottom:3
    widgetScreenLayout->setSpacing(0);
    widgetScreenLayout->addWidget(ui->img_screenView);
    ui->widget_screenView->setLayout(widgetScreenLayout);

    QGridLayout *grpBoxScreenViewLayout = new QGridLayout;
    grpBoxScreenViewLayout->setMargin(0);
    grpBoxScreenViewLayout->setSpacing(0);
    grpBoxScreenViewLayout->addWidget(ui->head_screenView,0,0,1,2);
    grpBoxScreenViewLayout->addWidget(ui->widget_screenView,1,0,1,2);
    ui->grpBox_screenView->setLayout(grpBoxScreenViewLayout);
    ui->grpBox_screenView->setFixedWidth(201);
#endif

    //---------------------整体布局----------------------------------------
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setContentsMargins(5,0,5,0);
    mainLayout->setSpacing(0);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(ui->grpBox_preview, 0, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_focus, 1, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_capture, 2, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_hist, 3, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_screenView, 4, 0, 1, 2);
    setLayout(mainLayout);



    //----------------------init members and controls----------------------
    cmenu_captureExp = NULL;

    ui->cBox_autoStretchList->clear();
    ui->cBox_autoStretchList->addItem(QString("Noise Floor"));
    ui->cBox_autoStretchList->addItem(QString("BackGround Level"));
    ui->cBox_autoStretchList->addItem(QString("3times BackGround"));
    ui->cBox_autoStretchList->addItem(QString("10times BackGround"));
    ui->cBox_autoStretchList->addItem(QString("Max Range"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X256"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X128"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X64"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X32"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X16"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X8"));


    //-----------------初始设置管理菜单中子项不显示----------------------
    ui->head_preview->setChecked(false);
    ui->widget_preview->setVisible(false);
    ui->head_focus->setChecked(false);
    ui->widget_focus->setVisible(false);
    ui->head_capture->setChecked(false);
    ui->widget_capture->setVisible(false);

#ifdef Q_OS_MAC
    ui->head_preview->setFixedHeight(27);
    ui->head_capture->setFixedHeight(27);
    ui->head_focus->setFixedHeight(27);
#endif

    ui->cBox_autoStretchList->setView(new QListView());  //结合style.qss中设置item高度

}

ManagementMenu::~ManagementMenu()
{
    delete ui;
}

void ManagementMenu::camera_connected()
{
    //offset init
    ui->label_offset_preview->setVisible(ix.canOffset);
    ui->label_offset_focus->setVisible(ix.canOffset);
    ui->label_offset_capture->setVisible(ix.canOffset);
    ui->hSlider_offset_preview->setVisible(ix.canOffset);
    ui->hSlider_offset_focus->setVisible(ix.canOffset);
    ui->hSlider_offset_capture->setVisible(ix.canOffset);
    if(ix.canOffset)
    {
        ui->hSlider_offset_preview->setMaximum(ix.offsetMax);
        ui->hSlider_offset_focus->setMaximum(ix.offsetMax);
        ui->hSlider_offset_capture->setMaximum(ix.offsetMax);
        ui->hSlider_offset_preview->setMinimum(ix.offsetMin);
        ui->hSlider_offset_focus->setMinimum(ix.offsetMin);
        ui->hSlider_offset_capture->setMinimum(ix.offsetMin);
        ui->hSlider_offset_preview->setSingleStep(ix.offsetStep);
        ui->hSlider_offset_focus->setSingleStep(ix.offsetStep);
        ui->hSlider_offset_capture->setSingleStep(ix.offsetStep);
    }

    //gain init
    ui->label_gain_preview->setVisible(ix.canGain);
    ui->label_gain_focus->setVisible(ix.canGain);
    ui->label_gain_capture->setVisible(ix.canGain);
    ui->hSlider_gain_preview->setVisible(ix.canGain);
    ui->hSlider_gain_focus->setVisible(ix.canGain);
    ui->hSlider_gain_capture->setVisible(ix.canGain);
    if(ix.canGain)
    {
        ui->hSlider_gain_preview->setMaximum(ix.gainMax);
        ui->hSlider_gain_focus->setMaximum(ix.gainMax);
        ui->hSlider_gain_capture->setMaximum(ix.gainMax);
        ui->hSlider_gain_preview->setMinimum(ix.gainMin);
        ui->hSlider_gain_focus->setMinimum(ix.gainMin);
        ui->hSlider_gain_capture->setMinimum(ix.gainMin);
        ui->hSlider_gain_preview->setSingleStep(ix.gainStep);
        ui->hSlider_gain_focus->setSingleStep(ix.gainStep);
        ui->hSlider_gain_capture->setSingleStep(ix.gainStep);
    }

    //binning init
    ui->bin1x1->setVisible(ix.canbin11);
    ui->bin2x2->setVisible(ix.canbin22);
    ui->bin3x3->setVisible(ix.canbin33);
    ui->bin4x4->setVisible(ix.canbin44);

    //high speed
    ui->checkBox_highSpeed->setVisible(ix.canHighSpeed);


    //-----------init controls status, after checked params from camera.
    /// NOTE:
    /// when managerMenu change to Preview mode, The Gain, Offset, Exp controls will be setted using ix.gain, ix.offset, ix.exp
    ///
    ui->head_preview->setCheckable(true);
    ui->head_focus->setCheckable(true);
    ui->head_capture->setCheckable(true);
    //goto Preview mode in default.
    ui->head_preview->click();

/*
    int preGain = ui->hSlider_gain_preview->value();
    ui->label_gain_preview->setText(tr("Gain:") + QString::number(preGain));
    int preOffset = ui->hSlider_offset_preview->value();
    ui->label_offset_preview->setText(tr("Offset:") + QString::number(preOffset));

    int focGain = ui->hSlider_gain_focus->value();
    ui->label_gain_focus->setText(tr("Gain:") + QString::number(focGain));
    int focOffset = ui->hSlider_offset_focus->value();
    ui->label_offset_focus->setText(tr("Offset:") + QString::number(focOffset));

    int capGain = ui->hSlider_gain_capture->value();
    ui->label_gain_capture->setText(tr("Gain:") + QString::number(capGain));
    int capOffset = ui->hSlider_offset_capture->value();
    ui->label_offset_capture->setText(tr("Offset:") + QString::number(capOffset));
    */
}

void ManagementMenu::camera_disconnected()
{
    if(ui->head_preview->isChecked())
    {
        ui->head_preview->click();
    }
    if(ui->head_focus->isChecked())
    {
        ui->head_focus->click();
    }
    if(ui->head_capture->isChecked())
    {
        ui->head_capture->click();
    }

}

//---------------------------------------------------------------------------------------

//---------------------preview groupBox--------------------------------------------------
void ManagementMenu::on_head_preview_clicked(bool checked)
{
    ui->head_preview->setChecked(checked);
    ui->widget_preview->setVisible(checked);
    if(checked)
    {
#ifdef Q_OS_MAC
        ui->head_preview->setFixedHeight(18);
        ui->head_capture->setFixedHeight(27);
        ui->head_focus->setFixedHeight(27);
#endif

        ui->head_focus->setChecked(false);
        ui->widget_focus->setVisible(false);
        ui->head_capture->setChecked(false);
        ui->widget_capture->setVisible(false);

        if(ix.workMode != WorkMode_Preview)
        {
            ix.workMode = WorkMode_Preview;

            emit switchWorkMode(ix.workMode);//emit signal to change the layout
        }
    }
    else
    {
#ifdef Q_OS_MAC
        ui->head_preview->setFixedHeight(27);
#endif
    }
}
void ManagementMenu::on_hSlider_gain_preview_valueChanged(int value)
{
    ui->label_gain_preview->setText(tr("Gain:") + QString::number(value));

    ix.gain = value;
    iniFileParams.gain = ix.gain;
    if(ix.lastGain != ix.gain)
    {
        unsigned int ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN,ix.gain);
        //ret = SetQHYCCDParam(camhandle,CONTROL_GAIN,ix.gain);
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
}

void ManagementMenu::on_hSlider_offset_preview_valueChanged(int value)
{
    ui->label_offset_preview->setText(tr("Offset:") + QString::number(value));

    ix.offset = value;
    iniFileParams.offset = ix.offset;
    if(ix.lastOffset != ix.offset)
    {
        unsigned int ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_OFFSET,ix.offset);
        //ret = SetQHYCCDParam(camhandle,CONTROL_OFFSET,ix.offset);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.lastOffset = ix.offset;
            qDebug() << "SetQHYCCDParam CONTROL_OFFSET" << ix.offset;
        }
        else
        {
            qCritical() << "SetQHYCCDParam CONTROL_OFFSET failure";
        }
    }
}

void ManagementMenu::on_hSlider_exposure_preview_valueChanged(int value)
{
    ui->label_exposure_preview->setText(tr("Exposure:") + QString::number(value) + tr("ms"));
    ix.Exptime = value;

}
//---------------------------------------------------------------------------------------

//---------------------focus groupBox----------------------------------------------------
void ManagementMenu::on_head_focus_clicked(bool checked)
{
    ui->head_focus->setChecked(checked);
    ui->widget_focus->setVisible(checked);
    if(checked)
    {
#ifdef Q_OS_MAC
        ui->head_focus->setFixedHeight(18);
        ui->head_capture->setFixedHeight(27);
        ui->head_preview->setFixedHeight(27);
#endif

        ui->head_preview->setChecked(false);
        ui->widget_preview->setVisible(false);
        ui->head_capture->setChecked(false);
        ui->widget_capture->setVisible(false);

        if(ix.workMode != WorkMode_Focus)
        {
            ix.workMode = WorkMode_Focus;

            emit switchWorkMode(ix.workMode);//emit signal to change the layout
        }
    }
    else
    {
#ifdef Q_OS_MAC
        ui->head_focus->setFixedHeight(27);
#endif
    }
}

void ManagementMenu::on_hSlider_gain_focus_valueChanged(int value)
{
    ui->label_gain_focus->setText(tr("Gain:") + QString::number(value));

    ix.gain = value;
    iniFileParams.gain = ix.gain;
    if(ix.lastGain != ix.gain)
    {
        unsigned int ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.gain);
        //ret = SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.gain);
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
}

void ManagementMenu::on_hSlider_offset_focus_valueChanged(int value)
{
    ui->label_offset_focus->setText(tr("Offset:") + QString::number(value));

    ix.offset = value;
    iniFileParams.offset = ix.offset;
    if(ix.lastOffset != ix.offset)
    {
        unsigned int ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_OFFSET,ix.offset);
        //ret = SetQHYCCDParam(camhandle,CONTROL_OFFSET,ix.offset);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.lastOffset = ix.offset;
            qDebug() << "SetQHYCCDParam CONTROL_OFFSET" << ix.offset;
        }
        else
        {
            qCritical() << "SetQHYCCDParam CONTROL_OFFSET failure";
        }
    }
}

void ManagementMenu::on_hSlider_exposure_focus_valueChanged(int value)
{
    ui->label_exposure_focus->setText(tr("Exposure:") + QString::number(value) + tr("ms"));
    ix.Exptime = value;

}
//---------------------------------------------------------------------------------------

//---------------------capture groupBox----------------------------------------------------
void ManagementMenu::on_head_capture_clicked(bool checked)
{
    ui->head_capture->setChecked(checked);
    ui->widget_capture->setVisible(checked);
    if(checked)
    {
#ifdef Q_OS_MAC
        ui->head_capture->setFixedHeight(18);
        ui->head_focus->setFixedHeight(27);
        ui->head_preview->setFixedHeight(27);
#endif

        ui->head_preview->setChecked(false);
        ui->widget_preview->setVisible(false);
        ui->head_focus->setChecked(false);
        ui->widget_focus->setVisible(false);

        if(ix.workMode != WorkMode_Capture)
        {
            ix.workMode = WorkMode_Capture;

            emit switchWorkMode(ix.workMode);//emit signal to change the layout
        }
    }
    else
    {
#ifdef Q_OS_MAC
        ui->head_capture->setFixedHeight(27);
#endif
    }
}

void ManagementMenu::on_hSlider_gain_capture_valueChanged(int value)
{
    ui->label_gain_capture->setText(tr("Gain:") + QString::number(value));

    ix.gain = value;
    iniFileParams.gain = ix.gain;
    if(ix.lastGain != ix.gain)
    {
        unsigned int ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN,ix.gain);
        //ret = SetQHYCCDParam(camhandle,CONTROL_GAIN,ix.gain);
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
}

void ManagementMenu::on_hSlider_offset_capture_valueChanged(int value)
{
    ui->label_offset_capture->setText(tr("Offset:") + QString::number(value));

    ix.offset = value;
    iniFileParams.offset = ix.offset;
    if(ix.lastOffset != ix.offset)
    {
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
}

void ManagementMenu::on_hSlider_exposure_capture_valueChanged(int value)
{
    if(ui->pBtn_expUnit_capture->text() == "ms")
    {
        ui->label_exposure_capture->setText(QString::number(value) + "ms");
        ix.Exptime = value;
    }
    else if(ui->pBtn_expUnit_capture->text() == "s")
    {
        ui->label_exposure_capture->setText(QString::number(value) + "s");
        ix.Exptime = value * 1000;
    }
}

void ManagementMenu::on_pBtn_expUnit_capture_clicked()
{
    if(ui->pBtn_expUnit_capture->text() == "ms")
    {
        ui->pBtn_expUnit_capture->setText("s");
    }
    else
    {
        ui->pBtn_expUnit_capture->setText("ms");
    }
    //同步修改显示信息
    this->on_hSlider_exposure_capture_valueChanged(ui->hSlider_exposure_capture->value());
}

void ManagementMenu::on_bin1x1_toggled(bool checked)
{
    if(checked)
    {
        ix.binx = 1;
        ix.biny = 1;
    }
}

void ManagementMenu::on_bin2x2_toggled(bool checked)
{
    if(checked)
    {
        ix.binx = 2;
        ix.biny = 2;
    }
}

void ManagementMenu::on_bin3x3_toggled(bool checked)
{
    if(checked)
    {
        ix.binx = 3;
        ix.biny = 3;
    }
}

void ManagementMenu::on_bin4x4_toggled(bool checked)
{
    if(checked)
    {
        ix.binx = 4;
        ix.biny = 4;
    }
}

void ManagementMenu::on_checkBox_highSpeed_toggled(bool checked)
{
    if(checked)
    {
        ix.DownloadSpeed = 1;
    }
    else
    {
        ix.DownloadSpeed = 0;
    }
}

void ManagementMenu::setCaptureExp1s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(1);
    ui->label_exposure_capture->setText("1s");
    ix.Exptime = 1000;
}

void ManagementMenu::setCaptureExp5s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(5);
    ui->label_exposure_capture->setText("5s");
    ix.Exptime = 5000;
}

void ManagementMenu::setCaptureExp10s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(10);
    ui->label_exposure_capture->setText("10s");
    ix.Exptime = 10000;
}

void ManagementMenu::setCaptureExp30s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(30);
    ui->label_exposure_capture->setText("30s");
    ix.Exptime = 30000;
}

void ManagementMenu::setCaptureExp60s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(60);
    ui->label_exposure_capture->setText("60s");
    ix.Exptime = 60000;
}

void ManagementMenu::setCaptureExp120s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(120);
    ui->label_exposure_capture->setText("120s");
    ix.Exptime = 120000;
}

void ManagementMenu::setCaptureExp180s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(180);
    ui->label_exposure_capture->setText("180s");
    ix.Exptime = 180000;
}

void ManagementMenu::setCaptureExp240s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(240);
    ui->label_exposure_capture->setText("240s");
    ix.Exptime = 240000;
}

void ManagementMenu::setCaptureExp5min()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(300);
    ui->label_exposure_capture->setText("300s");
    ix.Exptime = 300000;
}

void ManagementMenu::setCaptureExp10min()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(600);
    ui->label_exposure_capture->setText("600s");
    ix.Exptime = 600000;
}

void ManagementMenu::setCaptureExp15min()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(900);
    ui->label_exposure_capture->setText("900s");
    ix.Exptime = 900000;
}

void ManagementMenu::setCaptureExp30min()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(1800);
    ui->label_exposure_capture->setText("1800s");
    ix.Exptime = 1800000;
}

void ManagementMenu::setCaptureExp0s()
{
    ui->pBtn_expUnit_capture->setText("s");
    ui->hSlider_exposure_capture->setValue(0);
    ui->label_exposure_capture->setText("0s");
    ix.Exptime = 0;
}

void ManagementMenu::on_hSlider_exposure_capture_customContextMenuRequested(const QPoint &pos)
{
    if(cmenu_captureExp)//保证同时只存在一个menu，及时释放内存
    {
        delete cmenu_captureExp;
        cmenu_captureExp = NULL;
    }
    cmenu_captureExp = new QMenu(ui->hSlider_exposure_capture);

    QAction *exp1s = cmenu_captureExp->addAction("1s");
    QAction *exp5s = cmenu_captureExp->addAction("5s");
    QAction *exp10s = cmenu_captureExp->addAction("10s");
    QAction *exp30s = cmenu_captureExp->addAction("30s");
    QAction *exp60s = cmenu_captureExp->addAction("60s");
    QAction *exp120s = cmenu_captureExp->addAction("120s");
    QAction *exp180s = cmenu_captureExp->addAction("180s");
    QAction *exp240s = cmenu_captureExp->addAction("240s");
    QAction *exp5min = cmenu_captureExp->addAction("5min");
    QAction *exp10min = cmenu_captureExp->addAction("10min");
    QAction *exp15min = cmenu_captureExp->addAction("15min");
    QAction *exp30min = cmenu_captureExp->addAction("30min");
    QAction *exp0s = cmenu_captureExp->addAction("0s");

    connect(exp1s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp1s()));
    connect(exp5s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp5s()));
    connect(exp10s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp10s()));
    connect(exp30s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp30s()));
    connect(exp60s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp60s()));
    connect(exp120s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp120s()));
    connect(exp180s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp180s()));
    connect(exp240s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp240s()));
    connect(exp5min, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp5min()));
    connect(exp10min, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp10min()));
    connect(exp15min, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp15min()));
    connect(exp30min, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp30min()));
    connect(exp0s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp0s()));

    cmenu_captureExp->exec(QCursor::pos());//在当前鼠标位置显示

}
//-----------------------------------------------------------------------------------------

//---------------------histogram groupBox----------------------------------------------------
void ManagementMenu::on_head_hist_clicked(bool checked)
{
    ui->head_hist->setChecked(checked);
    ui->widget_hist->setVisible(checked);
    if(checked)
    {
#ifdef Q_OS_MAC
        ui->head_hist->setFixedHeight(18);
#endif
    }
    else
    {
#ifdef Q_OS_MAC
        ui->head_hist->setFixedHeight(27);
#endif
    }
}

void ManagementMenu::on_hSlider_wPos_valueChanged(int value)
{
    int dstPos = value * ui->img_hist->width() / 65536 + ui->img_hist->pos().x();
#ifdef Q_OS_MAC
    ui->lineW->move(value*192/65536+4-10+6,ui->lineW->y());
    ui->label_W->move(value*192/65536-5,ui->label_W->y());
#else
    ui->lineW->move(dstPos, ui->lineW->y());
    ui->label_W->move((dstPos - ui->label_W->width() / 2 + 1), ui->label_W->y());
#endif

    if(ui->hSlider_wPos->value() < ui->hSlider_bPos->value())
    {
        ui->hSlider_bPos->setValue(ui->hSlider_wPos->value() - 1);
    }
}

void ManagementMenu::on_hSlider_bPos_valueChanged(int value)
{
    int dstPos = value * ui->img_hist->width() / 65536 + ui->img_hist->pos().x();
#ifdef Q_OS_MAC
    ui->lineB->move(value*192/65536+4-10+6,ui->lineB->y());
    ui->label_B->move(value*192/65536-5,ui->label_B->y());
#else
    ui->lineB->move(dstPos, ui->lineB->y());
    ui->label_B->move((dstPos - ui->label_B->width()/2 + 1), ui->label_B->y());

#endif
    if(ui->hSlider_bPos->value() > ui->hSlider_wPos->value())
    {
        ui->hSlider_wPos->setValue(ui->hSlider_bPos->value() + 1);
    }
}

void ManagementMenu::on_pBtn_coarse_clicked()
{
    if(ui->pBtn_coarse->isChecked())
    {
        ix.StretchStep = 16;
        ui->pBtn_coarse->setText(tr("Fine"));
        ui->pBtn_stretchMinusB->setText("<");
        ui->pBtn_stretchPlusB->setText(">");
        ui->pBtn_stretchMinusW->setText("<");
        ui->pBtn_stretchPlusW->setText(">");
    }
    else
    {
        ix.StretchStep = 256;
        ui->pBtn_coarse->setText(tr("Coarse"));
        ui->pBtn_stretchMinusB->setText("<<");
        ui->pBtn_stretchPlusB->setText(">>");
        ui->pBtn_stretchMinusW->setText("<<");
        ui->pBtn_stretchPlusW->setText(">>");
    }
}

void ManagementMenu::on_cBox_autoStretchList_currentIndexChanged(int index)
{
    ix.autoStretchMode = index;
    iniFileParams.autoStretchMode = ix.autoStretchMode;

    ui->pBtn_auto_histogram->click();
}
//-----------------------------------------------------------------------------------------

//---------------------screenView groupBox--------------------------------------------------

void ManagementMenu::on_head_screenView_clicked(bool checked)
{
    ui->head_screenView->setChecked(checked);
    ui->widget_screenView->setVisible(checked);
    if(checked)
    {
#ifdef Q_OS_MAC
        ui->head_screenView->setFixedHeight(18);
#endif
    }
    else
    {
#ifdef Q_OS_MAC
        ui->head_screenView->setFixedHeight(27);
#endif
    }
}
//-----------------------------------------------------------------------------------------

void ManagementMenu::resetUI()
{
    ui->retranslateUi(this);
}

