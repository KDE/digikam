/* ============================================================
 * File  : imageguidewidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-20
 * Description : 
 * 
 * Copyright 2004-2006 Gilles Caulier
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

#ifndef IMAGEGUIDEWIDGET_H
#define IMAGEGUIDEWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qrect.h>
#include <qpoint.h>
#include <qcolor.h>

// Local includes

#include "dimg.h"
#include "dcolor.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{
class DColor;
class ImageIface;

class DIGIKAM_EXPORT ImageGuideWidget : public QWidget
{
Q_OBJECT

public:

    enum GuideToolMode 
    {
    HVGuideMode=0,
    PickColorMode
    };

    enum ColorPointSrc
    {
    OriginalImage=0,
    PreviewImage,
    TargetPreviewImage
    };

public:

    ImageGuideWidget(int w, int h, QWidget *parent=0, 
                     bool spotVisible=true, int guideMode=HVGuideMode,
                     QColor guideColor=Qt::red, int guideSize=1, bool blink=false,
                     int getColorFrom=OriginalImage);
    ~ImageGuideWidget();
        
    Digikam::ImageIface* imageIface();
    
    QPoint getSpotPosition(void);
    DColor getSpotColor(int getColorFrom);
    void   setSpotVisible(bool spotVisible, bool blink=false);
    void   resetSpotPosition(void);
    void   updatePreview(void);

public slots:
        
    void slotChangeGuideColor(const QColor &color);
    void slotChangeGuideSize(int size);    

signals:

    void spotPositionChanged( const Digikam::DColor &color, bool release, const QPoint &position );
    void signalResized(void);

protected:
    
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent * e );
    void timerEvent(QTimerEvent * e);
    void mousePressEvent( QMouseEvent * e );
    void mouseReleaseEvent( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent * e );
        
private:

    DImg                 m_preview;

    int                  m_w;
    int                  m_h;
    
    int                  m_timerID;
    int                  m_guideMode;
    int                  m_guideSize;
    int                  m_flicker;
    int                  m_getColorFrom;

    bool                 m_sixteenBit;
    bool                 m_focus;
    bool                 m_spotVisible;
    
    // Current spot position in preview coordinates.
    QPoint               m_spot;
    
    QRect                m_rect;       
    
    QColor               m_guideColor;
        
    QPixmap             *m_pixmap;
    
    Digikam::ImageIface *m_iface;    

private:

    void updatePixmap( void );
};

}  // NameSpace Digikam

#endif /* IMAGEGUIDEWIDGET_H */
