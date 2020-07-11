#ifndef BRIDGEGPIOWIDGET_H
#define BRIDGEGPIOWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QIntValidator>
#include <QStatusBar>

#include "bridge.h"
#include "bridgewidget.h"

namespace Ui {
class bridgeGPIOWidget;
}

class bridgeGPIOWidget : public bridgeWidget
{
    Q_OBJECT

public:
    explicit bridgeGPIOWidget(Brg *p_brg);
    ~bridgeGPIOWidget();

public slots:
    void handleDisconnect();
    void setBase(int base);
    Brg_StatusT getGPIOConfig(Brg_GpioMaskT gpio);
    Brg_StatusT setGPIOConfig();
    void getNewGpioSelection();
    void readGPIO();
    void handleReadCheckBox(int state);
    void handleReadIntervalEdit(const QString &text);
    void writeGPIO(Brg_GpioMaskT gpio);
    void handleWriteButtonClicked();

signals:
    void gpioSelectionChanged(Brg_GpioMaskT newGpio);
    void gpioToWrite(Brg_GpioMaskT gpio);

private:
    Ui::bridgeGPIOWidget *ui;
    Brg_GpioConfT gpioConf[BRG_GPIO_MAX_NB];
    QTimer *readTimer;
    QIntValidator *readLineEditValidator;
    quint8 gpioStates = 0x00;
};

#endif // BRIDGEGPIOWIDGET_H
