/* ============================================================
 * File  : undomanager.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-02-06
 * Description : 
 * 
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
 * 
 * ============================================================ */

#include <typeinfo>

#include <qstringlist.h>

#include "imlibinterface.h"
#include "undoaction.h"
#include "undocache.h"
#include "undomanager.h"

#include <kdebug.h>

UndoManager::UndoManager(Digikam::ImlibInterface* iface)
    : m_iface(iface)
{
    m_cache = new UndoCache;
}

UndoManager::~UndoManager()
{
    clear(true);
    delete m_cache;
}

void UndoManager::addAction(UndoAction* action)
{
    if (!action)
        return;

    // All redo actions are invalid now
    clearRedoActions();
   
    m_undoActions.push_back(action);

    if (typeid(*action) == typeid(UndoActionIrreversible))
    {
        int   w    = m_iface->origWidth();
        int   h    = m_iface->origHeight();
        uint* data = m_iface->getData();
        
        m_cache->putData(m_undoActions.size(), w, h, data);
    }
}

void UndoManager::undo()
{
    if (m_undoActions.isEmpty())
        return;

    UndoAction* action = m_undoActions.back();

    if (typeid(*action) == typeid(UndoActionIrreversible))
    {
        int   w    = m_iface->origWidth();
        int   h    = m_iface->origHeight();
        uint* data = m_iface->getData();

        // save the current state for the redo operation        
        m_cache->putData(m_undoActions.size() + 1, w, h, data);

        // and now, undo the action
        m_cache->getData(m_undoActions.size(), w, h, data, false);
        m_iface->putData(data, w, h);

        delete [] data;
    }
    else
    {
        action->rollBack();
    }
    
    m_undoActions.pop_back();
    m_redoActions.push_back(action);
}

void UndoManager::redo()
{
    if(m_redoActions.isEmpty())
        return;
    
    UndoAction *action = m_redoActions.back();
    
    if(typeid(*action) == typeid(UndoActionIrreversible))
    {
        int  w, h;
        uint *data;
        
        m_cache->getData(m_undoActions.size() + 2, w, h, data, false);
        m_iface->putData(data, w, h);
        
        delete[] data;
    }
    else
    {
        action->execute();
    }
    
    m_redoActions.pop_back();
    m_undoActions.push_back(action);
}

void UndoManager::clear(bool clearCache)
{
    clearUndoActions();
    clearRedoActions();
    
    if(clearCache)
        m_cache->clear();
}

void UndoManager::clearUndoActions()
{
    UndoAction *action;
    QValueList<UndoAction*>::iterator it;
    
    for(it = m_undoActions.begin(); it != m_undoActions.end(); ++it)
    {
        action = *it;
        delete action;
    }
    m_undoActions.clear();
}

void UndoManager::clearRedoActions()
{
    if(!anyMoreRedo())
        return;

    UndoAction *action;
    QValueList<UndoAction*>::iterator it;

    // get the level of the first redo action
    int level = m_undoActions.size() + 1;
    for(it = m_redoActions.begin(); it != m_redoActions.end(); ++it)
    {
        action = *it;
        m_cache->erase(level);
        delete action;
        level++;
    }
    m_cache->erase(level);
    m_redoActions.clear();
}

bool UndoManager::anyMoreUndo()
{
    return !m_undoActions.isEmpty();
}

bool UndoManager::anyMoreRedo()
{
    return !m_redoActions.isEmpty();
}

void UndoManager::getUndoHistory(QStringList &titles)
{
    QValueList<UndoAction*>::iterator it;

    for(it = m_undoActions.begin(); it != m_undoActions.end(); ++it)
    {
        titles.push_front((*it)->getTitle());
    }
}

void UndoManager::getRedoHistory(QStringList &titles)
{
    QValueList<UndoAction*>::iterator it;

    for(it = m_redoActions.begin(); it != m_redoActions.end(); ++it)
    {
        titles.push_front((*it)->getTitle());
    }
}
