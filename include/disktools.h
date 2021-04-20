#ifndef DISKTOOLS_H
#define DISKTOOLS_H


#include "QDebug"

class DiskTools
{
public:
    DiskTools();

    quint64 getDiskFreeSpace(QString driver);
};

#endif // DISKTOOLS_H
