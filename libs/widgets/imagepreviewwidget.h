/* ============================================================
 * File  : imagepreviewwidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-20
 * Description : 
 * 
 * Copyright 2004 Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qimage.h>
#include <qrect.h>

class QLabel;

namespace Digikam
{

class ImageRegionWidget;
class ImagePanIconWidget;

class ImagePreviewWidget : public QWidget
{
Q_OBJECT

public:
    ImagePreviewWidget(uint w, uint h, const QString &title, 
                       QWidget *parent=0);
    ~ImagePreviewWidget();
    
    QRect  getOriginalImageRegion(void);
    QImage getOriginalClipImage(void);
    void   setPreviewImageData(QImage img);
    void   setPreviewImageWaitCursor(bool enable);
    void   setCenterImageRegionPosition(void);

public slots:

    // Set the top/Left conner clip position.
    void slotSetImageRegionPosition(QRect rect, bool targetDone);
    
    // Slot used when the original image clip focus is changed by the user.
    void slotOriginalImageRegionChanged(void);
            
signals:

    void signalOriginalClipFocusChanged( void );
    
protected:
    
    Digikam::ImageRegionWidget  *m_imageRegionWidget;
    Digikam::ImagePanIconWidget *m_imagePanIconWidget;
    
    QLabel    *m_previewTargetLabel;
    QLabel    *m_topLeftSelectionInfoLabel;
    QLabel    *m_BottomRightSelectionInfoLabel;
    
    void updateSelectionInfo(QRect rect);
    
};

}  // NameSpace Digikam

#endif /* IMAGEPREVIEWWIDGET_H */
