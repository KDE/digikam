/* ============================================================
 * File  : undomanager.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-02-06
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include "imlibinterface.h"
#include "undoaction.h"
#include "undocache.h"
#include "undomanager.h"

UndoManager::UndoManager(Digikam::ImlibInterface* iface)
    : m_iface(iface)
{
    m_cache = new UndoCache;
}

UndoManager::~UndoManager()
{
    clear(false);
    delete m_cache;
}

void UndoManager::undo()
{
    if (m_actions.isEmpty())
        return;

    UndoAction* action = m_actions.front();

    if (typeid(*action) == typeid(UndoActionIrreversible))
    {
        int   w, h;
        uint* data;

        m_cache->popLevel(m_actions.size(), w, h, data);
        m_iface->putData(data, w, h, false);

        delete [] data;
    }
    else
    {
        action->rollBack();
    }
    
    m_actions.pop_front();
    delete action;
}

void UndoManager::clear(bool clearCache)
{
    UndoAction *action;
    for (QValueList<UndoAction*>::iterator it = m_actions.begin();
         it != m_actions.end(); ++it)
    {
        action = *it;
        delete action;
    }

    m_actions.clear();

    if (clearCache)
        m_cache->clear();
}

bool UndoManager::anyMoreUndo()
{
    return !m_actions.isEmpty();
}

void UndoManager::addAction(UndoAction* action)
{
    if (!action)
        return;

    m_actions.push_front(action);

    if (typeid(*action) == typeid(UndoActionIrreversible))
    {
        int   w    = m_iface->origWidth();
        int   h    = m_iface->origHeight();
        uint* data = m_iface->getData();

        m_cache->pushLevel(m_actions.size(), w, h, data);
    }
}
