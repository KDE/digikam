/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-02-06
 * Copyright 2005 by Renchi Raju
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

#include <qvaluelist.h>

class UndoManagerPriv;
class UndoAction;
class UndoCache;

namespace Digikam
{
class ImlibInterface;
}

class UndoManager
{
public:

    UndoManager(Digikam::ImlibInterface* iface);
    ~UndoManager();

    void undo();
    void clear(bool clearCache=true);
    bool anyMoreUndo();

    void addAction(UndoAction* action);

private:

    Digikam::ImlibInterface* m_iface;
    QValueList<UndoAction*>  m_actions;
    UndoCache*               m_cache;
};

#endif /* UNDOMANAGER_H */
