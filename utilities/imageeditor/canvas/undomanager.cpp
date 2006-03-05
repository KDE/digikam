/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Joern Ahrens <joern.ahrens@kdemail.net>
 * Date   : 2005-02-06
 * Description : an image editor actions undo/redo manager
 * 
 * Copyright 2005-2006 by Renchi Raju, Joern Ahrens
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

// C++ includes.

#include <typeinfo>

// Qt includes.

#include <qstringlist.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dimginterface.h"
#include "undoaction.h"
#include "undocache.h"
#include "undomanager.h"

namespace Digikam
{

class UndoManagerPriv
{
    
public:

    UndoManagerPriv()
    {
        dimgiface = 0;
        undoCache = 0;
        origin = 0;
    }

    QValueList<UndoAction*>  undoActions;
    QValueList<UndoAction*>  redoActions;
    int                      origin;
    
    UndoCache               *undoCache;
    
    DImgInterface           *dimgiface;
};

UndoManager::UndoManager(DImgInterface* iface)
{
    d = new UndoManagerPriv;
    d->dimgiface = iface;
    d->undoCache = new UndoCache;
}

UndoManager::~UndoManager()
{
    clear(true);
    delete d->undoCache;
    delete d;
}

void UndoManager::addAction(UndoAction* action)
{
    if (!action)
        return;

    // All redo actions are invalid now
    clearRedoActions();
   
    d->undoActions.push_back(action);

    if (typeid(*action) == typeid(UndoActionIrreversible))
    {
        int w          = d->dimgiface->origWidth();
        int h          = d->dimgiface->origHeight();
        int bytesDepth = d->dimgiface->bytesDepth();
        uchar* data    = d->dimgiface->getImage();
        
        d->undoCache->putData(d->undoActions.size(), w, h, bytesDepth, data);
    }

    // if origin is at one of the redo action that are now invalid,
    // it is no longer reachable
    if (d->origin < 0)
        d->origin = INT_MAX;
    else
        d->origin++;
}

void UndoManager::undo()
{
    if (d->undoActions.isEmpty())
        return;

    UndoAction* action = d->undoActions.back();

    if (typeid(*action) == typeid(UndoActionIrreversible))
    {
        // Save the current state for the redo operation

        int w          = d->dimgiface->origWidth();
        int h          = d->dimgiface->origHeight();
        int bytesDepth = d->dimgiface->bytesDepth();
        uchar* data    = d->dimgiface->getImage();

        d->undoCache->putData(d->undoActions.size() + 1, w, h, bytesDepth, data);
        
        // And now, undo the action

        int    newW, newH, newBytesDepth;
        uchar *newData = d->undoCache->getData(d->undoActions.size(), newW, newH, newBytesDepth, false);
        if (newData)
        {
            d->dimgiface->putImage(newData, newW, newH, newBytesDepth == 8 ? true : false);
            delete [] newData;
        }
    }
    else
    {
        action->rollBack();
    }
    
    d->undoActions.pop_back();
    d->redoActions.push_back(action);
    d->origin--;
}

void UndoManager::redo()
{
    if(d->redoActions.isEmpty())
        return;
    
    UndoAction *action = d->redoActions.back();
    
    if(typeid(*action) == typeid(UndoActionIrreversible))
    {
        int    w, h, bytesDepth;
        uchar *data = d->undoCache->getData(d->undoActions.size() + 2, w, h, bytesDepth, false);
        if (data)
        {
            d->dimgiface->putImage(data, w, h, bytesDepth == 8 ? true : false);
            delete[] data;
        }
    }
    else
    {
        action->execute();
    }
    
    d->redoActions.pop_back();
    d->undoActions.push_back(action);
    d->origin++;
}

void UndoManager::clear(bool clearCache)
{
    clearUndoActions();
    clearRedoActions();
    setOrigin();

    if(clearCache)
        d->undoCache->clear();
}

void UndoManager::clearUndoActions()
{
    UndoAction *action;
    QValueList<UndoAction*>::iterator it;
    
    for(it = d->undoActions.begin(); it != d->undoActions.end(); ++it)
    {
        action = *it;
        delete action;
    }
    d->undoActions.clear();
}

void UndoManager::clearRedoActions()
{
    if(!anyMoreRedo())
        return;

    UndoAction *action;
    QValueList<UndoAction*>::iterator it;

    // get the level of the first redo action
    int level = d->undoActions.size() + 1;
    for(it = d->redoActions.begin(); it != d->redoActions.end(); ++it)
    {
        action = *it;
        d->undoCache->erase(level);
        delete action;
        level++;
    }
    d->undoCache->erase(level);
    d->redoActions.clear();
}

bool UndoManager::anyMoreUndo()
{
    return !d->undoActions.isEmpty();
}

bool UndoManager::anyMoreRedo()
{
    return !d->redoActions.isEmpty();
}

void UndoManager::getUndoHistory(QStringList &titles)
{
    QValueList<UndoAction*>::iterator it;

    for(it = d->undoActions.begin(); it != d->undoActions.end(); ++it)
    {
        titles.push_front((*it)->getTitle());
    }
}

void UndoManager::getRedoHistory(QStringList &titles)
{
    QValueList<UndoAction*>::iterator it;

    for(it = d->redoActions.begin(); it != d->redoActions.end(); ++it)
    {
        titles.push_front((*it)->getTitle());
    }
}

bool UndoManager::isAtOrigin()
{
    return d->origin == 0;
}

void UndoManager::setOrigin()
{
    d->origin = 0;
}

}  // namespace Digikam
