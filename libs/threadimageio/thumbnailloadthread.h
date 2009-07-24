/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBNAIL_LOAD_THREAD_H
#define THUMBNAIL_LOAD_THREAD_H

// Qt includes

#include <QPixmap>

// Local includes

#include "managedloadsavethread.h"

class KFileItem;
class KJob;

namespace Digikam
{

class ThumbnailCreator;
class ThumbnailInfoProvider;
class ThumbnailLoadThreadPriv;

class DIGIKAM_EXPORT ThumbnailLoadThread : public ManagedLoadSaveThread
{
    Q_OBJECT

public:

    ThumbnailLoadThread();
    ~ThumbnailLoadThread();

    /**
     * Return application-wide default thumbnail threads.
     * It is perfectly all right to create an extra object of the class,
     * but it is useful to have default object
     */
    static ThumbnailLoadThread *defaultIconViewThread();
    static ThumbnailLoadThread *defaultThumbBarThread();
    static ThumbnailLoadThread *defaultThread();

    static void cleanUp();

    /**
     * Enable loading of thumbnails from a thumbnail database.
     * This shall be called once at application startup.
     * This need not be called, then the FreeDesktop standard is used.
     * You can optionally provide a thumbnail info provider.
     */
    static void initializeThumbnailDatabase(const QString &databaseType, const QString &databaseName,
            const QString &databaseHostName, int databasePort,
            const QString &databaseUserName, const QString &databaseUserPassword,
            const QString &databaseConnectOptions, ThumbnailInfoProvider *provider = 0);

    /**
     * Find a thumbnail.
     * If the pixmap is found in the cache, returns true and sets pixmap
     * to the found QPixmap.
     * If the pixmap is not found in the cache, load() is called to start the loading process,
     * false is returned and pixmap is not touched.
     */
    bool find(const QString& filePath, QPixmap& pixmap);

    /**
     * Same as above, but does not use the global size, but an extra specified size.
     */
    bool find(const QString& filePath, QPixmap& pixmap, int size);

    /**
     * Find a thumbnail.
     * This method sends the signals and does not return values like the method above.
     * If you certainly need asynchronous return, connect with Qt::QueuedConnection to the signals.
     * If you connect directly, the signals may be sent from within the method call.
     */
    void find(const QString& filePath);

    /**
     * Same as above, but does not use the global size, but an extra specified size.
     */
    void find(const QString& filePath, int size);

    /**
     * Find a group of thumbnails. The items will be loaded in order and signals will be sent.
     * Can be used to ensure that thumbnails are loaded in a particular order
     */
    void findGroup(const QStringList& filePaths);
    void findGroup(const QStringList& filePaths, int size);

    /**
     * Preload the thumbnail.
     */
    void preload(const QString& filePath);
    void preload(const QString& filePath, int size);

    /**
     * Load a thumbnail.
     * You do not need to use this method directly, it will not access the pixmap cache. Use find().
     * The LoadingDescription shall be constructed with the constructor for preview/thumbnail jobs.
     * (in the description constructor, you need to specify file path, thumbnail size and Exif rotation)
     */
    void load(const LoadingDescription& description);

    /// If the thread is currently loading thumbnails, there is no guarantee as to when
    /// the property change by one of the following methods takes effect.

    /**
     * Set the requested thumbnail size.
     * Default value: 128
     */
    void setThumbnailSize(int size);

    /**
     * Returns the maximum possible size of a thumbnail.
     * If you request a larger size, the thumbnail will not load.
     */
    static int maximumThumbnailSize();

    /**
     * Specify if the thumbnails shall be rotated by Exif.
     * Note: This only applies to newly created thumbnails. The rotation state of thumbnails
     * found in the disk cache is unknown, so they are not rotated.
     * (The only, unsatisfactory solution is the forced recreation of all thumbnails)
     * Default value: true
     */
    void setExifRotate(int exifRotate);

    /**
     * Return true is thumbnails shall be rotated by Exif.
     */
    bool exifRotate() const;

    /**
     * If you enable this, the signal thumbnailLoaded(LoadingDescription, QPixmap) will be emitted.
     * If you do not enable this, only the QImage-based signal (see LoadSaveThread) will be emitted.
     *
     * If you disable this, pay attention to the (global) setting of the LoadingCache, which per default
     * does not cache the images !!
     *
     * Default value: Enabled.
     */
    void setPixmapRequested(bool wantPixmap);

    /**
     * If you enable this, a highlighting border will be drawn around the pixmap.
     * This option has only an effect if pixmapRequested is true.
     * Default value: Enabled.
     */
    void setHighlightPixmap(bool highlight);

    /**
     * If you enable this, the thread will try hard to send a pixmap if thumbnail loading failed.
     * It will use standard system icons to replace the real thumbnail.
     * If you disable this, a null QPixmap will be sent.
     * This does not influence the QImage-based signal; this signal will be emitted with a null
     * QImage regardless of this setting here, if the loading failed.
     * Default value: Enabled.
     */
    void setSendSurrogatePixmap(bool send);

    /**
     * This is a tool to force regeneration of thumbnails.
     * All thumbnail files for the given file will be removed from disk,
     * and the cached instances will be removed as well.
     * Use this method if you know that the contents of the file has changed.
     * This method works independently from the multithreaded thumbnail loading.
     */
    static void deleteThumbnail(const QString& filePath);

Q_SIGNALS:

    // See LoadSaveThread for a QImage-based thumbnailLoaded() signal.
    void signalThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& pix);

public:

    // For internal use - may only be used from the thread
    ThumbnailCreator *thumbnailCreator() const;

protected:

    virtual void thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img);

private:

    void load(const LoadingDescription& description, bool preload);
    void loadWithKDE(const LoadingDescription& description);
    void startKdePreviewJob();
    QPixmap surrogatePixmap(const LoadingDescription& loadingDescription);
    bool checkSize(int size);

Q_SIGNALS:

    // For internal use
    void thumbnailsAvailable();

private Q_SLOTS:

    void slotThumbnailsAvailable();
    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& thumb);
    void gotKDEPreview(const KFileItem &, const QPixmap& pix);
    void failedKDEPreview(const KFileItem &);
    void kdePreviewFinished(KJob *);

private:

    ThumbnailLoadThreadPriv* const d;
};

}   // namespace Digikam

#endif // SHARED_LOAD_SAVE_THREAD_H
