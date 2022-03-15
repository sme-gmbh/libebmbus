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

#ifndef EBMBUS_GLOBAL_H
#define EBMBUS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EBMBUS_LIBRARY)
#  define EBMBUSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EBMBUSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBEBMBUS_GLOBAL_H
