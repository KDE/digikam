/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-16
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef PREVIEW_LOAD_THREAD_H
#define PREVIEW_LOAD_THREAD_H

// Local includes

#include "managedloadsavethread.h"
#include "previewsettings.h"

namespace Digikam
{

class DIGIKAM_EXPORT PreviewLoadThread : public ManagedLoadSaveThread
{
    Q_OBJECT

public:

    /**
     * Creates a preview load thread.
     * Provides three flavors of preview loading.
     * The default loading policy, for the typical usage in a preview widget,
     * always stops any previous tasks and loads the new task as soon as possible.
     */
    explicit PreviewLoadThread(QObject* const parent = 0);

    /**
     * Load a preview that is optimized for fast loading.
     * Raw decoding and color management settings will be adjusted.
     */
    void loadFast(const QString& filePath, int size);

    /**
     * Load a preview that is as large as possible without sacrificing speed
     * for performance. Especially, raw previews are taken if larger than the given size.
     * Raw decoding and color management settings will be adjusted.
     */
    void loadFastButLarge(const QString& filePath, int minimumSize);

    /**
     * Load a preview with higher resolution, trading more quality
     * for less speed.
     * Raw decoding and color management settings will be adjusted.
     */
    void loadHighQuality(const QString& filePath, PreviewSettings::RawLoading rawLoadingMode = PreviewSettings::RawPreviewAutomatic);

    /**
     * Load a preview.
     * Settings determine the loading mode.
     * For fast loading, size is preview area size.
     * For fast-but-large loading, it serves as a minimum size.
     * For high quality loading, it is ignored
     */
    void load(const QString& filePath, const PreviewSettings& settings, int size = 0);

    /**
     * Load a preview. Loading description will not be touched.
     */
    void load(const LoadingDescription& description);

    /// Optionally, set the displaying widget for color management
    void setDisplayingWidget(QWidget* const widget);

    /**
     * Synchronous versions of the above methods.
     * These are safe to call from the non-UI thread, as the IccProfile either passed or deduced independent from a displaying widget
     */
    static DImg loadFastSynchronously(const QString& filePath, int size, const IccProfile& profile = IccProfile());
    static DImg loadFastButLargeSynchronously(const QString& filePath, int minimumSize, const IccProfile& profile = IccProfile());
    static DImg loadHighQualitySynchronously(const QString& filePath, PreviewSettings::RawLoading rawLoadingMode = PreviewSettings::RawPreviewAutomatic, const IccProfile& profile = IccProfile());
    static DImg loadSynchronously(const QString& filePath, const PreviewSettings& previewSettings, int size, const IccProfile& profile = IccProfile());
    static DImg loadSynchronously(const LoadingDescription& description);

protected:

    static LoadingDescription createLoadingDescription(const QString& filePath, const PreviewSettings& settings, int size, const IccProfile& profile);
    LoadingDescription createLoadingDescription(const QString& filePath, const PreviewSettings& settings, int size);

protected:

    QWidget* m_displayingWidget;
};

}   // namespace Digikam

#endif // SHARED_LOAD_SAVE_THREAD_H
