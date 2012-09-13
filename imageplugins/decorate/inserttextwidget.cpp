/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a widget to insert a text over an image.
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "inserttextwidget.moc"

// C++ includes

#include <cstdio>
#include <cmath>

// Qt includes

#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>

// KDE includes

#include <kcursor.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// Local includes

#include "imageiface.h"

namespace DigikamDecorateImagePlugin
{

class InsertTextWidget::Private
{
public:

    Private() :
        currentMoving(false),
        textBorder(false),
        textTransparent(false),
        alignMode(0),
        h(0),
        textRotation(0),
        transparency(0),
        w(0),
        xpos(0),
        ypos(0),
        pixmap(0),
        iface(0)
    {
    }

    bool        currentMoving;
    bool        textBorder;
    bool        textTransparent;

    int         alignMode;
    int         h;
    int         textRotation;
    int         transparency;
    int         w;
    int         xpos;
    int         ypos;

    QColor      backgroundColor;   // For text
    QColor      bgColor;           // For Pixmap
    QColor      textColor;

    QFont       textFont;

    QPixmap*    pixmap;

    QRect       positionHint;
    QRect       rect;
    QRect       textRect;

    QString     textString;

    ImageIface* iface;
};

InsertTextWidget::InsertTextWidget(int w, int h, QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    d->currentMoving   = false;
    d->bgColor         = palette().color(QPalette::Background);
    d->backgroundColor = QColor(0xCC, 0xCC, 0xCC);
    d->transparency    = 210;

    d->iface  = new ImageIface(QSize(w, h));
    d->w      = d->iface->previewSize().width();
    d->h      = d->iface->previewSize().height();
    d->pixmap = new QPixmap(w, h);
    d->pixmap->fill(d->bgColor);

    setMinimumSize(w, h);
    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);

    d->rect     = QRect(width()/2-d->w/2, height()/2-d->h/2, d->w, d->h);
    d->textRect = QRect();
}

InsertTextWidget::~InsertTextWidget()
{
    delete d->iface;
    delete d->pixmap;
    delete d;
}

ImageIface* InsertTextWidget::imageIface() const
{
    return d->iface;
}

void InsertTextWidget::resetEdit()
{
    // signal this needs to be filled by makePixmap
    d->textRect = QRect();
    makePixmap();
    repaint();
}

void InsertTextWidget::setText(const QString& text, const QFont& font, const QColor& color, int alignMode,
                               bool border, bool transparent, int rotation)
{
    d->textString      = text;
    d->textColor       = color;
    d->textBorder      = border;
    d->textTransparent = transparent;
    d->textRotation    = rotation;

    switch (alignMode)
    {
        case ALIGN_LEFT:
            d->alignMode = Qt::AlignLeft;
            break;

        case ALIGN_RIGHT:
            d->alignMode = Qt::AlignRight;
            break;

        case ALIGN_CENTER:
            d->alignMode = Qt::AlignHCenter;
            break;

        case ALIGN_BLOCK:
            d->alignMode = Qt::AlignJustify;
            break;
    }

    // Center text if top left corner text area is not visible.

/*
    if ( d->textFont.pointSize() != font.pointSize() &&
         !rect().contains( d->textRect.x(), d->textRect.y() ) )
    {
        d->textFont = font;
        resetEdit();
        return;
    }
*/

    d->textFont = font;

    makePixmap();
    repaint();
}

void InsertTextWidget::setBackgroundColor(const QColor& bg)
{
    d->bgColor = bg;
    makePixmap();
    repaint();
}

void InsertTextWidget::setPositionHint(const QRect& hint)
{
    // interpreted by composeImage
    d->positionHint = hint;

    if (d->textRect.isValid())
    {
        // invalidate current position so that hint is certainly interpreted
        d->textRect = QRect();
        makePixmap();
        repaint();
    }
}

QRect InsertTextWidget::getPositionHint() const
{
    QRect hint;

    if (d->textRect.isValid())
    {
        // We normalize on the size of the image, but we store as int. Precision loss is no problem.
        hint.setX(      (int) ((float)(d->textRect.x() - d->rect.x())     / (float)d->rect.width()  * 10000.0) );
        hint.setY(      (int) ((float)(d->textRect.y() - d->rect.y())     / (float)d->rect.height() * 10000.0) );
        hint.setWidth(  (int) ((float)d->textRect.width()  / (float)d->rect.width()  * 10000.0) );
        hint.setHeight( (int) ((float)d->textRect.height() / (float)d->rect.height() * 10000.0) );
    }

    return hint;
}

DImg InsertTextWidget::makeInsertText()
{
    int orgW     = d->iface->originalSize().width();
    int orgH     = d->iface->originalSize().height();
    float ratioW = (float)orgW/(float)d->w;
    float ratioH = (float)orgH/(float)d->h;

    int x, y;

    if (d->textRect.isValid())
    {
        // convert from widget to image coordinates, then to original size
        x = lroundf( (d->textRect.x() - d->rect.x()) * ratioW);
        y = lroundf( (d->textRect.y() - d->rect.y()) * ratioH);
    }
    else
    {
        x = -1;
        y = -1;
    }

    // Get original image
    DImg image      = d->iface->original()->copy();
    int borderWidth = qMax(1, (int)lroundf(ratioW));

    // compose and draw result on image
    composeImage(&image, 0, x, y,
                 d->textFont, d->textFont.pointSizeF(),
                 d->textRotation, d->textColor, d->alignMode, d->textString,
                 d->textTransparent, d->backgroundColor,
                 d->textBorder ? BORDER_NORMAL : BORDER_NONE, borderWidth, borderWidth);

    return image;
}

void InsertTextWidget::makePixmap()
{
    int orgW = d->iface->originalSize().width();
    int orgH = d->iface->originalSize().height();
    float ratioW = (float)d->w / (float)orgW;
    float ratioH = (float)d->h / (float)orgH;

    int x, y;

    if (d->textRect.isValid())
    {
        // convert from widget to image coordinates
        x = d->textRect.x() - d->rect.x();
        y = d->textRect.y() - d->rect.y();
    }
    else
    {
        x = -1;
        y = -1;
    }

    // get preview image data
    DImg image = d->iface->preview();
    image.setIccProfile( d->iface->original()->getIccProfile() );

    // paint pixmap for drawing this widget
    // First, fill with background color
    d->pixmap->fill(d->bgColor);
    QPainter p(d->pixmap);

    // Convert image to pixmap and draw it
    QPixmap imagePixmap = d->iface->convertToPixmap(image);
    p.drawPixmap(d->rect.x(), d->rect.y(),
                 imagePixmap, 0, 0, imagePixmap.width(), imagePixmap.height());

    // prepare painter for use by compose image
    p.setClipRect(d->rect);
    p.translate(d->rect.x(), d->rect.y());

    // compose image and draw result directly on pixmap, with correct offset
    QRect textRect = composeImage(&image, &p, x, y,
                                  d->textFont, d->textFont.pointSizeF() * ((ratioW > ratioH) ? ratioW : ratioH),
                                  d->textRotation, d->textColor, d->alignMode, d->textString,
                                  d->textTransparent, d->backgroundColor,
                                  d->textBorder ? BORDER_NORMAL : BORDER_SUPPORT, 1, 1);

    p.end();

    // store new text rectangle
    // convert from image to widget coordinates
    d->textRect.setX(textRect.x() + d->rect.x());
    d->textRect.setY(textRect.y() + d->rect.y());
    d->textRect.setSize(textRect.size());
}

/**
   Take data from image, draw text at x|y with specified parameters.
   If destPainter is null, draw to image,
   if destPainter is not null, draw directly using the painter.
   Returns modified area of image.
*/
QRect InsertTextWidget::composeImage(DImg* const image, QPainter* const destPainter,
                                     int x, int y,
                                     QFont font, float pointSize, int textRotation, QColor textColor,
                                     int alignMode, const QString& textString,
                                     bool transparentBackground, QColor backgroundColor,
                                     BorderMode borderMode, int borderWidth, int spacing)
{
    /*
        The problem we have to solve is that we have no pixel access to font rendering,
        we have to let Qt do the drawing. On the other hand we need to support 16 bit, which
        cannot be done with QPixmap.
        The current solution cuts out the text area, lets Qt do its drawing, converts back and blits to original.
    */
    DColorComposer* composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);

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
    font.setPointSizeF(pointSize);
    QFontMetrics fontMt( font );
    QRect fontRect = fontMt.boundingRect(0, 0, maxWidth, maxHeight, 0, textString);

    int fontWidth, fontHeight;

    switch (textRotation)
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
        if (d->positionHint.isValid())
        {
            // We assume that people tend to orient text along the edges,
            // so we do some guessing so that positions such as "in the lower right corner"
            // will be remembered across different image sizes.

            // get relative positions
            float fromTop =          (float)d->positionHint.top()    / 10000.0;
            float fromBottom = 1.0 - (float)d->positionHint.bottom() / 10000.0;
            float fromLeft =         (float)d->positionHint.left()   / 10000.0;
            float fromRight =  1.0 - (float)d->positionHint.right()  / 10000.0;

            // calculate horizontal position
            if (fromLeft < fromRight)
            {
                x = (int)(fromLeft * maxWidth);

                // we are placing from the smaller distance,
                // so if now the larger distance is actually too small,
                // fall back to standard placement, nothing to lose.
                if (x + fontWidth > maxWidth)
                {
                    x = qMax( (maxWidth - fontWidth) / 2, 0);
                }
            }
            else
            {
                x = maxWidth - (int)(fromRight * maxWidth) - fontWidth;

                if ( x < 0 )
                {
                    x = qMax( (maxWidth - fontWidth) / 2, 0);
                }
            }

            // calculate vertical position
            if (fromTop < fromBottom)
            {
                y = (int)(fromTop * maxHeight);

                if (y + fontHeight > maxHeight)
                {
                    y = qMax( (maxHeight - fontHeight) / 2, 0);
                }
            }
            else
            {
                y = maxHeight - (int)(fromBottom * maxHeight) - fontHeight;

                if ( y < 0 )
                {
                    y = qMax( (maxHeight - fontHeight) / 2, 0);
                }
            }

            if (! QRect(x, y, fontWidth, fontHeight).
                intersects(QRect(0, 0, maxWidth, maxHeight)) )
            {
                // emergency fallback - nothing is visible
                x = qMax( (maxWidth - fontWidth) / 2, 0);
                y = qMax( (maxHeight - fontHeight) / 2, 0);
            }

            // invalidate position hint, use only once
            d->positionHint = QRect();
        }
        else
        {
            // use standard position
            x = qMax( (maxWidth - fontWidth) / 2, 0);
            y = qMax( (maxHeight - fontHeight) / 2, 0);
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
    DImg textArea = image->copy(drawRect);

    if (textArea.isNull())
    {
        return QRect();
    }

    // compose semi-transparent background over textArea
    if (transparentBackground)
    {
        DImg transparentLayer(textAreaBackgroundRect.width(), textAreaBackgroundRect.height(), textArea.sixteenBit(), true);
        DColor transparent(backgroundColor);
        transparent.setAlpha(d->transparency);

        if (image->sixteenBit())
        {
            transparent.convertToSixteenBit();
        }

        transparentLayer.fill(transparent);
        textArea.bitBlendImage(composer, &transparentLayer, 0, 0, transparentLayer.width(), transparentLayer.height(),
                               textAreaBackgroundRect.x(), textAreaBackgroundRect.y());
    }

    DImg textNotDrawn;

    if (textArea.sixteenBit())
    {
        textNotDrawn = textArea.copy();
        textNotDrawn.convertToEightBit();
    }
    else
    {
        textNotDrawn = textArea;
    }

    // We have no direct pixel access to font rendering, so now we need to use Qt/X11 for the drawing

    // convert text area to pixmap
    QPixmap pixmap;

    if (destPainter)
    {
        // We working on tool preview, deal with CM as well
        pixmap = d->iface->convertToPixmap(textNotDrawn);
    }
    else
    {
        // We working on target image. Do no apply double CM adjustement here.
        pixmap = textNotDrawn.convertToPixmap();
    }

    // paint on pixmap
    QPainter p(&pixmap);
    p.setPen( QPen(textColor, 1) ) ;
    p.setFont( font );
    p.save();

    // translate to origin of text, leaving space for the border
    p.translate(textAreaTextRect.x(), textAreaTextRect.y());

    switch (textRotation)
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
        QImage pixmapImage = pixmap.toImage();
        DImg textDrawn(pixmapImage.width(), pixmapImage.height(), false, true, pixmapImage.bits());

        // This does not work: during the conversion, colors are altered significantly (diffs of 1 to 10 in each component),
        // so we cannot find out which pixels have actually been touched.
        /*
        // Compare the result of drawing with the previous version.
        // Set all unchanged pixels to transparent
        DColor color, ncolor;
        uchar *ptr, *nptr;
        ptr = textDrawn.bits();
        nptr = textNotDrawn.bits();
        int bytesDepth = textDrawn.bytesDepth();
        int numPixels = textDrawn.width() * textDrawn.height();
        for (int i = 0; i < numPixels; ++i, ptr+= bytesDepth, nptr += bytesDepth)
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

void InsertTextWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, *d->pixmap);
    p.end();
}

void InsertTextWidget::resizeEvent(QResizeEvent* e)
{
    blockSignals(true);
    delete d->pixmap;

    int w     = e->size().width();
    int h     = e->size().height();

    int textX = d->textRect.x() - d->rect.x();
    int textY = d->textRect.y() - d->rect.y();
    int old_w = d->w;
    int old_h = d->h;
    d->iface->setPreviewSize(QSize(w, h));
    d->w      = d->iface->previewSize().width();
    d->h      = d->iface->previewSize().height();

    d->pixmap = new QPixmap(w, h);
    d->rect   = QRect(w/2-d->w/2, h/2-d->h/2, d->w, d->h);

    if (d->textRect.isValid())
    {
        int textWidth  = d->textRect.width();
        int textHeight = d->textRect.height();

        textX      = lroundf(textX      * (float)d->w / (float)old_w);
        textY      = lroundf(textY      * (float)d->h / (float)old_h);
        textWidth  = lroundf(textWidth  * (float)d->w / (float)old_w);
        textHeight = lroundf(textHeight * (float)d->h / (float)old_h);

        d->textRect.setX(textX + d->rect.x());
        d->textRect.setY(textY + d->rect.y());
        d->textRect.setWidth(textWidth);
        d->textRect.setHeight(textHeight);
        makePixmap();
    }

    blockSignals(false);
}

void InsertTextWidget::mousePressEvent(QMouseEvent* e)
{
    if ( e->button() == Qt::LeftButton &&
         d->textRect.contains( e->x(), e->y() ) )
    {
        d->xpos = e->x();
        d->ypos = e->y();
        setCursor ( Qt::SizeAllCursor );
        d->currentMoving = true;
    }
}

void InsertTextWidget::mouseReleaseEvent(QMouseEvent*)
{
    setCursor ( Qt::ArrowCursor );
    d->currentMoving = false;
}

void InsertTextWidget::mouseMoveEvent(QMouseEvent* e)
{
    if ( rect().contains( e->x(), e->y() ) )
    {
        if ( e->buttons() == Qt::LeftButton && d->currentMoving )
        {
            uint newxpos = e->x();
            uint newypos = e->y();

            d->textRect.translate(newxpos - d->xpos, newypos - d->ypos);

            makePixmap();
            repaint();

            d->xpos = newxpos;
            d->ypos = newypos;
            setCursor( Qt::PointingHandCursor );
        }
        else if ( d->textRect.contains( e->x(), e->y() ) )
        {
            setCursor ( Qt::SizeAllCursor );
        }
        else
        {
            setCursor ( Qt::ArrowCursor );
        }
    }
}

}  // namespace DigikamDecorateImagePlugin
