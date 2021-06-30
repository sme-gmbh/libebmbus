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

#include "ebmbustelegram.h"

EbmBusTelegram::EbmBusTelegram(int repeatCount)
{
    static quint64 id = 1;  // Start counting telegram id with 1. 0 is reserved for error
    this->m_id = id;
    id++;
    if (id == 0)            // Wrap around and avoid 0 - if that might ever happen with quint64... - just to be correct.
        id = 1;

    m_repeatCount = repeatCount;
    servicebit = false;
}

EbmBusTelegram::EbmBusTelegram(EbmBusCommand::Command command, quint8 fanAddress, quint8 fanGroup, QByteArray data, int repeatCount, bool servicebit)
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
    if (this->needsAnswer())
        m_repeatCount = repeatCount;
    else
        m_repeatCount = 1;
}

bool EbmBusTelegram::needsAnswer()
{
    if ((this->fanAddress == 0) || (this->fanGroup == 0))
        return false;
    else
        return true;
}

quint64 EbmBusTelegram::getID()
{
    return m_id;
}
