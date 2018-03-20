/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
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

#include "previewloadthread.h"

// Local includes

#include "iccmanager.h"
#include "previewtask.h"

namespace Digikam
{

PreviewLoadThread::PreviewLoadThread(QObject* const parent)
    : ManagedLoadSaveThread(parent),
      m_displayingWidget(0)
{
    m_loadingPolicy = LoadingPolicyFirstRemovePrevious;
}

LoadingDescription PreviewLoadThread::createLoadingDescription(const QString& filePath, const PreviewSettings& settings, int size)
{
    return createLoadingDescription(filePath, settings, size, IccManager::displayProfile(m_displayingWidget));
}

LoadingDescription PreviewLoadThread::createLoadingDescription(const QString& filePath, const PreviewSettings& previewSettings, int size, const IccProfile& displayProfile)
{
    LoadingDescription description(filePath, previewSettings, size);

    if (DImg::fileFormat(filePath) == DImg::RAW)
    {
        description.rawDecodingSettings.optimizeTimeLoading();
        description.rawDecodingSettings.rawPrm.sixteenBitsImage   = false;
        description.rawDecodingSettings.rawPrm.halfSizeColorImage = true;
        description.rawDecodingHint                               = LoadingDescription::RawDecodingTimeOptimized;
    }

    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (settings.enableCM && settings.useManagedPreviews)
    {
        description.postProcessingParameters.colorManagement = LoadingDescription::ConvertForDisplay;
        if (displayProfile.isNull())
        {
            description.postProcessingParameters.setProfile(IccProfile::sRGB());
        }
        else
        {
            description.postProcessingParameters.setProfile(displayProfile);
        }
    }

    return description;
}

void PreviewLoadThread::loadFast(const QString& filePath, int size)
{
    PreviewSettings settings(PreviewSettings::FastPreview);
    load(createLoadingDescription(filePath, settings, size));
}

void PreviewLoadThread::loadFastButLarge(const QString& filePath, int size)
{
    PreviewSettings settings(PreviewSettings::FastButLargePreview);
    load(createLoadingDescription(filePath, settings, size));
}

void PreviewLoadThread::loadHighQuality(const QString& filePath, PreviewSettings::RawLoading rawLoadingMode)
{
    PreviewSettings settings(PreviewSettings::HighQualityPreview, rawLoadingMode);
    load(createLoadingDescription(filePath, settings, 0));
}

void PreviewLoadThread::load(const QString& filePath, const PreviewSettings& settings, int size)
{
    load(createLoadingDescription(filePath, settings, size));
}

void PreviewLoadThread::load(const LoadingDescription& description)
{
    // creates a PreviewLoadingTask, which uses different mechanisms than a normal loading task
    ManagedLoadSaveThread::loadPreview(description, m_loadingPolicy);
}

void PreviewLoadThread::setDisplayingWidget(QWidget* const widget)
{
    m_displayingWidget = widget;
}

DImg PreviewLoadThread::loadFastSynchronously(const QString& filePath, int size, const IccProfile& profile)
{
    PreviewSettings settings(PreviewSettings::FastPreview);
    return loadSynchronously(createLoadingDescription(filePath, settings, size, profile));
}

DImg PreviewLoadThread::loadFastButLargeSynchronously(const QString& filePath, int minimumSize, const IccProfile& profile)
{
    PreviewSettings settings(PreviewSettings::FastButLargePreview);
    return loadSynchronously(createLoadingDescription(filePath, settings, minimumSize, profile));
}

DImg PreviewLoadThread::loadHighQualitySynchronously(const QString& filePath, PreviewSettings::RawLoading rawLoadingMode, const IccProfile& profile)
{
    PreviewSettings settings(PreviewSettings::HighQualityPreview, rawLoadingMode);
    return loadSynchronously(createLoadingDescription(filePath, settings, 0, profile));
}

DImg PreviewLoadThread::loadSynchronously(const QString& filePath, const PreviewSettings& previewSettings, int size, const IccProfile& profile)
{
    return loadSynchronously(createLoadingDescription(filePath, previewSettings, size, profile));
}

DImg PreviewLoadThread::loadSynchronously(const LoadingDescription& description)
{
    PreviewLoadingTask task(0, description);
    task.execute();
    return task.img();
}

}   // namespace Digikam
