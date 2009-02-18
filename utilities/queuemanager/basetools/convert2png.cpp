/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : PNG image Converter.
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

#include "convert2png.h"
#include "convert2png.moc"

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
#include "pngsettings.h"

namespace Digikam
{

Convert2PNG::Convert2PNG(QObject* parent)
           : BatchTool("Convert2PNG", BaseTool, parent)
{
    setToolTitle(i18n("Convert To PNG"));
    setToolDescription(i18n("A tool to convert image to PNG format"));
    setToolIcon(KIcon(SmallIcon("image-png")));

    m_settings = new PNGSettings();
    setSettingsWidget(m_settings);

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

Convert2PNG::~Convert2PNG()
{
}

BatchToolSettings Convert2PNG::defaultSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    int compression           = group.readEntry("PNGCompression", 9);
    BatchToolSettings settings;
    settings.insert("Quality", compression);
    return settings;
}

void Convert2PNG::assignSettings2Widget()
{
    m_settings->setCompressionValue(settings()["Quality"].toInt());
}

void Convert2PNG::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Quality", m_settings->getCompressionValue());
    setSettings(settings);
}

void Convert2PNG::setOutputUrlFromInputUrl()
{
    BatchTool::setOutputUrlFromInputUrl();
    KUrl url     = outputUrl();
    QString base = url.fileName();
    base.append(".png");
    url.setFileName(base);
    setOutputUrl(url);
}

bool Convert2PNG::toolOperations()
{
    DImg img;
    if (!img.load(inputUrl().path()))
        return false;

    img.setAttribute("quality", settings()["Quality"].toInt());
    img.updateMetadata("PNG", QString(), getExifSetOrientation());

    return( img.save(outputUrl().path(), "PNG") );
}

}  // namespace Digikam
