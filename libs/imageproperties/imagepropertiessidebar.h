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
 
#ifndef IMAGEPROPERTIESSIDEBAR_H
#define IMAGEPROPERTIESSIDEBAR_H

// Qt includes.

#include <qrect.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "sidebar.h"
#include "digikam_export.h"

namespace Digikam
{

class ImagePropertiesEXIFTab;
class ImagePropertiesHistogramTab;

class DIGIKAM_EXPORT ImagePropertiesSideBar : public Digikam::Sidebar
{
    Q_OBJECT

public:

    ImagePropertiesSideBar(QWidget* parent, QSplitter *splitter, Side side=Left, bool mimimizedDefault=false);
                    
    ~ImagePropertiesSideBar();
    
    void itemChanged(const KURL& url, QRect *rect, 
                     uchar *imageData, int imageWidth, int imageHeight, bool sixteenBit);
                    
    void imageSelectionChanged(QRect *rect);                 
    
    void noCurrentItem(void);                               

private:

    bool                         m_dirtyExifTab;
    bool                         m_dirtyHistogramTab;
    bool                         m_sixteenBit;
    
    int                          m_imageWidth;
    int                          m_imageHeight;
    uchar                       *m_imageData;
    
    QRect                       *m_currentRect;

    KURL                         m_currentURL;

    ImagePropertiesEXIFTab      *m_exifTab;   
    ImagePropertiesHistogramTab *m_histogramTab;
    
private slots:

    void slotChangedTab(QWidget* tab);
};

}  // NameSpace Digikam

#endif  // IMAGEPROPERTIESSIDEBAR_H
