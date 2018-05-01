// Copyright SME GmbH
// Open source license to be defined. GPL2?

#ifndef EBMBUS_H
#define EBMBUS_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QStringList>
#include <QTimer>
#include <QList>

#include "ebmbus_global.h"
#include "ebmbustelegram.h"


class EBMBUSSHARED_EXPORT EbmBus : public QObject
{
    Q_OBJECT
public:
    explicit EbmBus(QObject *parent, QString interface);
    ~EbmBus();

    bool open();
    void close();

    // High level access
    quint64 getSimpleStatus(quint8 fanAddress, quint8 fanGroup);
    quint64 getStatus(quint8 fanAddress, quint8 fanGroup, quint8 statusAddress);
    quint64 getActualSpeed(quint8 fanAddress, quint8 fanGroup);
    quint64 setSpeedSetpoint(quint8 fanAddress, quint8 fanGroup, quint8 speed);
    quint64 softwareReset(quint8 fanAddress, quint8 fanGroup);
    quint64 diagnosis(quint8 fanAddress, quint8 fanGroup, quint8 c, quint16 a, QByteArray d);
    quint64 writeEEPROM(quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 eepromData);
    quint64 readEEPROM(quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress);
    void startDaisyChainAddressing();
    void clearAllAddresses();
    bool isDaisyChainInProgress();

    // Low level access; writes to queue that is fed to the byte level access layer
    // Returns the assigned telegram id, which is unique
    quint64 writeTelegramToQueue(EbmBusTelegram* telegram);

private:
    QString m_interface;
    QSerialPort* m_port;
    QByteArray m_readBuffer;
    QStringList m_serialnumbers;
    QTimer m_dciTimer;
    QTimer m_requestTimer;
    bool m_dciClear;    // If this bit is set, DCI addressing will set (1,1) for all addresses (set all to factory default)
    quint8 m_dci_currentSerialNumber_byte_0;
    quint8 m_dci_currentSerialNumber_byte_1;
    quint8 m_dci_currentSerialNumber_byte_2;
    quint64 m_dci_telegramID;
    int m_dci_groupAddress;
    int m_dci_fanAddress;

    bool m_transactionPending;
    QList<EbmBusTelegram*> m_telegramQueue;
    EbmBusTelegram* m_currentTelegram;

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

    // Low level access; writes immediately to the bus
    quint64 writeTelegramNow(EbmBusTelegram* telegram);
    void writeTelegramRawNow(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);
    void tryToParseResponseRaw(QByteArray *buffer);
    void parseResponse(quint64 id, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);

signals:
    void signal_responseRaw(quint64 telegramID, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);
    void signal_transactionFinished();
    void signal_transactionLost(quint64 id);
    void signal_setDCIoutput(bool on);
    void signal_DaisyChainAddressingGotSerialNumber(quint8 fanAddress, quint8 fanGroup, quint32 serialNumber);
    void signal_DaisyChainAdressingFinished();

    // High level response signals
    void signal_simpleStatus(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, QString status);
    void signal_status(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 statusAddress, QString status, quint8 rawValue);
    void signal_actualSpeed(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 actualRawSpeed);
    void signal_setPointHasBeenSet(quint64 telegramID, quint8 fanAddress, quint8 fanGroup);
    void signal_EEPROMhasBeenWritten(quint64 telegramID, quint8 fanAddress, quint8 fanGroup);
    void signal_EEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte);
    // Todo: Implement more high level response signals

public slots:
    void slot_DCIloopResponse(bool on);

private slots:
    void slot_tryToSendNextTelegram();
    void slot_readyRead();
    void slot_dciTask();
    void slot_dciReceivedEEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte);
    void slot_requestTimer_fired();
};

#endif // EBMBUS_H
