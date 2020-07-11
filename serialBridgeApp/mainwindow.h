#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "bridge.h"
#include "stlink_interface.h"
#ifdef USING_ERRORLOG
#include "ErrLog.h"
#endif //USING_ERRORLOG

#include "bridgegpiowidget.h"
#include "bridgei2cwidget.h"

#include <QMainWindow>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();  

public slots:
    void handleDisconnect(void);
    void writeMessageToStatusBar(const QString& msg);
    void enumerateDevices(void);
    void connectDevice(void);
    void setBase(void);

signals:
    void deviceDisconnect();
    void baseChanged(int base);

private slots:
    void aboutDialog(void);

private:
    Ui::MainWindow *ui;

#ifdef USING_ERRORLOG
    cErrLog errorLog;
#endif // USING_ERRORLOG

    Brg *bridge;
    STLinkInterface *interface;

    bridgeGPIOWidget *gpioWidget;
    bridgeI2CWidget *i2cWidget;

    bool deviceConnected = false;
    QString deviceConnectedSN;
};
#endif // MAINWINDOW_H
