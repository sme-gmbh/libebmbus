#ifndef EBMBUSCOMMAND_H
#define EBMBUSCOMMAND_H


class EbmBusCommand
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

    EbmBusCommand();
};

#endif // EBMBUSCOMMAND_H
