// Copyright SME GmbH
// Open source license to be defined. GPL2?

#ifndef EBMBUS_H
#define EBMBUS_H

#include "ebmbus_global.h"
#include <QObject>
#include <QtSerialPort/QSerialPort>

class EBMBUSSHARED_EXPORT EbmBus : public QObject
{
    Q_OBJECT
public:
    explicit EbmBus(QObject *parent, QString interface);
    ~EbmBus();

    bool open();
    void close();

    typedef enum {
        GetStatus = 0x00,
        GetActualSpeed = 0x01,
        SetSetpoint = 0x02,
        SoftwareReset = 0x03,
        Diagnosis = 0x05,
        EEPROMwrite = 0x06,
        EEPROMread = 0x07
    } Command;

    // High level access
    void getSimpleStatus(quint8 fanAddress, quint8 fanGroup);
    void getStatus(quint8 fanAddress, quint8 fanGroup, quint8 statusAddress);
    void getActualSpeed(quint8 fanAddress, quint8 fanGroup);
    void setSpeedSetpoint(quint8 fanAddress, quint8 fanGroup, quint8 speed);
    void softwareReset(quint8 fanAddress, quint8 fanGroup);
    void diagnosis(quint8 fanAddress, quint8 fanGroup, quint8 c, quint16 a, QByteArray d);
    void writeEEPROM(quint8 fanAddress, quint8 fanGroup, quint8 eepromAddress, quint8 eepromData);
    void readEEPROM(quint8 fanAddress, quint8 fanGroup, quint8 eepromAddress);

    // Low level access
    void writeTelegramRaw(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);
    void writeTelegram(Command command, quint8 fanAddress, quint8 fanGroup, QByteArray data, bool servicebit = false);

private:
    QString m_interface;
    QSerialPort* m_port;
    QByteArray m_readBuffer;

    bool m_transactionPending;

    void tryToParseResponse(QByteArray *buffer);

signals:
    void signal_response(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);

public slots:

private slots:
    void slot_readyRead();
};

#endif // EBMBUS_H
