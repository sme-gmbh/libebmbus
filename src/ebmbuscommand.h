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
