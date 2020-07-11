#ifndef BRIDGEWIDGET_H
#define BRIDGEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QStatusBar>

#include "bridge.h"

class bridgeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit bridgeWidget(Brg *p_brg, QWidget *parent = nullptr);

public slots:
    virtual void handleDisconnect();
    QString displayNumber(quint64 number, quint8 base);
    virtual void setBase(int base);

protected:
    Brg *bridge;
    QString errorCodeToString(Brg_StatusT code);

signals:
    void postMessage(QString msg);

private slots:

private:

};

#endif // BRIDGEWIDGET_H
