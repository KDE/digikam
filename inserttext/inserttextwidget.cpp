/* ============================================================
 * File  : inserttextwidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-14
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

#define OPACITY  0.7
#define RCOL     0xAA
#define GCOL     0xAA
#define BCOL     0xAA

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qpainter.h>
#include <qfont.h> 
#include <qfontmetrics.h> 

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "inserttextwidget.h"

namespace DigikamInsertTextImagesPlugin
{

InsertTextWidget::InsertTextWidget(int w, int h, QWidget *parent)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_currentMoving = false;
    
    m_iface  = new Digikam::ImageIface(w, h);
    m_data   = m_iface->getPreviewData();
    m_w      = m_iface->previewWidth();
    m_h      = m_iface->previewHeight();
    m_pixmap = new QPixmap(w, h);
    
    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2, m_w, m_h);
    
    resetEdit();
}

InsertTextWidget::~InsertTextWidget()
{
    delete [] m_data;
    delete m_iface;
    delete m_pixmap;
}

Digikam::ImageIface* InsertTextWidget::imageIface()
{
    return m_iface;
}

void InsertTextWidget::resetEdit()
{
    m_textRect.moveCenter( QPoint::QPoint(width()/2, height()/2) );
    makePixmap();
    repaint(false);
}

void InsertTextWidget::setText(QString text, QFont font, QColor color, int alignMode,
                               bool border, bool transparent, int rotation)
{
    m_textString      = text;
    m_textColor       = color;
    m_textBorder      = border;
    m_textTransparent = transparent;
    m_textRotation    = rotation;
    
    switch (alignMode)
        {
        case ALIGN_LEFT:
           m_alignMode = Qt::AlignLeft;
           break;
        
        case ALIGN_RIGHT:
           m_alignMode = Qt::AlignRight;
           break;
        
        case ALIGN_CENTER:
           m_alignMode = Qt::AlignHCenter;
           break;
        
        case ALIGN_BLOCK:
           m_alignMode = Qt::AlignJustify;
           break;
        }

    // Center text if top left corner text area isn't visible.
        
    if ( m_textFont.pointSize() != font.pointSize() && 
         !rect().contains( m_textRect.x(), m_textRect.y() ) )
       {
       m_textFont = font;
       resetEdit();
       return;
       }
    
    m_textFont = font;

    makePixmap();
    repaint(false);
}

QImage InsertTextWidget::makeInsertText(void)
{
    int orgW = m_iface->originalWidth();
    int orgH = m_iface->originalHeight();
    float ratioW = (float)orgW/(float)m_w;
    float ratioH = (float)orgH/(float)m_h;
    
    QImage src( orgW, orgH, 32 );
    memcpy(src.bits(), m_iface->getOriginalData(), src.numBytes());
    QPixmap target( src );
    
    QRect targetRect;
    QFontMetrics fontMt( m_textFont );       
    QRect fontRect = fontMt.boundingRect(0, 0, targetRect.width(), 
                                         targetRect.height(), 0, m_textString); 
    
    targetRect.setX( (int)(m_textRect.x()*ratioW) );
    targetRect.setY( (int)(m_textRect.y()*ratioH) );
    
    // Calculate text area accordinly with rotation.
       
    switch(m_textRotation)
       {
       case ROTATION_NONE:
       case ROTATION_180:
          targetRect.setSize( QSize::QSize(fontRect.width(), fontRect.height() ) );
          break;
        
       case ROTATION_90:
       case ROTATION_270:
          targetRect.setSize( QSize::QSize(fontRect.height(), fontRect.width() ) );
          break;
       }

    // Drawing semi-transparent text background.
    
    if (m_textTransparent)
       {
       src = src.copy( targetRect );
       uint* ptr = (uint*)src.bits();
       uchar red, green, blue, alpha;
    
       for (int j = 0 ; j < src.numBytes()/4 ; ++j)
           {
           alpha = (*ptr >> 24) & 0xff;
           red   = (*ptr >> 16) & 0xff;
           green = (*ptr >> 8)  & 0xff;
           blue  = (*ptr)       & 0xff;
                
           red   += (uchar)((RCOL - red)   * OPACITY);
           green += (uchar)((GCOL - green) * OPACITY);
           blue  += (uchar)((BCOL - blue)  * OPACITY);
             
           *ptr = alpha << 24 | red << 16 | green << 8 | blue;
           ptr++;
           }
    
       QPixmap pix(src);
       
       bitBlt(&target, targetRect.x(), targetRect.y(), &pix);
       }
    
    // Drawing the text.

    QPainter p(&target);
    p.setFont( m_textFont );
    p.setPen( QPen::QPen(m_textColor, 1) ) ;
    p.save();
    
    switch(m_textRotation)
        {
        case ROTATION_NONE:
              p.drawText( targetRect, m_alignMode, m_textString );
           break;
        
        case ROTATION_90:
              p.translate(targetRect.x()+targetRect.width(), targetRect.y());
              p.rotate(90.0);
              p.drawText( 0, 0, targetRect.height(), targetRect.width(),
                          m_alignMode, m_textString );
           break;
        
        case ROTATION_180:
              p.translate(targetRect.x() + targetRect.width(), 
                          targetRect.y() + targetRect.height());
              p.rotate(180.0);
              p.drawText( 0, 0, targetRect.width(), targetRect.height(), 
                          m_alignMode, m_textString );
           break;
        
        case ROTATION_270:
              p.translate(targetRect.x(), targetRect.y()+targetRect.height());
              p.rotate(270.0);
              p.drawText( 0, 0, targetRect.height(), targetRect.width(), 
                          m_alignMode, m_textString );
           break;
        }
    
    p.restore();
                
    // Drawing border.
    
    if (m_textBorder)   
       {
       p.setPen( QPen::QPen(m_textColor, (int)(2*ratioW), 
                 Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin) ) ;
       p.drawRect(targetRect.x() - (int)(ratioW), 
                  targetRect.y() - (int)(ratioW), 
                  targetRect.width() + (int)(2*ratioW), 
                  targetRect.height() + (int)(2*ratioW));
       }                
       
    p.end();    
    
    return ( target.convertToImage().convertDepth(32) );
}

void InsertTextWidget::makePixmap(void)
{
    int orgW = m_iface->originalWidth();
    int orgH = m_iface->originalHeight();
    float ratioW = (float)m_w / (float)orgW;
    float ratioH = (float)m_h / (float)orgH;
    
    // Drawing image.
    
    m_iface->paint(m_pixmap, m_rect.x(), m_rect.y(), m_rect.width(), m_rect.height());

    // Adapt text rectangle from image coordinate to widget coordinate for rendering.
    QRect textRect = m_textRect;
    textRect.setX( m_textRect.x() + m_rect.x() );
    textRect.setY( m_textRect.y() + m_rect.y() );
       
    // Get Regions informations.
    
    QRect r(0, 0, width(), height());
    QRegion reg(r);
    reg -= m_rect;

    QFont previewFont = m_textFont;
    previewFont.setPointSizeFloat( m_textFont.pointSizeFloat() * ((ratioW > ratioH) ? ratioW : ratioH) );

    QFontMetrics fontMt( previewFont );
    QRect fontRect = fontMt.boundingRect(0, 0, m_rect.width(), m_rect.height(), 
                                         0, m_textString); 
    
    // Calculate text area accordinly with rotation.
    
    switch(m_textRotation)
        {
        case ROTATION_NONE:
        case ROTATION_180:
           textRect.setSize( QSize::QSize(fontRect.width(), fontRect.height() ) );
           break;
        
        case ROTATION_90:
        case ROTATION_270:
           textRect.setSize( QSize::QSize(fontRect.height(), fontRect.width() ) );
           break;
        }
    
    // Drawing semi-transparent text background.
    
    if (m_textTransparent)
       {
       QImage image((uchar*)m_data, m_w, m_h, 32, 0, 0, QImage::IgnoreEndian);
       
       image = image.copy( textRect.x()-m_rect.x(), textRect.y()-m_rect.y(),
                           textRect.width(), textRect.height() );
          
       uint* ptr = (uint*)image.bits();
       uchar red, green, blue, alpha;
    
       for (int j = 0 ; j < image.numBytes()/4 ; ++j)
           {
           alpha = (*ptr >> 24) & 0xff;
           red   = (*ptr >> 16) & 0xff;
           green = (*ptr >> 8)  & 0xff;
           blue  = (*ptr)       & 0xff;
                
           red   += (uchar)((RCOL - red)   * OPACITY);
           green += (uchar)((GCOL - green) * OPACITY);
           blue  += (uchar)((BCOL - blue)  * OPACITY);
             
           *ptr = alpha << 24 | red << 16 | green << 8 | blue;
           ptr++;
           }
    
       QPixmap pix(image);
       
       bitBlt(m_pixmap, textRect.x(), textRect.y(), &pix);
       }
            
    // Drawing text with rotation.
    
    QPainter p(m_pixmap);
    p.setPen( QPen::QPen(m_textColor, 1) ) ;
    p.setFont( previewFont );
    p.save();
    
    switch(m_textRotation)
        {
        case ROTATION_NONE:
           p.drawText( textRect.x(), textRect.y(), textRect.width(), 
                       textRect.height(), m_alignMode, m_textString );
           break;
        
        case ROTATION_90:
           p.translate(textRect.x()+textRect.width(), textRect.y());
           p.rotate(90.0);
           p.drawText( 0, 0, textRect.height(), textRect.width(), 
                       m_alignMode, m_textString );
           break;
        
        case ROTATION_180:
           p.translate(textRect.x() + textRect.width(), 
                       textRect.y() + textRect.height());
           p.rotate(180.0);
           p.drawText( 0, 0, textRect.width(), textRect.height(), 
                       m_alignMode, m_textString );
           break;
        
        case ROTATION_270:
           p.translate(textRect.x(), textRect.y() + textRect.height());
           p.rotate(270.0);
           p.drawText( 0, 0, textRect.height(), textRect.width(), 
                       m_alignMode, m_textString );
           break;
        }
    
    p.restore();
    
    // Drawing rectangle around text.
    
    if (m_textBorder)      // Decorative border using text color.
       {
       p.setPen( QPen::QPen(m_textColor, 2, Qt::SolidLine, 
                 Qt::SquareCap, Qt::RoundJoin) ) ;
       p.drawRect(textRect.x() - 1, textRect.y() - 1, 
                  textRect.width() + 2, textRect.height() + 2);
       }
    else   // Make simple dot line border to help user.
       {
       p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
       p.drawRect(textRect);
       p.setPen(QPen(Qt::red, 1, Qt::DotLine));
       p.drawRect(textRect);
       }
    
    // Drawing widget background.
    
    p.setClipRegion(reg);
    p.fillRect(r, colorGroup().background());
    p.end();
    
    // Save all text rectangle transformations from widget coordinate to image coordinate.
    m_textRect = textRect;
    m_textRect.setX( textRect.x() - m_rect.x() );
    m_textRect.setY( textRect.y() - m_rect.y() );    
}

void InsertTextWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap);
}

void InsertTextWidget::resizeEvent(QResizeEvent * e)
{
    blockSignals(true);
    delete m_pixmap;
    int w = e->size().width();
    int h = e->size().height();
    int old_w = m_w;
    int old_h = m_h;
    m_data = m_iface->setPreviewSize(w, h);
    m_w    = m_iface->previewWidth();
    m_h    = m_iface->previewHeight();
    m_pixmap = new QPixmap(w, h);
    m_rect = QRect(w/2-m_w/2, h/2-m_h/2, m_w, m_h);  
    m_textRect.setX((int)((float)m_textRect.x() * ( (float)m_w / (float)old_w)));
    m_textRect.setY((int)((float)m_textRect.y() * ( (float)m_h / (float)old_h)));
    makePixmap();
    blockSignals(false);
}

void InsertTextWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         m_textRect.contains( e->x(), e->y() ) )
       {
       m_xpos = e->x();
       m_ypos = e->y();
       setCursor ( KCursor::sizeAllCursor() );
       m_currentMoving = true;
       }
}

void InsertTextWidget::mouseReleaseEvent ( QMouseEvent * )
{
    setCursor ( KCursor::arrowCursor() );
    m_currentMoving = false;
}

void InsertTextWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( rect().contains( e->x(), e->y() ) )
       {
       if ( e->state() == Qt::LeftButton && m_currentMoving )
          {
          uint newxpos = e->x();
          uint newypos = e->y();
              
          m_textRect.moveBy(newxpos - m_xpos, newypos - m_ypos);
          makePixmap();
          repaint(false);
               
          m_xpos = newxpos;
          m_ypos = newypos;
          setCursor( KCursor::handCursor() );
          }
       else if ( m_textRect.contains( e->x(), e->y() ) )
          {
          setCursor ( KCursor::sizeAllCursor() );
          }
       else
          {
          setCursor ( KCursor::arrowCursor() );
          }
       }
}

}  // NameSpace DigikamInsertTextImagesPlugin


#include "inserttextwidget.moc"
