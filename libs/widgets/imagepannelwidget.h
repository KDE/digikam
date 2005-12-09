/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-01
 * Description : a widget to draw a control pannel image tool.
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
#include <qsize.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qstring.h>

// Local includes

#include "dimg.h"
#include "imageregionwidget.h"
#include "imagepaniconwidget.h"
#include "digikam_export.h"

class QHButtonGroup;

class KProgress;

namespace Digikam
{

class ImageRegionWidget;

class DIGIKAM_EXPORT ImagePannelWidget : public QWidget
{
Q_OBJECT

public:

    enum SeparateViewOptions 
    {
    SeparateViewNormal=0,
    SeparateViewDuplicate,
    SeparateViewAll
    };
    
public:

    ImagePannelWidget(uint w, uint h, QString settingsSection, QWidget *parent=0, bool progress=false, 
                      int separateViewMode=SeparateViewAll);
    ~ImagePannelWidget();
    
    QRect  getOriginalImageRegion(void);
    QRect  getOriginalImageRegionToRender(void);
    DImg   getOriginalRegionImage(void);
    void   setPreviewImage(DImg img);
    void   setPreviewImageWaitCursor(bool enable);
    void   setCenterImageRegionPosition(void);
    
    void   setEnable(bool b);
    
    void   setProgress(int val);
    void   setProgressVisible(bool b);
    void   setProgressWhatsThis(QString desc);

    void   setUserAreaWidget(QWidget *w, bool separator=true);
    
    void   setPanIconHighLightPoints(QPointArray pt);
    
    KProgress *progressBar(void) { return m_progressBar; };
           
    // FIXME remove these methods when all image plugins will be ported to DIMG.
    QImage getOriginalClipImage(void);
    void   setPreviewImageData(QImage img);
           
public slots:

    // Set the top/Left conner clip position.
    void slotSetImageRegionPosition(QRect rect, bool targetDone);
    
    // Slot used when the original image clip focus is changed by the user.
    void slotOriginalImageRegionChanged(bool target);

protected slots:

    void slotPanIconTakeFocus(void);    
    void slotInitGui(void);
                
signals:

    void signalOriginalClipFocusChanged( void );
    void signalResized( void );
    
protected:
    
    void resizeEvent(QResizeEvent *e);

private:
    
    Digikam::ImageRegionWidget  *m_imageRegionWidget;
    Digikam::ImagePanIconWidget *m_imagePanIconWidget;
    
    QGridLayout                 *m_mainLayout;
    
    QHButtonGroup               *m_separateView;
    
    KProgress                   *m_progressBar;
    
    QString                      m_settingsSection;

private:
        
    void updateSelectionInfo(QRect rect);
    void readSettings(void);
    void writeSettings(void);
};

}  // NameSpace Digikam

#endif /* IMAGEPANNELWIDGET_H */
