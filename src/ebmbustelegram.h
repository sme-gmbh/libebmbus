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

#ifndef EBMBUSTELEGRAM_H
#define EBMBUSTELEGRAM_H

#include <QByteArray>

#include "ebmbuscommand.h"
#include "ebmbusstatus.h"
#include "ebmbuseeprom.h"


class EbmBusTelegram
{
public:
    EbmBusTelegram(int repeatCount = 1);
    EbmBusTelegram(EbmBusCommand::Command command, quint8 fanAddress, quint8 fanGroup, QByteArray data, bool servicebit = false, int repeatCount = 1);

    EbmBusCommand::Command command;
    quint8 fanAddress;
    quint8 fanGroup;
    QByteArray data;
    bool servicebit;

    int m_repeatCount;    // Set to different value if that telegram is important and should be autorepeated

    bool needsAnswer();

    quint64 getID();

private:
    quint64 m_id; // Telegram id is unique accross all telegrams per bus
};

#endif // EBMBUSTELEGRAM_H
