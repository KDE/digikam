/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-09
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef IMAGESELECTIONWIDGET_H
#define IMAGESELECTIONWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qrect.h>

#include "digikam_export.h"
class QPixmap;
class QTimer;

namespace Digikam
{

class ImageIface;

class DIGIKAMIMAGEWIDGET_EXPORT ImageSelectionWidget : public QWidget
{
Q_OBJECT

public:
    
    enum RatioAspect           // Contrained Aspect Ratio list.
    {
    RATIOCUSTOM=0,             // Custom aspect ratio.
    RATIO01X01,                // 1:1
    RATIO02x03,                // 2:3
    RATIO03X04,                // 3:4
    RATIO04X05,                // 4:5
    RATIO05x07,                // 5:7
    RATIO07x10,                // 7:10
    RATIONONE                  // No aspect ratio.
    };

    enum Orient  
    {
    Landscape = 0,
    Paysage
    };

    ImageSelectionWidget(int width, int height, QWidget *parent=0, 
                         float aspectRatioValue=1.0, int aspectRatio=RATIO01X01, 
                         int orient=Landscape, bool ruleThirdLines=false);
    ~ImageSelectionWidget();

    void  setCenterSelection(void);
    void  setSelectionX(int x);
    void  setSelectionY(int y);
    void  setSelectionWidth(int w);
    void  setSelectionHeight(int h);
    void  setSelectionOrientation(int orient);
    void  setSelectionAspectRatioType(int aspectRatioType);
    void  setSelectionAspectRatioValue(float aspectRatioValue);
    
    int   getOriginalImageWidth(void);
    int   getOriginalImageHeight(void);
    QRect getRegionSelection(void);
    
    int   getMinWidthRange(void);
    int   getMinHeightRange(void);
    
    void  resetSelection(void);
    void  maxAspectSelection(void);

public slots:

    void slotRuleThirdLines(bool ruleThirdLines);
    
signals:

    void signalSelectionMoved( QRect rect );     
    void signalSelectionChanged( QRect rect );   
    void signalSelectionWidthChanged( int newWidth ); 
    void signalSelectionHeightChanged( int newHeight ); 
    
protected:
    
    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );

protected slots:

    void slotTimerDone(void);
                
private:

    enum ResizingMode
    {
    ResizingNone = 0,
    ResizingTopLeft,
    ResizingTopRight, 
    ResizingBottomLeft,   
    ResizingBottomRight
    };
    
    ImageIface *m_iface;
    
    bool        m_ruleThirdLines;
    
    uint       *m_data;
    int         m_w;
    int         m_h;
    
    int         m_xpos;
    int         m_ypos;

    QRect       m_rect;                    
    QRect       m_regionSelection;         // Real size image selection.
    QRect       m_localRegionSelection;    // Local size selection.
    
    // Draggable local region selection corners.
    QRect       m_localTopLeftCorner;
    QRect       m_localBottomLeftCorner;
    QRect       m_localTopRightCorner;
    QRect       m_localBottomRightCorner;
    
    QPixmap    *m_pixmap;
    
    QTimer     *m_timerW;
    QTimer     *m_timerH;
    
    int         m_currentAspectRatioType;
    int         m_currentAspectRatio;
    int         m_currentResizing;
    int         m_currentOrientation;
    
    float       m_currentAspectRatioValue;
    
    // Recalculate the target selection position and emit 'signalSelectionMoved'.
    void regionSelectionMoved( bool targetDone );
    
    void regionSelectionChanged(bool targetDone);
    void realToLocalRegion(bool updateSizeOnly=false);
    void localToRealRegion(void);
    void applyAspectRatio(bool WOrH, bool repaintWidget=true, bool updateChange=true);
    void updatePixmap(void);
};

}  // NameSpace Digikam

#endif /* IMAGESELECTIONWIDGET_H */
