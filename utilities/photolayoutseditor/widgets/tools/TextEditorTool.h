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

#ifndef TEXTEDITORTOOL_H
#define TEXTEDITORTOOL_H

#include "AbstractItemsTool.h"

class QtAbstractPropertyBrowser;

namespace PhotoLayoutsEditor
{
    class TextItem;
    class TextEditorToolPrivate;

    class TextEditorTool : public AbstractItemsTool
    {
            Q_OBJECT

            TextEditorToolPrivate * d;

            TextItem * m_text_item;
            TextItem * m_created_text_item;
            QtAbstractPropertyBrowser * m_browser;
            bool m_create_new_item;

        public:

            explicit TextEditorTool(Scene * scene, QWidget * parent = 0);
            ~TextEditorTool();

        Q_SIGNALS:

        public Q_SLOTS:

            virtual void currentItemAboutToBeChanged();
            virtual void currentItemChanged();
            virtual void positionAboutToBeChanged();
            virtual void positionChanged();

        protected Q_SLOTS:

            void createNewItem();

        friend class TextEditorToolPrivate;
    };
}

#endif // TEXTEDITORTOOL_H
