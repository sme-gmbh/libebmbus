#ifndef EBMBUSSTATUS_H
#define EBMBUSSTATUS_H


class EbmBusStatus
{
public:

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

    EbmBusStatus();
};

#endif // EBMBUSSTATUS_H
