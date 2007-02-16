/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2006-01-20
 * Description : image editor GUI saving context container
 *
 * Copyright 2006-2007 by Gilles Caulier, Marcel Wiesweg
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

#ifndef SAVINGCONTEXTCONTAINER_H
#define SAVINGCONTEXTCONTAINER_H

// QT includes.

#include <qstring.h>

// KDE includes

#include <kurl.h>
#include <ktempfile.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SavingContextContainer
{

public:

    SavingContextContainer()
    {
        savingState             = SavingStateNone;
        synchronizingState      = NormalSaving;
        saveTempFile            = 0;
        destinationExisted      = false;
        synchronousSavingResult = false;
        abortingSaving          = false;
    }

    enum SavingState
    {
        SavingStateNone,
        SavingStateSave,
        SavingStateSaveAs
    };

    enum SynchronizingState
    {
        NormalSaving,
        SynchronousSaving
    };

    SavingState         savingState;
    SynchronizingState  synchronizingState;
    bool                synchronousSavingResult;
    bool                destinationExisted;
    bool                abortingSaving;

    QString             originalFormat;
    QString             format;

    KURL                srcURL;
    KURL                destinationURL;

    KTempFile          *saveTempFile;
};

} // namespace Digikam

#endif /* SAVINGCONTEXTCONTAINER_H */
