/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-07-01
 * Description : a widget to draw a control pannel image tool.
 * 
 * Copyright 2005-2006 Gilles Caulier
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

#include <qhbox.h>
#include <qimage.h>
#include <qrect.h>
#include <qstring.h>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

class KProgress;

namespace Digikam
{

class ImagePannelWidgetPriv;
class ImageRegionWidget;

class DIGIKAM_EXPORT ImagePannelWidget : public QHBox
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

    ImagePannelWidget(uint w, uint h, QString settingsSection, QWidget *parent=0,
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

    void   setUserAreaWidget(QWidget *w);
    
    void   setPanIconHighLightPoints(QPointArray pt);
    
    KProgress *progressBar(void);
           
public slots:

    // Set the top/Left conner clip position.
    void slotSetImageRegionPosition(QRect rect, bool targetDone);
    
    // Slot used when the original image clip focus is changed by the user.
    void slotOriginalImageRegionChanged(bool target);

protected slots:

    void slotPanIconTakeFocus(void);    
    void slotInitGui(void);
    void slotZoomButtonReleased(int);
                
signals:

    void signalOriginalClipFocusChanged( void );
    void signalResized( void );
    void signalZoomFactorChanged( double );
    
protected:
    
    void resizeEvent(QResizeEvent *e);

private:
        
    void updateSelectionInfo(QRect rect);
    void readSettings(void);
    void writeSettings(void);

private:

    ImagePannelWidgetPriv* d;
        
};

}  // NameSpace Digikam

#endif /* IMAGEPANNELWIDGET_H */
