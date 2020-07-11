// The device event filter code used here is from StackOverflow Benjamin T
//  https://stackoverflow.com/questions/38528684/detected-new-usb-device-connected-disconnected-on-qt

#ifndef DEVICEEVENTFILTER_H
#define DEVICEEVENTFILTER_H

#include <QObject>
#include <QMainWindow>
#include <QAbstractNativeEventFilter>

class deviceEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit deviceEventFilter(QObject *parent = 0);
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

signals:
    void newUSBDevice();
    void lostUSBDevice();

public slots:
    void registerEvent(QMainWindow *window);
};

#endif // DEVICEEVENTFILTER_H
