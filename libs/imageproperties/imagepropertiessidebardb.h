/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description : image properties side bar using data from 
 *               digiKam database.
 *
 * Copyright 2004-2006 by Gilles Caulier
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
 
#ifndef IMAGEPROPERTIESSIDEBARDB_H
#define IMAGEPROPERTIESSIDEBARDB_H

// Qt includes.

#include <qrect.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "sidebar.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;
class AlbumIconItem;
class AlbumIconView;
class ImagePropertiesEXIFTab;
class ImagePropertiesColorsTab;
class ImageDescEditTab;

class DIGIKAM_EXPORT ImagePropertiesSideBarDB : public Digikam::Sidebar
{
    Q_OBJECT

public:

    ImagePropertiesSideBarDB(QWidget* parent, QSplitter *splitter, Side side=Left, 
                             bool mimimizedDefault=false, bool navBar=true);
                    
    ~ImagePropertiesSideBarDB();
    
    void itemChanged(const KURL& url, AlbumIconView* view, AlbumIconItem* item, 
                     QRect *rect=0, DImg *img=0);
                    
    void imageSelectionChanged(QRect *rect);                 
    
    void noCurrentItem(void);                               

    void populateTags(void);                               
    
signals:

    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void);  
                                   
private:

    bool                      m_dirtyExifTab;
    bool                      m_dirtyHistogramTab;
    bool                      m_dirtyDesceditTab;
    
    QRect                    *m_currentRect;

    KURL                      m_currentURL;
    
    AlbumIconView            *m_currentView;
    
    AlbumIconItem            *m_currentItem;

    DImg                     *m_img;
    
    ImagePropertiesEXIFTab   *m_exifTab;
    ImagePropertiesColorsTab *m_histogramTab;
    ImageDescEditTab         *m_desceditTab;

private slots:

    void slotChangedTab(QWidget* tab);
        
};

}  // NameSpace Digikam

#endif  // IMAGEPROPERTIESSIDEBARDB_H
