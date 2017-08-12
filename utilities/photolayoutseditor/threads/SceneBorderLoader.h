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

#ifndef SCENEBORDERLOADER_H
#define SCENEBORDERLOADER_H

#include <QThread>
#include <QDomDocument>

namespace PhotoLayoutsEditor
{
    class SceneBorder;
    class SceneBorderLoader : public QThread
    {
        public:

            explicit SceneBorderLoader(SceneBorder * border, QDomElement & element, QObject * parent = 0);

        protected:

            virtual void run();

        private:

            SceneBorder * m_border;
            QDomElement & m_element;
    };
}

#endif // SCENEBORDERLOADER_H
