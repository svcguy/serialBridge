// The device event filter code used here is from StackOverflow Benjamin T
//  https://stackoverflow.com/questions/38528684/detected-new-usb-device-connected-disconnected-on-qt

#include "deviceeventfilter.h"

#include "Windows.h"
#include "Dbt.h"
#include <QtDebug>

deviceEventFilter::deviceEventFilter(QObject *parent) :
    QObject(parent)
{

}

bool deviceEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);
#ifdef Q_OS_WIN32
    MSG *msg = (MSG *)message;
    if( WM_DEVICECHANGE == msg->message && DBT_DEVICEARRIVAL == msg->wParam )
    {
        qDebug().noquote() << tr("USB arrival detected!");
        emit newUSBDevice();
    }
    else if( WM_DEVICECHANGE == msg->message && DBT_DEVICEREMOVECOMPLETE == msg->wParam )
    {
        qDebug().noquote() << tr("USB departure detected!");
        emit lostUSBDevice();
    }
#endif
    // Return false so that the event is propagated
    return false;
}


void deviceEventFilter::registerEvent(QMainWindow *window)
{
    if( window != nullptr )
    {
#ifdef Q_OS_WIN32
        GUID guid; // {88bae032-5a81-49f0-bc3d-a4ff138216d6}
        guid.Data1 = 0x88bae032;
        guid.Data2 = 0x5a81;
        guid.Data3 = 0x49f0;
        guid.Data4[0] = 0xbc;
        guid.Data4[1] = 0x3d;
        guid.Data4[2] = 0xa4;
        guid.Data4[3] = 0xff;
        guid.Data4[4] = 0x13;
        guid.Data4[5] = 0x82;
        guid.Data4[6] = 0x16;
        guid.Data4[7] = 0xd6;

        DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

        ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
        NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        NotificationFilter.dbcc_classguid = guid;
        HDEVNOTIFY hDevNotify = RegisterDeviceNotification((HANDLE)window->winId(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
        if( NULL == hDevNotify )
        {
            // Print error
        }
#endif
    }

}
