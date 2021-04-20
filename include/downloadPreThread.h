#ifndef DOWNLOADPRETHREAD_H
#define DOWNLOADPRETHREAD_H

#include <QThread>

class DownloadPreThread : public QThread
{
    Q_OBJECT
public:
    explicit DownloadPreThread(QObject *parent = 0);

signals:

public slots:

protected:
    void run();

};

#endif // DOWNLOADPRETHREAD_H
