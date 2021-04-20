#include "disktools.h"

#ifdef WIN32
#include <windows.h>
#endif

DiskTools::DiskTools()
{
}

quint64 DiskTools::getDiskFreeSpace(QString driver)
{
#ifdef WIN32
    LPCWSTR lpcwstrDriver = (LPCWSTR)driver.utf16();

    ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;

    if( !GetDiskFreeSpaceEx( lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes) )
    {
        qDebug() << "ERROR: Call to GetDiskFreeSpaceEx() failed.";
        return 0;
    }
    return (quint64) liTotalFreeBytes.QuadPart/1024/1024;
#endif
}
