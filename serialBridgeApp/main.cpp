#include "mainwindow.h"
#include "deviceeventfilter.h"

#include <QApplication>
#include <QObject>
#include <QtDebug>
#include <QFile>

int main(int argc, char *argv[])
{
    // The device event filter code used here is from StackOverflow user Benjamin T
    //  https://stackoverflow.com/questions/38528684/detected-new-usb-device-connected-disconnected-on-qt

    qDebug().noquote() << "Creating USB device event filter";
    deviceEventFilter usbEventFilter;

    QApplication a(argc, argv);
    MainWindow w;

    qDebug().noquote() << "Registering event filter with MainWindow";
    usbEventFilter.registerEvent(&w);
    qApp->installNativeEventFilter(&usbEventFilter);

    qDebug().noquote() << "Connecting event filter to MainWindow";
    QObject::connect(&usbEventFilter, &deviceEventFilter::newUSBDevice, &w, &MainWindow::enumerateDevices);
    QObject::connect(&usbEventFilter, &deviceEventFilter::lostUSBDevice, &w, &MainWindow::handleDisconnect);

    w.show();
    int ret = a.exec();

    return ret;
}
