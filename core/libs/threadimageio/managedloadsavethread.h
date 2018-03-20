/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-16
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2005-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MANAGED_LOAD_SAVE_THREAD_H
#define MANAGED_LOAD_SAVE_THREAD_H

// Local includes

#include "loadsavethread.h"

namespace Digikam
{

class LoadingTask;
class LoadSaveTask;

class DIGIKAM_EXPORT ManagedLoadSaveThread : public LoadSaveThread
{

public:

    enum LoadingPolicy
    {
        /// Load image immediately, remove and stop all previous loading tasks.
        LoadingPolicyFirstRemovePrevious,
        /// Prepend loading in front of all other tasks, but wait for the current task to finish.
        /// No other tasks will be removed, preloading tasks will be stopped and postponed.
        LoadingPolicyPrepend,
        /// Prepend in front of all other tasks (not touching the current task).
        /// Do not check for duplicate tasks, do not check for preloading tasks.
        LoadingPolicySimplePrepend,
        /// Append loading task to the end of the list, but in front of all preloading tasks.
        /// No other tasks will be removed, preloading tasks will be stopped and postponed.
        /// This is similar to the simple load() operation from LoadSaveThread, except for the
        /// special care taken for preloading.
        LoadingPolicyAppend,
        /// Append to the lists of tasks.
        /// Do not check for duplicate tasks, do not check for preloading tasks.
        LoadingPolicySimpleAppend,
        /// Preload image, i.e. load it with low priority when no other tasks are scheduled.
        /// All other tasks will take precedence, and preloading tasks will be stopped and
        /// postponed when another task is added.
        /// No progress info will be sent for preloaded images
        LoadingPolicyPreload
    };

    enum TerminationPolicy
    {
        /// Wait for saving tasks, stop and remove loading tasks
        /// This is the default.
        TerminationPolicyTerminateLoading,
        /// Wait for loading and saving tasks, stop and remove preloading tasks
        TerminationPolicyTerminatePreloading,
        /// Wait for all pending tasks
        TerminationPolicyWait,
        /// Stop all pending tasks
        TerminationPolicyTerminateAll
    };

    enum LoadingTaskFilter
    {
        /// filter all loading tasks
        LoadingTaskFilterAll,
        /// filter only tasks with preloading policy
        LoadingTaskFilterPreloading
    };

    /// used by SharedLoadSaveThread only
    enum LoadingMode
    {
        /// no sharing of loading process, no caching of image
        LoadingModeNormal,
        /// loading process is shared, image is cached
        LoadingModeShared
    };

public:

    /// Termination is controlled by setting the TerminationPolicy
    /// Default is TerminationPolicyTerminateLoading
    explicit ManagedLoadSaveThread(QObject* const parent = 0);
    ~ManagedLoadSaveThread();

    /// Append a task to load the given file to the task list.
    /// If there is already a task for the given file, it will possibly be rescheduled,
    /// but no second task will be added.
    /// Only loading tasks will - if required by the policy - be stopped or removed,
    /// saving tasks will not be touched.
    void load(const LoadingDescription& description);
    void load(const LoadingDescription& description, LoadingPolicy policy);

    /// Stop and remove tasks filtered by filePath and policy.
    /// If filePath isNull, applies to all file paths.
    void stopLoading(const QString& filePath = QString(), LoadingTaskFilter filter = LoadingTaskFilterAll);

    /// Same than previous method, but Stop and remove tasks filtered by LoadingDescription.
    void stopLoading(const LoadingDescription& desc, LoadingTaskFilter filter = LoadingTaskFilterAll);

    /// Stop and remove saving tasks filtered by filePath.
    /// If filePath isNull, applies to all file paths.
    void stopSaving(const QString& filePath = QString());

    void stopAllTasks();

    /// Append a task to save the image to the task list
    void save(DImg& image, const QString& filePath, const QString& format);

    void              setTerminationPolicy(TerminationPolicy terminationPolicy);
    TerminationPolicy terminationPolicy() const;

    /**
     * Set the loading policy.
     * Default is LoadingPolicyAppend.
     * You can override the default value for each operation.
     */
    void          setLoadingPolicy(LoadingPolicy policy);
    LoadingPolicy loadingPolicy() const;

protected:

    void shutDown();

    void load(const LoadingDescription& description, LoadingMode loadingMode,
              AccessMode mode = AccessModeReadWrite);

    void load(const LoadingDescription& description, LoadingMode loadingMode,
              LoadingPolicy policy, AccessMode mode = AccessModeReadWrite);

    void loadPreview(const LoadingDescription& description, LoadingPolicy policy);
    void loadThumbnail(const LoadingDescription& description);

    void preloadThumbnail(const LoadingDescription& description);
    void preloadThumbnailGroup(const QList<LoadingDescription>& descriptions);
    void prependThumbnailGroup(const QList<LoadingDescription>& descriptions);

protected:

    LoadingPolicy     m_loadingPolicy;
    TerminationPolicy m_terminationPolicy;

private:

    LoadingTask* checkLoadingTask(LoadSaveTask* const task, LoadingTaskFilter filter) const;
    LoadingTask* findExistingTask(const LoadingDescription& description) const;
    LoadingTask* createLoadingTask(const LoadingDescription& description, bool preloading,
                                   LoadingMode loadingMode, AccessMode accessMode);

    void removeLoadingTasks(const LoadingDescription& description, LoadingTaskFilter filter);
};

}  // namespace Digikam

#endif // MANAGED_LOAD_SAVE_THREAD_H
