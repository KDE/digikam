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

#ifndef INSERTTEXTWIDGET_H
#define INSERTTEXTWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qimage.h>
#include <qrect.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>

// KDE includes.

#include <kurl.h>


class QPixmap;

namespace Digikam
{
class ImageIface;
}

namespace DigikamInsertTextImagesPlugin
{
enum Action
{
    ALIGN_LEFT=0,
    ALIGN_RIGHT,
    ALIGN_CENTER,
    ALIGN_BLOCK,
    BORDER_TEXT,
    TRANSPARENT_TEXT
};

enum TextRotation
{
    ROTATION_NONE=0,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270
};

class InsertTextWidget : public QWidget
{
Q_OBJECT

public:

    InsertTextWidget(int w, int h, QWidget *parent=0);
    ~InsertTextWidget();

    Digikam::ImageIface* imageIface();
    
    void   setText(QString text, QFont font, QColor color, int alignMode,
                   bool border, bool transparent, int rotation);
    void   resetEdit(void);
    QImage makeInsertText(void);
    
protected:

    Digikam::ImageIface *m_iface;
    
    bool        m_currentMoving;
    bool        m_textBorder;
    bool        m_textTransparent;
    
    int         m_alignMode;
    int         m_textRotation;
    
    uint       *m_data;
    int         m_w;
    int         m_h;
    
    int         m_xTextPos;            
    int         m_yTextPos;
    int         m_xpos;            
    int         m_ypos;
    
    QPixmap    *m_pixmap;  
    
    QRect       m_rect;
    QRect       m_textRect;

    QString     m_textString;
    
    QFont       m_textFont;
    
    QColor      m_textColor;
    
    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );    
    
    void makePixmap(void);
};

}  // NameSpace DigikamInsertTextImagesPlugin

#endif /* INSERTTEXTWIDGET_H */
