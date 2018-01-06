/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2005-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef LOAD_SAVE_THREAD_H
#define LOAD_SAVE_THREAD_H

// Qt includes

#include <QThread>
#include <QMutex>
#include <QList>
#include <QDateTime>
#include <QWaitCondition>
#include <QEvent>

// Local includes

#include "dimg.h"
#include "digikam_export.h"
#include "dynamicthread.h"
#include "loadingdescription.h"

namespace Digikam
{

class DMetadata;
class LoadSaveTask;

class DIGIKAM_EXPORT LoadSaveNotifier
{
public:

    virtual ~LoadSaveNotifier() {};

    virtual void imageStartedLoading(const LoadingDescription& loadingDescription) = 0;
    virtual void loadingProgress(const LoadingDescription& loadingDescription, float progress) = 0;
    virtual void imageLoaded(const LoadingDescription& loadingDescription, const DImg& img) = 0;
    virtual void moreCompleteLoadingAvailable(const LoadingDescription& oldLoadingDescription,
                                              const LoadingDescription& newLoadingDescription) = 0;
    virtual void imageStartedSaving(const QString& filePath) = 0;
    virtual void savingProgress(const QString& filePath, float progress) = 0;
    virtual void imageSaved(const QString& filePath, bool success) = 0;
    virtual void thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img) = 0;
};

// -------------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT LoadSaveFileInfoProvider
{
public:

    virtual ~LoadSaveFileInfoProvider() {}
    /**
     * Gives a hint at the orientation of the image.
     * This can be used to supersede the Exif information in the file.
     * Will not be used if DMetadata::ORIENTATION_UNSPECIFIED (default value)
     */
    virtual int orientationHint(const QString& path) = 0;
};

// -------------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT LoadSaveThread : public DynamicThread, public LoadSaveNotifier
{
    Q_OBJECT

public:

    enum NotificationPolicy
    {
        /** Always send notification, unless the last event is still in the event queue */
        NotificationPolicyDirect,
        /**
         * Always wait for a certain amount of time after the last event sent.
         * In particular, the first event will be sent only after waiting for this time span.
         * (Or no event will be sent, when the loading has finished before)
         * This is the default.
         */
        NotificationPolicyTimeLimited
    };

    // used by SharedLoadSaveThread only
    enum AccessMode
    {
        // image will only be used for reading
        AccessModeRead,
        // image data will possibly be changed
        AccessModeReadWrite
    };

public:

    explicit LoadSaveThread(QObject* parent = 0);
    /**
     * Destructor:
     * The thread will execute all pending tasks and wait for this upon destruction
     */
    ~LoadSaveThread();

    /** Append a task to load the given file to the task list */
    void load(const LoadingDescription& description);
    /** Append a task to save the image to the task list */
    void save(DImg& image, const QString& filePath, const QString& format);

    void setNotificationPolicy(NotificationPolicy notificationPolicy);

    static void setInfoProvider(LoadSaveFileInfoProvider* infoProvider);
    static LoadSaveFileInfoProvider* infoProvider();

    /**
     * Utility to make sure that an image is rotated according to Exif tag.
     * Detects if an image has previously already been rotated: You can
     * call this method more than one time on the same image.
     * Returns true if the image has actually been rotated or flipped.
     * Returns false if a rotation was not needed.
     */
    static bool exifRotate(DImg& image, const QString& filePath);
    static bool wasExifRotated(DImg& image);

    /**
     * Reverses the previous function
     */
    static bool reverseExifRotate(DImg& image, const QString& filePath);

    /**
     * Retrieves the Exif orientation, either from the info provider if available,
     * or from the metadata
     */
    static int exifOrientation(const DImg& image, const QString& filePath);
    static int exifOrientation(const QString& filePath, const DMetadata& metadata,
                               bool isRaw, bool fromRawEmbeddedPreview);

Q_SIGNALS:

    /** All signals are delivered to the thread from where the LoadSaveThread object
     *  has been created. This thread must use its event loop to get the signals.
     *  You must connect to these signals with Qt::AutoConnection (default) or Qt::QueuedConnection.
     */

    /** This signal is emitted when the loading process begins. */
    void signalImageStartedLoading(const LoadingDescription& loadingDescription);

    /**
     * This signal is emitted whenever new progress info is available
     * and the notification policy allows emitting the signal.
     * No progress info will be sent for preloaded images (ManagedLoadSaveThread).
     */
    void signalLoadingProgress(const LoadingDescription& loadingDescription, float progress);

    /**
     * This signal is emitted when the loading process has finished.
     * If the process failed, img is null.
     */
    void signalImageLoaded(const LoadingDescription& loadingDescription, const DImg& img);

    /**
     * This signal is emitted if
     *  - you are doing shared loading (SharedLoadSaveThread)
     *  - you started a loading operation with a LoadingDescription for
     *    a reduced version of the image
     *  - another thread started a loading operation for a more complete version
     * You may want to cancel the current operation and start with the given loadingDescription
     */
    void signalMoreCompleteLoadingAvailable(const LoadingDescription& oldLoadingDescription,
                                            const LoadingDescription& newLoadingDescription);

    void signalImageStartedSaving(const QString& filePath);
    void signalSavingProgress(const QString& filePath, float progress);
    void signalImageSaved(const QString& filePath, bool success);

    void signalThumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img);

public:

    virtual void imageStartedLoading(const LoadingDescription& loadingDescription);
    virtual void loadingProgress(const LoadingDescription& loadingDescription, float progress);
    virtual void imageLoaded(const LoadingDescription& loadingDescription, const DImg& img);
    virtual void moreCompleteLoadingAvailable(const LoadingDescription& oldLoadingDescription,
                                              const LoadingDescription& newLoadingDescription);
    virtual void imageStartedSaving(const QString& filePath);
    virtual void savingProgress(const QString& filePath, float progress);
    virtual void imageSaved(const QString& filePath, bool success);
    virtual void thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img);

    virtual bool querySendNotifyEvent();
    virtual void taskHasFinished();

protected:

    virtual void run();
    void notificationReceived();

protected:

    QMutex               m_mutex;

    QList<LoadSaveTask*> m_todo;

    LoadSaveTask*        m_currentTask;

    NotificationPolicy   m_notificationPolicy;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // LOAD_SAVE_THREAD_H
