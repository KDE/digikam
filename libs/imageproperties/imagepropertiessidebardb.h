/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2005 by Gilles Caulier
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

#include "digikam_export.h"
#include "sidebar.h"

class AlbumIconItem;
class AlbumIconView;

namespace Digikam
{
class ImagePropertiesEXIFTab;
class ImagePropertiesHistogramTab;
class ImageDescEditTab;

class DIGIKAM_EXPORT ImagePropertiesSideBarDB : public Digikam::Sidebar
{
    Q_OBJECT

public:

    ImagePropertiesSideBarDB(QWidget* parent, QSplitter *splitter, Side side=Left, 
                             bool mimimizedDefault=false, bool navBar=true);
                    
    ~ImagePropertiesSideBarDB();
    
    void itemChanged(const KURL& url, AlbumIconView* view, AlbumIconItem* item, 
                     QRect *rect=0, uint *imageData=0, int imageWidth=0, int imageHeight=0);
                    
    void imageSelectionChanged(QRect *rect);                 
    
    void noCurrentItem(void);                               

    void populateTags(void);                               
    
signals:

    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void);  
                                   
private:

    bool                         m_dirtyExifTab;
    bool                         m_dirtyHistogramTab;
    bool                         m_dirtyDesceditTab;

    int                          m_imageWidth;
    int                          m_imageHeight;
    uint                        *m_imageData;
    
    QRect                       *m_currentRect;

    KURL                         m_currentURL;
    
    AlbumIconView               *m_currentView; 
    
    AlbumIconItem               *m_currentItem;
    
    ImagePropertiesEXIFTab      *m_exifTab;   
    ImagePropertiesHistogramTab *m_histogramTab;
    ImageDescEditTab            *m_desceditTab;

private slots:

    void slotChangedTab(QWidget* tab);
        
};

}  // NameSpace Digikam

#endif  // IMAGEPROPERTIESSIDEBARDB_H
