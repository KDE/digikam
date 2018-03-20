/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : Database engine hint data containers for Dbus
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DATABASE_ENGINE_DBUS_UTILITIES_H
#define DATABASE_ENGINE_DBUS_UTILITIES_H

class QDBusArgument;

#define DECLARE_METATYPE_FOR_DBUS(x)                                                \
Q_DECLARE_METATYPE(x)                                                               \
                                                                                    \
inline QDBusArgument& operator<<(QDBusArgument& argument, const x& changeset)       \
{                                                                                   \
    changeset >> argument;                                                          \
    return argument;                                                                \
}                                                                                   \
                                                                                    \
inline const QDBusArgument& operator>>(const QDBusArgument& argument, x& changeset) \
{                                                                                   \
    changeset << argument;                                                          \
    return argument;                                                                \
}

#endif // DATABASE_ENGINE_DBUS_UTILITIES_H
