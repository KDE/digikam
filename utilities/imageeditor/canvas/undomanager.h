/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-02-06
 * Copyright 2005 by Renchi Raju, Jörn Ahrens
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
 * ============================================================ */

#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

// QT includes.

#include <qvaluelist.h>

class QStringList;

namespace Digikam
{

class UndoManagerPriv;
class UndoAction;
class UndoCache;

class DImgInterface;

class UndoManager
{
public:

    UndoManager(Digikam::DImgInterface* iface);
    ~UndoManager();

    void undo();
    void redo();
    
    void clear(bool clearCache=true);
    bool anyMoreUndo();
    bool anyMoreRedo();    
    void getUndoHistory(QStringList &titles);
    void getRedoHistory(QStringList &titles);

    void addAction(UndoAction* action);

private:
    
    void clearUndoActions();
    void clearRedoActions();

private:
    
    DImgInterface           *m_iface;
    
    QValueList<UndoAction*>  m_undoActions;
    QValueList<UndoAction*>  m_redoActions;
    
    UndoCache*               m_cache;
};

}  // namespace Digikam

#endif /* UNDOMANAGER_H */
