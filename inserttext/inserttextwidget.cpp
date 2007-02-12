/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-02-14
 * Description : 
 * 
 * Copyright 2005 Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qpainter.h>
#include <qfont.h> 
#include <qfontmetrics.h> 

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kglobal.h> 

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "inserttextwidget.h"
#include "inserttextwidget.moc"

namespace DigikamInsertTextImagesPlugin
{

InsertTextWidget::InsertTextWidget(int w, int h, QWidget *parent)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_currentMoving = false;

    m_iface  = new Digikam::ImageIface(w, h);
    m_data   = m_iface->getPreviewImage();
    m_w      = m_iface->previewWidth();
    m_h      = m_iface->previewHeight();
    m_pixmap = new QPixmap(w, h);
    m_pixmap->fill(colorGroup().background());

    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2, m_w, m_h);
    m_textRect = QRect();

    m_backgroundColor = QColor(0xCC, 0xCC, 0xCC);
    m_transparency    = 210;
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
    // signal this needs to be filled by makePixmap
    m_textRect = QRect();
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

    /*
    if ( m_textFont.pointSize() != font.pointSize() && 
         !rect().contains( m_textRect.x(), m_textRect.y() ) )
    {
        m_textFont = font;
        resetEdit();
        return;
    }
    */

    m_textFont = font;

    makePixmap();
    repaint(false);
}

void InsertTextWidget::setPositionHint(QRect hint)
{
    // interpreted by composeImage
    m_positionHint = hint;
    if (m_textRect.isValid())
    {
        // invalidate current position so that hint is certainly interpreted
        m_textRect = QRect();
        makePixmap();
        repaint();
    }
}

QRect InsertTextWidget::getPositionHint()
{
    QRect hint;
    if (m_textRect.isValid())
    {
        // We normalize on the size of the image, but we store as int. Precision loss is no problem.
        hint.setX(      (int) ((float)(m_textRect.x() - m_rect.x())     / (float)m_rect.width()  * 10000.0) );
        hint.setY(      (int) ((float)(m_textRect.y() - m_rect.y())     / (float)m_rect.height() * 10000.0) );
        hint.setWidth(  (int) ((float)m_textRect.width()  / (float)m_rect.width()  * 10000.0) );
        hint.setHeight( (int) ((float)m_textRect.height() / (float)m_rect.height() * 10000.0) );
    }
    return hint;
}

Digikam::DImg InsertTextWidget::makeInsertText(void)
{
    int orgW = m_iface->originalWidth();
    int orgH = m_iface->originalHeight();
    float ratioW = (float)orgW/(float)m_w;
    float ratioH = (float)orgH/(float)m_h;

    int x, y;
    if (m_textRect.isValid())
    {
        // convert from widget to image coordinates, then to original size
        x = lroundf( (m_textRect.x() - m_rect.x()) * ratioW);
        y = lroundf( (m_textRect.y() - m_rect.y()) * ratioH);
    }
    else
    {
        x = -1;
        y = -1;
    }

    // Get original image
    Digikam::DImg image = m_iface->getOriginalImg()->copy();

    int borderWidth = QMAX(1, lroundf(ratioW));
    // compose and draw result on image
    composeImage(&image, 0, x, y,
                  m_textFont, m_textFont.pointSizeFloat(),
                  m_textRotation, m_textColor, m_alignMode, m_textString,
                  m_textTransparent, m_backgroundColor,
                  m_textBorder ? BORDER_NORMAL : BORDER_NONE, borderWidth, borderWidth);

    return image;
}

void InsertTextWidget::makePixmap(void)
{
    int orgW = m_iface->originalWidth();
    int orgH = m_iface->originalHeight();
    float ratioW = (float)m_w / (float)orgW;
    float ratioH = (float)m_h / (float)orgH;

    int x, y;
    if (m_textRect.isValid())
    {
        // convert from widget to image coordinates
        x = m_textRect.x() - m_rect.x();
        y = m_textRect.y() - m_rect.y();
    }
    else
    {
        x = -1;
        y = -1;
    }

    // get preview image data
    uchar *data = m_iface->getPreviewImage();
    Digikam::DImg image(m_iface->previewWidth(), m_iface->previewHeight(), m_iface->previewSixteenBit(),
                        m_iface->previewHasAlpha(), data);
    delete [] data;

    // paint pixmap for drawing this widget
    // First, fill with background color
    m_pixmap->fill(colorGroup().background());
    QPainter p(m_pixmap);
    // Convert image to pixmap and draw it
    QPixmap imagePixmap = image.convertToPixmap();
    p.drawPixmap(m_rect.x(), m_rect.y(),
                 imagePixmap, 0, 0, imagePixmap.width(), imagePixmap.height());

    // prepare painter for use by compose image
    p.setClipRect(m_rect);
    p.translate(m_rect.x(), m_rect.y());

    // compose image and draw result directly on pixmap, with correct offset
    QRect textRect = composeImage(&image, &p, x, y,
                                   m_textFont, m_textFont.pointSizeFloat() * ((ratioW > ratioH) ? ratioW : ratioH),
                                   m_textRotation, m_textColor, m_alignMode, m_textString,
                                   m_textTransparent, m_backgroundColor,
                                   m_textBorder ? BORDER_NORMAL : BORDER_SUPPORT, 1, 1);

    p.end();

    // store new text rectangle
    // convert from image to widget coordinates
    m_textRect.setX(textRect.x() + m_rect.x());
    m_textRect.setY(textRect.y() + m_rect.y());
    m_textRect.setSize(textRect.size());
}

/*
   Take data from image, draw text at x|y with specified parameters.
   If destPainter is null, draw to image,
   if destPainter is not null, draw directly using the painter.
   Returns modified area of image.
*/
QRect InsertTextWidget::composeImage(Digikam::DImg *image, QPainter *destPainter,
                                     int x, int y,
                                     QFont font, float pointSize, int textRotation, QColor textColor,
                                     int alignMode, const QString &textString,
                                     bool transparentBackground, QColor backgroundColor,
                                     BorderMode borderMode, int borderWidth, int spacing)
{
    /*
        The problem we have to solve is that we have no pixel access to font rendering,
        we have to let Qt do the drawing. On the other hand we need to support 16 bit, which
        cannot be done with QPixmap.
        The current solution cuts out the text area, lets Qt do its drawing, converts back and blits to original.
    */
    Digikam::DColorComposer *composer = Digikam::DColorComposer::getComposer(Digikam::DColorComposer::PorterDuffNone);

    int maxWidth, maxHeight;
    if (x == -1 && y == -1)
    {
        maxWidth = image->width();
        maxHeight = image->height();
    }
    else
    {
        maxWidth = image->width() - x;
        maxHeight = image->height() - y;
    }

    // find out size of the area that we are drawing to
    font.setPointSizeFloat(pointSize);
    QFontMetrics fontMt( font );
    QRect fontRect = fontMt.boundingRect(0, 0, maxWidth, maxHeight, 0, textString);

    int fontWidth, fontHeight;

    switch(textRotation)
    {
        case ROTATION_NONE:
        case ROTATION_180:
        default:
            fontWidth = fontRect.width();
            fontHeight = fontRect.height();
            break;

        case ROTATION_90:
        case ROTATION_270:
            fontWidth = fontRect.height();
            fontHeight = fontRect.width();
            break;
    }

    // x, y == -1 means that we have to find a good initial position for the text here
    if (x == -1 && y == -1)
    {
        // was a valid position hint stored from last use?
        if (m_positionHint.isValid())
        {
            // We assume that people tend to orient text along the edges,
            // so we do some guessing so that positions such as "in the lower right corner"
            // will be remembered across different image sizes.

            // get relative positions
            float fromTop =          (float)m_positionHint.top()    / 10000.0;
            float fromBottom = 1.0 - (float)m_positionHint.bottom() / 10000.0;
            float fromLeft =         (float)m_positionHint.left()   / 10000.0;
            float fromRight =  1.0 - (float)m_positionHint.right()  / 10000.0;

            // calculate horizontal position
            if (fromLeft < fromRight)
            {
                x = (int)(fromLeft * maxWidth);

                // we are placing from the smaller distance,
                // so if now the larger distance is actually too small,
                // fall back to standard placement, nothing to lose.
                if (x + fontWidth > maxWidth)
                    x = QMAX( (maxWidth - fontWidth) / 2, 0);
            }
            else
            {
                x = maxWidth - (int)(fromRight * maxWidth) - fontWidth;
                if ( x < 0 )
                    x = QMAX( (maxWidth - fontWidth) / 2, 0);
            }

            // calculate vertical position
            if (fromTop < fromBottom)
            {
                y = (int)(fromTop * maxHeight);
                if (y + fontHeight > maxHeight)
                    y = QMAX( (maxHeight - fontHeight) / 2, 0);
            }
            else
            {
                y = maxHeight - (int)(fromBottom * maxHeight) - fontHeight;
                if ( y < 0 )
                    y = QMAX( (maxHeight - fontHeight) / 2, 0);
            }

            if (! QRect(x, y, fontWidth, fontHeight).
                   intersects(QRect(0, 0, maxWidth, maxHeight)) )
            {
                // emergency fallback - nothing is visible
                x = QMAX( (maxWidth - fontWidth) / 2, 0);
                y = QMAX( (maxHeight - fontHeight) / 2, 0);
            }

            // invalidate position hint, use only once
            m_positionHint = QRect();
        }
        else
        {
            // use standard position
            x = QMAX( (maxWidth - fontWidth) / 2, 0);
            y = QMAX( (maxHeight - fontHeight) / 2, 0);
        }
    }

    // create a rectangle relative to image
    QRect drawRect( x, y, fontWidth + 2 * borderWidth + 2 * spacing, fontHeight + 2 * borderWidth  + 2 * spacing);

    // create a rectangle relative to textArea, excluding the border
    QRect textAreaBackgroundRect( borderWidth, borderWidth, fontWidth + 2 * spacing, fontHeight + 2 * spacing);

    // create a rectangle relative to textArea, excluding the border and spacing
    QRect textAreaTextRect( borderWidth + spacing, borderWidth + spacing, fontWidth, fontHeight );

    // create a rectangle relative to textArea, including the border,
    // for drawing the rectangle, taking into account that the width of the QPen goes in and out in equal parts
    QRect textAreaDrawRect( borderWidth / 2, borderWidth / 2, fontWidth + borderWidth + 2 * spacing,
                            fontHeight + borderWidth + 2 * spacing );

    // cut out the text area
    Digikam::DImg textArea = image->copy(drawRect);

    if (textArea.isNull())
        return QRect();

    // compose semi-transparent background over textArea
    if (transparentBackground)
    {
        Digikam::DImg transparentLayer(textAreaBackgroundRect.width(), textAreaBackgroundRect.height(), textArea.sixteenBit(), true);
        Digikam::DColor transparent(backgroundColor);
        transparent.setAlpha(m_transparency);
        if (image->sixteenBit())
            transparent.convertToSixteenBit();
        transparentLayer.fill(transparent);
        textArea.bitBlendImage(composer, &transparentLayer, 0, 0, transparentLayer.width(), transparentLayer.height(),
                               textAreaBackgroundRect.x(), textAreaBackgroundRect.y());
    }

    Digikam::DImg textNotDrawn;
    if (textArea.sixteenBit())
    {
        textNotDrawn = textArea.copy();
        textNotDrawn.convertToEightBit();
    }
    else
        textNotDrawn = textArea;

    // We have no direct pixel access to font rendering, so now we need to use Qt/X11 for the drawing

    // convert text area to pixmap
    QPixmap pixmap = textNotDrawn.convertToPixmap();
    // paint on pixmap
    QPainter p(&pixmap);
    p.setPen( QPen(textColor, 1) ) ;
    p.setFont( font );
    p.save();

    // translate to origin of text, leaving space for the border
    p.translate(textAreaTextRect.x(), textAreaTextRect.y());

    switch(textRotation)
    {
        case ROTATION_NONE:
            p.drawText( 0, 0, textAreaTextRect.width(),
                        textAreaTextRect.height(), alignMode, textString );
            break;
        case ROTATION_90:
            p.translate(textAreaTextRect.width(), 0);
            p.rotate(90.0);
            p.drawText( 0, 0, textAreaTextRect.height(), textAreaTextRect.width(),
                        alignMode, textString );
            break;
        case ROTATION_180:
            p.translate(textAreaTextRect.width(), textAreaTextRect.height());
            p.rotate(180.0);
            p.drawText( 0, 0, textAreaTextRect.width(), textAreaTextRect.height(),
                        alignMode, textString );
            break;
        case ROTATION_270:
            p.translate(0, textAreaTextRect.height());
            p.rotate(270.0);
            p.drawText( 0, 0, textAreaTextRect.height(), textAreaTextRect.width(),
                        alignMode, textString );
            break;
    }

    p.restore();

    // Drawing rectangle around text.

    if (borderMode == BORDER_NORMAL)      // Decorative border using text color.
    {
        p.setPen( QPen(textColor, borderWidth, Qt::SolidLine,
                  Qt::SquareCap, Qt::RoundJoin) ) ;
        p.drawRect(textAreaDrawRect);
    }
    else if (borderMode == BORDER_SUPPORT)  // Make simple dot line border to help user.
    {
        p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        p.drawRect(textAreaDrawRect);
        p.setPen(QPen(Qt::red, 1, Qt::DotLine));
        p.drawRect(textAreaDrawRect);
    }
    p.end();

    if (!destPainter)
    {
        // convert to QImage, then to DImg
        QImage pixmapImage = pixmap.convertToImage();
        Digikam::DImg textDrawn(pixmapImage.width(), pixmapImage.height(), false, true, pixmapImage.bits());

        // This does not work: during the conversion, colors are altered significantly (diffs of 1 to 10 in each component),
        // so we cannot find out which pixels have actually been touched.
        /*
        // Compare the result of drawing with the previous version.
        // Set all unchanged pixels to transparent
        Digikam::DColor color, ncolor;
        uchar *ptr, *nptr;
        ptr = textDrawn.bits();
        nptr = textNotDrawn.bits();
        int bytesDepth = textDrawn.bytesDepth();
        int numPixels = textDrawn.width() * textDrawn.height();
        for (int i = 0; i < numPixels; i++, ptr+= bytesDepth, nptr += bytesDepth)
        {
            color.setColor(ptr, false);
            ncolor.setColor(nptr, false);
            if ( color.red()   == ncolor.red() &&
                color.green() == ncolor.green() &&
                color.blue()  == ncolor.blue())
            {
                color.setAlpha(0);
                color.setPixel(ptr);
            }
        }
        // convert to 16 bit if needed
        */
        textDrawn.convertToDepthOfImage(&textArea);

        // now compose to original: only pixels affected by drawing text and border are changed, not whole area
        textArea.bitBlendImage(composer, &textDrawn, 0, 0, textDrawn.width(), textDrawn.height(), 0, 0);

        // copy result to original image
        image->bitBltImage(&textArea, drawRect.x(), drawRect.y());
    }
    else
    {
        destPainter->drawPixmap(drawRect.x(), drawRect.y(), pixmap, 0, 0, pixmap.width(), pixmap.height());
    }

    delete composer;

    return drawRect;
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

    int textX = m_textRect.x() - m_rect.x();
    int textY = m_textRect.y() - m_rect.y();
    int old_w = m_w;
    int old_h = m_h;
    m_data    = m_iface->setPreviewImageSize(w, h);
    m_w       = m_iface->previewWidth();
    m_h       = m_iface->previewHeight();

    m_pixmap = new QPixmap(w, h);
    m_rect = QRect(w/2-m_w/2, h/2-m_h/2, m_w, m_h);

    if (m_textRect.isValid())
    {
        int textWidth  = m_textRect.width();
        int textHeight = m_textRect.height();

        textX = lroundf( textX * (float)m_w / (float)old_w );
        textY = lroundf( textY * (float)m_h / (float)old_h );
        textWidth  = lroundf(textWidth  * (float)m_w / (float)old_w );
        textHeight = lroundf(textHeight * (float)m_h / (float)old_h );

        m_textRect.setX(textX + m_rect.x());
        m_textRect.setY(textY + m_rect.y());
        m_textRect.setWidth(textWidth);
        m_textRect.setHeight(textHeight);
        makePixmap();
    }

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
