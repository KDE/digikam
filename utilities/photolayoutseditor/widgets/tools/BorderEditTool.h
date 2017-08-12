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

#ifndef BORDEREDITTOOL_H
#define BORDEREDITTOOL_H

#include "AbstractItemsListViewTool.h"

#include <QDockWidget>
#include <QDoubleSpinBox>

#include <QComboBox>
#include <kcolorbutton.h>
#include <QPushButton>

namespace PhotoLayoutsEditor
{
    class BorderEditTool : public AbstractItemsListViewTool
    {
            Q_OBJECT

        public:

            explicit BorderEditTool(Scene * scene, QWidget * parent = 0);
            virtual QStringList options() const;
            virtual AbstractMovableModel * model();
            virtual QObject * createItem(const QString & name);
            virtual QWidget * createEditor(QObject * item, bool createCommands = true);

    };
}

#endif // BORDEREDITTOOL_H
