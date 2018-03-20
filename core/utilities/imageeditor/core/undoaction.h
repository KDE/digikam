/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-06
 * Description : undo actions manager for image editor.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <klocalizedstring.h>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace Digikam
{

class EditorCore;

class DIGIKAM_EXPORT UndoMetadataContainer
{
public:

    /** Fill a container from the DImg
     */
    static UndoMetadataContainer fromImage(const DImg& img);

    /** Write this container's values to the DImg
     */
    void toImage(DImg& img) const;

    bool changesIccProfile(const DImg& target) const;

public:

    DImageHistory  history;
    IccProfile     profile;
};

// -------------------------------------------------------------------

class DIGIKAM_EXPORT UndoAction
{

public:

    explicit UndoAction(EditorCore* const core);
    virtual ~UndoAction();

    void                  setTitle(const QString& title);
    QString               getTitle()                  const;

    void                  setMetadata(const UndoMetadataContainer&);
    UndoMetadataContainer getMetadata()               const;

    void                  setFileOriginData(const QVariant& data, const DImageHistory& resolvedInitialHistory);
    bool                  hasFileOriginData()         const;
    QVariant              fileOriginData()            const;
    DImageHistory         fileOriginResolvedHistory() const;

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT UndoActionReversible : public UndoAction
{

public:

    UndoActionReversible(EditorCore* const core, const DImgBuiltinFilter& reversibleFilter);

    DImgBuiltinFilter getFilter()        const;
    DImgBuiltinFilter getReverseFilter() const;

protected:

    DImgBuiltinFilter m_filter;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT UndoActionIrreversible : public UndoAction
{

public:

    explicit UndoActionIrreversible(EditorCore* const core, const QString& caller = i18n("Unknown"));
    ~UndoActionIrreversible();
};

}  // namespace Digikam

#endif /* UNDOACTION_H */
