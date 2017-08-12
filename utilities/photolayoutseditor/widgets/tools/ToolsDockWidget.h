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

#ifndef TOOLSDOCKWIDGET_H
#define TOOLSDOCKWIDGET_H

#include <QDockWidget>
#include <QLayout>
#include <QUndoCommand>
#include <QStackedLayout>

#include <QPushButton>

namespace PhotoLayoutsEditor
{
    class Scene;
    class AbstractTool;
    class AbstractPhoto;
    class AbstractItemsTool;
    class CanvasEditTool;

    class ToolsDockWidget : public QDockWidget
    {
            Q_OBJECT

            QPushButton * m_tool_pointer;
            QPushButton * m_tool_hand;
            QPushButton * m_tool_zoom;
            QPushButton * m_canvas_button;
            QPushButton * m_effects_button;
            QPushButton * m_text_button;
            QPushButton * m_rotate_button;
            QPushButton * m_scale_button;
            QPushButton * m_crop_button;
            QPushButton * m_tool_border;
//            QPushButton * m_tool_colorize_button;

//            bool m_has_selection;

//            QStackedLayout * m_tool_widget_layout;
//            AbstractTool   * m_zoom_widget;
//            CanvasEditTool * m_canvas_widget;
//            AbstractItemsTool * m_effects_widget;
//            AbstractItemsTool * m_text_widget;
//            AbstractItemsTool * m_border_widget;

            AbstractPhoto * m_current_item;

            Scene * m_scene;

            static ToolsDockWidget * m_instance;

        public:

            static ToolsDockWidget * instance(QWidget * parent = 0);
            ~ToolsDockWidget();
            void setDefaultTool();

        Q_SIGNALS:

            void undoCommandCreated(QUndoCommand * command);
            void newItemCreated(AbstractPhoto * item);

            void requireSingleSelection();
            void requireMultiSelection();

            void pointerToolSelected();
            void handToolSelected();
            // Zoom tool selection signals
            void zoomToolSelectionChanged(bool);
            void zoomToolSelected();
            // Effects tool selection signals
            void canvasToolSelectionChanged(bool);
            void canvasToolSelected();
            // Effects tool selection signals
            void effectsToolSelectionChanged(bool);
            void effectsToolSelected();
            // Text tool selection signals
            void textToolSelectionChanged(bool);
            void textToolSelected();
            // Rotate tool selection signals
            void rotateToolSelectionChanged(bool);
            void rotateToolSelected();
            // Scale tool selection signals
            void scaleToolSelectionChanged(bool);
            void scaleToolSelected();
            // Crop tool selection signals
            void cropToolSelectionChanged(bool);
            void cropToolSelected();
            // Border tool selection signals
            void borderToolSelectionChanged(bool);
            void borderToolSelected();

        public Q_SLOTS:

            void setScene(Scene * scene = 0);
            void itemSelected(AbstractPhoto * photo);
            void mousePositionChoosen(const QPointF & position);
            void emitNewItemCreated(AbstractPhoto * item);
            void setPointerToolVisible(bool isSelected = true);
            void setHandToolVisible(bool isSelected = true);
            void setZoomWidgetVisible(bool isSelected = true);
            void setCanvasWidgetVisible(bool isVisible = true);
            void setEffectsWidgetVisible(bool isVisible = true);
            void setTextWidgetVisible(bool isVisible = true);
            void setRotateWidgetVisible(bool isVisible = true);
            void setScaleWidgetVisible(bool isVisible = true);
            void setCropWidgetVisible(bool isVisible = true);
            void setBordersWidgetVisible(bool isVisible = true);

        protected:

            virtual void resizeEvent(QResizeEvent * event);

        private:

            explicit ToolsDockWidget(QWidget * parent = 0);

            class ToolsDockWidgetPrivate;
            ToolsDockWidgetPrivate * d;
            friend class ToolsDockWidgetPrivate;
    };
}

#endif // TOOLSDOCKWIDGET_H
