/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : JPEG image Converter batch tool.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "convert2jpeg.h"
#include "convert2jpeg.moc"

// Qt includes.

#include <QWidget>
#include <QFileInfo>

// KDE includes.

#include <kconfiggroup.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

// Local includes.

#include "dimg.h"
#include "jpegsettings.h"

namespace Digikam
{

Convert2JPEG::Convert2JPEG(QObject* parent)
            : BatchTool("Convert2JPEG", BaseTool, parent)
{
    setToolTitle(i18n("Convert To JPEG"));
    setToolDescription(i18n("A tool to convert image to JPEG format"));
    setToolIcon(KIcon(SmallIcon("image-jpeg")));

    m_settings = new JPEGSettings;
    setSettingsWidget(m_settings);

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

Convert2JPEG::~Convert2JPEG()
{
}

BatchToolSettings Convert2JPEG::defaultSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    int compression           = group.readEntry("JPEGCompression", 75);
    int subSampling           = group.readEntry("JPEGSubSampling", 1);  // Medium subsampling
    BatchToolSettings settings;
    settings.insert("Quality",     compression);
    settings.insert("SubSampling", subSampling);
    return settings;
}

void Convert2JPEG::assignSettings2Widget()
{
    m_settings->setCompressionValue(settings()["Quality"].toInt());
    m_settings->setSubSamplingValue(settings()["SubSampling"].toInt());
}

void Convert2JPEG::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Quality",     m_settings->getCompressionValue());
    settings.insert("SubSampling", m_settings->getSubSamplingValue());
    setSettings(settings);
}

void Convert2JPEG::setOutputUrlFromInputUrl()
{
    BatchTool::setOutputUrlFromInputUrl();
    KUrl url     = outputUrl();
    QString base = url.fileName();
    base.append(".jpg");
    url.setFileName(base);
    setOutputUrl(url);
}

bool Convert2JPEG::toolOperations()
{
    DImg img;
    if (!img.load(inputUrl().path()))
        return false;

    img.setAttribute("quality",     settings()["Quality"].toInt());
    img.setAttribute("subsampling", settings()["SubSampling"].toInt());
    img.updateMetadata("JPEG", QString(), getExifSetOrientation());

    return( img.save(outputUrl().path(), "JPEG") );
}

}  // namespace Digikam
