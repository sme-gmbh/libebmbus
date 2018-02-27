#include "ebmbus.h"

EbmBus::EbmBus(QObject *parent, QString interface) : QObject(parent)
{
    m_interface = interface;
    m_port = new QSerialPort(interface, this);
    m_transactionPending = false;
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

// High level access

void EbmBus::getSimpleStatus(quint8 fanAddress, quint8 fanGroup)
{
    writeTelegram(EbmBus::GetStatus, fanAddress, fanGroup, QByteArray());
}

void EbmBus::getStatus(quint8 fanAddress, quint8 fanGroup, quint8 statusAddress)
{
    writeTelegram(EbmBus::GetStatus, fanAddress, fanGroup, QByteArray(1, statusAddress));
}

void EbmBus::getActualSpeed(quint8 fanAddress, quint8 fanGroup)
{
    writeTelegram(EbmBus::GetActualSpeed, fanAddress, fanGroup, QByteArray());
}

void EbmBus::setSpeedSetpoint(quint8 fanAddress, quint8 fanGroup, quint8 speed)
{
    writeTelegram(EbmBus::SetSetpoint, fanAddress, fanGroup, QByteArray(1, speed));
}

void EbmBus::softwareReset(quint8 fanAddress, quint8 fanGroup)
{
    writeTelegram(EbmBus::SoftwareReset, fanAddress, fanGroup, QByteArray());
}

void EbmBus::diagnosis(quint8 fanAddress, quint8 fanGroup, quint8 c, quint16 a, QByteArray d)
{
    // tbd.
}

// Write EEPROM data must be committed by software reset (except setpoint, p, i, d controller parameters and reduction factor)
void EbmBus::writeEEPROM(quint8 fanAddress, quint8 fanGroup, quint8 eepromAddress, quint8 eepromData)
{
    QByteArray payload;

    payload.append(eepromAddress);
    payload.append(eepromData);

    writeTelegram(EbmBus::EEPROMwrite, fanAddress, fanGroup, payload);
}

void EbmBus::readEEPROM(quint8 fanAddress, quint8 fanGroup, quint8 eepromAddress)
{
    writeTelegram(EbmBus::EEPROMread, fanAddress, fanGroup, QByteArray(1, eepromAddress));
}


// Low level access

void EbmBus::writeTelegramRaw(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data)
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

    m_transactionPending = true;
    m_port->write(out);
    m_port->waitForBytesWritten(100);
}

void EbmBus::writeTelegram(EbmBus::Command command, quint8 fanAddress, quint8 fanGroup, QByteArray data, bool servicebit)
{
    // Cut off bits that may not be set by address
    fanAddress &= 0x1f;

    // Append command to address
    fanAddress |= (command << 5) & 0xe0;

    // Construct preamble
    quint8 dataLength = data.size();
    if (dataLength > 7)
        return;

    quint8 preamble = 0x15;
    preamble |= dataLength << 5;
    if (servicebit)
        preamble |= 0x08;

    writeTelegramRaw(preamble, fanAddress, fanGroup, data);
}

void EbmBus::tryToParseResponse(QByteArray* buffer)
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
        m_transactionPending = false;
        emit signal_response(preamble, commandAndFanaddress, fanGroup, data);
    }

    buffer->clear();
}

void EbmBus::slot_readyRead()
{
    while (!m_port->atEnd())
    {
        m_readBuffer += m_port->read(1);

        tryToParseResponse(&m_readBuffer);
    }
}

