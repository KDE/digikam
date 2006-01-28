/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at free.fr>
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2006-01-20
 * Description : image editor GUI saving context container
 *
 * Copyright 2006 by Gilles Caulier, Marcel Wiesweg
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

namespace Digikam
{

class SavingContextContainer
{

public:

    SavingContextContainer()
    {
        savingState             = SavingStateNone;
        synchronizingState      = NormalSaving;
        saveTempFile            = 0;
        fileExists              = false;
        synchronousSavingResult = false;
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

    SavingState              savingState;
    SynchronizingState       synchronizingState;
    bool                     synchronousSavingResult;
    bool                     fileExists;
    
    QString                  format;
    QString                  tmpFile;

    KURL                     currentURL;
    KURL                     saveURL;
    KURL                     saveAsURL;

    KTempFile               *saveTempFile;
};

} // namespace Digikam

#endif /* SAVINGCONTEXTCONTAINER_H */
