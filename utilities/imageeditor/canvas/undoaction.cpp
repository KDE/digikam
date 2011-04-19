/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-06
 * Description : undo actions manager for image editor.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
        iface = 0;
    }

    QString        title;
    QVariant       fileOrigin;
    DImageHistory  history;
    DImageHistory  fileOriginResolvedHistory;
    DImgInterface* iface;
};

UndoAction::UndoAction(DImgInterface* iface)
    : d(new UndoActionPriv)
{
    d->iface   = iface;
    d->history = iface->getImageHistory();
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

void UndoAction::setHistory(const DImageHistory& history)
{
    d->history = history;
}

DImageHistory UndoAction::getHistory() const
{
    return d->history;
}

bool UndoAction::hasFileOriginData()
{
    return !d->fileOrigin.isNull();
}

void UndoAction::setFileOriginData(const QVariant& data, const DImageHistory& resolvedInitialHistory)
{
    d->fileOrigin = data;
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

UndoActionReversible::UndoActionReversible(DImgInterface* iface, const DImgBuiltinFilter& reversibleFilter)
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

UndoActionIrreversible::UndoActionIrreversible(DImgInterface* iface, const QString& title)
    : UndoAction(iface)
{
    setTitle(title);
}

UndoActionIrreversible::~UndoActionIrreversible()
{
}

}  // namespace Digikam
