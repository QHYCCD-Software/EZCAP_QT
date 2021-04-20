#include "downloadPreThread.h"
#include "include/dllqhyccd.h"
//#include "qhyccdStatus.h"
#include "myStruct.h"

#include <QDebug>
#include <QTime>

extern qhyccd_handle *camhandle;

DownloadPreThread::DownloadPreThread(QObject *parent) :
    QThread(parent)
{
}

void DownloadPreThread::run()
{
    unsigned int ret = QHYCCD_ERROR;

    QTime t0_prev = QTime::currentTime();
    t0_prev.start();

    //get one frame data
    ret = libqhyccd->GetQHYCCDSingleFrame(camhandle,&ix.imageX,&ix.imageY,&ix.imgBpp,&ix.imgChannels,ix.ImgData);
    //ret = GetQHYCCDSingleFrame(camhandle,&ix.imageX,&ix.imageY,&ix.imgBpp,&ix.imgChannels,ix.ImgData);
    if(ret != QHYCCD_SUCCESS)
    {
        qCritical() << "GetQHYCCDSingleFrame: failed";
        ix.imageReady = GetSingleFrame_Failed;
    }
    else
    {
        qDebug() << "GetQHYCCDSingleFrame: success, Time elapsed:" << t0_prev.elapsed();

        if(ix.imgBpp == 8)
        {
            qDebug("GetQHYCCDSingleFrame the readout image depth is 8bit");
            unsigned char *temparray = (unsigned char *)malloc(ix.imageX * ix.imageY * 2);

            int i = 0,j = 1;
            for(;i < (int)(ix.imageX * ix.imageY);i++)
            {
                temparray[j] = ix.ImgData[i];
                j += 2;
            }

            memcpy(ix.ImgData,temparray,ix.imageX * ix.imageY * 2);
            delete(temparray);
        }

        memset(ix.lastImgData, 0, ix.ccdMaxImgW * ix.ccdMaxImgH * 2);
        memcpy(ix.lastImgData, ix.ImgData, ix.imageX * ix.imageY * 2);
        ix.lastImageX = ix.imageX;
        ix.lastImageY = ix.imageY;

        ix.imageReady = GetSingleFrame_Success;

    }

}
