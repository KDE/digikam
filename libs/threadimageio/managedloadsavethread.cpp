/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2006-01-20
 * Description : image file IO threaded interface.
 *
 * Copyright 2005-2006 by Marcel Wiesweg, Gilles Caulier
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

// Local includes.

#include "ddebug.h"
#include "managedloadsavethread.h"
#include "loadsavetask.h"
#include "previewtask.h"

namespace Digikam
{

ManagedLoadSaveThread::ManagedLoadSaveThread()
{
    m_terminationPolicy  = TerminationPolicyTerminateLoading;
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

LoadingTask *ManagedLoadSaveThread::findExistingTask(const LoadingDescription &loadingDescription)
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
    for (LoadSaveTask *task = m_todo.first(); task; task = m_todo.next())
    {
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
    LoadingTask *loadingTask = 0;
    LoadingTask *existingTask = findExistingTask(description);

    //DDebug() << "ManagedLoadSaveThread::load " << description.filePath << ", policy " << policy << endl;
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
            //DDebug() << "LoadingPolicyFirstRemovePrevious, Existing task " << existingTask <<
             //", m_currentTask " << m_currentTask << ", loadingTask " << loadingTask << endl;
            // remove all loading tasks
            for (LoadSaveTask *task = m_todo.first(); task; task = m_todo.next())
            {
                if (task != existingTask && checkLoadingTask(task, LoadingTaskFilterAll))
                {
                    //DDebug() << "Removing task " << task << " from list" << endl;
                    m_todo.remove();
                    m_todo.prev();
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
                    load(loadingTask->filePath(), LoadingPolicyPreload);
                }
            }
            //DDebug() << "LoadingPolicyPrepend, Existing task " << existingTask << ", m_currentTask " << m_currentTask << endl;
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
                    load(loadingTask->filePath(), LoadingPolicyPreload);
                }
            }
            if (existingTask)
                break;
            //DDebug() << "LoadingPolicyAppend, Existing task " << existingTask << ", m_currentTask " << m_currentTask << endl;
            // append new loading task, put it in front of preloading tasks
            for (uint i = 0; i<m_todo.count(); i++)
            {
                LoadSaveTask *task = m_todo.at(i);
                if ( (loadingTask = checkLoadingTask(task, LoadingTaskFilterPreloading)) )
                {
                    m_todo.insert(i, createLoadingTask(description, false, loadingMode, accessMode));
                    break;
                }
            }
            break;
        case LoadingPolicyPreload:
            // append to the very end of the list
            //DDebug() << "LoadingPolicyPreload, Existing task " << existingTask << endl;
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
    LoadingTask *loadingTask = 0;
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
    for (LoadSaveTask *task = m_todo.first(); task; task = m_todo.next())
    {
        if (task != existingTask && checkLoadingTask(task, LoadingTaskFilterAll))
        {
            m_todo.remove();
            m_todo.prev();
        }
    }
    //DDebug()<<"loadPreview for " << description.filePath << " existingTask " << existingTask << " currentTask " << m_currentTask << endl;
    // append new loading task
    if (existingTask)
        return;
    m_todo.append(new PreviewLoadingTask(this, description));
    m_condVar.wakeAll();
}


LoadingTask *ManagedLoadSaveThread::createLoadingTask(const LoadingDescription &description,
         bool preloading, LoadingMode loadingMode, AccessMode accessMode)
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
    for (LoadSaveTask *task = m_todo.first(); task; task = m_todo.next())
    {
        if (task->type() ==  LoadSaveTask::TaskTypeSaving)
        {
            SavingTask *savingTask = (SavingTask *)m_currentTask;
            if (filePath.isNull() || savingTask->filePath() == filePath)
            {
                m_todo.remove();
                m_todo.prev();
            }
        }
    }
}


void ManagedLoadSaveThread::removeLoadingTasks(const LoadingDescription &description, LoadingTaskFilter filter)
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
    for (LoadSaveTask *task = m_todo.first(); task; task = m_todo.next())
    {
        if ( (loadingTask = checkLoadingTask(task, filter)) )
        {
            if (description.filePath.isNull() || loadingTask->loadingDescription() == description)
            {
                m_todo.remove();
                m_todo.prev();
            }
        }
    }
}

void ManagedLoadSaveThread::save(DImg &image, const QString& filePath, const QString &format)
{
    QMutexLocker lock(&m_mutex);
    LoadingTask *loadingTask;

    // stop and postpone current task if it is a preloading task
    if (m_currentTask && (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)))
    {
        loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
        load(loadingTask->filePath(), LoadingPolicyPreload);
    }
    // append new loading task, put it in front of preloading tasks
    uint i;
    for (i = 0; i<m_todo.count(); i++)
    {
        LoadSaveTask *task = m_todo.at(i);
        if ( (loadingTask = checkLoadingTask(task, LoadingTaskFilterPreloading)) )
            break;
    }
    m_todo.insert(i, new SavingTask(this, image, filePath, format));
}

}   // namespace Digikam

