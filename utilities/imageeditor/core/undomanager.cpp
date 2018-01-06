/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date   : 2005-02-06
 * Description : an image editor actions undo/redo manager
 *
 * Copyright (C) 2005-2006 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 Joern Ahrens <joern dot ahrens at kdemail dot net>
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

#include "undomanager.h"

// C++ includes

#include <typeinfo>
#include <climits>

// Qt includes

#include <QList>

// Local includes

#include "digikam_debug.h"
#include "editorcore.h"
#include "undoaction.h"
#include "undocache.h"

namespace Digikam
{

class UndoManager::Private
{
public:

    Private() :
        origin(0),
        undoCache(0),
        core(0)
    {
    }

    QList<UndoAction*> undoActions;
    QList<UndoAction*> redoActions;
    int                origin;

    UndoCache*         undoCache;

    EditorCore*        core;
};

UndoManager::UndoManager(EditorCore* const core)
    : d(new Private)
{
    d->core      = core;
    d->undoCache = new UndoCache;
}

UndoManager::~UndoManager()
{
    clear(true);
    delete d->undoCache;
    delete d;
}

void UndoManager::addAction(UndoAction* const action)
{
    if (!action)
    {
        return;
    }

    // All redo actions are invalid now
    clearRedoActions();

    // If the _last_ action was irreversible, we need to snapshot it
    UndoAction* const lastAction               = d->undoActions.isEmpty() ? 0 : d->undoActions.last();

    d->undoActions << action;

    // action has already read the "history before step" from EditorCore in its constructor
    UndoActionIrreversible* const irreversible = dynamic_cast<UndoActionIrreversible*>(action);

    // we always make an initial snapshot to be able to do a flying rollback in one step
    if (irreversible || !lastAction || isAtOrigin())
    {
        makeSnapshot(d->undoActions.size() - 1);
    }

    if (isAtOrigin())
    {
        QVariant      originDataBeforeStep    = d->core->getImg()->fileOriginData();
        DImageHistory originHistoryBeforeStep = d->core->getResolvedInitialHistory();
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

    d->core->setModified();
}

void UndoManager::redo()
{
    if (d->redoActions.isEmpty())
    {
        return;
    }

    redoStep(true, false);

    d->core->setModified();
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

    d->core->setModified();
}

void UndoManager::undoStep(bool saveRedo, bool execute, bool flyingRollback)
{
    UndoAction* const action                   = d->undoActions.back();
    UndoMetadataContainer dataBeforeStep       = action->getMetadata();
    UndoMetadataContainer dataAfterStep        = UndoMetadataContainer::fromImage(*d->core->getImg());
    UndoActionIrreversible* const irreversible = dynamic_cast<UndoActionIrreversible*>(action);
    UndoActionReversible* const reversible     = dynamic_cast<UndoActionReversible*>(action);
    QVariant originDataAfterStep               = d->core->getImg()->fileOriginData();
    QVariant originDataBeforeStep; // only needed if isAtOrigin()

    DImageHistory originHistoryAfterStep       = d->core->getResolvedInitialHistory();
    DImageHistory originHistoryBeforeStep;

    int lastOrigin = 0;

    if (isAtOrigin())
    {
        // undoing from an origin: need to switch to previous origin?
        for (lastOrigin = d->undoActions.size() - 1; lastOrigin >= 0; lastOrigin--)
        {
            if (d->undoActions.at(lastOrigin)->hasFileOriginData())
            {
                originDataBeforeStep    = d->undoActions.at(lastOrigin)->fileOriginData();
                originHistoryBeforeStep = d->undoActions.at(lastOrigin)->fileOriginResolvedHistory();
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
        // in case of flyingRollback, the data in core is not in sync
        if (irreversible || flyingRollback)
        {
            // undo the action
            restoreSnapshot(d->undoActions.size() - 1, dataBeforeStep);
        }
        else if (reversible) // checking pointer just to check for null pointer in case of a bug
        {
            reversible->getReverseFilter().apply(*d->core->getImg());
            d->core->imageUndoChanged(dataBeforeStep);
        }
    }
    else
    {
        // if we do not copy the data (fast roll-back), we at least set the history for subsequent steps
        d->core->imageUndoChanged(dataBeforeStep);
    }

    // Record history and origin for redo
    action->setMetadata(dataAfterStep);

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
        d->core->setFileOriginData(originDataBeforeStep);
        d->core->setResolvedInitialHistory(originHistoryBeforeStep);
    }
    else
    {
        d->origin--;
    }

}

void UndoManager::redoStep(bool execute, bool flyingRollback)
{
    UndoAction* const action                   = d->redoActions.back();
    UndoMetadataContainer dataBeforeStep       = UndoMetadataContainer::fromImage(*d->core->getImg());
    UndoMetadataContainer dataAfterStep        = action->getMetadata();
    QVariant originDataBeforeStep              = d->core->getImg()->fileOriginData();
    QVariant originDataAfterStep               = action->fileOriginData();
    DImageHistory originHistoryBeforeStep      = d->core->getResolvedInitialHistory();
    DImageHistory originHistoryAfterStep       = action->fileOriginResolvedHistory();
    UndoActionIrreversible* const irreversible = dynamic_cast<UndoActionIrreversible*>(action);
    UndoActionReversible* const reversible     = dynamic_cast<UndoActionReversible*>(action);

    if (execute)
    {
        if (irreversible || flyingRollback)
        {
            restoreSnapshot(d->undoActions.size() + 1, dataAfterStep);
        }
        else if (reversible) // checking pointer just to check for null pointer in case of a bug
        {
            reversible->getFilter().apply(*d->core->getImg());
            d->core->imageUndoChanged(dataAfterStep);
        }
    }
    else
    {
        // if we do not copy the data (fast roll-back), we at least set the history for subsequent steps
        d->core->imageUndoChanged(dataAfterStep);
    }

    action->setMetadata(dataBeforeStep);

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
        d->core->setFileOriginData(originDataAfterStep);
        d->core->setResolvedInitialHistory(originHistoryAfterStep);
    }
    else
    {
        d->origin++;
    }
}

void UndoManager::makeSnapshot(int index)
{
    d->undoCache->putData(index, *d->core->getImg());
}

void UndoManager::restoreSnapshot(int index, const UndoMetadataContainer& c)
{
    DImg img = d->undoCache->getData(index);

    if (!img.isNull())
    {
        d->core->setUndoImg(c, img);
    }
}

void UndoManager::getSnapshot(int index, DImg* const img) const
{
    DImg data = d->undoCache->getData(index);

    // Pass ownership of buffer. If data is null, img will be null
    img->putImageData(data.width(), data.height(), data.sixteenBit(), data.hasAlpha(), data.bits(), true);
}

void UndoManager::clearPreviousOriginData()
{
    for (int i = d->undoActions.size() - 1; i >= 0; i--)
    {
        UndoAction* const action = d->undoActions[i];

        if (action->hasFileOriginData())
        {
            action->setFileOriginData(QVariant(), DImageHistory());
            return;
        }
    }
}

bool UndoManager::putImageDataAndHistory(DImg* const img, int stepsBack) const
{
    if (stepsBack <= 0 || stepsBack > d->undoActions.size())
    {
        return false;
    }

    /*
     * We need to find a snapshot, for the state the given number of steps back.
     * 0 steps back is the current state of the EditorCore.
     * 1 step back is the snapshot of the last undo action, at d->undoActions.size() - 1.
     * The next problem is that if the corresponding action is reversible,
     * we do not have a snapshot, but need to walk forward to the first snapshot (or current
     * state), then apply the reversible steps.
     */
    int step = d->undoActions.size() - stepsBack;
    int snapshot;

    for (snapshot = step; snapshot < d->undoActions.size(); ++snapshot)
    {
        if (dynamic_cast<UndoActionIrreversible*>(d->undoActions.at(snapshot)))
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
            reverting = d->core->getImg()->copyImageData();
        }

        // revert reversible actions, until reaching desired step
        for (; snapshot > step; snapshot--)
        {
            UndoActionReversible* const reversible = dynamic_cast<UndoActionReversible*>(d->undoActions.at(snapshot - 1));
            if (!reversible) // would be a bug
            {
                continue;
            }
            reversible->getReverseFilter().apply(reverting);
        }

        img->putImageData(reverting.width(), reverting.height(), reverting.sixteenBit(),
                          reverting.hasAlpha(), reverting.stripImageData(), false);
    }

    // adjust history
    UndoAction* const action             = d->undoActions.at(step);
    UndoMetadataContainer dataBeforeStep = action->getMetadata();
    dataBeforeStep.toImage(*img);

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
    QList<UndoAction*>::const_iterator it;

    for (it = d->undoActions.constBegin(); it != d->undoActions.constEnd(); ++it)
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

    foreach(UndoAction* const action, d->undoActions)
    {
        titles.prepend(action->getTitle());
    }

    return titles;
}

QStringList UndoManager::getRedoHistory() const
{
    QStringList titles;

    foreach(UndoAction* const action, d->redoActions)
    {
        titles.prepend(action->getTitle());
    }

    return titles;
}

bool UndoManager::isAtOrigin() const
{
    return (d->origin == 0);
}

bool UndoManager::hasChanges() const
{
    if (!isAtOrigin())
    {
        return true;
    }
    else
    {
        DImageHistory currentHistory = d->core->getImageHistory();
        DImageHistory initialHistory = d->core->getInitialImageHistory();

        if (currentHistory == initialHistory)
        {
            return false;
        }
        else
        {
            return currentHistory.actionCount() > initialHistory.actionCount();
        }
    }
}

void UndoManager::setOrigin() const
{
    d->origin = 0;
}

DImageHistory UndoManager::getImageHistoryOfFullRedo() const
{
    if (!d->redoActions.isEmpty())
    {
        return d->redoActions.first()->getMetadata().history;
    }

    return d->core->getImageHistory();
}

}  // namespace Digikam
