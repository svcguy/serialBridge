#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QMessageBox>

// Include Windows.h to get STLinkUSBDriver.dll file version
#ifdef WIN32
#include "Windows.h"
#endif //WIN32

#define APP_NAME "Serial Bridge App"
#define APP_VER "0.1"
#define LOG_FILE_NAME "./app_log.txt"

static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

void logFileMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    switch (type) {
        case QtDebugMsg:
            txt = QString("Debug: %1").arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
            break;
        case QtInfoMsg:
            txt = QString("Info: %1").arg(msg);
            break;
    }

    QFile outFile(LOG_FILE_NAME);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << Qt::endl;

    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug().noquote() << tr("Creating app error log");
    QFile outFile(LOG_FILE_NAME);
    if(!outFile.open(QIODevice::WriteOnly))
    {
        qWarning().noquote() << tr("Could not open app error log for writing");
    }
    outFile.close();
    qInstallMessageHandler(logFileMessageHandler);

#ifdef USING_ERRORLOG
    qDebug().noquote() << tr("Creating bridge dll error log");
    errorLog.Init("./dll_error_log.txt", true);
#endif // USING_ERRORLOG

    deviceConnectedSN = tr("<none>");
    ui->setupUi(this);

    // MainWindow Setup
    this->setWindowTitle(tr(APP_NAME));
    ui->centralwidget->setLayout(ui->mainLayout);
    ui->protocolSelect->setEnabled(deviceConnected);
    ui->actionHexadecimal_16->setChecked(true);
    connect(ui->deviceSelectButton, SIGNAL(clicked()), this, SLOT(connectDevice()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(aboutDialog()));
    connect(ui->actionBinary_2, SIGNAL(triggered()), this, SLOT(setBase()));
    connect(ui->actionOctal_8, SIGNAL(triggered()), this, SLOT(setBase()));
    connect(ui->actionDecimal_10, SIGNAL(triggered()), this, SLOT(setBase()));
    connect(ui->actionHexadecimal_16, SIGNAL(triggered()), this, SLOT(setBase()));

    // Load the STLinkUSBDriver
    qDebug().noquote() << tr("Creating STLinkInterface");
    interface = new STLinkInterface;

#ifdef USING_ERRORLOG
    qDebug().noquote() << tr("Binding STLinkInterface error log");
    interface->BindErrLog(&errorLog);
#endif // USING_ERRORLOG

    qDebug().noquote() << tr("Loading STLink Library");
    STLinkIf_StatusT ret = interface->LoadStlinkLibrary("");
    if(ret != STLINKIF_NO_ERR)
    {
        QString error;
        if(ret == STLINKIF_DLL_ERR)
        {
            error = tr("STLinkUSBDriver library not loaded");
        }
        else if(ret == STLINKIF_NOT_SUPPORTED)
        {
            error = tr("Only BRIDGE interface supported currently");
        }
        else
        {
            error = tr("Undefined error loading STLinkUSBDriver, code: %1").arg(ret);
        }

        qCritical().noquote() << error;

        // No DLL means no go, pop an error message to the user and quit
        int ret = QMessageBox::critical(this, tr(APP_NAME), error, QMessageBox::Abort);
        if(ret == QMessageBox::Abort)
        {
            qDebug().noquote() << tr("Aborting, no dll loaded");
            // This is the only way to get a clean exit from the constructor
            //  has positve side effect of never showing the main window
            //  also has positive side effect of calling the destructor and
            //  cleaning up nicely
            QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        }
    }

    QString message;
    message = tr("STLink USB Library Loaded");
    ui->statusbar->showMessage(message, 5000);
    qDebug().noquote() << message;

    // Create bridge object
    qDebug().noquote() << tr("Creating bridge object");
    bridge = new Brg(*interface);

#ifdef USING_ERRORLOG
    qDebug().noquote() << tr("Binding bridge error log");
    bridge->BindErrLog(&errorLog);
#endif // USING_ERRORLOG

    // Create the bridgeWidgets.  Done here because they each need a reference to the
    //  bridge object
    //  Don't specify a parent for widgets to be set to the tab widget
    qDebug().noquote() << tr("%1: Creating protocol bridge widgets").arg(this->objectName());
    gpioWidget = new bridgeGPIOWidget(bridge);
    i2cWidget = new bridgeI2CWidget(bridge);

    connect(this, SIGNAL(deviceDisconnect()), gpioWidget, SLOT(handleDisconnect()));
    connect(this, SIGNAL(deviceDisconnect()), i2cWidget, SLOT(handleDisconnect()));

    connect(gpioWidget, SIGNAL(postMessage(QString)), ui->statusbar, SLOT(showMessage(QString)));
    connect(i2cWidget, SIGNAL(postMessage(QString)), ui->statusbar, SLOT(showMessage(QString)));

    connect(this, SIGNAL(baseChanged(int)), i2cWidget, SLOT(setBase(int)));

    ui->protocolSelect->addTab(gpioWidget, tr("GPIO"));
    ui->protocolSelect->addTab(i2cWidget, tr("I2C"));

    emit baseChanged(16);

    // Enumerate STLink Devices
    enumerateDevices();
}

MainWindow::~MainWindow()
{
    // Close STLink device
    qDebug().noquote() << tr("Closing Bridge");
    bridge->CloseBridge(COM_UNDEF_ALL);
    qDebug().noquote() << tr("Closing ST Link Device");
    bridge->CloseStlink();

    delete ui;
}

void MainWindow::handleDisconnect()
{
    qDebug().noquote() << tr("%1 handling disconnect").arg(this->objectName());

    // First, reenumerate the devices
    enumerateDevices();

    // Do we even have a device connected?
    if(!deviceConnected)
    {
        qDebug().noquote() << tr("%1: no device connected").arg(this->objectName());
        return;
    }

    // Is the disconnected device ours?
    uint32_t deviceCount = 0;
    interface->EnumDevices(&deviceCount, false);
    bool deviceFound = false;

    for(quint8 i = 0; i < deviceCount; i++)
    {
        STLink_DeviceInfo2T deviceInfo;

        if(interface->GetDeviceInfo2(i, &deviceInfo, 0) == STLINKIF_NO_ERR)
        {
            if(deviceInfo.EnumUniqueId == deviceConnectedSN)
            {
                deviceFound = true;
            }
        }
    }

    if(deviceFound)
    {
        qDebug().noquote()
                << tr("%1: device was disconnected, but it was not the current device").arg(this->objectName());
        return;
    }

    // We had a device connected and now its gone
    ui->statusbar->showMessage(tr("ST-LINK device disconnected"));
    qDebug().noquote() << tr("%1: disconnected device was the current device").arg(this->objectName());

    deviceConnected = false;
    ui->deviceSelectButton->setText(tr("Connect"));
    ui->deviceSelectBox->setEnabled(true);
    ui->protocolSelect->setEnabled(deviceConnected);

    // Inform the other protocol widgets
    emit deviceDisconnect();

    qDebug().noquote() << tr("%1: closing bridge").arg(this->objectName());
    bridge->CloseBridge(COM_UNDEF_ALL);
    bridge->CloseStlink();
}

void MainWindow::writeMessageToStatusBar(const QString &msg)
{
    ui->statusbar->showMessage(msg, 5000);
}

void MainWindow::enumerateDevices()
{
    qDebug().noquote() << tr("Clearing device list");
    ui->deviceSelectBox->clear();

    uint32_t deviceCount = 0;

    qDebug().noquote() << tr("Enumerating STLink devices");

    interface->EnumDevices(&deviceCount, true);
    qDebug().noquote() << tr("Found") << deviceCount << tr("STLink device(s)");
    ui->statusbar->showMessage(tr("Found %1 STLink device(s)").arg(deviceCount), 5000);

    for(quint8 i = 0; i < deviceCount; i++)
    {
        STLink_DeviceInfo2T deviceInfo;

        if( interface->GetDeviceInfo2(i, &deviceInfo, 0) == STLINKIF_NO_ERR )
        {
            ui->deviceSelectBox->addItem(deviceInfo.EnumUniqueId);
            qDebug().noquote() << tr("Device [") << i << tr("] - Unique ID:") << deviceInfo.EnumUniqueId
                    << tr(", STLink USB ID:") << deviceInfo.StLinkUsbId
                    << tr(", VID:") << deviceInfo.VendorId
                    << tr(", PID:") << deviceInfo.ProductId
                    << tr(", Used? ") << deviceInfo.DeviceUsed;
        }
    }
}

void MainWindow::connectDevice()
{
    if(!deviceConnected)
    {
        qDebug().noquote() << tr("Opening ST-LINK S/N:") << ui->deviceSelectBox->currentText().toUtf8().constData();

        Brg_StatusT ret = bridge->OpenStlink(ui->deviceSelectBox->currentText().toUtf8().constData(), true);
        if(ret == BRG_NO_ERR)
        {
            QString message;
            message = tr("Opened device %1 successfully").arg(ui->deviceSelectBox->currentText());

            qDebug().noquote() << message;
            ui->statusbar->showMessage(message, 5000);

            deviceConnected = true;
            deviceConnectedSN = ui->deviceSelectBox->currentText();
            ui->deviceSelectButton->setText(tr("Disconnect"));
            ui->deviceSelectBox->setEnabled(false);
            ui->protocolSelect->setEnabled(deviceConnected);
        }
        else
        {
            if(ret == BRG_CONNECT_ERR)
            {
                QString message;
                message = tr("Opening device %1 falied").arg(ui->deviceSelectBox->currentText());

                qWarning().noquote() << message << tr(", return code = BRG_CONNECT_ERR");
                ui->statusbar->showMessage(message, 0);
            }
            if(ret == BRG_OLD_FIRMWARE_WARNING)
            {
                // Handle old firmware by suggesting user update fw
                QString message;
                message = tr("Opening device %1 successful, but firmware may be out of date")
                            .arg(ui->deviceSelectBox->currentText());

                qWarning().noquote() << message;

                int ret = QMessageBox::warning(this, tr(APP_NAME),
                                               tr("Device opened successfully, but firmware may be out of date, consider updating firmware"),
                                               QMessageBox::Ok | QMessageBox::Abort);
                if(ret == QMessageBox::Abort)
                {
                    this->close();
                }
                ui->statusbar->showMessage(message, 5000);

                deviceConnected = true;
                ui->deviceSelectButton->setText(tr("Disconnect"));
                ui->protocolSelect->setEnabled(deviceConnected);
            }
        }
    }
    else // deviceConnected == true
    {
        qDebug().noquote() << tr("Closing ST-LINK S/N:") << ui->deviceSelectBox->currentText().toUtf8().constData();

        Brg_StatusT ret = bridge->CloseStlink();
        if(ret == BRG_NO_ERR)
        {
            QString message;
            message = tr("Closed device %1 successfully").arg(ui->deviceSelectBox->currentText());

            qDebug().noquote() << message;
            ui->statusbar->showMessage(message, 5000);

            deviceConnected = false;
            ui->deviceSelectButton->setText(tr("Connect"));
            ui->deviceSelectBox->setEnabled(true);
            ui->protocolSelect->setEnabled(deviceConnected);
        }
        else
        {
            QString message;
            message = tr("Unable to close device %1, error code %2")
                        .arg(ui->deviceSelectBox->currentText()).arg(ret);

            qWarning().noquote() << message;
            ui->statusbar->showMessage(message, 5000);
        }

        // Here we purposely disconnected the device via the disconnect button, so all we need to do
        //  is let the protocol widgets know that the device is gone via their disconnect handlers
        emit deviceDisconnect();
    }
}

void MainWindow::setBase()
{
    if(this->sender()->objectName() == "actionBinary_2")
    {
        ui->actionBinary_2->setChecked(true);
        ui->actionOctal_8->setChecked(false);
        ui->actionDecimal_10->setChecked(false);
        ui->actionHexadecimal_16->setChecked(false);

        emit baseChanged(2);
    }
    else if(this->sender()->objectName() == "actionOctal_8")
    {
        ui->actionBinary_2->setChecked(false);
        ui->actionOctal_8->setChecked(true);
        ui->actionDecimal_10->setChecked(false);
        ui->actionHexadecimal_16->setChecked(false);

        emit baseChanged(8);
    }
    else if(this->sender()->objectName() == "actionDecimal_10")
    {
        ui->actionBinary_2->setChecked(false);
        ui->actionOctal_8->setChecked(false);
        ui->actionDecimal_10->setChecked(true);
        ui->actionHexadecimal_16->setChecked(false);

        emit baseChanged(10);
    }
    else
    {
        ui->actionBinary_2->setChecked(false);
        ui->actionOctal_8->setChecked(false);
        ui->actionDecimal_10->setChecked(false);
        ui->actionHexadecimal_16->setChecked(true);

        emit baseChanged(16);
    }
}

void MainWindow::aboutDialog()
{
    // Get the device firmware version (must be connected to work)
    Stlk_VersionExtT deviceVersion;
    QString firmwareString;

    if(bridge->ST_GetVersionExt(&deviceVersion) == BRG_NO_ERR)
    {

        qDebug().noquote() << tr("Device Version:") << Qt::endl
                           << tr("\tPID:") << deviceVersion.PID << Qt::endl
                           << tr("\tVID:") << deviceVersion.VID << Qt::endl
                           << tr("\tMSC:") << deviceVersion.Msc_Ver << Qt::endl
                           << tr("\tJTAG:") << deviceVersion.Jtag_Ver << Qt::endl
                           << tr("\tRes1:") << deviceVersion.Res1_Ver << Qt::endl
                           << tr("\tRes2:") << deviceVersion.Res2_Ver << Qt::endl
                           << tr("\tSWIM:") << deviceVersion.Swim_Ver << Qt::endl
                           << tr("\tMajor Ver:") << deviceVersion.Major_Ver << Qt::endl
                           << tr("\tPower:") << deviceVersion.Power_Ver << Qt::endl
                           << tr("\tBridge Ver:") << deviceVersion.Bridge_Ver;

        firmwareString += "V" + QString::number(deviceVersion.Major_Ver) +
                "J" + QString::number(deviceVersion.Jtag_Ver) +
                "M" + QString::number(deviceVersion.Msc_Ver) +
                "B" + QString::number(deviceVersion.Bridge_Ver) +
                "S" + QString::number(deviceVersion.Swim_Ver);
    }
    else
    {
        firmwareString = "<N/C>";
    }

    // Get the USB library version
    QString dllVersion;
    QString fName = "STLinkUSBDriver.dll";

#ifdef WIN32
    // The following code comes from:
    //  http://amin-ahmadi.com/2015/04/03/get-exe-version-using-windows-api-in-qt/

    // first of all, GetFileVersionInfoSize
    DWORD dwHandle;
    DWORD dwLen = GetFileVersionInfoSize(fName.toStdWString().c_str(), &dwHandle);

    // GetFileVersionInfo
    LPVOID lpData = new BYTE[dwLen];
    if (!GetFileVersionInfo(fName.toStdWString().c_str(), dwHandle, dwLen, lpData))
    {
        qDebug().noquote() << tr("error in GetFileVersionInfo");
        //delete[] lpData;
        dllVersion = "";
    }

    // VerQueryValue
    VS_FIXEDFILEINFO* lpBuffer = NULL;
    UINT uLen;

    if (!VerQueryValue(lpData,
        QString("\\").toStdWString().c_str(),
        (LPVOID*)& lpBuffer,
        &uLen))
    {

        qDebug().noquote() << tr("error in VerQueryValue");
        //delete[] lpData;
        dllVersion = "";
    }
    else
    {
        dllVersion = QString::number((lpBuffer->dwFileVersionMS >> 16) & 0xffff) + "." +
                QString::number((lpBuffer->dwFileVersionMS) & 0xffff) + "." +
                QString::number((lpBuffer->dwFileVersionLS >> 16) & 0xffff) + "." +
                QString::number((lpBuffer->dwFileVersionLS) & 0xffff);
    }

    // End of code from:
    //  http://amin-ahmadi.com/2015/04/03/get-exe-version-using-windows-api-in-qt/
#endif //WIN32

    // Get the Qt version
    QString qtVersionString;

    qtVersionString += QString::number(QT_VERSION_MAJOR)
            + "." + QString::number(QT_VERSION_MINOR)
            + "." + QString::number(QT_VERSION_PATCH);

    // Put it all together
    QString aboutText;
    aboutText += tr(APP_NAME) + tr(" ver ") + tr(APP_VER) + "\r\n";
    aboutText += tr("Copyright 2020, Andy Josephson") + "\r\n";
    aboutText += "\r\n\r\n";
    aboutText += tr("STLinkUSBDriver DLL version: ") + dllVersion + "\r\n";
    aboutText += tr("STLinkV3Bridge API version: ") + QString::number(bridge->GetBridgeApiVersion()) + "\r\n";
    aboutText += tr("Device Firmware version: ") + firmwareString + "\r\n";
    aboutText += tr("Qt version: ") + qtVersionString + "\r\n";

    // Finally show it
    QMessageBox::about(this, tr(APP_NAME), aboutText);
}

