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

#ifndef DIGIKAM_GLVIEWERPLUGIN_GLVIEWERTEXTURE_H
#define DIGIKAM_GLVIEWERPLUGIN_GLVIEWERTEXTURE_H

// Qt includes

#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QImage>
#include <QString>

// Local includes

#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericGLViewerPlugin
{

class GLViewerTexture : public QOpenGLTexture
{

public:

    explicit GLViewerTexture(DInfoInterface* const iface);
    ~GLViewerTexture();

    bool load(const QString& fn, const QSize& size);
    bool load(const QImage& im, const QSize& size);

    GLfloat vertex_bottom() const;
    GLfloat vertex_top()    const;
    GLfloat vertex_left()   const;
    GLfloat vertex_right()  const;

    void setViewport(int w, int h);
    void zoom(float delta, const QPoint& mousepos);
    void reset();
    void move(const QPoint& diff);
    bool setNewSize(QSize size);
    void rotate();
    void zoomToOriginal();

private:

    bool loadInternal();
    void calcVertex();

private:

    // No copy constructor
    GLViewerTexture(const GLViewerTexture&);

    class Private;
    Private* const d;
};

} // namespace DigikamGenericGLViewerPlugin

#endif // DIGIKAM_GLVIEWERPLUGIN_GLVIEWERTEXTURE_H
