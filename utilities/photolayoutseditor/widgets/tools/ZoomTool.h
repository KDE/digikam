/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ZOOMTOOL_H
#define ZOOMTOOL_H

#include "AbstractTool.h"

namespace PhotoLayoutsEditor
{
    class ZoomTool : public AbstractTool
    {
            Q_OBJECT

        public:

            explicit ZoomTool(Scene * scene, QWidget * parent = 0);
            ~ZoomTool();

        public Q_SLOTS:

            void zoom(const QPointF & point);

        protected:

            virtual void sceneChange();
            virtual void sceneChanged();

        private:

            class ZoomToolPrivate;
            ZoomToolPrivate * d;
            friend class ZoomToolPrivate;
    };
}

#endif // ZOOMTOOL_H
