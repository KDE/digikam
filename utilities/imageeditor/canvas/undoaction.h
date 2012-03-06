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

#ifndef UNDOACTION_H
#define UNDOACTION_H

// KDE includes

#include <klocale.h>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"
#include "dimgbuiltinfilter.h"

namespace Digikam
{

class DImgInterface;

class DIGIKAM_EXPORT UndoAction
{

public:

    explicit UndoAction(DImgInterface* const iface);
    virtual ~UndoAction();

    void          setTitle(const QString& title);
    QString       getTitle() const;

    void          setHistory(const DImageHistory& history);
    DImageHistory getHistory() const;

    bool          hasFileOriginData() const;
    void          setFileOriginData(const QVariant& data, const DImageHistory& resolvedInitialHistory);
    QVariant      fileOriginData() const;
    DImageHistory fileOriginResolvedHistory() const;

private:

    class UndoActionPriv;
    UndoActionPriv* const d;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT UndoActionReversible : public UndoAction
{

public:

    UndoActionReversible(DImgInterface* const iface, const DImgBuiltinFilter& reversibleFilter);

    DImgBuiltinFilter getFilter() const;
    DImgBuiltinFilter getReverseFilter() const;

protected:

    DImgBuiltinFilter m_filter;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT UndoActionIrreversible : public UndoAction
{

public:

    explicit UndoActionIrreversible(DImgInterface* const iface,
                                    const QString& caller=i18n("Unknown"));
    ~UndoActionIrreversible();
};

}  // namespace Digikam

#endif /* UNDOACTION_H */
