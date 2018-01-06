/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QImage>

// Local includes

#include "managedloadsavethread.h"
#include "thumbnailinfo.h"

namespace Digikam
{

class DbEngineParameters;
class ThumbnailCreator;
class ThumbnailInfoProvider;

class DIGIKAM_EXPORT ThumbnailLoadThread : public ManagedLoadSaveThread
{
    Q_OBJECT

public:

    explicit ThumbnailLoadThread(QObject* const parent = 0);
    ~ThumbnailLoadThread();

    /**
     * Return application-wide default thumbnail threads.
     * It is perfectly all right to create an extra object of the class,
     * but it is useful to have default object
     */
    static ThumbnailLoadThread* defaultIconViewThread();
    static ThumbnailLoadThread* defaultThumbBarThread();
    static ThumbnailLoadThread* defaultThread();

    static void cleanUp();

    /**
     * Enable loading of thumbnails from a thumbnail database.
     * This shall be called once at application startup.
     * This need not be called, then the FreeDesktop standard is used.
     * You can optionally provide a thumbnail info provider.
     */
    static void initializeThumbnailDatabase(const DbEngineParameters& params, ThumbnailInfoProvider* const provider = 0);

    /**
     * For color management, this sets the widget the thumbnails will be color managed for.
     * (currently it is only possible to set one global widget)
     */
    static void setDisplayingWidget(QWidget* const widget);

    /**
     * Find a thumbnail.
     * If the pixmap is found in the cache, returns true and sets pixmap
     * to the found QPixmap.
     * If the pixmap is not found in the cache, load() is called to start the loading process,
     * false is returned and pixmap is not touched.
     */
    bool find(const ThumbnailIdentifier& identifier, QPixmap& pixmap);

    /**
     * Same as above, but does not use the global size, but an extra specified size.
     */
    bool find(const ThumbnailIdentifier& identifier, QPixmap& pixmap, int size);

    /**
     * Find a thumbnail.
     * This method sends the signals and does not return values like the method above.
     * If you certainly need asynchronous return, connect with Qt::QueuedConnection to the signals.
     * If you connect directly, the signals may be sent from within the method call.
     */
    void find(const ThumbnailIdentifier& identifier);

    /**
     * Same as above, but does not use the global size, but an extra specified size.
     */
    void find(const ThumbnailIdentifier& identifier, int size);

    /**
     * Find a group of thumbnails. The items will be loaded in order and signals will be sent.
     * Can be used to ensure that thumbnails are loaded in a particular order
     */
    void findGroup(QList<ThumbnailIdentifier>& identifiers);
    void findGroup(QList<ThumbnailIdentifier>& identifiers, int size);

    /**
     * All tastes of find() methods, for loading the thumbnail of a detail
     */
    bool find(const ThumbnailIdentifier& identifier, const QRect& rect, QPixmap& pixmap);
    bool find(const ThumbnailIdentifier& identifier, const QRect& rect, QPixmap& pixmap, int size);
    void find(const ThumbnailIdentifier& identifier, const QRect& rect);
    void find(const ThumbnailIdentifier& identifier, const QRect& rect, int size);
    void findGroup(const QList<QPair<ThumbnailIdentifier, QRect> >& filePathAndRects);
    void findGroup(const QList<QPair<ThumbnailIdentifier, QRect> >& filePathsAndRects, int size);

    /**
     * Preload the thumbnail or thumbnail group.
     * This is essentially the same as loading, but with a lower priority.
     */
    void preload(const ThumbnailIdentifier& identifier);
    void preload(const ThumbnailIdentifier& identifier, int size);
    void preloadGroup(QList<ThumbnailIdentifier>& identifiers);
    void preloadGroup(QList<ThumbnailIdentifier>& identifiers, int size);

    /**
     * Pregenerate the thumbnail group.
     * No signals will be emitted when these are loaded.
     */
    void pregenerateGroup(const QList<ThumbnailIdentifier>& identifiers);
    void pregenerateGroup(const QList<ThumbnailIdentifier>& identifiers, int size);

    /**
     * Load a thumbnail.
     * You do not need to use this method directly, it will not access the pixmap cache. Use find().
     * The LoadingDescription shall be constructed with the constructor for preview/thumbnail jobs.
     * (in the description constructor, you need to specify file path, thumbnail size and Exif rotation)
     */
    void load(const LoadingDescription& description);

    /**
     * Returns the descriptions used by the last call to any of the above methods.
     * After calling single-thumbnail methods (find, preload) the list will have size 1,
     * after the group methods (findGroup, preloadGroup, pregenerateGroup) the list can
     * be larger than 1.
     * There is no information if the description was ever scheduled in the thread,
     * already processed, skipped or canceled.
     */
    QList<LoadingDescription> lastDescriptions() const;

    /// If the thread is currently loading thumbnails, there is no guarantee as to when
    /// the property change by one of the following methods takes effect.

    /**
     * Set the requested thumbnail size.
     * Default value: 128
     */
    void setThumbnailSize(int size, bool forFace = false);

    /**
     * Returns the maximum possible size of a thumbnail.
     * If you request a larger size, the thumbnail will not load.
     * The size of the pixmap can slightly differ, especially when highlighting.
     */
    static int maximumThumbnailSize();
    static int maximumThumbnailPixmapSize(bool withHighlighting);

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
     * Computes the pixmap size for the give thumbnail size.
     * These can differ when highlighting is turned on.
     */
    int        thumbnailToPixmapSize(int size) const;
    static int thumbnailToPixmapSize(bool withHighlight, int size);

    /**
     * Computes the thumbnail size for the give pixmap size.
     */
    int pixmapToThumbnailSize(int size) const;

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
     * Stores the given detail thumbnail on disk.
     * Use this if possible because generation of detail thumbnails
     * is potentially slower.
     * The image should at least have storedSize().
     */
    void storeDetailThumbnail(const QString& filePath, const QRect& detailRect, const QImage& image, bool isFace = false);
    int  storedSize() const;

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
    ThumbnailCreator* thumbnailCreator() const;

protected:

    virtual void thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img);

private:

    bool find(const ThumbnailIdentifier& identifier, int size, QPixmap* retPixmap, bool emitSignal, const QRect& detailRect);
    void load(const LoadingDescription& description, bool pregenerate);
    void loadVideoThumbnail(const LoadingDescription& description);
    bool checkSize(int size);
    QPixmap surrogatePixmap(const LoadingDescription& loadingDescription);

Q_SIGNALS:

    // For internal use
    void thumbnailsAvailable();
    void ThumbnailLoaded(const LoadingDescription&, const QImage&);

private Q_SLOTS:

    void slotThumbnailsAvailable();
    void slotThumbnailLoaded(const LoadingDescription&, const QImage&);
    void slotVideoThumbnailDone(const QString&, const QImage&);
    void slotVideoThumbnailFailed(const QString&);
    void slotVideoThumbnailFinished();

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbnailImageCatcher : public QObject
{
    Q_OBJECT

public:

    /**
     *  Use this class to get a thumbnail synchronously.
     *  1. Create the ThumbnailImageCatcher object with your ThumbnailLoadThread
     *  2. a) Request a thumbnail
     *     b) Call enqueue()
     *  3. Call waitForThumbnails which returns the thumbnail QImage(s).
     *
     *  Note: Not meant for loading QPixmap thumbnails.
     */

    explicit ThumbnailImageCatcher(QObject* const parent = 0);
    explicit ThumbnailImageCatcher(ThumbnailLoadThread* const thread, QObject* const parent = 0);
    ~ThumbnailImageCatcher();

    ThumbnailLoadThread* thread() const;
    void setThumbnailLoadThread(ThumbnailLoadThread* const thread);

    /**
     * After requesting a thumbnail from the thread, call enqueue()
     * each time. Enqueue records the requested loading operation in an internal list.
     * A loading operation can result in the return of more than one thumbnail,
     * so enqueue() returns the number of expected results.
     * Then call waitForThumbnails. The returned list is the sum of previous calls
     * to enqueue, one entry per expected result, in order.
     * If stopped prematurely or loading failed, the respective entries will be null.
     */
    int           enqueue();
    QList<QImage> waitForThumbnails();

public Q_SLOTS:

    /**
     * The catcher is active per default after construction.
     * Deactivate it if you use the catcher as a longer-lived
     * object and do not use it for some time,
     * then activate it before you request a thumbnail from the thread again.
     */
    void setActive(bool active);

    /**
     * If the catcher is waiting in waitForThumbnails() in a different thread,
     * cancels the waiting. The results will be returned as received so far.
     */
    void cancel();

protected Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QImage&);

private:

    class Private;
    Private* const d;
};

}   // namespace Digikam

#endif // SHARED_LOAD_SAVE_THREAD_H
