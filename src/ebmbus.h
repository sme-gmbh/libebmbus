// Copyright SME GmbH
// Open source license to be defined. GPL2?

#ifndef EBMBUS_H
#define EBMBUS_H

#include "ebmbus_global.h"
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QStringList>
#include <QTimer>

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
    void startDaisyChainAddressing();
    bool isDaisyChainInProgress();

    // Low level access
    void writeTelegramRaw(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);
    void writeTelegram(Command command, quint8 fanAddress, quint8 fanGroup, QByteArray data, bool servicebit = false);

private:
    QString m_interface;
    QSerialPort* m_port;
    QByteArray m_readBuffer;
    QStringList m_serialnumbers;
    QTimer m_dciTimer;
    bool m_transactionPending;

    typedef enum {
        Idle = 0,
        OutLow_1 = 1,
        RelaisOff = 2,
        OutHigh = 3,
        AddressingGA = 4,
        AddressingFA = 5,
        ReadSerialnumber_1 = 6,
        ReadSerialnumber_2 = 7,
        ReadSerialnumber_3 = 8,
        RelaisOn = 9,
        OutLow_2 = 10
    } DCI_State;

    DCI_State m_dciState;

    void tryToParseResponse(QByteArray *buffer);

signals:
    void signal_response(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);
    void signal_setDCIoutput(bool on);
    void signal_DaisyChainAdressingFinished();

public slots:
    void slot_DCIloopResponse(bool on);

private slots:
    void slot_readyRead();
    void slot_dciTask();
};

#endif // EBMBUS_H
