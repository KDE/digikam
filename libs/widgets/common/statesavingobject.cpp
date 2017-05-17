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

#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class StateSavingObject::Private
{
public:

    Private() :
        host(0),
        group(),
        prefix(),
        groupSet(false),
        depth(StateSavingObject::INSTANCE)
    {
    }

    inline KConfigGroup getGroupFromObjectName()
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();

        if (host->objectName().isEmpty())
        {
            qCWarning(DIGIKAM_WIDGETS_LOG) << "Object name for " << host
                                           << " is empty. Returning the default config group";
        }

        return config->group(host->objectName());
    }

    void recurse(const QObjectList& children, const bool save)
    {
        for (QObjectList::const_iterator childIt = children.constBegin();
             childIt != children.constEnd(); ++childIt)
        {
            StateSavingObject* const statefulChild = dynamic_cast<StateSavingObject*>(*childIt);

            if (statefulChild)
            {
                // if the child implements the interface, it can be save /
                // restored

                // but before invoking these actions, avoid duplicate calls to
                // the methods of deeper children
                const StateSavingObject::StateSavingDepth oldState = statefulChild->getStateSavingDepth();
                statefulChild->setStateSavingDepth(StateSavingObject::INSTANCE);

                // decide which action to invoke
                if (save)
                {
                    statefulChild->saveState();
                }
                else
                {
                    statefulChild->loadState();
                }

                statefulChild->setStateSavingDepth(oldState);
            }

            // recurse children every time
            recurse((*childIt)->children(), save);
        }
    }

    void recurseOperation(const bool save)
    {
        QString action = QLatin1String("loading");

        if (save)
        {
            action = QLatin1String("saving");
        }

        if (depth == StateSavingObject::DIRECT_CHILDREN)
        {
            //qCDebug(DIGIKAM_WIDGETS_LOG) << "Also restoring " << action << " of direct children";
            for (QObjectList::const_iterator childIt = host->children().begin();
                 childIt != host->children().end(); ++childIt)
            {
                StateSavingObject* const statefulChild = dynamic_cast<StateSavingObject*>(*childIt);

                if (statefulChild)
                {
                    if (save)
                    {
                        statefulChild->saveState();
                    }
                    else
                    {
                        statefulChild->loadState();
                    }
                }
            }
        }
        else if (depth == StateSavingObject::RECURSIVE)
        {
            //qCDebug(DIGIKAM_WIDGETS_LOG) << "Also " << action << " state of all children (recursive)";
            recurse(host->children(), save);
        }
    }

public:

    QObject*                            host;
    KConfigGroup                        group;
    QString                             prefix;
    bool                                groupSet;
    StateSavingObject::StateSavingDepth depth;
};

StateSavingObject::StateSavingObject(QObject* const host) :
    d(new Private)
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

StateSavingObject::StateSavingDepth StateSavingObject::getStateSavingDepth() const
{
    return d->depth;
}

void StateSavingObject::setStateSavingDepth(const StateSavingObject::StateSavingDepth depth)
{
    d->depth = depth;
}

void StateSavingObject::setConfigGroup(const KConfigGroup& group)
{
    //qCDebug(DIGIKAM_WIDGETS_LOG) << "received new config group: " << group.name();
    d->group    = group;
    d->groupSet = true;
}

void StateSavingObject::setEntryPrefix(const QString& prefix)
{
    d->prefix = prefix;
}

void StateSavingObject::loadState()
{
    //qCDebug(DIGIKAM_WIDGETS_LOG) << "Loading state";

    doLoadState();

    d->recurseOperation(false);
}

void StateSavingObject::saveState()
{
    //qCDebug(DIGIKAM_WIDGETS_LOG) << "Saving state";

    doSaveState();

    d->recurseOperation(true);
}

KConfigGroup StateSavingObject::getConfigGroup() const
{
    if (!d->groupSet)
    {
        //qCDebug(DIGIKAM_WIDGETS_LOG) << "No config group set, returning one based on object name";
        return d->getGroupFromObjectName();
    }

    if (!d->group.isValid())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "KConfigGroup set via setConfigGroup is invalid. "
                                       << "Using object name based group.";
        return d->getGroupFromObjectName();
    }

    return d->group;
}

QString StateSavingObject::entryName(const QString& base) const
{
    return d->prefix + base;
}

} // namespace Digikam
