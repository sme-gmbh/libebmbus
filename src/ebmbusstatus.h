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
