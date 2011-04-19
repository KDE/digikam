/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a
 *               template to an image.
 *
 * Copyright (C) 2005-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QWidget>
#include <QImage>
#include <QRect>
#include <QSize>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>

// KDE includes

#include <kurl.h>

// Local includes

#include "imageiface.h"
#include "dimg.h"

class QPixmap;

using namespace Digikam;

namespace DigikamDecorateImagePlugin
{

enum Action
{
    ZOOMIN=0,
    ZOOMOUT,
    MOVE
};

class SuperImposeWidget : public QWidget
{
    Q_OBJECT

public:

    SuperImposeWidget(int w, int h, QWidget* parent=0);
    ~SuperImposeWidget();

    void   setBackgroundColor(const QColor& bg);
    void   setEditMode(int mode);
    void   resetEdit();
    QRect  getCurrentSelection();
    QSize  getTemplateSize();
    DImg   makeSuperImpose();

public Q_SLOTS:

    void slotEditModeChanged(int mode);
    void slotSetCurrentTemplate(const KUrl& url);

protected:

    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

    bool zoomSelection(float deltaZoomFactor);
    void moveSelection(int x, int y);
    void makePixmap(void);
    void setEditModeCursor();

private:

    int      m_w;
    int      m_h;

    int      m_xpos;
    int      m_ypos;
    int      m_editMode;
    float    m_zoomFactor;

    QColor   m_bgColor;           // Pixmap background color.

    QPixmap* m_pixmap;            // For image region selection manipulations.

    QRect    m_rect;              // For mouse drag position.
    QRect    m_currentSelection;  // Region selection in image displayed in the widget.

    DImg     m_template;          // Full template data.
    DImg     m_templateScaled;    // Template scaled to preview widget
};

}  // namespace DigikamDecorateImagePlugin

#endif /* SUPERIMPOSEWIDGET_H */
