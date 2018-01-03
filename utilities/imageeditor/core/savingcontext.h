/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : image editor GUI saving context container
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SAVING_CONTEXT_H
#define SAVING_CONTEXT_H

// Qt includes

#include <QString>
#include <QTemporaryFile>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "versionfileoperation.h"

namespace Digikam
{

class DIGIKAM_EXPORT SavingContext
{
public:

    enum SavingState
    {
        SavingStateNone,
        SavingStateSave,
        SavingStateSaveAs,
        SavingStateVersion
    };

    enum SynchronizingState
    {
        NormalSaving,
        SynchronousSaving
    };

public:

    SavingContext()
    {
        savingState             = SavingStateNone;
        synchronizingState      = NormalSaving;
        saveTempFile            = 0;
        destinationExisted      = false;
        synchronousSavingResult = false;
        abortingSaving          = false;
        executedOperation       = SavingStateNone;
    }

public:

    SavingState          savingState;
    SynchronizingState   synchronizingState;
    bool                 synchronousSavingResult;
    bool                 destinationExisted;
    bool                 abortingSaving;
    SavingState          executedOperation;

    QString              originalFormat;
    QString              format;

    QUrl                 srcURL;
    QUrl                 destinationURL;
    QUrl                 moveSrcURL;

    QTemporaryFile*      saveTempFile;
    QString              saveTempFileName;

    VersionFileOperation versionFileOperation;
};

} // namespace Digikam

#endif /* SAVING_CONTEXT_H */
