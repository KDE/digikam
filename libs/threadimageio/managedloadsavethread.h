/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-16
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

#ifndef MANAGED_LOAD_SAVE_THREAD_H
#define MANAGED_LOAD_SAVE_THREAD_H

#include "loadsavethread.h"

namespace Digikam
{

class LoadingTask;

class DIGIKAM_EXPORT ManagedLoadSaveThread : public LoadSaveThread
{

public:

    // Termination is controlled by setting the TerminationPolicy
    // Default is TerminationPolicyTerminateLoading
    ManagedLoadSaveThread();
    ~ManagedLoadSaveThread();

    enum LoadingPolicy
    {
        // Load image immediately, remove and stop all previous loading tasks.
        LoadingPolicyFirstRemovePrevious,
        // Prepend loading in front of all other tasks, but wait for the current task to finish.
        // No other tasks will be removed, preloading tasks will be stopped and postponed.
        LoadingPolicyPrepend,
        // Append loading task to the end of the list, but in front of all preloading tasks.
        // No other tasks will be removed, preloading tasks will be stopped and postponed.
        // This is similar to the simple load() operation from LoadSaveThread, except for the
        // special care taken for preloading.
        LoadingPolicyAppend,
        // Preload image, i.e. load it with low priority when no other tasks are scheduled.
        // All other tasks will take precedence, and preloading tasks will be stopped and
        // postponed when another task is added.
        // No progress info will be sent for preloaded images
        LoadingPolicyPreload
    };

    enum TerminationPolicy
    {
        // Wait for saving tasks, stop and remove loading tasks
        // This is the default.
        TerminationPolicyTerminateLoading,
        // Wait for loading and saving tasks, stop and remove preloading tasks
        TerminationPolicyTerminatePreloading,
        // Wait for all pending tasks
        TerminationPolicyWait
    };

    enum LoadingTaskFilter
    {
        // filter all loading tasks
        LoadingTaskFilterAll,
        // filter only tasks with preloading policy
        LoadingTaskFilterPreloading
    };

    // used by SharedLoadSaveThread only
    enum LoadingMode
    {
        // no sharing of loading process, no caching of image
        LoadingModeNormal,
        // loading process is shared, image is cached
        LoadingModeShared
    };

    // Append a task to load the given file to the task list.
    // If there is already a task for the given file, it will possibly be rescheduled,
    // but no second task will be added.
    // Only loading tasks will - if required by the policy - be stopped or removed,
    // saving tasks will not be touched.
    void load(LoadingDescription description, LoadingPolicy policy = LoadingPolicyAppend);
    // Stop and remove tasks filtered by filePath and policy.
    // If filePath isNull, applies to all file paths.
    void stopLoading(const QString& filePath = QString(), LoadingTaskFilter filter = LoadingTaskFilterAll);
    // Stop and remove saving tasks filtered by filePath.
    // If filePath isNull, applies to all file paths.
    void stopSaving(const QString& filePath = QString());

    // Append a task to save the image to the task list
    void save(DImg &image, const QString& filePath, const QString &format);

    void setTerminationPolicy(TerminationPolicy terminationPolicy);

protected:

    void load(LoadingDescription description, LoadingMode loadingMode,
              LoadingPolicy policy = LoadingPolicyAppend, AccessMode mode = AccessModeReadWrite);
    void loadPreview(LoadingDescription description);

private:

    LoadingTask *checkLoadingTask(class LoadSaveTask *task, LoadingTaskFilter filter);
    LoadingTask *findExistingTask(const LoadingDescription &description);
    LoadingTask *createLoadingTask(const LoadingDescription &description, bool preloading, LoadingMode loadingMode, AccessMode accessMode);
    void removeLoadingTasks(const LoadingDescription &description, LoadingTaskFilter filter);

    TerminationPolicy m_terminationPolicy;
};

}   // namespace Digikam


#endif // MANAGED_LOAD_SAVE_THREAD_H
