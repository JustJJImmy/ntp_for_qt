#include "localtimedaemon.h"
#include <QDebug>
#include <QtCore>


#define daemon_interval  1000


LocalTimeDaemonThread::LocalTimeDaemonThread(QObject *parent)
    : QThread (parent)
    , _predictTimestamp(0)
{

}

void LocalTimeDaemonThread::run() {
    _predictTimestamp = 0;

    while (!isInterruptionRequested()) {
        _predictTimestamp += daemon_interval;
        qint64 currentTS = QDateTime::currentMSecsSinceEpoch();
        qint64 offset = currentTS - _predictTimestamp;
        if (offset < -500 || offset > 500) {
            _predictTimestamp = currentTS;
            // 不符合预期，说明时间戳不准了，重新同步
            emit localtimeChanged();
        }

        // sleep 的方式其实不算特别准，因为上面那些代码的执行需要时间。定时器是最准的
        msleep(daemon_interval);
    }
}










