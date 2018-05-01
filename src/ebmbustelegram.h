#ifndef EBMBUSTELEGRAM_H
#define EBMBUSTELEGRAM_H

#include <QByteArray>

class EbmBusTelegram
{
public:

    typedef enum {
        GetStatus = 0x00,
        GetActualSpeed = 0x01,
        SetSetpoint = 0x02,
        SoftwareReset = 0x03,
        Diagnosis = 0x05,
        EEPROMwrite = 0x06,
        EEPROMread = 0x07
    } Command;

    typedef enum {
        MotorStatusLowByte = 0x00,
        MotorStatusHighByte = 0x01,
        Warnings = 0x02,
        DCvoltage = 0x03,
        DCcurrent = 0x04,
        TemperatureOfPowerModule = 0x05,
        SetPoint = 0x06,
        ActualValue = 0x07,
        ModeOfControl = 0x08,
        DirectionOfRotation = 0x09,
        PWMdutyCycle = 0x0B,
        SteppingSwitch_1_2 = 0x0E,
        SteppingSwitch_3_4 = 0x0F,
        TemperatureOfMotor = 0x10,
        LineVoltage = 0x11,
        LineCurrent = 0x13,
        MaxVolumetricFlowRate = 0x14,
        MinVolumericFlowRate = 0x15,
        MaxPressure = 0x16,
        MinPressure = 0x17,
        ElectronicBoxTemperature = 0x20,
        EEPROMchecksumLSB = 0x21,
        EEPROMchecksumMSB = 0x22
    } StatusAddress;


    EbmBusTelegram();
    EbmBusTelegram(Command command, quint8 fanAddress, quint8 fanGroup, QByteArray data, bool servicebit = false);

    Command command;
    quint8 fanAddress;
    quint8 fanGroup;
    QByteArray data;
    bool servicebit = false;

    int repeatCount = 1;    // Set to different value if that telegram is important and should be autorepeated

    bool needsAnswer;

    quint64 getID();

private:
    quint64 m_id; // Telegram id is unique accross all telegrams per bus
};

#endif // EBMBUSTELEGRAM_H
