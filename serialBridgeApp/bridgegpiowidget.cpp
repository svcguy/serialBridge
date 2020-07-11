#include "bridgegpiowidget.h"
#include "ui_bridgegpiowidget.h"

#include <QtDebug>
#include <QMessageBox>
#include <cmath>

bridgeGPIOWidget::bridgeGPIOWidget(Brg *p_brg) :
    bridgeWidget(p_brg),
    ui(new Ui::bridgeGPIOWidget)
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    this->setObjectName("bridgeGPIOWidget");

    qDebug().noquote() << tr("%1: creating GUI read timer").arg(this->objectName());
    readTimer = new QTimer(this);
    readTimer->setTimerType(Qt::CoarseTimer);

    qDebug().noquote() << tr("%1: setting up UI elements").arg(this->objectName());
    ui->setupUi(this);

    ui->modeBox->addItem(tr("Output"));
    ui->modeBox->addItem(tr("Input"));
    ui->modeBox->addItem(tr("Analog"));

    ui->speedBox->addItem(tr("Low"));
    ui->speedBox->addItem(tr("Medium"));
    ui->speedBox->addItem(tr("High"));
    ui->speedBox->addItem(tr("Very High"));

    ui->pupdBox->addItem(tr("None"));
    ui->pupdBox->addItem(tr("Pull up"));
    ui->pupdBox->addItem(tr("Pull down"));

    ui->otBox->addItem(tr("Push-Pull"));
    ui->otBox->addItem(tr("Open drain"));

    ui->boxGPIO0->addItem(tr("SET"));
    ui->boxGPIO0->addItem(tr("RESET"));
    ui->boxGPIO1->addItem(tr("SET"));
    ui->boxGPIO1->addItem(tr("RESET"));
    ui->boxGPIO2->addItem(tr("SET"));
    ui->boxGPIO2->addItem(tr("RESET"));
    ui->boxGPIO3->addItem(tr("SET"));
    ui->boxGPIO3->addItem(tr("RESET"));

    readLineEditValidator = new QIntValidator(10, INT_MAX, this);
    ui->contReadIntEdit->setValidator(readLineEditValidator);

    this->setLayout(ui->mainLayout);

    qDebug().noquote() << tr("%1: Connecting signals and slots").arg(this->objectName());

    connect(ui->buttonGPIO0, SIGNAL(clicked()), this, SLOT(getNewGpioSelection()));
    connect(ui->buttonGPIO1, SIGNAL(clicked()), this, SLOT(getNewGpioSelection()));
    connect(ui->buttonGPIO2, SIGNAL(clicked()), this, SLOT(getNewGpioSelection()));
    connect(ui->buttonGPIO3, SIGNAL(clicked()), this, SLOT(getNewGpioSelection()));
    connect(this, SIGNAL(gpioSelectionChanged(Brg_GpioMaskT)), this, SLOT(getGPIOConfig(Brg_GpioMaskT)));
    connect(ui->buttonConfigSet, SIGNAL(clicked()), this, SLOT(setGPIOConfig()));
    connect(ui->contReadBox, SIGNAL(stateChanged(int)), this, SLOT(handleReadCheckBox(int)));
    connect(ui->contReadIntEdit, SIGNAL(textChanged(const QString&)), this, SLOT(handleReadIntervalEdit(const QString&)));
    connect(this->readTimer, SIGNAL(timeout()), this, SLOT(readGPIO()));
    connect(ui->readButton, SIGNAL(clicked()), this, SLOT(readGPIO()));
    connect(ui->buttonWriteGPIO0, SIGNAL(clicked()), this, SLOT(handleWriteButtonClicked()));
    connect(ui->buttonWriteGPIO1, SIGNAL(clicked()), this, SLOT(handleWriteButtonClicked()));
    connect(ui->buttonWriteGPIO2, SIGNAL(clicked()), this, SLOT(handleWriteButtonClicked()));
    connect(ui->buttonWriteGPIO3, SIGNAL(clicked()), this, SLOT(handleWriteButtonClicked()));
    connect(this, SIGNAL(gpioToWrite(Brg_GpioMaskT)), this, SLOT(writeGPIO(Brg_GpioMaskT)));

    // Set the defaults for GPIO Configuration
    //  I'm using the reset states as defined in the datasheet
    for(quint8 i = 0; i < BRG_GPIO_MAX_NB; i++)
    {
        gpioConf[i].Mode = GPIO_MODE_INPUT;
        gpioConf[i].Speed = GPIO_SPEED_LOW;
        gpioConf[i].Pull = GPIO_NO_PULL;
        gpioConf[i].OutputType = GPIO_OUTPUT_PUSHPULL;
    }

    qDebug().noquote() << tr("%1: setup complete").arg(this->objectName());
    qDebug().noquote() << tr("Exiting") << tr(Q_FUNC_INFO);
}

bridgeGPIOWidget::~bridgeGPIOWidget()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    bridge->CloseBridge(COM_GPIO);

    readTimer->stop();

    delete ui;

    qDebug().noquote() << tr("Exiting") << tr(Q_FUNC_INFO);
}

void bridgeGPIOWidget::handleDisconnect()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);
    qWarning().noquote() << tr("%1: notified of disconnect event").arg(this->objectName());

    readTimer->stop();
    ui->contReadBox->setCheckState(Qt::Unchecked);
    ui->contReadIntEdit->clear();

    bridge->CloseBridge(COM_GPIO);
}

void bridgeGPIOWidget::setBase(int base)
{
    Q_UNUSED(base);
}

Brg_StatusT bridgeGPIOWidget::getGPIOConfig(Brg_GpioMaskT gpio)
{
    Brg_StatusT ret = BRG_NO_ERR;

    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO) << tr("Brg_GpioMaskT gpio = %1").arg(gpio);

    quint8 index = 0;
    switch(gpio)
    {
        case BRG_GPIO_0:
            index = 0;
            break;
        case BRG_GPIO_1:
            index = 1;
            break;
        case BRG_GPIO_2:
            index = 2;
            break;
        case BRG_GPIO_3:
            index = 3;
            break;
        case BRG_GPIO_ALL:
        default:
            index = 255;
            break;
    }

    if(index == 255)
    {
        return BRG_PARAM_ERR;
    }

    qDebug().noquote() << tr("Retrieving config for GPIO") << index;
    qDebug().noquote().nospace() << tr("GPIO[") << index
                                 << tr("]: Mode-") << gpioConf[index].Mode
                                 << tr(", Speed-") << gpioConf[index].Speed
                                 << tr(", Pull-") << gpioConf[index].Pull
                                 << tr(", Output Type-") << gpioConf[index].OutputType;

    // Handle odd number of Brg_GpioModeT enum
    if(gpioConf[index].Mode == GPIO_MODE_ANALOG)
    {
        ui->modeBox->setCurrentIndex(2);
    }
    else
    {
        ui->modeBox->setCurrentIndex(gpioConf[index].Mode);
    }
    ui->speedBox->setCurrentIndex(gpioConf[index].Speed);
    ui->pupdBox->setCurrentIndex(gpioConf[index].Pull);
    ui->otBox->setCurrentIndex(gpioConf[index].OutputType);

    qDebug().noquote() << tr("Finished retrieving config for GPIO") << index;

    return ret;
}

Brg_StatusT bridgeGPIOWidget::setGPIOConfig()
{
    Brg_StatusT ret = BRG_NO_ERR;
    Brg_GpioConfT newConfig;
    Brg_GpioMaskT gpio = BRG_GPIO_ALL;
    quint8 index = 255;

    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    // First figure out which GPIO we're going to set
    if(ui->buttonGPIO0->isChecked())
    {
        gpio = BRG_GPIO_0;
        index  = 0;
    }
    if(ui->buttonGPIO1->isChecked())
    {
        gpio = BRG_GPIO_1;
        index = 1;
    }
    if(ui->buttonGPIO2->isChecked())
    {
        gpio = BRG_GPIO_2;
        index = 2;
    }
    if(ui->buttonGPIO3->isChecked())
    {
        gpio = BRG_GPIO_3;
        index = 3;
    }

    if(index == 255)
    {
        return BRG_PARAM_ERR;
    }

    // Handle odd number of Brg_GpioModeT enum
    if(ui->modeBox->currentIndex() == 2)
    {
        newConfig.Mode = GPIO_MODE_ANALOG;
    }
    else
    {
        newConfig.Mode = static_cast<Brg_GpioModeT>(ui->modeBox->currentIndex());
    }
    // Get the rest of the config
    newConfig.Speed = static_cast<Brg_GpioSpeedT>(ui->speedBox->currentIndex());
    newConfig.Pull = static_cast<Brg_GpioPullT>(ui->pupdBox->currentIndex());
    newConfig.OutputType = static_cast<Brg_GpioOutputT>(ui->otBox->currentIndex());

    qDebug().noquote() << tr("Setting config for GPIO") << index;
    qDebug().noquote().nospace() << tr("GPIO[") << index
                                 << tr("]: Mode-") << newConfig.Mode
                                 << tr(", Speed-") << newConfig.Speed
                                 << tr(", Pull-") << newConfig.Pull
                                 << tr(", Output Type-") << newConfig.OutputType;

    // Write config to device, if success, update stored config
    Brg_GpioInitT initToWrite;
    initToWrite.GpioMask = gpio;
    initToWrite.ConfigNb = 1;
    initToWrite.pGpioConf = &newConfig;

    qDebug().noquote() << tr("Writing gpio init to bridge");

    ret = bridge->InitGPIO(&initToWrite);

    if(ret != BRG_NO_ERR)
    {
        QString error;
        error = tr("Writing gpio init to bridge failed, error: %1").arg(errorCodeToString(ret));

        qWarning().noquote() << error;
        emit postMessage(error);

    }
    else
    {
        QString msg;
        msg = tr("Writing gpio init to bridge success!  Updating config");
        qWarning().noquote() << msg;

        emit postMessage(msg);

        gpioConf[index] = newConfig;
    }

    return ret;
}

void bridgeGPIOWidget::getNewGpioSelection()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);
    if(ui->buttonGPIO0->isChecked())
    {
        qDebug().noquote() << tr("Emitting gpioSelectionChanged(BRG_GPIO_0)");
        emit gpioSelectionChanged(BRG_GPIO_0);
    }
    if(ui->buttonGPIO1->isChecked())
    {
        qDebug().noquote() << tr("Emitting gpioSelectionChanged(BRG_GPIO_1)");
        emit gpioSelectionChanged(BRG_GPIO_1);
    }
    if(ui->buttonGPIO2->isChecked())
    {
        qDebug().noquote() << tr("Emitting gpioSelectionChanged(BRG_GPIO_2)");
        emit gpioSelectionChanged(BRG_GPIO_2);
    }
    if(ui->buttonGPIO3->isChecked())
    {
        qDebug().noquote() << tr("Emitting gpioSelectionChanged(BRG_GPIO_3)");
        emit gpioSelectionChanged(BRG_GPIO_3);
    }
}

void bridgeGPIOWidget::handleReadCheckBox(int state)
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO)
                       << tr("int state = %1, interval = %2").arg(state).arg(ui->contReadIntEdit->text().toInt());

    if(ui->contReadIntEdit->text().toInt() == 0)
    {
        QString error;
        error = tr("No interval specified or 0. Specify a valid interval");

        qWarning().noquote() << error;

        emit postMessage(error);

        ui->contReadBox->setCheckState(Qt::Unchecked);
        return;
    }

    if(state == Qt::Checked)
    {
        qDebug().noquote() << tr("Continuous Read Checkbox checked");
        readTimer->setInterval(ui->contReadIntEdit->text().toInt());
        readTimer->start();

        // Disable read button
        ui->readButton->setEnabled(false);

        QString msg;
        msg = tr("Continuous read started");

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }
    if(state == Qt::Unchecked)
    {
        qDebug().noquote()<< tr("Continuous Read Checkbox unchecked");
        readTimer->stop();

        // Enable read button
        ui->readButton->setEnabled(true);

        QString msg;
        msg = tr("Continuous read stopeed");

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }
}

void bridgeGPIOWidget::handleReadIntervalEdit(const QString &text)
{
    qDebug().noquote() << tr("Entering")
                       << tr(Q_FUNC_INFO) << tr("const QString &text =") << text;

    int interval = 0;
    QString valText(text);

    if(readLineEditValidator->validate(valText, interval) == QValidator::Acceptable)
    {
        ui->contReadBox->setEnabled(true);
        readTimer->setInterval(interval);
    }
    else
    {
        ui->contReadBox->setEnabled(false);
    }
}

void bridgeGPIOWidget::writeGPIO(Brg_GpioMaskT gpio)
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO) << "Brg_GpioMaskT gpio =" << gpio;

    Brg_StatusT ret = BRG_NO_ERR;
    quint8 errorMask = 0;
    Brg_GpioValT value;

    if(gpio == BRG_GPIO_0)
    {
        if(ui->boxGPIO0->currentText() == "SET")
        {
            value = GPIO_SET;
        }
        else
        {
            value = GPIO_RESET;
        }

        qDebug().noquote() << tr("Writing %1 to GPIO(%2)").arg(value).arg(log2(gpio));
        ret = bridge->SetResetGPIO(static_cast<uint8_t>(gpio), &value, &errorMask);
    }

    if(gpio == BRG_GPIO_1)
    {
        if(ui->boxGPIO1->currentText() == "SET")
        {
            value = GPIO_SET;
        }
        else
        {
            value = GPIO_RESET;
        }

        qDebug().noquote() << tr("Writing %1 to GPIO(%2)").arg(value).arg(log2(gpio));
        ret = bridge->SetResetGPIO(static_cast<uint8_t>(gpio), &value, &errorMask);
    }

    if(gpio == BRG_GPIO_2)
    {
        if(ui->boxGPIO2->currentText() == "SET")
        {
            value = GPIO_SET;
        }
        else
        {
            value = GPIO_RESET;
        }

        qDebug().noquote() << tr("Writing %1 to GPIO(%2)").arg(value).arg(log2(gpio));
        ret = bridge->SetResetGPIO(static_cast<uint8_t>(gpio), &value, &errorMask);
    }

    if(gpio == BRG_GPIO_3)
    {
        if(ui->boxGPIO3->currentText() == "SET")
        {
            value = GPIO_SET;
        }
        else
        {
            value = GPIO_RESET;
        }

        qDebug().noquote() << tr("Writing %1 to GPIO(%2)").arg(value).arg(log2(gpio));
        ret = bridge->SetResetGPIO(static_cast<uint8_t>(gpio), &value, &errorMask);
    }

    if(ret != BRG_NO_ERR)
    {
        QString msg;
        msg = tr("Error writing GPIO state to bridge, error: %1").arg(errorCodeToString(ret));

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }
    else
    {
        qDebug().noquote() << tr("Write Success! errorMask = %1").arg(errorMask);

        QString msg;
        msg = tr("Write success!");

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }
}

void bridgeGPIOWidget::handleWriteButtonClicked()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    if(sender()->objectName() == "buttonWriteGPIO0")
    {
        emit gpioToWrite(BRG_GPIO_0);
    }
    if(sender()->objectName() == "buttonWriteGPIO1")
    {
        emit gpioToWrite(BRG_GPIO_1);
    }
    if(sender()->objectName() == "buttonWriteGPIO2")
    {
        emit gpioToWrite(BRG_GPIO_2);
    }
    if(sender()->objectName() == "buttonWriteGPIO3")
    {
        emit gpioToWrite(BRG_GPIO_3);
    }
}

void bridgeGPIOWidget::readGPIO()
{
    qDebug().noquote() << tr("Entering") << tr(Q_FUNC_INFO);

    Brg_StatusT ret = BRG_NO_ERR;
    Brg_GpioValT gpioVals[BRG_GPIO_MAX_NB];
    quint8 errorMask;

    ret = bridge->ReadGPIO(BRG_GPIO_ALL, gpioVals, &errorMask);

    qDebug().noquote() << tr("GPIO0:") << gpioVals[0]
                       << tr("GPIO1:") << gpioVals[1]
                       << tr("GPIO2:") << gpioVals[2]
                       << tr("GPIO3:") << gpioVals[3];

    if(ret == BRG_NO_ERR)
    {
        // Check GPIO_0
        if(!(errorMask & BRG_GPIO_0))
        {
            if(gpioVals[0] == GPIO_SET)
            {
                ui->labelStateGPIO0->setText(tr("SET"));
            }
            else
            {
                ui->labelStateGPIO0->setText(tr("RESET"));
            }
        }
        else
        {
            ui->labelStateGPIO0->setText(tr("ERROR"));
        }

        // Check GPIO_1
        if(!(errorMask & BRG_GPIO_1))
        {
            if(gpioVals[1] == GPIO_SET)
            {
                ui->labelStateGPIO1->setText(tr("SET"));
            }
            else
            {
                ui->labelStateGPIO1->setText(tr("RESET"));
            }
        }
        else
        {
            ui->labelStateGPIO1->setText(tr("ERROR"));
        }

        // Check GPIO_2
        if(!(errorMask & BRG_GPIO_2))
        {
            if(gpioVals[2] == GPIO_SET)
            {
                ui->labelStateGPIO2->setText(tr("SET"));
            }
            else
            {
                ui->labelStateGPIO2->setText(tr("RESET"));
            }
        }
        else
        {
            ui->labelStateGPIO2->setText(tr("ERROR"));
        }

        // Check GPIO_3
        if(!(errorMask & BRG_GPIO_3))
        {
            if(gpioVals[3] == GPIO_SET)
            {
                ui->labelStateGPIO3->setText(tr("SET"));
            }
            else
            {
                ui->labelStateGPIO3->setText(tr("RESET"));
            }
        }
        else
        {
            ui->labelStateGPIO3->setText(tr("ERROR"));
        }

        QString msg;
        msg = tr("Read Success!");

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;
        emit postMessage(msg);
    }
    else
    {
        QString msg;
        msg = tr("Error Reading GPIO from bridge, error: %1").arg(errorCodeToString(ret));

        qWarning().noquote() << tr(Q_FUNC_INFO) << msg;

        emit postMessage(msg);
    }
}
