/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-01
 * Description : 
 * 
 * Copyright 2005 Gilles Caulier
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

#ifndef IMAGEPANNELWIDGET_H
#define IMAGEPANNELWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qimage.h>
#include <qrect.h>

class QLabel;
class QVGroupBox;

class KProgress;

namespace Digikam
{

class ImageRegionWidget;
class ImagePanIconWidget;

class ImagePannelWidget : public QWidget
{
Q_OBJECT

public:
    ImagePannelWidget(uint w, uint h, const QString &title, 
                      QWidget *parent=0, bool progress=false);
    ~ImagePannelWidget();
    
    QRect  getOriginalImageRegion(void);
    QImage getOriginalClipImage(void);
    void   setPreviewImageData(QImage img);
    void   setPreviewImageWaitCursor(bool enable);
    void   setCenterImageRegionPosition(void);
    
    void   setEnable(bool b);
    
    void   setProgress(int val);
    void   setProgressVisible(bool b);
    void   setProgressWhatsThis(QString desc);

    KProgress  *progressBar(void)      { return m_progressBar; };
    QVGroupBox *settingsGroupBox(void) { return m_gboxSettings; };
        
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
    
    QLabel     *m_topLeftSelectionInfoLabel;
    QLabel     *m_BottomRightSelectionInfoLabel;
    
    QVGroupBox *m_gboxSettings;
    
    KProgress  *m_progressBar;
    
    void updateSelectionInfo(QRect rect);
    
};

}  // NameSpace Digikam

#endif /* IMAGEPANNELWIDGET_H */
