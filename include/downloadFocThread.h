#ifndef DOWNLOADFOCTHREAD_H
#define DOWNLOADFOCTHREAD_H

#include <QThread>

class DownloadFocThread : public QThread
{
    Q_OBJECT
public:
    explicit DownloadFocThread(QObject *parent = 0);

signals:

public slots:

protected:
    void run();
};

#endif // DOWNLOADFOCTHREAD_H
