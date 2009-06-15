/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2005-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "managedloadsavethread.h"

// KDE includes

#include <kdebug.h>

// Local includes

#include "loadsavetask.h"
#include "previewtask.h"
#include "thumbnailtask.h"

namespace Digikam
{

ManagedLoadSaveThread::ManagedLoadSaveThread()
{
    m_terminationPolicy = TerminationPolicyTerminateLoading;
}

ManagedLoadSaveThread::~ManagedLoadSaveThread()
{
    LoadingTask *loadingTask;
    switch (m_terminationPolicy)
    {
        case TerminationPolicyTerminateLoading:
        {
            QMutexLocker lock(&m_mutex);
            if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterAll)) )
                loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
            removeLoadingTasks(LoadingDescription(QString()), LoadingTaskFilterAll);
            break;
        }
        case TerminationPolicyTerminatePreloading:
        {
            QMutexLocker lock(&m_mutex);
            if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)) )
                loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
            removeLoadingTasks(LoadingDescription(QString()), LoadingTaskFilterPreloading);
            break;
        }
        case TerminationPolicyWait:
            break;
    }
}

LoadingTask *ManagedLoadSaveThread::checkLoadingTask(LoadSaveTask *task, LoadingTaskFilter filter)
{
    if (task && task->type() == LoadSaveTask::TaskTypeLoading)
    {
        LoadingTask *loadingTask = (LoadingTask *)task;
        if (filter == LoadingTaskFilterAll)
            return loadingTask;
        else if (filter == LoadingTaskFilterPreloading)
            if (loadingTask->status() == LoadingTask::LoadingTaskStatusPreloading)
                return loadingTask;
    }
    return 0;
}

LoadingTask *ManagedLoadSaveThread::findExistingTask(const LoadingDescription& loadingDescription)
{
    LoadingTask *loadingTask;
    if (m_currentTask)
    {
        if (m_currentTask->type() == LoadSaveTask::TaskTypeLoading)
        {
            loadingTask = (LoadingTask *)m_currentTask;
            LoadingDescription taskDescription = loadingTask->loadingDescription();
            if (taskDescription == loadingDescription)
                return loadingTask;
        }
    }
    for (int i=0; i < m_todo.size(); ++i)
    {
        LoadSaveTask *task = m_todo[i];
        if (task->type() == LoadSaveTask::TaskTypeLoading)
        {
            loadingTask = (LoadingTask *)task;
            if (loadingTask->loadingDescription() == loadingDescription)
                return loadingTask;
        }
    }
    return 0;
}

void ManagedLoadSaveThread::setTerminationPolicy(TerminationPolicy terminationPolicy)
{
    m_terminationPolicy = terminationPolicy;
}

void ManagedLoadSaveThread::load(LoadingDescription description, LoadingPolicy policy)
{
    load(description, LoadingModeNormal, policy);
}

void ManagedLoadSaveThread::load(LoadingDescription description, LoadingMode loadingMode, LoadingPolicy policy, AccessMode accessMode)
{
    QMutexLocker lock(&m_mutex);
    LoadingTask *loadingTask  = 0;
    LoadingTask *existingTask = findExistingTask(description);

    //kDebug(50003) << "ManagedLoadSaveThread::load " << description.filePath << ", policy " << policy;
    switch(policy)
    {
        case LoadingPolicyFirstRemovePrevious:
            // reuse task if it exists
            if (existingTask)
            {
                existingTask->setStatus(LoadingTask::LoadingTaskStatusLoading);
            }
            // stop current task
            if (m_currentTask && m_currentTask != existingTask)
            {
                if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterAll)) )
                    loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
            }
            //kDebug(50003) << "LoadingPolicyFirstRemovePrevious, Existing task " << existingTask <<
             //", m_currentTask " << m_currentTask << ", loadingTask " << loadingTask << endl;
            // remove all loading tasks
            for (int i=0; i < m_todo.size(); ++i)
            {
                LoadSaveTask *task = m_todo[i];
                if (task != existingTask && checkLoadingTask(task, LoadingTaskFilterAll))
                {
                    //kDebug(50003) << "Removing task " << task << " from list";
                    delete m_todo.takeAt(i--);
                }
            }
            // append new, exclusive loading task
            if (existingTask)
                break;
            m_todo.append(createLoadingTask(description, false, loadingMode, accessMode));
            break;

        case LoadingPolicyPrepend:
            if (existingTask)
            {
                existingTask->setStatus(LoadingTask::LoadingTaskStatusLoading);
            }
            // stop and postpone current task if it is a preloading task
            if (m_currentTask)
            {
                if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)) )
                {
                    loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
                    load(loadingTask->loadingDescription(), LoadingPolicyPreload);
                }
            }
            //kDebug(50003) << "LoadingPolicyPrepend, Existing task " << existingTask << ", m_currentTask " << m_currentTask;
            // prepend new loading task
            if (existingTask)
                break;
            m_todo.prepend(createLoadingTask(description, false, loadingMode, accessMode));
            break;

        case LoadingPolicyAppend:
            if (existingTask)
            {
                existingTask->setStatus(LoadingTask::LoadingTaskStatusLoading);
            }
            // stop and postpone current task if it is a preloading task
            if (m_currentTask)
            {
                if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)) )
                {
                    loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
                    load(loadingTask->loadingDescription(), LoadingPolicyPreload);
                }
            }
            if (existingTask)
                break;
            //kDebug(50003) << "LoadingPolicyAppend, Existing task " << existingTask << ", m_currentTask " << m_currentTask;
            // append new loading task, put it in front of preloading tasks
            int i;
            for (i = 0; i<m_todo.count(); ++i)
            {
                if (checkLoadingTask(m_todo[i], LoadingTaskFilterPreloading))
                    break;
            }
            m_todo.insert(i, createLoadingTask(description, false, loadingMode, accessMode));
            break;

        case LoadingPolicyPreload:
            // append to the very end of the list
            //kDebug(50003) << "LoadingPolicyPreload, Existing task " << existingTask;
            if (existingTask)
                break;
            m_todo.append(createLoadingTask(description, true, loadingMode, accessMode));
            break;
    }
    m_condVar.wakeAll();
}

void ManagedLoadSaveThread::loadPreview(LoadingDescription description)
{
    // This is similar to the LoadingPolicyFirstRemovePrevious policy with normal loading tasks.
    // Preview threads typically only support preview tasks,
    // so no need to differentiate with normal loading tasks.

    QMutexLocker lock(&m_mutex);
    LoadingTask *loadingTask  = 0;
    LoadingTask *existingTask = findExistingTask(description);

    // reuse task if it exists
    if (existingTask)
    {
        existingTask->setStatus(LoadingTask::LoadingTaskStatusLoading);
    }
    // stop current task
    if (m_currentTask && m_currentTask != existingTask)
    {
        if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterAll)) )
            loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
    }
    // remove all loading tasks
    for (int i=0; i < m_todo.size(); ++i)
    {
        LoadSaveTask *task = m_todo[i];
        if (task != existingTask && checkLoadingTask(task, LoadingTaskFilterAll))
        {
            delete m_todo.takeAt(i--);
        }
    }
    //kDebug(50003)<<"loadPreview for " << description.filePath << " existingTask " << existingTask << " currentTask " << m_currentTask;
    // append new loading task
    if (existingTask)
        return;
    m_todo.append(new PreviewLoadingTask(this, description));
    m_condVar.wakeAll();
}

void ManagedLoadSaveThread::loadThumbnail(LoadingDescription description)
{
    // Thumbnail threads typically only support thumbnail tasks,
    // so no need to differentiate with normal loading tasks.

    QMutexLocker lock(&m_mutex);
    LoadingTask *existingTask = findExistingTask(description);

    // reuse task if it exists
    if (existingTask)
    {
        existingTask->setStatus(LoadingTask::LoadingTaskStatusLoading);
        return;
    }

    // append new loading task, put it in front of preloading tasks
    m_todo.prepend(new ThumbnailLoadingTask(this, description));

    // append new loading task
    m_condVar.wakeAll();
}

void ManagedLoadSaveThread::preloadThumbnail(LoadingDescription description)
{
    QMutexLocker lock(&m_mutex);
    LoadingTask *existingTask = findExistingTask(description);

    // reuse task if it exists
    if (existingTask)
        return;

    // create new loading task
    ThumbnailLoadingTask *task = new ThumbnailLoadingTask(this, description);
    // mark as preload task
    task->setStatus(LoadingTask::LoadingTaskStatusPreloading);
    // append to the end of the list
    m_todo.append(task);
    m_condVar.wakeAll();
}

void ManagedLoadSaveThread::prependThumbnailGroup(QList<LoadingDescription> descriptions)
{
    // This method is meant to prepend a group of loading tasks after the current task,
    // in the order they are given here, pushing the existing tasks to the back respectively removing double tasks.
    QMutexLocker lock(&m_mutex);

    int index = 0;
    for (int i=0; i<descriptions.size(); ++i)
    {
        LoadingTask *existingTask = findExistingTask(descriptions[i]);

        // remove task, if not the current task
        if (existingTask && existingTask != m_currentTask)
        {
            m_todo.removeAll(existingTask);
            delete existingTask;
        }

        // insert new loading task, in the order given by descriptions list
        m_todo.insert(index++, new ThumbnailLoadingTask(this, descriptions[i]));
    }
    m_condVar.wakeAll();
}

LoadingTask *ManagedLoadSaveThread::createLoadingTask(const LoadingDescription& description,
                                                      bool preloading, LoadingMode loadingMode, 
                                                      AccessMode accessMode)
{
    if (loadingMode == LoadingModeShared)
    {
        if (preloading)
            return new SharedLoadingTask(this, description, accessMode, LoadingTask::LoadingTaskStatusPreloading);
        else
            return new SharedLoadingTask(this, description, accessMode);
    }
    else
    {
        if (preloading)
            return new LoadingTask(this, description, LoadingTask::LoadingTaskStatusPreloading);
        else
            return new LoadingTask(this, description);
    }
}

void ManagedLoadSaveThread::stopLoading(const QString& filePath, LoadingTaskFilter filter)
{
    QMutexLocker lock(&m_mutex);
    removeLoadingTasks(LoadingDescription(filePath), filter);
}

void ManagedLoadSaveThread::stopLoading(const LoadingDescription& desc, LoadingTaskFilter filter)
{
    QMutexLocker lock(&m_mutex);
    removeLoadingTasks(desc, filter);
}

void ManagedLoadSaveThread::stopSaving(const QString& filePath)
{
    QMutexLocker lock(&m_mutex);

    // stop current task if it is matching the criteria
    if (m_currentTask && m_currentTask->type() == LoadSaveTask::TaskTypeSaving)
    {
        SavingTask *savingTask = (SavingTask *)m_currentTask;
        if (filePath.isNull() || savingTask->filePath() == filePath)
        {
            savingTask->setStatus(SavingTask::SavingTaskStatusStopping);
        }
    }

    // remove relevant tasks from list
    for (int i=0; i < m_todo.size(); ++i)
    {
        LoadSaveTask *task = m_todo[i];
        if (task->type() ==  LoadSaveTask::TaskTypeSaving)
        {
            SavingTask *savingTask = (SavingTask *)m_currentTask;
            if (filePath.isNull() || savingTask->filePath() == filePath)
            {
                delete m_todo.takeAt(i--);
            }
        }
    }
}

void ManagedLoadSaveThread::removeLoadingTasks(const LoadingDescription& description, LoadingTaskFilter filter)
{
    LoadingTask *loadingTask;

    // stop current task if it is matching the criteria
    if ( (loadingTask = checkLoadingTask(m_currentTask, filter)) )
    {
        if (description.filePath.isNull() || loadingTask->loadingDescription() == description)
        {
            loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
        }
    }

    // remove relevant tasks from list
    for (int i=0; i < m_todo.size(); ++i)
    {
        LoadSaveTask *task = m_todo[i];
        if ( (loadingTask = checkLoadingTask(task, filter)) )
        {
            if (description.filePath.isNull() || loadingTask->loadingDescription() == description)
            {
                delete m_todo.takeAt(i--);
            }
        }
    }
}

void ManagedLoadSaveThread::save(DImg& image, const QString& filePath, const QString& format)
{
    QMutexLocker lock(&m_mutex);
    LoadingTask *loadingTask;

    // stop and postpone current task if it is a preloading task
    if (m_currentTask && (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)))
    {
        loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
        load(loadingTask->loadingDescription(), LoadingPolicyPreload);
    }
    // append new loading task, put it in front of preloading tasks
    int i;
    for (i = 0; i<m_todo.count(); ++i)
    {
        LoadSaveTask *task = m_todo[i];
        if ( (loadingTask = checkLoadingTask(task, LoadingTaskFilterPreloading)) )
            break;
    }
    m_todo.insert(i, new SavingTask(this, image, filePath, format));
    m_condVar.wakeAll();
}

}   // namespace Digikam
