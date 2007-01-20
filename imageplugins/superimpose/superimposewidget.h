/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-01-04
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

#ifndef SUPERIMPOSEWIDGET_H
#define SUPERIMPOSEWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qimage.h>
#include <qrect.h>
#include <qsize.h>
#include <qpixmap.h>

// KDE includes.

#include <kurl.h>

// Digikam includes.

#include <digikamheaders.h>

class QPixmap;

namespace Digikam
{
class ImageIface;
}

namespace DigikamSuperImposeImagesPlugin
{

enum Action
{
    ZOOMIN=0,
    ZOOMOUT,
    MOVE,
};

class SuperImposeWidget : public QWidget
{
Q_OBJECT

public:

    SuperImposeWidget(int w, int h, QWidget *parent=0);
    ~SuperImposeWidget();

    void   setEditMode(int mode);
    void   resetEdit(void);
    QRect  getCurrentSelection(void);
    QSize  getTemplateSize(void);
    Digikam::DImg makeSuperImpose(void);

public slots:

    void slotEditModeChanged(int mode);
    void slotSetCurrentTemplate(const KURL& url);

protected:

    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent * e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );

    bool zoomSelection(float deltaZoomFactor);
    void moveSelection(int x, int y);
    void makePixmap(void);
    void setEditModeCursor();

private:

    int            m_w;
    int            m_h;

    int            m_xpos;
    int            m_ypos;
    int            m_editMode;
    float          m_zoomFactor;

    QPixmap       *m_pixmap;            // For image region selection manipulations.

    QRect          m_rect;              // For mouse drag position.
    QRect          m_currentSelection;  // Region selection in image displayed in the widget.
 
    Digikam::DImg  m_template;          // Full template data.
    Digikam::DImg  m_templateScaled;    // Template scaled to preview widget
};

}  // NameSpace DigikamSuperImposeImagesPlugin

#endif /* SUPERIMPOSEWIDGET_H */
