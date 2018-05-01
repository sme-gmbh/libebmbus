#ifndef EBMBUSTELEGRAM_H
#define EBMBUSTELEGRAM_H

#include <QByteArray>

#include "ebmbuscommand.h"
#include "ebmbusstatus.h"
#include "ebmbuseeprom.h"


class EbmBusTelegram
{
public:
    EbmBusTelegram();
    EbmBusTelegram(EbmBusCommand::Command command, quint8 fanAddress, quint8 fanGroup, QByteArray data, bool servicebit = false);

    EbmBusCommand::Command command;
    quint8 fanAddress;
    quint8 fanGroup;
    QByteArray data;
    bool servicebit;

    int repeatCount;    // Set to different value if that telegram is important and should be autorepeated

    bool needsAnswer();

    quint64 getID();

private:
    quint64 m_id; // Telegram id is unique accross all telegrams per bus
};

#endif // EBMBUSTELEGRAM_H
