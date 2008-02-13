/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : simple image properties side bar (without support 
 *               of digiKam database).
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QRect>
#include <QSplitter>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "imagepropertiesgpstab.h"
#include "imagepropertiestab.h"
#include "imagepropertiesmetadatatab.h"
#include "imagepropertiescolorstab.h"
#include "imagepropertiessidebar.h"
#include "imagepropertiessidebar.moc"

namespace Digikam
{

ImagePropertiesSideBar::ImagePropertiesSideBar(QWidget *parent,
                                               QSplitter *splitter, Side side, 
                                               bool mimimizedDefault)
                      : Sidebar(parent, side, mimimizedDefault)
{
    m_image              = 0;
    m_currentRect        = QRect();
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;

    m_propertiesTab = new ImagePropertiesTab(parent);
    m_metadataTab   = new ImagePropertiesMetaDataTab(parent);
    m_colorTab      = new ImagePropertiesColorsTab(parent);
    m_gpsTab        = new ImagePropertiesGPSTab(parent);

    setSplitter(splitter);

    appendTab(m_propertiesTab, SmallIcon("document-properties"), i18n("Properties"));
    appendTab(m_metadataTab, SmallIcon("exifinfo"), i18n("Metadata"));
    appendTab(m_colorTab, SmallIcon("format-fill-color"), i18n("Colors"));
    appendTab(m_gpsTab, SmallIcon("applications-internet"), i18n("Geolocation"));

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBar::~ImagePropertiesSideBar()
{
}

void ImagePropertiesSideBar::itemChanged(const KUrl& url, const QRect &rect, DImg *img)
{
    if (!url.isValid())
        return;

    m_currentURL         = url;
    m_currentRect        = rect;
    m_image              = img;
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;

    slotChangedTab( getActiveTab() );
}

void ImagePropertiesSideBar::slotNoCurrentItem()
{
    m_currentURL = KUrl();

    m_propertiesTab->setCurrentURL();
    m_metadataTab->setCurrentURL();
    m_colorTab->setData();
    m_gpsTab->setCurrentURL();

    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
}

void ImagePropertiesSideBar::slotImageSelectionChanged(const QRect &rect)
{
    m_currentRect = rect;

    if (m_dirtyColorTab)
       m_colorTab->setSelection(rect);
    else
       slotChangedTab(m_colorTab);
}

void ImagePropertiesSideBar::slotChangedTab(QWidget* tab)
{
    if (!m_currentURL.isValid())
        return;

    setCursor(Qt::WaitCursor);

    if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
    {
       m_propertiesTab->setCurrentURL(m_currentURL);
       m_dirtyPropertiesTab = true;
    }
    else if (tab == m_metadataTab && !m_dirtyMetadataTab)
    {
       m_metadataTab->setCurrentURL(m_currentURL);
       m_dirtyMetadataTab = true;
    }
    else if (tab == m_colorTab && !m_dirtyColorTab)
    {
       m_colorTab->setData(m_currentURL, m_currentRect, m_image);
       m_dirtyColorTab = true;
    }
    else if (tab == m_gpsTab && !m_dirtyGpsTab)
    {
       m_gpsTab->setCurrentURL(m_currentURL);
       m_dirtyGpsTab = true;
    }

    unsetCursor();
}

}  // NameSpace Digikam
