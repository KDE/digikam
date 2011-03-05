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

    explicit UndoAction(DImgInterface* iface);
    virtual ~UndoAction();

    QString getTitle() const;

    void setHistory(const DImageHistory& history);
    DImageHistory getHistory() const;

    bool hasFileOriginData();
    void setFileOriginData(const QVariant& data, const DImageHistory& resolvedInitialHistory);
    QVariant fileOriginData() const;
    DImageHistory fileOriginResolvedHistory() const;

protected:

    DImgInterface* m_iface;
    QString        m_title;
    DImageHistory  m_history;
    QVariant       m_fileOrigin;
    DImageHistory  m_fileOriginResolvedHistory;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT UndoActionReversible : public UndoAction
{

public:

    UndoActionReversible(DImgInterface* iface, const DImgBuiltinFilter& reversibleFilter);

    DImgBuiltinFilter getFilter() const;
    DImgBuiltinFilter getReverseFilter() const;

protected:

    DImgBuiltinFilter m_filter;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT UndoActionIrreversible : public UndoAction
{

public:

    explicit UndoActionIrreversible(DImgInterface* iface,
                                    const QString& caller=i18n("Unknown"));
    ~UndoActionIrreversible();
};

}  // namespace Digikam

#endif /* UNDOACTION_H */
