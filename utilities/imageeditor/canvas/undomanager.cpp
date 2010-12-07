/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date   : 2005-02-06
 * Description : an image editor actions undo/redo manager
 *
 * Copyright (C) 2005-2006 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2006 Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "undomanager.h"

// C++ includes

#include <typeinfo>
#include <climits>

// Qt includes

#include <QList>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimginterface.h"
#include "undoaction.h"
#include "undocache.h"

namespace Digikam
{

class UndoManagerPriv
{

public:

    UndoManagerPriv() :
        origin(0),
        undoCache(0),
        dimgiface(0)
    {
    }

    QList<UndoAction*>    undoActions;
    QList<UndoAction*>    redoActions;
    int                   origin;

    UndoCache*            undoCache;

    DImgInterface*        dimgiface;
};

UndoManager::UndoManager(DImgInterface* iface)
    : d(new UndoManagerPriv)
{
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
    {
        return;
    }

    // All redo actions are invalid now
    clearRedoActions();

    d->undoActions << action;

    UndoActionIrreversible* irreversible = dynamic_cast<UndoActionIrreversible*>(action);
    // action has already read the "history before step" from DImgInterface in its constructor

    // we always make an initial snapshot to be able to do a flying rollback in one step
    if (irreversible || isAtOrigin())
    {
        makeSnapshot(d->undoActions.size());
    }

    // if origin is at one of the redo action that are now invalid,
    // it is no longer reachable
    if (d->origin < 0)
    {
        d->origin = INT_MAX;
    }
    else
    {
        d->origin++;
    }
}

void UndoManager::undo()
{
    if (d->undoActions.isEmpty())
    {
        return;
    }

    undoStep(true, true, false);

    d->dimgiface->setModified();
}

void UndoManager::redo()
{
    if (d->redoActions.isEmpty())
    {
        return;
    }

    redoStep(true, false);

    d->dimgiface->setModified();
}

void UndoManager::rollbackToOrigin()
{
    if (d->undoActions.isEmpty() || isAtOrigin())
    {
        return;
    }

    if (d->origin > 0)
    {
        if (d->undoActions.size() == 1)
        {
            undo();
            return;
        }
        else
        {
            undoStep(true, false, true);

            while (d->origin > 1)
            {
                undoStep(false, false, true);
            }

            undoStep(false, true, true);
        }
    }
    else
    {
        if (d->redoActions.size() == 1)
        {
            redo();
            return;
        }
        else
        {
            while (d->origin < -1)
            {
                redoStep(false, true);
            }

            redoStep(true, true);
        }
    }

    d->dimgiface->setModified();
}

void UndoManager::undoStep(bool saveRedo, bool execute, bool flyingRollback)
{
    UndoAction* action = d->undoActions.back();

    DImageHistory historyBeforeStep = action->getHistory();
    DImageHistory historyAfterStep  = d->dimgiface->getImageHistory();

    UndoActionIrreversible* irreversible = dynamic_cast<UndoActionIrreversible*>(action);
    UndoActionReversible*   reversible   = dynamic_cast<UndoActionReversible*>(action);

    if (saveRedo && d->redoActions.isEmpty())
    {
        // Undoing from the tip of the list:
        // Save the "last", current state for the redo operation
        if (irreversible)
        {
            //d->undoCache->erase(d->undoActions.size() + 1);
            makeSnapshot(d->undoActions.size() + 1);
        }
    }

    if (execute)
    {
        // in case of flyingRollback, the data in dimgiface is not in sync
        if (irreversible || flyingRollback)
        {
            // undo the action
            restoreSnapshot(d->undoActions.size(), historyBeforeStep);
        }
        else
        {
            reversible->getReverseFilter().apply(*d->dimgiface->getImg());
            d->dimgiface->imageUndoChanged(historyBeforeStep);
        }
    }
    else
    {
        // if we dont copy the data (fast roll-back), we at least set the history for subsequent steps
        d->dimgiface->imageUndoChanged(historyBeforeStep);
    }

    d->undoActions.removeLast();
    action->setHistory(historyAfterStep);
    d->redoActions << action;
    d->origin--;
}

void UndoManager::redoStep(bool execute, bool flyingRollback)
{
    UndoAction* action = d->redoActions.back();

    DImageHistory historyBeforeStep = d->dimgiface->getImageHistory();
    DImageHistory historyAfterStep  = action->getHistory();

    UndoActionIrreversible* irreversible = dynamic_cast<UndoActionIrreversible*>(action);
    UndoActionReversible*   reversible   = dynamic_cast<UndoActionReversible*>(action);

    if (execute)
    {
        if (irreversible || flyingRollback)
        {
            restoreSnapshot(d->undoActions.size() + 2, historyAfterStep);
        }
        else
        {
            reversible->getFilter().apply(*d->dimgiface->getImg());
            d->dimgiface->imageUndoChanged(historyAfterStep);
        }
    }
    else
    {
        // if we dont copy the data (fast roll-back), we at least set the history for subsequent steps
        d->dimgiface->imageUndoChanged(historyAfterStep);
    }

    d->redoActions.removeLast();
    action->setHistory(historyBeforeStep);
    d->undoActions << action;
    d->origin++;
}

void UndoManager::makeSnapshot(int index)
{
    int w          = d->dimgiface->origWidth();
    int h          = d->dimgiface->origHeight();
    int bytesDepth = d->dimgiface->bytesDepth();
    uchar* data    = d->dimgiface->getImage();

    d->undoCache->putData(index, w, h, bytesDepth, data);
}

void UndoManager::restoreSnapshot(int index, const DImageHistory& history)
{
    int    newW, newH, newBytesDepth;
    uchar* newData = d->undoCache->getData(index, newW, newH, newBytesDepth, false);

    if (newData)
    {
        d->dimgiface->setUndoImageData(history, newData, newW, newH, newBytesDepth == 8 ? true : false);
        delete [] newData;
    }
}

void UndoManager::clear(bool clearCache)
{
    clearUndoActions();
    clearRedoActions();
    setOrigin();

    if (clearCache)
    {
        d->undoCache->clear();
    }
}

void UndoManager::clearUndoActions()
{
    UndoAction* action;
    QList<UndoAction*>::iterator it;

    for (it = d->undoActions.begin(); it != d->undoActions.end(); ++it)
    {
        action = *it;
        delete action;
    }

    d->undoActions.clear();
}

void UndoManager::clearRedoActions()
{
    if (!anyMoreRedo())
    {
        return;
    }

    UndoAction* action;
    QList<UndoAction*>::iterator it;

    // get the level of the first redo action
    int level = d->undoActions.size() + 1;

    for (it = d->redoActions.begin(); it != d->redoActions.end(); ++it)
    {
        action = *it;
        d->undoCache->erase(level);
        delete action;
        ++level;
    }

    d->undoCache->erase(level);
    d->redoActions.clear();
}

bool UndoManager::anyMoreUndo() const
{
    return !d->undoActions.isEmpty();
}

bool UndoManager::anyMoreRedo() const
{
    return !d->redoActions.isEmpty();
}

int UndoManager::availableUndoSteps() const
{
    return d->undoActions.isEmpty();
}

int UndoManager::availableRedoSteps() const
{
    return d->redoActions.isEmpty();
}

QStringList UndoManager::getUndoHistory() const
{
    QStringList titles;
    foreach (UndoAction* action, d->undoActions)
    titles << action->getTitle();
    return titles;
}

QStringList UndoManager::getRedoHistory() const
{
    QStringList titles;
    foreach (UndoAction* action, d->redoActions)
    titles.prepend(action->getTitle());
    return titles;
}

bool UndoManager::isAtOrigin() const
{
    return d->origin == 0;
}

void UndoManager::setOrigin() const
{
    d->origin = 0;
}

DImageHistory UndoManager::getImageHistoryOfFullRedo() const
{
    kDebug() << d->redoActions.count();

    if (!d->redoActions.isEmpty())
    {
        return d->redoActions.first()->getHistory();
    }

    return d->dimgiface->getImageHistory();
}

}  // namespace Digikam
