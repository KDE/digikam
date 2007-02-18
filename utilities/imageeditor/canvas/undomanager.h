/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Joern Ahrens <joern.ahrens@kdemail.net>
 * Date   : 2005-02-06
 * Description : an image editor actions undo/redo manager
 *
 * Copyright 2005-2006 by Renchi Raju <renchi@pooh.tam.uiuc.edu>, Joern Ahrens <joern.ahrens@kdemail.net>
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

// QT includes.

#include <qvaluelist.h>

// Local includes.

#include "digikam_export.h"

class QStringList;

namespace Digikam
{

class DImgInterface;
class UndoManagerPriv;
class UndoAction;

class DIGIKAM_EXPORT UndoManager
{

public:

    UndoManager(DImgInterface* iface);
    ~UndoManager();

    void undo();
    void redo();
    
    void clear(bool clearCache=true);
    bool anyMoreUndo();
    bool anyMoreRedo();    
    void getUndoHistory(QStringList &titles);
    void getRedoHistory(QStringList &titles);
    bool isAtOrigin();
    void setOrigin();

    void addAction(UndoAction* action);

private:
    
    void clearUndoActions();
    void clearRedoActions();

private:

    UndoManagerPriv *d;
};

}  // namespace Digikam

#endif /* UNDOMANAGER_H */
