/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a tool to show image using an OpenGL interface.
 *
 * Copyright (C) 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 * Copyright (C) 2008-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_GLVIEWERPLUGIN_GLVIEWERWIDGET_H
#define DIGIKAM_GLVIEWERPLUGIN_GLVIEWERWIDGET_H

// Qt includes

#include <QCursor>
#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWheelEvent>
#include <QUrl>

// Local includes

#include "glviewertexture.h"
#include "glviewerglobal.h"
#include "dplugin.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace GenericGLViewerPlugin
{

class GLViewerWidget : public QGLWidget
{
    Q_OBJECT

public:

    explicit GLViewerWidget(DPlugin* const plugin, DInfoInterface* const iface);
    ~GLViewerWidget();

    void prevImage();
    void nextImage();
    bool listOfFilesIsEmpty() const;

    OGLstate getOGLstate() const;

private:

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    bool isReallyFullScreen() const;

    void keyPressEvent(QKeyEvent* k);
    void keyReleaseEvent(QKeyEvent* e);
    void wheelEvent(QWheelEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

    GLViewerTexture* loadImage(int file_index) const;
    void drawImage(GLViewerTexture* const tex);
    void downloadTexture(GLViewerTexture* const tex);

    void zoom(int mdelta, const QPoint& pos, float factor);

private Q_SLOTS:

    void slotTimeoutMouseMove();

private:

    class Private;
    Private* const d;
};

} // namespace GenericGLViewerPlugin

#endif // DIGIKAM_GLVIEWERPLUGIN_GLVIEWERWIDGET_H
