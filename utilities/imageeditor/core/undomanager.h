/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-06
 * Description : an image editor actions undo/redo manager
 *
 * Copyright (C) 2005-2006 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
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

#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

// Qt includes

#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImageHistory;
class DImg;
class EditorCore;
class UndoAction;
class UndoMetadataContainer;

class DIGIKAM_EXPORT UndoManager
{

public:

    explicit UndoManager(EditorCore* const core);
    ~UndoManager();

    void addAction(UndoAction* const action);
    void undo();
    void redo();
    void rollbackToOrigin();
    bool putImageDataAndHistory(DImg* const img, int stepsBack) const;

    void clear(bool clearCache = true);

    bool          anyMoreUndo()               const;
    bool          anyMoreRedo()               const;
    int           availableUndoSteps()        const;
    int           availableRedoSteps()        const;
    QStringList   getUndoHistory()            const;
    QStringList   getRedoHistory()            const;
    bool          isAtOrigin()                const;
    void          setOrigin()                 const;
    bool          hasChanges()                const;

    /// The history if all available redo steps are redone
    DImageHistory getImageHistoryOfFullRedo() const;

    void clearPreviousOriginData();

private:

    void clearUndoActions();
    void clearRedoActions();
    void undoStep(bool saveRedo, bool execute, bool flyingRollback);
    void redoStep(bool execute, bool flyingRollback);
    void makeSnapshot(int index);
    void restoreSnapshot(int index, const UndoMetadataContainer& c);
    void getSnapshot(int index, DImg* const img) const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* UNDOMANAGER_H */
