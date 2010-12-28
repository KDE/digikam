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

UndoAction::UndoAction(DImgInterface* iface)
    : m_iface(iface)
{
    m_history = iface->getImageHistory();
}

UndoAction::~UndoAction()
{
}

QString UndoAction::getTitle() const
{
    return m_title;
}

void UndoAction::setHistory(const DImageHistory& history)
{
    m_history = history;
}

DImageHistory UndoAction::getHistory() const
{
    return m_history;
}

bool UndoAction::hasFileOriginData()
{
    return !m_fileOrigin.isNull();
}

void UndoAction::setFileOriginData(const QVariant& data, const DImageHistory& resolvedInitialHistory)
{
    m_fileOrigin = data;
    m_fileOriginResolvedHistory = resolvedInitialHistory;
}

QVariant UndoAction::fileOriginData() const
{
    return m_fileOrigin;
}

DImageHistory UndoAction::fileOriginResolvedHistory() const
{
    return m_fileOriginResolvedHistory;
}

// ---------------------------------------------------------------------------------------------

UndoActionReversible::UndoActionReversible(DImgInterface* iface, const DImgBuiltinFilter& reversibleFilter)
    : UndoAction(iface), m_filter(reversibleFilter)
{
    m_title = m_filter.i18nDisplayableName();
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
    m_title = title;
}

UndoActionIrreversible::~UndoActionIrreversible()
{
}

}  // namespace Digikam
