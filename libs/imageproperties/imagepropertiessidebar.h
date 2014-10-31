/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : simple image properties side bar (without support
 *               of digiKam database).
 *
 * Copyright (C) 2004-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPROPERTIESSIDEBAR_H
#define IMAGEPROPERTIESSIDEBAR_H

// KDE includes

#include <kurl.h>

// Local includes

#include "config-digikam.h"
#include "sidebar.h"
#include "digikam_export.h"
#include "searchtextbar.h"

class QWidget;
class QRect;

namespace Digikam
{

class DImg;
class SidebarSplitter;
class ImagePropertiesTab;
class ImagePropertiesMetaDataTab;
class ImagePropertiesColorsTab;

#ifdef HAVE_KGEOMAP
class ImagePropertiesGPSTab;
#endif // HAVE_KGEOMAP

class DIGIKAM_EXPORT ImagePropertiesSideBar : public Sidebar
{
    Q_OBJECT

public:

    ImagePropertiesSideBar(QWidget* const parent, SidebarSplitter* const splitter,
                           KMultiTabBarPosition side=KMultiTabBar::Left, bool mimimizedDefault=false);
    ~ImagePropertiesSideBar();

    virtual void itemChanged(const KUrl& url, const QRect& rect = QRect(), DImg* const img = 0);

Q_SIGNALS:

    void signalSetupMetadataFilters(int);

public Q_SLOTS:

    void slotLoadMetadataFilters();
    void slotImageSelectionChanged(const QRect& rect);
    virtual void slotNoCurrentItem();

protected Q_SLOTS:

    virtual void slotChangedTab(QWidget* tab);

protected:

    /**
     * load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState();

    /**
     * save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState();

    virtual void setImagePropertiesInformation(const KUrl& url);

protected:

    bool                        m_dirtyPropertiesTab;
    bool                        m_dirtyMetadataTab;
    bool                        m_dirtyColorTab;
    bool                        m_dirtyGpsTab;
    bool                        m_dirtyHistoryTab;

    QRect                       m_currentRect;

    KUrl                        m_currentURL;

    DImg*                       m_image;

    ImagePropertiesTab*         m_propertiesTab;
    ImagePropertiesMetaDataTab* m_metadataTab;
    ImagePropertiesColorsTab*   m_colorTab;

#ifdef HAVE_KGEOMAP
    ImagePropertiesGPSTab*      m_gpsTab;
#endif // HAVE_KGEOMAP
};

}  // namespace Digikam

#endif  // IMAGEPROPERTIESSIDEBAR_H
