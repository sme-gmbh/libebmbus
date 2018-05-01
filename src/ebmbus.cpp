// Copyright SME GmbH
// Open source license to be defined. GPL2?

#include "ebmbus.h"

EbmBus::EbmBus(QObject *parent, QString interface) : QObject(parent)
{
    m_interface = interface;
    m_port = new QSerialPort(interface, this);
    m_transactionPending = false;
    m_currentTelegram = NULL;

    connect(this, SIGNAL(signal_transactionFinished()), this, SLOT(slot_tryToSendNextTelegram()));

    m_dciClear = false;
    m_dciTimer.setInterval(200);
    connect(&m_dciTimer, SIGNAL(timeout()), this, SLOT(slot_dciTask()));

    // This timer notifies about a telegram timeout if a unit does not answer
    m_requestTimer.setSingleShot(true);
    m_requestTimer.setInterval(200);
    connect(&m_requestTimer, SIGNAL(timeout()), this, SLOT(slot_requestTimer_fired()));
}

EbmBus::~EbmBus()
{
    if (m_port->isOpen())
        this->close();
    delete m_port;
}

bool EbmBus::open()
{
    m_port->setBaudRate(QSerialPort::Baud9600);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    connect(m_port, SIGNAL(readyRead()), this, SLOT(slot_readyRead()));
    return m_port->open(QIODevice::ReadWrite);
}

void EbmBus::close()
{
    if (m_port->isOpen())
        m_port->close();
}

// High level access, these functions return the telegram id that can be compared to receives messages to identify the sender

quint64 EbmBus::getSimpleStatus(quint8 fanAddress, quint8 fanGroup)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusTelegram::GetStatus, fanAddress, fanGroup, QByteArray()));
}

quint64 EbmBus::getStatus(quint8 fanAddress, quint8 fanGroup, quint8 statusAddress)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusTelegram::GetStatus, fanAddress, fanGroup, QByteArray(1, statusAddress)));
}

quint64 EbmBus::getActualSpeed(quint8 fanAddress, quint8 fanGroup)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusTelegram::GetActualSpeed, fanAddress, fanGroup, QByteArray()));
}

quint64 EbmBus::setSpeedSetpoint(quint8 fanAddress, quint8 fanGroup, quint8 speed)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusTelegram::SetSetpoint, fanAddress, fanGroup, QByteArray(1, speed)));
}

quint64 EbmBus::softwareReset(quint8 fanAddress, quint8 fanGroup)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusTelegram::SoftwareReset, fanAddress, fanGroup, QByteArray()));
}

quint64 EbmBus::diagnosis(quint8 fanAddress, quint8 fanGroup, quint8 c, quint16 a, QByteArray d)
{
    Q_UNUSED(fanAddress);
    Q_UNUSED(fanGroup);
    Q_UNUSED(c);
    Q_UNUSED(a);
    Q_UNUSED(d);
    // tbd.
    return 0;
}

// Write EEPROM data must be committed by software reset (except setpoint, p, i, d controller parameters and reduction factor)
quint64 EbmBus::writeEEPROM(quint8 fanAddress, quint8 fanGroup, quint8 eepromAddress, quint8 eepromData)
{
    QByteArray payload;

    payload.append(eepromAddress);
    payload.append(eepromData);

    return writeTelegramToQueue(new EbmBusTelegram(EbmBusTelegram::EEPROMwrite, fanAddress, fanGroup, payload));
}

quint64 EbmBus::readEEPROM(quint8 fanAddress, quint8 fanGroup, quint8 eepromAddress)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusTelegram::EEPROMread, fanAddress, fanGroup, QByteArray(1, eepromAddress)));
}

void EbmBus::startDaisyChainAddressing()
{
    m_serialnumbers.clear();
    m_dciState = Idle;
    m_dciTimer.start();
}

void EbmBus::clearAllAddresses()
{
    m_dciClear = true;
    m_serialnumbers.clear();
    m_dciState = Idle;
    m_dciTimer.start();
}

bool EbmBus::isDaisyChainInProgress()
{
    if (m_dciTimer.isActive())
        return true;
    else
        return false;
}

void EbmBus::slot_tryToSendNextTelegram()
{
    if (m_telegramQueue.isEmpty())
        return;

    // Delete last telegram if it exists
    // If repeat counter is not zero, then repeat current telegram, otherwise take new
    // telegram from the queue
    if ((m_currentTelegram != NULL) && (m_currentTelegram->repeatCount == 0))
        delete m_currentTelegram;
    else
        m_currentTelegram = m_telegramQueue.takeFirst();

    m_transactionPending = true;
    m_requestTimer.start();
    writeTelegramNow(m_currentTelegram);
}

quint64 EbmBus::writeTelegramToQueue(EbmBusTelegram *telegram)
{
    m_telegramQueue.append(telegram);

    if (!m_transactionPending)
        slot_tryToSendNextTelegram();

    return telegram->getID();
}


// Low level access

void EbmBus::writeTelegramRawNow(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data)
{
    QByteArray out;

    out.append(preamble);
    out.append(commandAndFanaddress);
    out.append(fanGroup);
    out.append(data);

    // Checksum calculation
    quint8 cs = 0xff;

    foreach(quint8 byte, out)
    {
        cs ^= byte;
    }

    out.append(cs);

    m_port->write(out);
    m_port->waitForBytesWritten(100);
}

quint64 EbmBus::writeTelegramNow(EbmBusTelegram* telegram)
{
    EbmBusTelegram::Command command = telegram->command;
    quint8 commandAndFanaddress = telegram->fanAddress;
    quint8 fanGroup = telegram->fanGroup;
    QByteArray data = telegram->data;
    bool servicebit = telegram->servicebit;
    telegram->repeatCount--;

    // Cut off bits that may not be set by address
    commandAndFanaddress &= 0x1f;

    // Append command to address
    commandAndFanaddress |= (command << 5) & 0xe0;

    // Construct preamble
    quint8 dataLength = data.size();
    if (dataLength > 7)
        return 0;

    quint8 preamble = 0x15;
    preamble |= dataLength << 5;
    if (servicebit)
        preamble |= 0x08;

    writeTelegramRawNow(preamble, commandAndFanaddress, fanGroup, data);
    return telegram->getID();
}

void EbmBus::tryToParseResponseRaw(QByteArray* buffer)
{
    if (buffer->size() < 4)
        return;

    quint8 preamble = buffer->at(0);
    quint8 commandAndFanaddress = buffer->at(1);
    quint8 fanGroup = buffer->at(2);

    quint8 dataLength = preamble >> 5;

    if (buffer->size() < 3 + dataLength + 1)
        return;

    QByteArray data = buffer->mid(3, dataLength);

    quint8 cs = 0xff;

    foreach(quint8 byte, buffer->mid(0, 3 + dataLength + 1))
    {
        cs ^= byte;
    }

    if (cs == 0)    // checksum ok
    {
        m_requestTimer.stop();
        m_transactionPending = false;
        emit signal_responseRaw(m_currentTelegram->getID(), preamble, commandAndFanaddress, fanGroup, data);
        parseResponse(m_currentTelegram->getID(), preamble, commandAndFanaddress, fanGroup, data);
        emit signal_transactionFinished();
    }

    buffer->clear();
}

void EbmBus::parseResponse(quint64 id, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data)
{
    Q_UNUSED(preamble);

    quint8 command = (commandAndFanaddress >> 5) & 0x08;
    quint8 fanAddress = commandAndFanaddress & 0x1f;

    switch (command) {
    case EbmBusTelegram::GetStatus:
        if (data.length() == 1)
        {
            QString status;

            emit signal_simpleStatus(id, fanAddress, fanGroup, status);
        }
        else if (data.length() == 2)
        {
            QString str;
            quint8 value = data.at(0);
            quint8 statusAddress = data.at(1);

            switch (statusAddress)
            {
            case EbmBusTelegram::MotorStatusLowByte:
                if (value & 0x80)
                    str += "Blocked Motor; ";
                if (value & 0x40)
                    str += "Hall sensor failure; ";
                if (value & 0x20)
                    str += "Thermal overload of motor; ";
                if (value & 0x10)
                    str += "Fan-Bad - common error; ";
                if (value & 0x08)
                    str += "Communication error between Master-PIC and Slave-PIC; ";
                if (value & 0x04)
                    str += "Thermal overload of electronic power module; ";
                if (value & 0x02)
                    str += "Communication error with remote unit; ";
                if (value & 0x01)
                    str += "Phase failure; ";

                if (str.isEmpty())
                    str = "No errors in low byte status.";
                break;
            case EbmBusTelegram::MotorStatusHighByte:
                if (value & 0x80)
                    str += "Brake error; ";
                if (value & 0x40)
                    str += "Unknown error - high byte 0x40; ";
                if (value & 0x20)
                    str += "Low line voltage; ";
                if (value & 0x10)
                    str += "Low DC-link voltage; ";
                if (value & 0x08)
                    str += "High DC-link voltage; ";
                if (value & 0x04)
                    str += "Driver problem; ";
                if (value & 0x02)
                    str += "Electronic box overheat; ";
                if (value & 0x01)
                    str += "Excessive DC-link current; ";

                if (str.isEmpty())
                    str = "No errors in high byte status.";
                break;
            case EbmBusTelegram::Warnings:
                if (value & 0x80)
                    str += "Unknown warning - 0x80; ";
                if (value & 0x40)
                    str += "Low DC-link voltage; ";
                if (value & 0x20)
                    str += "Electronic box high temperature; ";
                if (value & 0x10)
                    str += "Motor high temperature; ";
                if (value & 0x08)
                    str += "Electronic power module high temperature; ";
                if (value & 0x04)
                    str += "Unknown warning - 0x04; ";
                if (value & 0x02)
                    str += "Set value not reached; ";
                if (value & 0x01)
                    str += "DC-link current limit; ";

                if (str.isEmpty())
                    str = "No warnings.";
                break;
            case EbmBusTelegram::DCvoltage:
                str = QString().sprintf("DC voltage raw value: %i", value); // Reference at EEPROM-ADR C7 (MSB) + C8 (LSB), get these first for further calculation
                break;
            case EbmBusTelegram::DCcurrent:
                str = QString().sprintf("DC current raw value: %i", value); // Reference at EEPROM-ADR C9 (MSB) + CA (LSB), get these first for further calculation
                break;
            case EbmBusTelegram::TemperatureOfPowerModule:
                str = QString().sprintf("Power module temperature: %i °C", value);
                break;
            case EbmBusTelegram::SetPoint:
                str = QString().sprintf("Target speed setpoint raw: %i", value);
                break;
            case EbmBusTelegram::ActualValue:
                str = QString().sprintf("Actual speed raw: %i", value);     // Note that identification == 9 type motors do not provide this data
                break;
            case EbmBusTelegram::ModeOfControl:
                if (value == 0)
                    str = "Open loop PWM control";
                else if (value == 1)
                    str = "Closed loop rpm control";
                else if (value == 2)
                    str = "Closed loop sensor control";
                else
                    str = "EbmBusTelegram::ModeOfControl unkown mode";
                break;
            case EbmBusTelegram::DirectionOfRotation:
                if (value == 0)
                    str = "Rotation counter clock wise";
                if (value == 1)
                    str = "Rotation clock wise";
                break;
            case EbmBusTelegram::PWMdutyCycle:
                str = QString().sprintf("PWM duty cycle: %i", value);
                break;
            case EbmBusTelegram::SteppingSwitch_1_2:
                str = "Request type not supported yet.";
                break;
            case EbmBusTelegram::SteppingSwitch_3_4:
                str = "Request type not supported yet.";
                break;
            case EbmBusTelegram::TemperatureOfMotor:
                str = QString().sprintf("Motor temperature: %i °C", value);
                break;
            case EbmBusTelegram::LineVoltage:
                str = "Request type not supported yet.";
                break;
            case EbmBusTelegram::LineCurrent:
                str = "Request type not supported yet.";
                break;
            case EbmBusTelegram::MaxVolumetricFlowRate:
                str = "Request type not supported yet.";
                break;
            case EbmBusTelegram::MinVolumericFlowRate:
                str = "Request type not supported yet.";
                break;
            case EbmBusTelegram::MaxPressure:
                str = "Request type not supported yet.";
                break;
            case EbmBusTelegram::MinPressure:
                str = "Request type not supported yet.";
                break;
            case EbmBusTelegram::ElectronicBoxTemperature:
                str = QString().sprintf("Electronic box temperature: %i °C", value);
                break;
            case EbmBusTelegram::EEPROMchecksumLSB:
                str = QString().sprintf("EEPROM checksum LSB: %i", value);
                break;
            case EbmBusTelegram::EEPROMchecksumMSB:
                str = QString().sprintf("EEPROM checksum MSB: %i", value);
                break;
            default:
                str = "Unsupported status address in request.";
            }

            emit signal_status(id, fanAddress, fanGroup, statusAddress, str, value);
        }
        break;
    case EbmBusTelegram::GetActualSpeed:
        if (data.length() == 1)
        {
            quint8 actualRawSpeed = data.at(0);
            emit signal_actualSpeed(id, fanAddress, fanGroup, actualRawSpeed);
        }
        break;
    case EbmBusTelegram::SetSetpoint:
        emit signal_setPointHasBeenSet(id, fanAddress, fanGroup);
        break;
    case EbmBusTelegram::SoftwareReset:
        break;
    case EbmBusTelegram::Diagnosis:
        break;
    case EbmBusTelegram::EEPROMwrite:
        emit signal_EEPROMhasBeenWritten(id, fanAddress, fanGroup);
        break;
    case EbmBusTelegram::EEPROMread:
        if (data.length() == 1)
        {
            quint8 dataByte = data.at(0);
            emit signal_EEPROMdata(id, fanAddress, fanGroup, dataByte);
        }
        break;
    }
}

void EbmBus::slot_DCIloopResponse(bool on)
{
    if (on) // Adressing finished
    {
        m_dciTimer.stop();
        m_dciState = Idle;
        m_dciClear = false;
        emit signal_DaisyChainAdressingFinished();
    }
}

void EbmBus::slot_readyRead()
{
    while (!m_port->atEnd())
    {
        m_readBuffer += m_port->read(1);

        tryToParseResponseRaw(&m_readBuffer);
    }
}

void EbmBus::slot_dciTask()
{
    int groupaddress = 2;
    static int fanaddress = 2;

    switch (m_dciState)
    {
    case Idle:
        fanaddress = 2; // Todo: dynamic addressing by user selectable address table
        m_dciState = OutLow_1;
        break;
    case OutLow_1:
        emit signal_setDCIoutput(false);
        m_dciState = RelaisOff;
        break;
    case RelaisOff:
        writeEEPROM(0, 0, 0x9d, 0);
        m_dciState = OutHigh;
        break;
    case OutHigh:
        emit signal_setDCIoutput(true);
        m_dciState = AddressingGA;
        break;
    case AddressingGA:
        if (m_dciClear)
            writeEEPROM(0, 0, 0x00, 1);
        else
            writeEEPROM(0, 0, 0x00, groupaddress);
        m_dciState = AddressingFA;
        break;
    case AddressingFA:
        if (m_dciClear)
            writeEEPROM(0, 0, 0x01, 1);
        else
            writeEEPROM(0, 0, 0x01, fanaddress++);
        m_dciState = ReadSerialnumber_1;
        break;
    case ReadSerialnumber_1:
        readEEPROM(0, 0, 0x83);
        m_dciState = ReadSerialnumber_2;
        break;
    case ReadSerialnumber_2:
        readEEPROM(0, 0, 0x84);
        m_dciState = ReadSerialnumber_3;
        break;
    case ReadSerialnumber_3:
        readEEPROM(0, 0, 0x85);
        m_dciState = RelaisOn;
        break;
    case RelaisOn:
        writeEEPROM(0, 0, 0x9d, 0xff);
        m_dciState = OutLow_2;
        break;
    case OutLow_2:
        emit signal_setDCIoutput(false);
        m_dciState = OutHigh;
        break;
    }
}

void EbmBus::slot_requestTimer_fired()
{
    if (m_currentTelegram->needsAnswer)
    {
        emit signal_transactionLost(m_currentTelegram->getID());
    }
    slot_tryToSendNextTelegram();
}

