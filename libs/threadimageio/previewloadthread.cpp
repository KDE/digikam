/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewloadthread.moc"

// Local includes

#include "iccmanager.h"
#include "previewtask.h"

namespace Digikam
{

PreviewLoadThread::PreviewLoadThread(QObject* parent)
    : ManagedLoadSaveThread(parent),
      m_displayingWidget(0)
{
    m_loadingPolicy = LoadingPolicyFirstRemovePrevious;
}

LoadingDescription PreviewLoadThread::createLoadingDescription(const QString& filePath, int size, bool exifRotate)
{
    LoadingDescription description(filePath, size, exifRotate);

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
        description.postProcessingParameters.setProfile(IccManager::displayProfile(m_displayingWidget));
    }

    return description;
}

void PreviewLoadThread::load(const QString& filePath, int size, bool exifRotate)
{
    load(createLoadingDescription(filePath, size, exifRotate));
}

void PreviewLoadThread::loadFastButLarge(const QString& filePath, int size, bool exifRotate)
{
    LoadingDescription description = createLoadingDescription(filePath, size, exifRotate);
    description.previewParameters.flags |= LoadingDescription::PreviewParameters::FastButLarge;
    load(description);
}

void PreviewLoadThread::loadHighQuality(const QString& filePath, bool exifRotate)
{
    load(filePath, 0, exifRotate);
}

void PreviewLoadThread::load(LoadingDescription description)
{
    // creates a PreviewLoadingTask, which uses different mechanisms than a normal loading task
    ManagedLoadSaveThread::loadPreview(description, m_loadingPolicy);
}

void PreviewLoadThread::setDisplayingWidget(QWidget* widget)
{
    m_displayingWidget = widget;
}

}   // namespace Digikam
