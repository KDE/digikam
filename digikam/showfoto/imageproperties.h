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
 
#ifndef IMAGEPROPERTIES_H
#define IMAGEPROPERTIES_H

// KDE includes.

#include <kdialogbase.h>
#include <kurl.h>

class ImagePropertiesGeneral;
class ImagePropertiesEXIF;
class ImagePropertiesHistogram;

class ImageProperties : public KDialogBase
{
    Q_OBJECT

public:

    // Single file mode (to be called from elsewhere)
    ImageProperties(QWidget* parent, const KURL& url, QRect* selectionArea=0, 
                    uint* imageData=0, int imageWidth=0, int imageHeight=0);
                    
    ~ImageProperties();

private:

    KURL                          m_currURL;
    QRect                        *m_selectionArea;
    
    // Image data when using from Image Editor.
    uint                         *m_imageData;
    int                           m_imageWidth;
    int                           m_imageHeight;

    ImagePropertiesGeneral       *m_generalPage;
    ImagePropertiesEXIF          *m_exifPage;
    ImagePropertiesHistogram     *m_histogramPage;
    
    void setupGui(void);

private slots:

    void slotItemChanged();
};

#endif  // IMAGEPROPERTIES_H
