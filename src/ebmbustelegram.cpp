#include "ebmbustelegram.h"

EbmBusTelegram::EbmBusTelegram()
{
    static quint64 id = 1;  // Start counting telegram id with 1. 0 is reserved for error
    this->m_id = id;
    id++;
    if (id == 0)            // Wrap around and avoid 0 - if that might ever happen with quint64... - just to be correct.
        id = 1;

    repeatCount = 1;
    servicebit = false;
    needsAnswer = false;
}

EbmBusTelegram::EbmBusTelegram(EbmBusCommand::Command command, quint8 fanAddress, quint8 fanGroup, QByteArray data, bool servicebit)
{
    static quint64 id = 1;  // Start counting telegram id with 1. 0 is reserved for error
    this->m_id = id;
    id++;
    if (id == 0)            // Wrap around and avoid 0 - if that might ever happen with quint64... - just to be correct.
        id = 1;

    this->command = command;
    this->fanAddress = fanAddress;
    this->fanGroup = fanGroup;
    this->data = data;
    this->servicebit = servicebit;

    needsAnswer = false;
}

quint64 EbmBusTelegram::getID()
{
    return m_id;
}
