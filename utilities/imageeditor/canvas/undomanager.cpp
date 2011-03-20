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
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class UndoManager::UndoManagerPriv
{

public:

    UndoManagerPriv() :
        origin(0),
        undoCache(0),
        dimgiface(0)
    {
    }

    QList<UndoAction*> undoActions;
    QList<UndoAction*> redoActions;
    int                origin;

    UndoCache*         undoCache;

    DImgInterface*     dimgiface;
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

    // If the _last_ action was irreversible, we need to snapshot it
    UndoAction* lastAction = d->undoActions.isEmpty() ? 0 : d->undoActions.last();

    d->undoActions << action;

    // action has already read the "history before step" from DImgInterface in its constructor
    UndoActionIrreversible* irreversible = dynamic_cast<UndoActionIrreversible*>(action);

    // we always make an initial snapshot to be able to do a flying rollback in one step
    if (irreversible || !lastAction || isAtOrigin())
    {
        makeSnapshot(d->undoActions.size() - 1);
    }

    if (isAtOrigin())
    {
        QVariant      originDataBeforeStep = d->dimgiface->getImg()->fileOriginData();
        DImageHistory originHistoryBeforeStep = d->dimgiface->getResolvedInitialHistory();
        action->setFileOriginData(originDataBeforeStep, originHistoryBeforeStep);
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
    UndoAction* action                   = d->undoActions.back();
    DImageHistory historyBeforeStep      = action->getHistory();
    DImageHistory historyAfterStep       = d->dimgiface->getImageHistory();
    UndoActionIrreversible* irreversible = dynamic_cast<UndoActionIrreversible*>(action);
    UndoActionReversible*   reversible   = dynamic_cast<UndoActionReversible*>(action);
    QVariant originDataAfterStep         = d->dimgiface->getImg()->fileOriginData();
    QVariant originDataBeforeStep; // only needed if isAtOrigin()

    DImageHistory originHistoryAfterStep = d->dimgiface->getResolvedInitialHistory();
    DImageHistory originHistoryBeforeStep;

    int lastOrigin = 0;

    if (isAtOrigin())
    {
        // undoing from an origin: need to switch to previous origin?
        for (lastOrigin = d->undoActions.size() - 1; lastOrigin >= 0; lastOrigin--)
        {
            if (d->undoActions[lastOrigin]->hasFileOriginData())
            {
                originDataBeforeStep    = d->undoActions[lastOrigin]->fileOriginData();
                originHistoryBeforeStep = d->undoActions[lastOrigin]->fileOriginResolvedHistory();
                break;
            }
        }
    }

    if (saveRedo)
    {
        bool needSnapshot = false;

        if (d->redoActions.isEmpty())
        {
            // Undoing from the tip of the list:
            // Save the "last", current state for the redo operation
            needSnapshot = irreversible;
        }
        else
        {
            // Undoing an irreversible with next redo reversible:
            // Here, no snapshot was made in addAction, but we need it now
            needSnapshot = dynamic_cast<UndoActionReversible*>(d->redoActions.last());
        }

        if (needSnapshot)
        {
            //d->undoCache->erase(d->undoActions.size() + 1);
            makeSnapshot(d->undoActions.size());
        }
    }

    if (execute)
    {
        // in case of flyingRollback, the data in dimgiface is not in sync
        if (irreversible || flyingRollback)
        {
            // undo the action
            restoreSnapshot(d->undoActions.size() - 1, historyBeforeStep);
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

    // Record history and origin for redo
    action->setHistory(historyAfterStep);

    if (isAtOrigin())
    {
        action->setFileOriginData(originDataAfterStep, originHistoryAfterStep);
    }
    else
    {
        action->setFileOriginData(QVariant(), DImageHistory());
    }

    d->undoActions.removeLast();
    d->redoActions << action;

    if (!originDataBeforeStep.isNull())
    {
        d->origin = d->undoActions.size() - lastOrigin;
        d->dimgiface->setFileOriginData(originDataBeforeStep);
        d->dimgiface->setResolvedInitialHistory(originHistoryBeforeStep);
    }
    else
    {
        d->origin--;
    }

}

void UndoManager::redoStep(bool execute, bool flyingRollback)
{
    UndoAction* action                    = d->redoActions.back();
    DImageHistory historyBeforeStep       = d->dimgiface->getImageHistory();
    DImageHistory historyAfterStep        = action->getHistory();
    QVariant originDataBeforeStep         = d->dimgiface->getImg()->fileOriginData();
    QVariant originDataAfterStep          = action->fileOriginData();
    DImageHistory originHistoryBeforeStep = d->dimgiface->getResolvedInitialHistory();
    DImageHistory originHistoryAfterStep  = action->fileOriginResolvedHistory();
    UndoActionIrreversible* irreversible  = dynamic_cast<UndoActionIrreversible*>(action);
    UndoActionReversible* reversible      = dynamic_cast<UndoActionReversible*>(action);

    if (execute)
    {
        if (irreversible || flyingRollback)
        {
            restoreSnapshot(d->undoActions.size() + 1, historyAfterStep);
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

    action->setHistory(historyBeforeStep);

    if (isAtOrigin())
    {
        action->setFileOriginData(originDataBeforeStep, originHistoryBeforeStep);
    }
    else
    {
        action->setFileOriginData(QVariant(), DImageHistory());
    }

    d->redoActions.removeLast();
    d->undoActions << action;

    if (!originDataAfterStep.isNull())
    {
        d->origin = 0;
        d->dimgiface->setFileOriginData(originDataAfterStep);
        d->dimgiface->setResolvedInitialHistory(originHistoryAfterStep);
    }
    else
    {
        d->origin++;
    }
}

void UndoManager::makeSnapshot(int index)
{
    int w           = d->dimgiface->origWidth();
    int h           = d->dimgiface->origHeight();
    bool sixteenBit = d->dimgiface->sixteenBit();
    bool hasAlpha   = d->dimgiface->hasAlpha();
    uchar* data     = d->dimgiface->getImage();

    d->undoCache->putData(index, w, h, sixteenBit, hasAlpha, data);
}

void UndoManager::restoreSnapshot(int index, const DImageHistory& history)
{
    int    newW, newH;
    bool   sixteenBit, hasAlpha;
    uchar* newData = d->undoCache->getData(index, newW, newH, sixteenBit, hasAlpha);

    if (newData)
    {
        d->dimgiface->setUndoImageData(history, newData, newW, newH, sixteenBit);
        delete [] newData;
    }
}

void UndoManager::getSnapshot(int index, DImg* img)
{
    int    newW, newH;
    bool   sixteenBit, hasAlpha;
    uchar* newData = d->undoCache->getData(index, newW, newH, sixteenBit, hasAlpha);

    // Pass ownership of buffer. If newData is null, img will be null
    img->putImageData(newW, newH, sixteenBit, hasAlpha, newData, false);
}

void UndoManager::clearPreviousOriginData()
{
    for (int i = d->undoActions.size() - 1; i >= 0; i--)
    {
        UndoAction* action = d->undoActions[i];

        if (action->hasFileOriginData())
        {
            action->setFileOriginData(QVariant(), DImageHistory());
            return;
        }
    }
}

bool UndoManager::putImageDataAndHistory(DImg* img, int stepsBack)
{
    if (stepsBack <= 0 || stepsBack > d->undoActions.size())
    {
        return false;
    }

    /*
     * We need to find a snapshot, for the state the given number of steps back.
     * 0 steps back is the current state of the DImgInterface.
     * 1 step back is the snapshot of the last undo action, at d->undoActions.size() - 1.
     * The next problem is that if the corresponding action is reversible,
     * we dont have a snapshot, but need to walk forward to the first snapshot (or current
     * state), then apply the reversible steps.
     */
    int step = d->undoActions.size() - stepsBack;
    int snapshot;

    for (snapshot = step; snapshot < d->undoActions.size(); snapshot++)
    {
        if (dynamic_cast<UndoActionIrreversible*>(d->undoActions[snapshot]))
        {
            break;
        }
    }

    if (snapshot == step)
    {
        getSnapshot(step, img);
    }
    else
    {
        DImg reverting;

        // Get closest available snapshot
        if (snapshot < d->undoActions.size())
        {
            getSnapshot(snapshot, &reverting);
        }
        else
        {
            reverting = d->dimgiface->getImg()->copyImageData();
        }

        // revert reversible actions, until reaching desired step
        for (; snapshot > step; snapshot--)
        {
            UndoActionReversible* reversible = dynamic_cast<UndoActionReversible*>(d->undoActions[snapshot - 1]);
            reversible->getReverseFilter().apply(reverting);
        }

        img->putImageData(reverting.width(), reverting.height(), reverting.sixteenBit(),
                          reverting.hasAlpha(), reverting.stripImageData(), false);
    }

    // adjust history
    UndoAction* action = d->undoActions[step];
    DImageHistory historyBeforeStep = action->getHistory();
    img->setImageHistory(historyBeforeStep);

    return true;
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
    UndoAction* action = 0;
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

    // clear from the level of the first redo action
    d->undoCache->clearFrom(d->undoActions.size() + 1);

    qDeleteAll(d->redoActions);
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
    return (d->origin == 0);
}

void UndoManager::setOrigin() const
{
    d->origin = 0;
}

DImageHistory UndoManager::getImageHistoryOfFullRedo() const
{
    if (!d->redoActions.isEmpty())
    {
        return d->redoActions.first()->getHistory();
    }

    return d->dimgiface->getImageHistory();
}

}  // namespace Digikam
