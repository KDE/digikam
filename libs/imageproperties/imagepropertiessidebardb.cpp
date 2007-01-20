/* ============================================================
 * Authors: Caulier Gilles <caulier dot gilles at kdemail dot net>
 *         Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2004-11-17
 * Description : image properties side bar using data from 
 *               digiKam database.
 *
 * Copyright 2004-2006 by Gilles Caulier
 * Copyright 2007 by Gilles Caulier and Marcel Wiesweg
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

#include <qrect.h>
#include <qcolor.h>
#include <qsplitter.h>

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
#include "themeengine.h"
#include "albumiconitem.h"
#include "albumiconview.h"
#include "navigatebarwidget.h"
#include "imagedescedittab.h"
#include "imageattributeswatch.h"
#include "imagepropertiestab.h"
#include "imagepropertiesmetadatatab.h"
#include "imagepropertiescolorstab.h"
#include "imagepropertiessidebardb.h"
#include "imagepropertiessidebardb.moc"

namespace Digikam
{

class ImagePropertiesSideBarDBPriv
{
public:

    ImagePropertiesSideBarDBPriv()
    {
        desceditTab         = 0;
        dirtyDesceditTab    = false;
        hasPrevious         = false;
        hasNext             = false;
    }

    bool              dirtyDesceditTab;

    QPtrList<ImageInfo> currentInfos;

    ImageDescEditTab *desceditTab;

    bool              hasPrevious;
    bool              hasNext;
};

ImagePropertiesSideBarDB::ImagePropertiesSideBarDB(QWidget *parent, const char *name, QSplitter *splitter, 
                                                   Side side, bool mimimizedDefault, bool navBar)
                        : ImagePropertiesSideBar(parent, name, splitter, side, mimimizedDefault, navBar)
{
    d = new ImagePropertiesSideBarDBPriv;
    d->desceditTab = new ImageDescEditTab(parent, navBar);

    appendTab(d->desceditTab, SmallIcon("imagecomment"), i18n("Comments/Tags"));

    slotThemeChanged();

    // ----------------------------------------------------------

    connectTab(m_propertiesTab);
    connectTab(m_metadataTab);
    connectTab(m_colorTab);
    connectTab(d->desceditTab);

    connect(this, SIGNAL(signalViewChanged()),
            this, SLOT(slotSetFocus()));

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(const KURL &)),
            this, SLOT(slotFileMetadataChanged(const KURL &)));
}

ImagePropertiesSideBarDB::~ImagePropertiesSideBarDB()
{
    delete d;
}

void ImagePropertiesSideBarDB::connectTab(NavigateBarTab *tab)
{
    connect(tab, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));

    connect(tab, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(tab, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(tab, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
}

void ImagePropertiesSideBarDB::itemChanged(ImageInfo *info,
                                           const QRect &rect, DImg *img)
{
    itemChanged(info->kurl(), info, rect, img);
}

void ImagePropertiesSideBarDB::itemChanged(const KURL& url, const QRect &rect, DImg *img)
{
    itemChanged(url, 0, rect, img);
}

void ImagePropertiesSideBarDB::itemChanged(const KURL& url, ImageInfo *info,
                                           const QRect &rect, DImg *img)
{
    if ( !url.isValid() )
        return;

    m_currentURL         = url;
    m_currentRect        = rect;
    m_image              = img;

    QPtrList<ImageInfo> list;
    if (info)
        list.append(info);
    d->currentInfos      = list;

    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    d->dirtyDesceditTab  = false;

    slotChangedTab( getActiveTab() );
}

void ImagePropertiesSideBarDB::itemChanged(QPtrList<ImageInfo> infos)
{
    if (infos.isEmpty())
        return;

    m_currentURL         = infos.first()->kurl();
    m_currentRect        = QRect();
    m_image              = 0;
    d->currentInfos      = infos;

    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    d->dirtyDesceditTab  = false;

    slotChangedTab( getActiveTab() );
}


void ImagePropertiesSideBarDB::slotNoCurrentItem(void)
{
    ImagePropertiesSideBar::slotNoCurrentItem();
    d->currentInfos.clear();
    d->desceditTab->setItem();
    d->desceditTab->setNavigateBarFileName();
    d->dirtyDesceditTab = false;
}

void ImagePropertiesSideBarDB::populateTags(void)
{
    d->desceditTab->populateTags();
}

void ImagePropertiesSideBarDB::setPreviousNextState(bool hasPrevious, bool hasNext)
{
    d->hasPrevious = hasPrevious;
    d->hasNext     = hasNext;

    NavigateBarTab *navtab = dynamic_cast<NavigateBarTab *>(getActiveTab());
    if (navtab)
        navtab->setNavigateBarState(d->hasPrevious, d->hasNext);
}

void ImagePropertiesSideBarDB::slotChangedTab(QWidget* tab)
{
    setCursor(KCursor::waitCursor());

    // No database data available, for example in the case of image editor is 
    // started from camera GUI.
    if (d->currentInfos.isEmpty())
    {
        if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
        {
            m_propertiesTab->setCurrentURL(m_currentURL);
            m_dirtyPropertiesTab = true;
        }
        else if (tab == m_metadataTab && !m_dirtyMetadataTab)
        {
            if (m_image)
                m_metadataTab->setCurrentData(m_image->getExif(), m_image->getIptc(), 
                                              m_currentURL.fileName());
            else
                m_metadataTab->setCurrentURL(m_currentURL);

            m_dirtyMetadataTab = true;
        }
        else if (tab == m_colorTab && !m_dirtyColorTab)
        {
            m_colorTab->setData(m_currentURL, m_currentRect, m_image);
            m_dirtyColorTab = true;
        }
        else if (tab == d->desceditTab && !d->dirtyDesceditTab)
        {
            // Do nothing here. We cannot get data from database !
            d->desceditTab->setItem();
            d->dirtyDesceditTab = true;
        }
    }
    else if (d->currentInfos.count() == 1)   // Data from database available...
    {
        if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
        {
            m_propertiesTab->setCurrentURL(m_currentURL);
            m_dirtyPropertiesTab = true;
        }
        else if (tab == m_metadataTab && !m_dirtyMetadataTab)
        {
            if (m_image)
                m_metadataTab->setCurrentData(m_image->getExif(), m_image->getIptc(),
                                              m_currentURL.fileName());
            else
                m_metadataTab->setCurrentURL(m_currentURL);

            m_dirtyMetadataTab = true;
        }
        else if (tab == m_colorTab && !m_dirtyColorTab)
        {
            m_colorTab->setData(m_currentURL, m_currentRect, m_image);
            m_dirtyColorTab = true;
        }
        else if (tab == d->desceditTab && !d->dirtyDesceditTab)
        {
            d->desceditTab->setItem(d->currentInfos.first());
            d->dirtyDesceditTab = true;
        }
    }
    else  // Data from database available, multiple selection
    {
        if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
        {
            //TODO
            m_propertiesTab->setCurrentURL(m_currentURL);
            m_dirtyPropertiesTab = true;
        }
        else if (tab == m_metadataTab && !m_dirtyMetadataTab)
        {
            // any ideas?
            m_metadataTab->setCurrentURL();
            m_dirtyMetadataTab = true;
        }
        else if (tab == m_colorTab && !m_dirtyColorTab)
        {
            // any ideas?
            m_colorTab->setData();
            m_dirtyColorTab = true;
        }
        else if (tab == d->desceditTab && !d->dirtyDesceditTab)
        {
            d->desceditTab->setItems(d->currentInfos);
            d->dirtyDesceditTab = true;
        }
    }

    // setting of NavigateBar, common for all tabs
    // there may be tabs added that we don't know of
    NavigateBarTab *navtab = dynamic_cast<NavigateBarTab *>(tab);
    if (navtab)
    {
        if (d->currentInfos.count() == 1)
        {
            navtab->setNavigateBarState(d->hasPrevious, d->hasNext);
            navtab->setNavigateBarFileName(m_currentURL.filename());
        }
        else
        {
            navtab->setNavigateBarState(false, false);
            if (tab == d->desceditTab)
                navtab->setLabelText(i18n("<qt>Editing <b>%1</b> pictures</qt>").arg(d->currentInfos.count()));
            else
                navtab->setLabelText(i18n("<qt><b>%1</b> pictures selected</qt>").arg(d->currentInfos.count()));
        }
    }

    slotSetFocus();

    unsetCursor();
}

void ImagePropertiesSideBarDB::slotSetFocus()
{
    // See B.K.O #131632 and #131743 : always give focus to Comments widget 
    // when we toogle between tab and when we change current item.

    if (getActiveTab() == d->desceditTab && isExpanded())
        d->desceditTab->setFocusToComments(true);
    else
        d->desceditTab->setFocusToComments(false);
}

void ImagePropertiesSideBarDB::slotFileMetadataChanged(const KURL &url)
{
    if (url == m_currentURL)
    {
        if (getActiveTab() == m_metadataTab)
        {
            // reuse code form slotChangedTab
            m_dirtyMetadataTab = false;
            slotChangedTab( getActiveTab() );
        }
    }
}

void ImagePropertiesSideBarDB::slotAssignRating(int rating)
{
    d->desceditTab->assignRating(rating);
}

void ImagePropertiesSideBarDB::slotAssignRatingNoStar()
{
    d->desceditTab->assignRating(0);
}

void ImagePropertiesSideBarDB::slotAssignRatingOneStar()
{
    d->desceditTab->assignRating(1);
}

void ImagePropertiesSideBarDB::slotAssignRatingTwoStar()
{
    d->desceditTab->assignRating(2);
}

void ImagePropertiesSideBarDB::slotAssignRatingThreeStar()
{
    d->desceditTab->assignRating(3);
}

void ImagePropertiesSideBarDB::slotAssignRatingFourStar()
{
    d->desceditTab->assignRating(4);
}

void ImagePropertiesSideBarDB::slotAssignRatingFiveStar()
{
    d->desceditTab->assignRating(5);
}

void ImagePropertiesSideBarDB::slotThemeChanged()
{
    QColor backgroundColor(ThemeEngine::instance()->baseColor());
    QColor foregroundColor(ThemeEngine::instance()->textRegColor());
    m_propertiesTab->colorChanged(backgroundColor, foregroundColor);
}

}  // NameSpace Digikam
