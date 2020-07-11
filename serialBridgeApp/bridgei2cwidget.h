#ifndef BRIDGEI2CWIDGET_H
#define BRIDGEI2CWIDGET_H

#include <QWidget>
#include <QStatusBar>
#include <QIntValidator>

#include "bridge.h"
#include "bridgewidget.h"

#define I2C_BUFFER_SIZE 4096

namespace Ui {
class bridgeI2CWidget;
}

class bridgeI2CWidget : public bridgeWidget
{
    Q_OBJECT

public:
    explicit bridgeI2CWidget(Brg *p_brg);
    ~bridgeI2CWidget();

public slots:
    void handleDisconnect();
    void setBase(int base);
    void onSpeedSelectionChange(int index);
    void onDigitalFilterChange(int state);
    void onReadWriteSelectChange(int selection);
    void onSendButtonClicked();
    Brg_StatusT setI2CConfig();

private:
    Ui::bridgeI2CWidget *ui;
    Brg_I2cInitT i2cConfig;
    QIntValidator *riseTimeValidator;
    QIntValidator *fallTimeValidator;
    int numberBase;
};

#endif // BRIDGEI2CWIDGET_H
