#ifndef DOWNLOADCAPTHREAD_H
#define DOWNLOADCAPTHREAD_H

#include <QThread>

class DownloadCapThread : public QThread
{
    Q_OBJECT
public:
    explicit DownloadCapThread(QObject *parent = 0);

signals:

public slots:

protected:
    void run();
};

#endif // DOWNLOADCAPTHREAD_H
