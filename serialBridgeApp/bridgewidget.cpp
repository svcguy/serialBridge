#include "bridgewidget.h"

#include <QApplication>
#include <QtDebug>
#include <QMessageBox>

bridgeWidget::bridgeWidget(Brg *p_brg, QWidget *parent) : QWidget(parent)
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);
    qDebug().noquote() << tr(Q_FUNC_INFO) << tr("checking bridge pointer");
    if(p_brg == nullptr)
    {
        QString error;
        error = tr(Q_FUNC_INFO);
        error += tr(": No bridge object passed, quitting");
        qCritical().noquote() << error;

        int ret = QMessageBox::critical(this, tr("Critical Error"), error, QMessageBox::Abort);
        if(ret == QMessageBox::Abort)
        {
            qApp->quit();
        }
    }

    qDebug().noquote() << tr(Q_FUNC_INFO) << tr("taking pointer to the bridge object");

    bridge = p_brg;

    qDebug().noquote() << tr("Exiting") << tr(Q_FUNC_INFO);
}

void bridgeWidget::handleDisconnect()
{

}

void bridgeWidget::setBase(int base)
{
    Q_UNUSED(base);
}

QString bridgeWidget::displayNumber(quint64 number, quint8 base)
{
    // How many bytes is this number?
    quint8 bytes = 0;

    if(number < UINT64_MAX)
    {
        bytes = 8;
    }
    if(number < UINT32_MAX)
    {
        bytes = 4;
    }
    if(number < UINT16_MAX)
    {
        bytes = 2;
    }
    if(number < UINT8_MAX)
    {
        bytes = 1;
    }

    // Make the string
    QString numberString;
    switch (base)
    {
        case 2:
            numberString = QString("0b%1").arg(number, bytes*8, base, QLatin1Char('0'));
            break;
        case 8:
            numberString = QString("0o%1").arg(number, bytes, base, QLatin1Char('0'));
            break;
        case 10:
            numberString = QString("%1").arg(number);
            break;
        default:
        case 16:
            numberString = QString("0x%1").arg(number, bytes*2, base, QLatin1Char('0'));
            break;
    }

    return numberString;
}

QString bridgeWidget::errorCodeToString(Brg_StatusT code)
{
    QString error;

    switch(code)
    {
        case BRG_NO_ERR:
            error = tr("OK");
            break;
        case BRG_CONNECT_ERR:
            error = tr("USB connection error");
            break;
        case BRG_DLL_ERR:
            error = tr("USB DLL error");
            break;
        case BRG_USB_COMM_ERR:
            error = tr("USB communication error");
            break;
        case BRG_NO_DEVICE:
            error = tr("No bridge device target found error");
            break;
        case BRG_OLD_FIRMWARE_WARNING:
            error = tr("Warning: current bridge firmware is not the last one available");
            break;
        case BRG_TARGET_CMD_ERR:
            error = tr("Target communication or command error");
            break;
        case BRG_PARAM_ERR:
            error = tr("Wrong parameter error");
            break;
        case BRG_CMD_NOT_SUPPORTED:
            error = tr("Firmware command not supported by the current firmware version");
            break;
        case BRG_GET_INFO_ERR:
            error = tr("Error getting STLink bridge device information");
            break;
        case BRG_STLINK_SN_NOT_FOUND:
            error = tr("Required STLink serial number not found error");
            break;
        case BRG_NO_STLINK:
            error = tr("STLink bridge device not opened error");
            break;
        case BRG_NOT_SUPPORTED:
            error = tr("Parameter error");
            break;
        case BRG_PERMISSION_ERR:
            error = tr("STLink bridge device already in use by another program error");
            break;
        case BRG_ENUM_ERR:
            error = tr("USB enumeration error");
            break;
        case BRG_COM_FREQ_MODIFIED:
            error = tr("Warning: required frequency is not exactly the one applied");
            break;
        case BRG_COM_FREQ_NOT_SUPPORTED:
            error = tr("Required frequency cannot be applied error");
            break;
        case BRG_SPI_ERR:
            error = tr("SPI communication error");
            break;
        case BRG_I2C_ERR:
            error = tr("I2C communication error");
            break;
        case BRG_CAN_ERR:
            error = tr("CAN communication error");
            break;
        case BRG_TARGET_CMD_TIMEOUT:
            error = tr("Timeout error during bridge communication");
            break;
        case BRG_COM_INIT_NOT_DONE:
            error = tr("Bridge init function not called error");
            break;
        case BRG_COM_CMD_ORDER_ERR:
            error = tr("Sequential bridge function order error");
            break;
        case BRG_BL_NACK_ERR:
            error = tr("Bootloader NACK error");
            break;
        case BRG_VERIF_ERR:
            error = tr("Data verification error");
            break;
        case BRG_MEM_ALLOC_ERR:
            error = tr("Memory allocation error");
            break;
        case BRG_GPIO_ERR:
            error = tr("GPIO communication error");
            break;
        case BRG_OVERRUN_ERR:
            error = tr("Overrun error during bridge communication");
            break;
        case BRG_CMD_BUSY:
            error = tr("Command busy: only Brg::GetLastReadWriteStatus() command allowed in that case");
            break;
        case BRG_CLOSE_ERR:
            error = tr("Error during device close");
            break;
        case BRG_INTERFACE_ERR:
        default:
            error = tr("Unknown default error returned by STLinkInterface");
    }

    return error;
}
