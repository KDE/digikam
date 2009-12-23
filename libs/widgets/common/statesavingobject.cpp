/* ============================================================
 *
 * This file is a part of digikam project
 * http://www.digikam.org
 *
 * Date        : 2009-20-12
 * Description : Interface class for objects that can store their state.
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "statesavingobject.h"

// KDE includes

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <ksharedconfig.h>

namespace Digikam
{

class StateSavingObjectPriv
{
public:

    StateSavingObjectPriv() :
        host(0),
        groupSet(false)
    {
    }

    QObject *host;
    KConfigGroup group;
    QString prefix;
    bool groupSet;

    inline KConfigGroup getGroupFromObjectName()
    {
        KSharedConfig::Ptr config = KGlobal::config();
        if (host->objectName().isEmpty())
        {
            kWarning() << "Object name for " << host
                       << " is empty. Returning the default config group";
        }
        return config->group(host->objectName());
    }

};

StateSavingObject::StateSavingObject(QObject *host) :
                d(new StateSavingObjectPriv)
{
    d->host = host;
    // we cannot safely create the default config group here, because the host
    // may not have been properly initialized or its object name is set after
    // the constructor call
}

StateSavingObject::~StateSavingObject()
{
    delete d;
}

void StateSavingObject::setConfigGroup(KConfigGroup group)
{
    kDebug() << "received new config group: " << group.name();
    d->group = group;
    d->groupSet = true;
}

void StateSavingObject::setEntryPrefix(const QString &prefix)
{
    d->prefix = prefix;
}

void StateSavingObject::loadState()
{
    doLoadState();
}

void StateSavingObject::saveState()
{
    doSaveState();
}

KConfigGroup StateSavingObject::getConfigGroup()
{

    if (!d->groupSet)
    {
        kDebug() << "No config group set, returning one based on object name";
        return d->getGroupFromObjectName();
    }

    if (!d->group.isValid())
    {
        kWarning() << "KConfigGroup set via setConfigGroup is invalid. "
                   << "Using object name based group.";
        return d->getGroupFromObjectName();
    }

    return d->group;
}

QString StateSavingObject::entryName(const QString &base)
{
    return d->prefix + base;
}

}
