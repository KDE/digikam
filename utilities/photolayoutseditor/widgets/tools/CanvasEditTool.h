/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
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

#ifndef CANVAS_EDIT_TOOL_H
#define CANVAS_EDIT_TOOL_H

#include <QWidget>
#include <QComboBox>

#include "AbstractTool.h"

namespace PhotoLayoutsEditor
{
    class CanvasEditToolPrivate;
    class CanvasEditTool : public AbstractTool
    {
            Q_OBJECT

            CanvasEditToolPrivate * d;
            bool hold_update;

        public:

            explicit CanvasEditTool(Scene * scene, QWidget * parent = 0);
            virtual ~CanvasEditTool();

        Q_SIGNALS:

        public Q_SLOTS:

            void backgroundTypeChanged(const QString & typeName);

        protected:

            virtual void sceneChange();
            virtual void sceneChanged();

        protected Q_SLOTS:

            // Type of background selection
            void colorBackgroundSelected();
            void gradientBackgroundSelected();
            void imageBackgroundSelected();
            void patternBackgroundSelected();

            // Solid background
            void solidColorChanged(const QColor & color);
            void imageBackgroundColorChanged(const QColor & color);

            // Pattern background
            void patternFirstColorChanged(const QColor & color);
            void patternSecondColorChanged(const QColor & color);
            void patternStyleChanged(Qt::BrushStyle patternStyle);

            // Image background
            void imageUrlRequest();
            void borderImageUrlRequest();
            void imageScallingChanged(const QString & scallingName);
            void imageTiledChanged(int state);
            void imageHorizontalAlignmentChanged(int index);
            void imageVerticalAlignmentChanged(int index);
            void imageWidthChanged();
            void imageHeightChanged();

            // Other (currently unused... :P)
            void readMousePosition(const QPointF & scenePos);

        private:

            void setImageBackground();
            void setPatternBackground();
            void setImageBorder();
            void setupGUI();

        private Q_SLOTS:

            void updateWidgets();
    };
}

#endif // CANVAS_EDIT_TOOL_H
