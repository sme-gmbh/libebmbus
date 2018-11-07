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

#ifndef EBMBUSEEPROM_H
#define EBMBUSEEPROM_H


class EbmBusEEPROM
{
public:

    typedef enum {
        FanGroupAddress = 0x00,
        FanAddress = 0x01,
        OperationModes_1 = 0x02,
        SetTargetValue = 0x03,

        Controller_P_factor = 0x05,
        Controller_I_factor = 0x06,
        Controller_D_factor = 0x07,
        MaxSpeed_MSB = 0x08,
        MaxSpeed_Mid = 0x09,
        MaxSpeed_LSB = 0x0A,
        DutyCycleMax = 0x0B,
        DutyCycleMin = 0x0C,
        DutyCycleStart = 0x0D,
        TargetValue_0 = 0x0E,
        TargetValue_1 = 0x0F,

        OperationModes_2 = 0x10,
        RatingFactor = 0x11,

        // Sensor values coded in IEEE754
        SensorValueMin_Byte_1 = 0x18,
        SensorValueMin_Byte_2 = 0x19,
        SensorValueMin_Byte_3 = 0x1A,
        SensorValueMin_Byte_4 = 0x1B,
        SensorValueMax_Byte_1 = 0x1C,
        SensorValueMax_Byte_2 = 0x1D,
        SensorValueMax_Byte_3 = 0x1E,
        SensorValueMax_Byte_4 = 0x1F,

        PhysicalUnitOfMeasuredQuantity_Char_01 = 0x20,
        PhysicalUnitOfMeasuredQuantity_Char_02 = 0x21,
        PhysicalUnitOfMeasuredQuantity_Char_03 = 0x22,
        PhysicalUnitOfMeasuredQuantity_Char_04 = 0x23,
        PhysicalUnitOfMeasuredQuantity_Char_05 = 0x24,
        PhysicalUnitOfMeasuredQuantity_Char_06 = 0x25,
        PhysicalUnitOfMeasuredQuantity_Char_07 = 0x26,
        PhysicalUnitOfMeasuredQuantity_Char_08 = 0x27,
        PhysicalUnitOfMeasuredQuantity_Char_09 = 0x28,
        PhysicalUnitOfMeasuredQuantity_Char_10 = 0x29,

        // Conversion factor is coded in IEEE754
        ConversionFactor_Byte_1 = 0x2A,
        ConversionFactor_Byte_2 = 0x2B,
        ConversionFactor_Byte_3 = 0x2C,
        ConversionFactor_Byte_4 = 0x2D,

        EEPROMstatus = 0x40,
        MotorDesign_NumberOfPoles = 0x41,
        MotorDesign_Function = 0x42,
        DutyCycleMaxAdmissible = 0x43,

        DutyCycleMinAdmissible = 0x49,
        Identification = 0x4A,

        FailureDisplay_N0_HighByte = 0x6D,
        FailureDisplay_N1_HighByte = 0x6E,
        FailureDisplay_N2_HighByte = 0x6F,
        FailureDisplay_N0_LowByte = 0x70,
        FailureDisplay_N1_LowByte = 0x71,
        FailureDisplay_N2_LowByte = 0x72,

        OperationHours_MSB = 0x73,
        OperationHours_LSB = 0x74,

        ManufacturingDateCode_Day = 0x80,
        ManufacturingDateCode_Month = 0x81,
        ManufacturingDateCode_Year = 0x82,

        SerialNumber_Byte_2 = 0x83,
        SerialNumber_Byte_1 = 0x84,
        SerialNumber_Byte_0 = 0x85,

        DClinkCurrentMax = 0x86,
        AmbientTemperatureMax = 0x87,

        // Ramps are defined in 10 ms steps for one increment of setPoint value
        // E.g. a value of 20 means increment one step every 200 ms.
        RampUpAccelerationRate = 0x95,
        RampDownDecelerationRate = 0x96,

        DClinkVoltageMin = 0x97,
        LineVoltageMin = 0x98,

        // DCIrelais is undocumented in the official EEPROM address reference, be careful!
        // It is used for daisy chain addressing.
        DCIrelais = 0x9d,

        ReferenceDClinkVoltage_MSB = 0xC7,
        ReferenceDClinkVoltage_LSB = 0xC8,
        ReferenceDClinkCurrent_MSB = 0xC9,
        ReferenceDClinkCurrent_LSB = 0xCA,
        ReferenceAClineVoltage_MSB = 0xCB,
        ReferenceAClineVoltage_LSB = 0xCC,
        ReferenceAClineCurrent_MSB = 0xCD,
        ReferenceAClineCurrent_LSB = 0xCE,

        ActualOperationMode = 0xFB,
        ActualMaxPWMdutyCycle = 0xFC,
        ActualMinPWMdutyCycle = 0xFD,
        ActualTargetValue = 0xFE,
        ActualSensorValue = 0xFF
    } EEPROMaddress;

    EbmBusEEPROM();
};

#endif // EBMBUSEEPROM_H
