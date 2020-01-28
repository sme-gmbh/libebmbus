/**********************************************************************
** libebmbus - a library to control ebm papst fans with ebmbus
** Copyright (C) 2018 Smart Micro Engineering GmbH, Peter Diener
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "ebmbus.h"

EbmBus::EbmBus(QObject *parent, QString interface_startOfLoop, QString interface_endOfLoop) : QObject(parent)
{
    m_interface_startOfLoop = interface_startOfLoop;
    m_interface_endOfLoop = interface_endOfLoop;
    m_port_startOfLoop = new QSerialPort(interface_startOfLoop, this);
    if (!interface_endOfLoop.isEmpty())
        m_port_endOfLoop = new QSerialPort(interface_endOfLoop, this);
    else
        m_port_endOfLoop = nullptr;
    m_transactionPending = false;
    m_currentTelegram = nullptr;
    m_dci_telegramID = 0;

    m_dci_currentSerialNumber_byte_0 = 0;
    m_dci_currentSerialNumber_byte_1 = 0;
    m_dci_currentSerialNumber_byte_2 = 0;

//    connect(this, SIGNAL(signal_transactionFinished()), this, SLOT(slot_tryToSendNextTelegram()));

    m_dciClear = false;
    m_dciTimer.setInterval(200);
    connect(&m_dciTimer, SIGNAL(timeout()), this, SLOT(slot_dciTask()));

    // This timer notifies about a telegram timeout if a unit does not answer
    m_requestTimer.setSingleShot(true);
    m_requestTimer.setInterval(200);
    connect(&m_requestTimer, SIGNAL(timeout()), this, SLOT(slot_requestTimer_fired()));

    // This timer delays tx after rx to wait for line clearance
    m_delayTxTimer.setSingleShot(true);
    m_delayTxTimer.setInterval(10);
    connect(this, SIGNAL(signal_transactionFinished()), &m_delayTxTimer, SLOT(start()));
    connect(&m_delayTxTimer, SIGNAL(timeout()), this, SLOT(slot_tryToSendNextTelegram()));
}

EbmBus::~EbmBus()
{
    if (this->isOpen())
        this->close();
    delete m_port_startOfLoop;
    delete m_port_startOfLoop;
}

bool EbmBus::open()
{
    bool openFailed = true;

    if (m_port_startOfLoop != nullptr)
    {
        m_port_startOfLoop->setBaudRate(QSerialPort::Baud9600);
        m_port_startOfLoop->setDataBits(QSerialPort::Data8);
        m_port_startOfLoop->setParity(QSerialPort::NoParity);
        m_port_startOfLoop->setStopBits(QSerialPort::OneStop);
        m_port_startOfLoop->setFlowControl(QSerialPort::NoFlowControl);
        connect(m_port_startOfLoop, SIGNAL(readyRead()), this, SLOT(slot_readyRead_startOfLoop()));
        openFailed = !m_port_startOfLoop->open(QIODevice::ReadWrite);
        m_port_startOfLoop->setBreakEnabled(false);
        m_port_startOfLoop->setTextModeEnabled(false);
    }

    if (m_port_endOfLoop != nullptr)
    {
        m_port_endOfLoop->setBaudRate(QSerialPort::Baud9600);
        m_port_endOfLoop->setDataBits(QSerialPort::Data8);
        m_port_endOfLoop->setParity(QSerialPort::NoParity);
        m_port_endOfLoop->setStopBits(QSerialPort::OneStop);
        m_port_endOfLoop->setFlowControl(QSerialPort::NoFlowControl);
        connect(m_port_endOfLoop, SIGNAL(readyRead()), this, SLOT(slot_readyRead_endOfLoop()));
        openFailed |= !m_port_endOfLoop->open(QIODevice::ReadWrite);
        m_port_endOfLoop->setBreakEnabled(false);
        m_port_endOfLoop->setTextModeEnabled(false);
    }

    return !openFailed;
}

bool EbmBus::isOpen()
{
    if (m_port_startOfLoop != nullptr)
    {
        if (m_port_startOfLoop->isOpen())
            return true;
    }

    if (m_port_endOfLoop != nullptr)
    {
        if (m_port_endOfLoop->isOpen())
            return true;
    }

    return false;
}

void EbmBus::close()
{
    if (m_port_startOfLoop != nullptr)
    {
        if (m_port_startOfLoop->isOpen())
            m_port_startOfLoop->close();
    }
    if (m_port_endOfLoop != nullptr)
    {
        if (m_port_endOfLoop->isOpen())
            m_port_endOfLoop->close();
    }
}

// High level access, these functions return the telegram id that can be compared to receives messages to identify the sender

quint64 EbmBus::getSimpleStatus(quint8 fanAddress, quint8 fanGroup)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusCommand::GetStatus, fanAddress, fanGroup, QByteArray()));
}

quint64 EbmBus::getStatus(quint8 fanAddress, quint8 fanGroup, EbmBusStatus::StatusAddress statusAddress)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusCommand::GetStatus, fanAddress, fanGroup, QByteArray(1, statusAddress)));
}

quint64 EbmBus::getActualSpeed(quint8 fanAddress, quint8 fanGroup)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusCommand::GetActualSpeed, fanAddress, fanGroup, QByteArray()));
}

quint64 EbmBus::setSpeedSetpoint(quint8 fanAddress, quint8 fanGroup, quint8 speed)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusCommand::SetSetpoint, fanAddress, fanGroup, QByteArray(1, speed)));
}

quint64 EbmBus::softwareReset(quint8 fanAddress, quint8 fanGroup)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusCommand::SoftwareReset, fanAddress, fanGroup, QByteArray()));
}

quint64 EbmBus::diagnosis(quint8 fanAddress, quint8 fanGroup, quint8 c, quint16 a, QByteArray d)
{
    Q_UNUSED(fanAddress)
    Q_UNUSED(fanGroup)
    Q_UNUSED(c)
    Q_UNUSED(a)
    Q_UNUSED(d)
    // tbd.
    return 0;
}

// Write EEPROM data must be committed by software reset (except setpoint, p, i, d controller parameters and reduction factor)
quint64 EbmBus::writeEEPROM(quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 eepromData)
{
    QByteArray payload;

    payload.append(eepromAddress);
    payload.append(eepromData);

    return writeTelegramToQueue(new EbmBusTelegram(EbmBusCommand::EEPROMwrite, fanAddress, fanGroup, payload));
}

quint64 EbmBus::readEEPROM(quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress)
{
    return writeTelegramToQueue(new EbmBusTelegram(EbmBusCommand::EEPROMread, fanAddress, fanGroup, QByteArray(1, eepromAddress)));
}

void EbmBus::startDaisyChainAddressing()
{
    m_dci_fanAddress = 2;
    m_dci_groupAddress = 1; // Starting at 2 (preincrement)!
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

int EbmBus::getSizeOfTelegramQueue()
{
    m_telegramQueueMutex.lock();
    return m_telegramQueue.length();
    m_telegramQueueMutex.unlock();
}

void EbmBus::slot_tryToSendNextTelegram()
{
    m_telegramQueueMutex.lock();
    // Delete last telegram if it exists
    // If repeat counter is not zero, then repeat current telegram, otherwise take new
    // telegram from the queue
    if ((m_currentTelegram != nullptr) && (m_currentTelegram->repeatCount == 0))
    {
        delete m_currentTelegram;
        m_currentTelegram = nullptr;
    }

    if (m_currentTelegram == nullptr)
    {
        if (m_telegramQueue.isEmpty())
        {
            m_telegramQueueMutex.unlock();
            return;
        }
        m_currentTelegram = m_telegramQueue.takeFirst();
    }

    m_requestTimer.start();
    m_telegramQueueMutex.unlock();

    writeTelegramNow(m_currentTelegram);
}

quint64 EbmBus::writeTelegramToQueue(EbmBusTelegram *telegram)
{
    quint64 telegramID = telegram->getID();
    m_telegramQueueMutex.lock();
    m_telegramQueue.append(telegram);

    if (!m_requestTimer.isActive()) // If we inserted the first packet, we have to start the sending process
    {
        m_telegramQueueMutex.unlock();
        slot_tryToSendNextTelegram();
    }
    else
        m_telegramQueueMutex.unlock();

    return telegramID;
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

    if (m_port_startOfLoop->isOpen())
    {
        m_port_startOfLoop->write(out);
        m_port_startOfLoop->flush();
    }
}

quint64 EbmBus::writeTelegramNow(EbmBusTelegram* telegram)
{
    EbmBusCommand::Command command = telegram->command;
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
    bool senderEcho = (preamble & 0x04);

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
        if (!senderEcho)
        {
            m_requestTimer.stop();
            emit signal_responseRaw(m_currentTelegram->getID(), preamble, commandAndFanaddress, fanGroup, data);
            parseResponse(m_currentTelegram->getID(), preamble, commandAndFanaddress, fanGroup, data);
            emit signal_transactionFinished();
        }
        else
            emit signal_senderEchoReceived();   // Todo: decide to repeat telegram from the other side of the loop
    }

    buffer->clear();
}

void EbmBus::parseResponse(quint64 id, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data)
{
    Q_UNUSED(preamble)

    quint8 command = (commandAndFanaddress >> 5) & 0x07;
    quint8 fanAddress = commandAndFanaddress & 0x1f;

    switch (command) {
    case EbmBusCommand::GetStatus:
        if (data.length() == 1) // This is the old version of status response using only one byte without addressable status registers
        {
            QString str;
            quint8 value = data.at(0);

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

            emit signal_simpleStatus(id, fanAddress, fanGroup, str.trimmed());
        }
        else if (data.length() == 2)
        {
            QString str;
            quint8 value = data.at(0);
            quint8 statusAddress = data.at(1);

            switch (statusAddress)
            {
            case EbmBusStatus::MotorStatusLowByte:
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
            case EbmBusStatus::MotorStatusHighByte:
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
            case EbmBusStatus::Warnings:
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
            case EbmBusStatus::DCvoltage:
                str = QString().sprintf("DC voltage raw value: %i", value); // Reference at EEPROM-ADR C7 (MSB) + C8 (LSB), get these first for further calculation
                break;
            case EbmBusStatus::DCcurrent:
                str = QString().sprintf("DC current raw value: %i", value); // Reference at EEPROM-ADR C9 (MSB) + CA (LSB), get these first for further calculation
                break;
            case EbmBusStatus::TemperatureOfPowerModule:
                str = QString().sprintf("Power module temperature: %i °C", value);
                break;
            case EbmBusStatus::SetPoint:
                str = QString().sprintf("Target speed setpoint raw: %i", value);
                break;
            case EbmBusStatus::ActualValue:
                str = QString().sprintf("Actual speed raw: %i", value);     // Note that identification == 9 type motors do not provide this data
                break;
            case EbmBusStatus::ModeOfControl:
                if (value == 0)
                    str = "Open loop PWM control";
                else if (value == 1)
                    str = "Closed loop rpm control";
                else if (value == 2)
                    str = "Closed loop sensor control";
                else
                    str = "EbmBusTelegram::ModeOfControl unkown mode";
                break;
            case EbmBusStatus::DirectionOfRotation:
                if (value == 0)
                    str = "Rotation counter clock wise";
                if (value == 1)
                    str = "Rotation clock wise";
                break;
            case EbmBusStatus::PWMdutyCycle:
                str = QString().sprintf("PWM duty cycle: %i", value);
                break;
            case EbmBusStatus::SteppingSwitch_1_2:
                str = "Request type not supported yet.";
                break;
            case EbmBusStatus::SteppingSwitch_3_4:
                str = "Request type not supported yet.";
                break;
            case EbmBusStatus::TemperatureOfMotor:
                str = QString().sprintf("Motor temperature: %i °C", value);
                break;
            case EbmBusStatus::LineVoltage:
                str = "Request type not supported yet.";
                break;
            case EbmBusStatus::LineCurrent:
                str = "Request type not supported yet.";
                break;
            case EbmBusStatus::MaxVolumetricFlowRate:
                str = "Request type not supported yet.";
                break;
            case EbmBusStatus::MinVolumericFlowRate:
                str = "Request type not supported yet.";
                break;
            case EbmBusStatus::MaxPressure:
                str = "Request type not supported yet.";
                break;
            case EbmBusStatus::MinPressure:
                str = "Request type not supported yet.";
                break;
            case EbmBusStatus::ElectronicBoxTemperature:
                str = QString().sprintf("Electronic box temperature: %i °C", value);
                break;
            case EbmBusStatus::EEPROMchecksumLSB:
                str = QString().sprintf("EEPROM checksum LSB: %i", value);
                break;
            case EbmBusStatus::EEPROMchecksumMSB:
                str = QString().sprintf("EEPROM checksum MSB: %i", value);
                break;
            default:
                str = "Unsupported status address in request.";
            }

            emit signal_status(id, fanAddress, fanGroup, statusAddress, str.trimmed(), value);
        }
        break;
    case EbmBusCommand::GetActualSpeed:
        if (data.length() == 1)
        {
            quint8 actualRawSpeed = data.at(0);
            emit signal_actualSpeed(id, fanAddress, fanGroup, actualRawSpeed);
        }
        break;
    case EbmBusCommand::SetSetpoint:
        emit signal_setPointHasBeenSet(id, fanAddress, fanGroup);
        break;
    case EbmBusCommand::SoftwareReset:
        break;
    case EbmBusCommand::Diagnosis:
        break;
    case EbmBusCommand::EEPROMwrite:
        emit signal_EEPROMhasBeenWritten(id, fanAddress, fanGroup);
        break;
    case EbmBusCommand::EEPROMread:
        if (data.length() == 2)
        {
            quint8 dataByte = data.at(0);
            quint8 dataAddress = data.at(1);
            emit signal_EEPROMdata(id, fanAddress, fanGroup, (EbmBusEEPROM::EEPROMaddress)dataAddress, dataByte);
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
        disconnect(this, SIGNAL(signal_EEPROMdata(quint64,quint8,quint8,EbmBusEEPROM::EEPROMaddress,quint8)),
                   this, SLOT(slot_dciReceivedEEPROMdata(quint64,quint8,quint8,EbmBusEEPROM::EEPROMaddress,quint8)));
    }
}

void EbmBus::slot_readyRead_startOfLoop()
{
    if (m_port_startOfLoop == nullptr)
        return;

    while (!m_port_startOfLoop->atEnd())
    {
        //m_readBuffer += m_port->read(1);
        char c;
        m_port_startOfLoop->getChar(&c);
        m_readBuffer_startOfLoop.append(c);

        tryToParseResponseRaw(&m_readBuffer_startOfLoop);
    }
}

void EbmBus::slot_readyRead_endOfLoop()
{
    if (m_port_endOfLoop == nullptr)
        return;

    while (!m_port_endOfLoop->atEnd())
    {
        //m_readBuffer += m_port->read(1);
        char c;
        m_port_endOfLoop->getChar(&c);
        m_readBuffer_endOfLoop.append(c);

        tryToParseResponseRaw(&m_readBuffer_endOfLoop); // Need to switch this from normal return way to redundancy in some way
    }
}

void EbmBus::slot_dciTask()
{
    switch (m_dciState)
    {
    case Idle:
        m_dci_fanAddress = 2; // Todo: dynamic addressing by user selectable address table
        //groupaddress =
        connect(this, SIGNAL(signal_EEPROMdata(quint64,quint8,quint8,EbmBusEEPROM::EEPROMaddress,quint8)),
                this, SLOT(slot_dciReceivedEEPROMdata(quint64,quint8,quint8,EbmBusEEPROM::EEPROMaddress,quint8)));
        m_dciState = OutLow_1;
        break;
    case OutLow_1:
        emit signal_setDCIoutput(false);
        m_dciState = RelaisOff;
        break;
    case RelaisOff:
        m_dci_telegramID = writeEEPROM(0, 0, EbmBusEEPROM::DCIrelais, 0);
        m_dciState = OutHigh;
        break;
    case OutHigh:
        emit signal_setDCIoutput(true);
        m_dciState = AddressingGA;
        break;
    case AddressingGA:
        if (m_dciClear)
            m_dci_telegramID = writeEEPROM(0, 0, EbmBusEEPROM::FanGroupAddress, 1);
        else
            m_dci_telegramID = writeEEPROM(0, 0, EbmBusEEPROM::FanGroupAddress, ++m_dci_groupAddress);
        m_dciState = AddressingFA;
        break;
    case AddressingFA:
        if (m_dciClear)
            m_dci_telegramID = writeEEPROM(0, 0, EbmBusEEPROM::FanAddress, 1);
        else
            m_dci_telegramID = writeEEPROM(0, 0, EbmBusEEPROM::FanAddress, m_dci_fanAddress);
        m_dciState = ReadSerialnumber_1;
        break;
    case ReadSerialnumber_1:
        m_dci_telegramID = readEEPROM(0, 0, EbmBusEEPROM::SerialNumber_Byte_2);
        m_dciState = ReadSerialnumber_2;
        break;
    case ReadSerialnumber_2:
        m_dci_telegramID = readEEPROM(0, 0, EbmBusEEPROM::SerialNumber_Byte_1);
        m_dciState = ReadSerialnumber_3;
        break;
    case ReadSerialnumber_3:
        m_dci_telegramID = readEEPROM(0, 0, EbmBusEEPROM::SerialNumber_Byte_0);
        m_dciState = RelaisOn;
        break;
    case RelaisOn:
        m_dci_telegramID = writeEEPROM(0, 0, EbmBusEEPROM::DCIrelais, 0xff);
        m_dciState = OutLow_2;
        break;
    case OutLow_2:
        emit signal_setDCIoutput(false);
        m_dciState = OutHigh;
        break;
    }
}

void EbmBus::slot_dciReceivedEEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte)
{
    Q_UNUSED(fanAddress)
    Q_UNUSED(fanGroup)

    // Check if that telegram has been sent by our dci job, otherwise ignore it
    if (telegramID != m_dci_telegramID)
        return;

    if (eepromAddress == EbmBusEEPROM::SerialNumber_Byte_2)
        m_dci_currentSerialNumber_byte_2 = dataByte;
    else if (eepromAddress == EbmBusEEPROM::SerialNumber_Byte_1)
        m_dci_currentSerialNumber_byte_1 = dataByte;
    else if (eepromAddress == EbmBusEEPROM::SerialNumber_Byte_0)
    {
        m_dci_currentSerialNumber_byte_0 = dataByte;

        quint32 serialNumber = ((m_dci_currentSerialNumber_byte_2 << 16) | (m_dci_currentSerialNumber_byte_1 << 8) | m_dci_currentSerialNumber_byte_0);
        emit signal_DaisyChainAddressingGotSerialNumber(m_dci_groupAddress - 2, m_dci_fanAddress, m_dci_groupAddress, serialNumber);
    }
}

void EbmBus::slot_requestTimer_fired()
{
    if (m_currentTelegram->needsAnswer())
    {
        emit signal_transactionLost(m_currentTelegram->getID());
    }
    emit signal_transactionFinished();
}

