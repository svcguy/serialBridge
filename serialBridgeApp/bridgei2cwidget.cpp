#include "bridgei2cwidget.h"
#include "ui_bridgei2cwidget.h"

#include <QtDebug>
#include <QMessageBox>
#include <QListIterator>

bridgeI2CWidget::bridgeI2CWidget(Brg *p_brg) :
    bridgeWidget(p_brg),
    ui(new Ui::bridgeI2CWidget)
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    this->setObjectName("bridgeI2CWidget");

    qDebug().noquote() << tr("%1: setting up UI elements").arg(this->objectName());

    ui->setupUi(this);

    this->setLayout(ui->mainLayout);

    // As of Bridge API v1, I2C slave mode is not enabled so disable config options
    //  associated with slave mode
    ui->checkBoxSlaveMode->setEnabled(false);
    ui->lineEditOwnAddress->setEnabled(false);
    ui->checkBoxMasterMode->setCheckState(Qt::Checked);
    ui->lineEditRiseTime->setText("0");
    ui->lineEditFallTime->setText("0");

    // By default the line edit and combo boxes don't have the same height
    //  Fix that here
    ui->lineEditOwnAddress->setMaximumHeight(ui->comboBoxAddressMode->size().height());
    ui->lineEditFallTime->setMaximumHeight(ui->comboBoxAddressMode->size().height());
    ui->lineEditRiseTime->setMaximumHeight(ui->comboBoxAddressMode->size().height());
    ui->lineEditData->setMaximumHeight(ui->comboBoxReadWriteSelect->size().height());
    ui->lineEditAddress->setMaximumHeight(ui->comboBoxReadWriteSelect->size().height());

    riseTimeValidator = new QIntValidator(this);
    fallTimeValidator = new QIntValidator(this);
    riseTimeValidator->setRange(0, 1000);
    fallTimeValidator->setRange(0, 300);
    ui->lineEditRiseTime->setValidator(riseTimeValidator);
    ui->lineEditFallTime->setValidator(fallTimeValidator);

    ui->comboBoxAddressMode->addItem(tr("7-bit"));
    ui->comboBoxAddressMode->addItem(tr("10-bit"));

    ui->comboBoxSpeed->addItem(tr("Standard Mode (100kHz)"));
    ui->comboBoxSpeed->addItem(tr("Fast Mode (400kHz)"));
    ui->comboBoxSpeed->addItem(tr("Fast Mode Plus (1MHz)"));

    ui->comboBoxAnFilter->addItem(tr("Disabled"));
    ui->comboBoxAnFilter->addItem(tr("Enabled"));

    ui->comboBoxDgFilter->addItem(tr("Disabled"));
    ui->comboBoxDgFilter->addItem(tr("Enabled"));

    ui->comboBoxReadWriteSelect->addItem(tr("WRITE"));
    ui->comboBoxReadWriteSelect->addItem(tr("READ"));

    // Digital filter will start disabled, so disable DNF box
    ui->comboBoxDNF->setDisabled(true);

    ui->comboBoxDNF->addItem(tr("0"));
    ui->comboBoxDNF->addItem(tr("1"));
    ui->comboBoxDNF->addItem(tr("2"));
    ui->comboBoxDNF->addItem(tr("3"));
    ui->comboBoxDNF->addItem(tr("4"));
    ui->comboBoxDNF->addItem(tr("5"));
    ui->comboBoxDNF->addItem(tr("6"));
    ui->comboBoxDNF->addItem(tr("7"));
    ui->comboBoxDNF->addItem(tr("8"));
    ui->comboBoxDNF->addItem(tr("9"));
    ui->comboBoxDNF->addItem(tr("10"));
    ui->comboBoxDNF->addItem(tr("11"));
    ui->comboBoxDNF->addItem(tr("12"));
    ui->comboBoxDNF->addItem(tr("13"));
    ui->comboBoxDNF->addItem(tr("14"));
    ui->comboBoxDNF->addItem(tr("15"));

    qDebug().noquote() << tr("%1: Connecting signals and slots").arg(this->objectName());

    connect(ui->comboBoxSpeed, SIGNAL(currentIndexChanged(int)), this, SLOT(onSpeedSelectionChange(int)));
    connect(ui->pushButtonParamsSet, SIGNAL(clicked()), this, SLOT(setI2CConfig()));
    connect(ui->comboBoxDgFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(onDigitalFilterChange(int)));
    connect(ui->pushButtonSend, SIGNAL(clicked()), this, SLOT(onSendButtonClicked()));
    connect(ui->comboBoxReadWriteSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(onReadWriteSelectChange(int)));
    connect(ui->pushButtonMonitorClear, SIGNAL(clicked()), ui->monitor, SLOT(clear()));

    // Setting the default I2C Config
    //  I'm using the reset states as defined in the datasheet
    i2cConfig.AddrMode = I2C_ADDR_7BIT;
    i2cConfig.OwnAddr = 0x00;
    i2cConfig.AnFilterEn = I2C_FILTER_DISABLE;
    i2cConfig.DigitalFilterEn = I2C_FILTER_DISABLE;
    i2cConfig.Dnf = 0x0;

    qDebug().noquote() << tr("%1: setup complete").arg(this->objectName());
    qDebug().noquote() << tr("Exiting") << tr(Q_FUNC_INFO);
}

bridgeI2CWidget::~bridgeI2CWidget()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    bridge->CloseBridge(COM_I2C);

    delete ui;

    qDebug().noquote() << tr("Exiting") << tr(Q_FUNC_INFO);
}

void bridgeI2CWidget::handleDisconnect()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);
    qWarning().noquote() << tr("%1: notified of disconnect event").arg(this->objectName());

    bridge->CloseBridge(COM_I2C);
}

void bridgeI2CWidget::setBase(int base)
{
    numberBase = base;
    qDebug().noquote() << tr("%1: number base changed to: %2").arg(this->objectName()).arg(numberBase);
}

void bridgeI2CWidget::onSpeedSelectionChange(int index)
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO) << tr("int index = %1").arg(index);

    switch(index)
    {
        case 0: // Standard mode 0-1000ns rise, 0-300 fall
            riseTimeValidator->setTop(1000);
            fallTimeValidator->setTop(300);
            break;
        case 1: // Fast mode 0-300ns rise, 0-300 fall
            riseTimeValidator->setTop(300);
            fallTimeValidator->setTop(300);
            break;
        case 2: // Fast plus mode 0-120s rise, 0-120 fall
            riseTimeValidator->setTop(120);
            fallTimeValidator->setTop(120);
            break;
        default:
            riseTimeValidator->setTop(120);
            fallTimeValidator->setTop(120);
            break;
    }
}

void bridgeI2CWidget::onDigitalFilterChange(int state)
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO) << tr("int state = %1").arg(state);

    if(state == 0)
    {   // Disabled
        ui->comboBoxDNF->setCurrentIndex(0);
        ui->comboBoxDNF->setDisabled(true);
    }
    if(state == 1)
    {   // Enabled
        ui->comboBoxDNF->setDisabled(false);
    }
}

void bridgeI2CWidget::onReadWriteSelectChange(int selection)
{
    qDebug().noquote() << tr(Q_FUNC_INFO) << tr("readWriteSelect changed: %1").arg(selection);
    if(selection == 0)
    {   // WRITE
        ui->labelData->setText(tr("Data (comma seperated)"));
        ui->lineEditData->clear();
        ui->lineEditData->setPlaceholderText(tr("0x00, 0x20, 0x02, ..."));
    }
    if(selection == 1)
    {   // READ
        ui->labelData->setText(tr("Number of bytes to read"));
        ui->lineEditData->clear();
        ui->lineEditData->setPlaceholderText(tr("0-255"));
    }
}

void bridgeI2CWidget::onSendButtonClicked()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    if(ui->comboBoxReadWriteSelect->currentIndex() == 0)
    {   // WRITE

        bool ok = false;

        // Get the address that we're writing to
        uint16_t address = ui->lineEditAddress->text().toInt(&ok, numberBase);
        qDebug().noquote() << tr("%1: I2C send address = %2").arg(this->objectName()).arg(address);

        if(!ok)
        {
            QString msg;
            msg = tr("Invalid I2C Address");

            qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
            emit postMessage(msg);

            return;
        }

        // Get the data
        QList<QString> stringData(ui->lineEditData->text().split(QChar(','), Qt::SkipEmptyParts));
        qDebug().noquote() << tr("%1: I2C data (size %2) -").arg(this->objectName()).arg(stringData.size()) << stringData;

        if(stringData.size() == 0)
        {
            QString msg;
            msg = tr("Invalid I2C Data");

            qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
            emit postMessage(msg);

            return;
        }

        QListIterator<QString> i(stringData);
        QVector<uint8_t> data;

        while(i.hasNext())
        {
            data.append(i.next().toInt(&ok, numberBase));

            if(!ok)
            {
                QString msg;
                msg = tr("Invalid I2C Data");

                qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
                emit postMessage(msg);

                return;
            }
        }

        qDebug().noquote() << tr("%1: preparing I2C transaction - address = %2, size = %3, data =")
                              .arg(this->objectName()).arg(address).arg(data.size())
                           << data;


        qDebug().noquote() << tr("%1: Sending WRITE transaction").arg(this->objectName());

        uint16_t bytesWritten = 0;

        Brg_StatusT ret = bridge->WriteI2C(data.constData(), address, data.size(), &bytesWritten);

        if(ret != BRG_NO_ERR)
        {
            QString msg;
            msg = tr("I2C Write Fail: error - %1, (%2 byte(s) written").arg(errorCodeToString(ret)).arg(bytesWritten);

            qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
            emit postMessage(msg);

            // Write to monitor
            msg = tr("WRITE (%1): ").arg(displayNumber(address, numberBase));
            for(uint8_t i = 0; i < data.size(); i++)
            {
                msg += displayNumber(data.at(i), numberBase);
                msg += ", ";
            }
            msg += "NACK";

            ui->monitor->append(msg);
        }
        else
        {
            QString msg;
            msg = tr("I2C Write Success!");

            qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
            emit postMessage(msg);

            // Write to monitor
            msg = tr("WRITE (%1): ").arg(displayNumber(address, numberBase));
            for(uint8_t i = 0; i < data.size(); i++)
            {
                msg += displayNumber(data.at(i), numberBase);
                msg += ", ";
            }
            msg += "ACK OK";

            ui->monitor->append(msg);
        }
    }
    else
    {   // READ
        bool ok = false;

        // Get the address that we're reading from
        uint16_t address = ui->lineEditAddress->text().toInt(&ok, numberBase);
        qDebug().noquote() << tr("%1: I2C send address = %2").arg(this->objectName()).arg(address);

        if(!ok)
        {
            QString msg;
            msg = tr("Invalid I2C Address");

            qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
            emit postMessage(msg);

            return;
        }

        // Get number of bytes to read
        uint16_t bytesToRead = ui->lineEditData->text().toInt(&ok, numberBase);
        qDebug().noquote() << tr("%1: I2C bytes to read = %2").arg(this->objectName()).arg(bytesToRead);

        if(!ok)
        {
            QString msg;
            msg = tr("Invalid I2C bytes to read");

            qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
            emit postMessage(msg);
        }

        // Create a vector to hold data
        QVector<uint8_t> dataRead;
        // Create a buffer for the data read
        uint8_t i2cBuffer[I2C_BUFFER_SIZE];

        qDebug().noquote() << tr("%1: Read buffer prepared - size %2").arg(this->objectName()).arg(dataRead.capacity());

        qDebug().noquote() << tr("%1: Sending READ transaction").arg(this->objectName());
        uint16_t bytesRead = 0;

        Brg_StatusT ret = bridge->ReadI2C(i2cBuffer, address, bytesToRead, &bytesRead);

        // Copy data into the vector
        for(int i = 0; i < bytesToRead; i++)
        {
            dataRead.append(i2cBuffer[i]);
        }

        if(ret != BRG_NO_ERR)
        {
            QString msg;
            msg = tr("I2C Read Fail: error - %1, (%2 byte(s) read").arg(errorCodeToString(ret)).arg(bytesRead);

            qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
            emit postMessage(msg);

            // Write to monitor
            msg = tr("READ (%1): ").arg(displayNumber(address, numberBase));
            for(uint8_t i = 0; i < dataRead.size(); i++)
            {
                msg += displayNumber(dataRead.at(i), numberBase);
                msg += ", ";
            }
            msg += "NACK";

            ui->monitor->append(msg);
        }
        else
        {
            QString msg;
            msg = tr("I2C Read Success!");

            qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
            emit postMessage(msg);

            // Write to monitor
            msg = tr("READ (%1): ").arg(displayNumber(address, numberBase));
            for(uint8_t i = 0; i < dataRead.size(); i++)
            {
                msg += displayNumber(dataRead.at(i), numberBase);
                msg += ", ";
            }
            msg += "ACK OK";

            ui->monitor->append(msg);
        }

        qDebug().noquote() << tr("%1: Read %2 byte(s) - ").arg(this->objectName()).arg(dataRead.size()) << dataRead;
    }
}

Brg_StatusT bridgeI2CWidget::setI2CConfig()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    Brg_StatusT ret = BRG_NO_ERR;

    // Get the selected speed
    I2cModeT speed;
    int frequency;
    int riseTime, fallTime;

    QString tempString = ui->lineEditRiseTime->text();
    int tempInt = 0;

    if(riseTimeValidator->validate(tempString, tempInt) != QValidator::Acceptable)
    {
        QString msg;

        msg = tr("Rise time parameter invalid");

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);

        return BRG_PARAM_ERR;
    }

    tempString = ui->lineEditFallTime->text();

    if(fallTimeValidator->validate(tempString, tempInt) != QValidator::Acceptable)
    {
        QString msg;

        msg = tr("Fall time parameter invalid");

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);

        return BRG_PARAM_ERR;
    }

    switch(ui->comboBoxSpeed->currentIndex())
    {
        case(0):
            // I2C Standard speed 0-100kHz
            speed = I2C_STANDARD;
            frequency = 100;
            riseTime = ui->lineEditRiseTime->text().toInt();
            fallTime = ui->lineEditFallTime->text().toInt();
            break;
        case(1):
            // I2C Fast speed 0-400kHz
            speed = I2C_FAST;
            frequency = 400;
            riseTime = ui->lineEditRiseTime->text().toInt();
            fallTime = ui->lineEditFallTime->text().toInt();
            break;
        case(2):
            // I2C Fast Plus speed 0-1000kHz
            speed = I2C_FAST_PLUS;
            frequency = 1000;
            riseTime = ui->lineEditRiseTime->text().toInt();
            fallTime = ui->lineEditFallTime->text().toInt();
            break;
         default:
            return BRG_PARAM_ERR;
    }

    // Get bridge clocks
    uint32_t i2cClock = 0;
    uint32_t hClock = 0;
    ret = bridge->GetClk(COM_I2C, &i2cClock, &hClock);

    if(ret != BRG_NO_ERR)
    {
        QString msg;
        msg = tr("Error getting bridge clocks: error - %1").arg(errorCodeToString(ret));

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }

    qDebug().noquote() << tr("%1: HCLK - %2, I2CCLK - %3").arg(this->objectName()).arg(hClock).arg(i2cClock);

    // Collect the rest of the config parameters
    i2cConfig.Dnf = ui->comboBoxDNF->currentIndex();
    i2cConfig.OwnAddr = ui->lineEditOwnAddress->text().toInt();
    i2cConfig.AddrMode = static_cast<Brg_I2cAddrModeT>(ui->comboBoxAddressMode->currentIndex());
    i2cConfig.AnFilterEn = static_cast<Brg_I2cFilterT>(ui->comboBoxAnFilter->currentIndex());
    i2cConfig.DigitalFilterEn = static_cast<Brg_I2cFilterT>(ui->comboBoxDgFilter->currentIndex());

    qDebug().noquote() << tr("Getting timing register for params speed=%1, freq=%2, rise=%3, fall=%4, anf=%5")
                          .arg(speed).arg(frequency).arg(riseTime).arg(fallTime)
                          .arg(static_cast<bool>(i2cConfig.AnFilterEn));

    // Get the timing reg
    uint32_t timingReg = 0;
    ret = bridge->GetI2cTiming(speed, frequency,
                               i2cConfig.Dnf, riseTime,
                               fallTime, static_cast<bool>(i2cConfig.AnFilterEn),
                               &timingReg);

    if(ret != BRG_NO_ERR)
    {
        QString msg;
        msg = tr("Unable to set timing register for params speed=%1, freq=%2, rise=%3, fall=%4, anf=%5")
                .arg(speed).arg(frequency).arg(riseTime).arg(fallTime).arg(i2cConfig.AnFilterEn);
        msg += tr(" Timing reg = %1, error: %2").arg(timingReg).arg(errorCodeToString(ret));

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }
    else
    {
        QString msg;
        msg = tr("Got timing register for params speed=%1, freq=%2, rise=%3, fall=%4, anf=%5")
                .arg(speed).arg(frequency).arg(riseTime).arg(fallTime).arg(i2cConfig.AnFilterEn);
        msg += tr(" Timing reg = %1").arg(timingReg);

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }

    qDebug().noquote() << tr("%1: Setting I2C config - Address Mode: %2, Own Address: %3, Timing Reg: %4, AFilter: %5, DFilter: %6, DNF: %7")
                          .arg(this->objectName()).arg(i2cConfig.AddrMode).arg(i2cConfig.OwnAddr).arg(i2cConfig.TimingReg)
                          .arg(i2cConfig.AnFilterEn).arg(i2cConfig.DigitalFilterEn).arg(i2cConfig.Dnf);

    // Send the config to the bridge
    ret = bridge->InitI2C(&i2cConfig);

    if(ret != BRG_NO_ERR)
    {
        QString msg;
        msg = tr("I2C init failed - error: %2").arg(errorCodeToString(ret));

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }
    else
    {
        QString msg;
        msg = tr("I2C init success!");

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }

    return ret;
}
