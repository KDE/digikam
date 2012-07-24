/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-06
 * Description : undo actions manager for image editor.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005      by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "undoaction.h"

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimginterface.h"

namespace Digikam
{

class UndoAction::UndoActionPriv
{
public:

    UndoActionPriv()
    {
    }

    QString        title;
    QVariant       fileOrigin;

    UndoMetadataContainer container;
    DImageHistory         fileOriginResolvedHistory;
};

UndoMetadataContainer UndoMetadataContainer::fromImage(const DImg& img)
{
    UndoMetadataContainer container;
    container.history = img.getImageHistory();
    container.profile = img.getIccProfile();
    return container;
}

void UndoMetadataContainer::toImage(DImg& img) const
{
    img.setImageHistory(history);
    img.setIccProfile(profile);
}

bool UndoMetadataContainer::changesIccProfile(const DImg& target) const
{
    return !(profile == target.getIccProfile());
}

UndoAction::UndoAction(DImgInterface* const iface)
    : d(new UndoActionPriv)
{
    d->container = UndoMetadataContainer::fromImage(*iface->getImg());
}

UndoAction::~UndoAction()
{
    delete d;
}

void UndoAction::setTitle(const QString& title)
{
    d->title = title;
}

QString UndoAction::getTitle() const
{
    return d->title;
}

void UndoAction::setMetadata(const UndoMetadataContainer& c)
{
    d->container = c;
}

UndoMetadataContainer UndoAction::getMetadata() const
{
    return d->container;
}

bool UndoAction::hasFileOriginData() const
{
    return !d->fileOrigin.isNull();
}

void UndoAction::setFileOriginData(const QVariant& data, const DImageHistory& resolvedInitialHistory)
{
    d->fileOrigin                = data;
    d->fileOriginResolvedHistory = resolvedInitialHistory;
}

QVariant UndoAction::fileOriginData() const
{
    return d->fileOrigin;
}

DImageHistory UndoAction::fileOriginResolvedHistory() const
{
    return d->fileOriginResolvedHistory;
}

// ---------------------------------------------------------------------------------------------

UndoActionReversible::UndoActionReversible(DImgInterface* const iface, const DImgBuiltinFilter& reversibleFilter)
    : UndoAction(iface), m_filter(reversibleFilter)
{
    setTitle(m_filter.i18nDisplayableName());
}

DImgBuiltinFilter UndoActionReversible::getFilter() const
{
    return m_filter;
}

DImgBuiltinFilter UndoActionReversible::getReverseFilter() const
{
    return m_filter.reverseFilter();
}

// ---------------------------------------------------------------------------------------------

UndoActionIrreversible::UndoActionIrreversible(DImgInterface* const iface, const QString& title)
    : UndoAction(iface)
{
    setTitle(title);
}

UndoActionIrreversible::~UndoActionIrreversible()
{
}

}  // namespace Digikam
