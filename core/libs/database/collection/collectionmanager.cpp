/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "collectionmanager_p.h"

// This is because of the CollectionManager private slot.
#include "moc_collectionmanager.cpp"

namespace Digikam
{

CollectionManager* CollectionManager::m_instance = 0;

CollectionManager* CollectionManager::instance()
{
    if (!m_instance)
    {
        m_instance = new CollectionManager;
    }

    return m_instance;
}

void CollectionManager::cleanUp()
{
    delete m_instance;
    m_instance = 0;
}

CollectionManager::CollectionManager()
    : d(new Private(this))
{
    qRegisterMetaType<CollectionLocation>("CollectionLocation");

    connect(Solid::DeviceNotifier::instance(),
            SIGNAL(deviceAdded(QString)),
            this,
            SLOT(deviceAdded(QString)));

    connect(Solid::DeviceNotifier::instance(),
            SIGNAL(deviceRemoved(QString)),
            this,
            SLOT(deviceRemoved(QString)));

    // CoreDbWatch slot is connected at construction of CoreDbWatch, which may be later.
}

CollectionManager::~CollectionManager()
{
    qDeleteAll(d->locations.values());
    delete d;
}

void CollectionManager::refresh()
{
    {
        // if called from the CoreDbAccess constructor itself, it will
        // hold a flag to prevent endless recursion
        CoreDbAccess access;
        clear_locked();
    }

    updateLocations();
}

void CollectionManager::setWatchDisabled()
{
    d->watchEnabled = false;
}

void CollectionManager::deviceAdded(const QString& udi)
{
    if (!d->watchEnabled)
    {
        return;
    }

    Solid::Device device(udi);

    if (device.is<Solid::StorageAccess>())
    {
        updateLocations();
    }
}

void CollectionManager::deviceRemoved(const QString& udi)
{
    if (!d->watchEnabled)
    {
        return;
    }

    // we can't access the Solid::Device to check because it is removed
    CoreDbAccess access;

    if (!d->udisToWatch.contains(udi))
    {
        return;
    }

    updateLocations();
}

void CollectionManager::accessibilityChanged(bool accessible, const QString& udi)
{
    Q_UNUSED(accessible);
    Q_UNUSED(udi);
    updateLocations();
}

void CollectionManager::clear_locked()
{
    // Internal method: Called with mutex locked
    // Cave: Difficult recursions with CoreDbAccess constructor and setParameters
    foreach(AlbumRootLocation* const location, d->locations)
    {
        CollectionLocation::Status oldStatus = location->status();
        location->setStatus(CollectionLocation::LocationDeleted);
        emit locationStatusChanged(*location, oldStatus);
        delete location;
    }

    d->locations.clear();
}

} // namespace Digikam
