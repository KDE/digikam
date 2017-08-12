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

#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "AbstractPhoto.h"

#include <QObject>
#include <QUndoCommand>

namespace PhotoLayoutsEditor
{
    class TextItemLoader;

    class TextChangeUndoCommand;
    class TextColorUndoCommand;
    class TextFontUndoCommand;
    class AddTextUndoCommand;
    class RemoveTextUndoCommand;
    class AddLineUndoCommand;
    class MergeLineUndoCommand;

    class TextItem : public AbstractPhoto
    {
            static QColor DEFAULT_COLOR;
            static QFont DEFAULT_FONT;

        public:

            explicit TextItem(const QString & text = QString(), Scene * scene = 0);
            virtual void focusInEvent(QFocusEvent *event);
            virtual void focusOutEvent(QFocusEvent *event);
            virtual void keyPressEvent(QKeyEvent * event);
            virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

            Q_PROPERTY(QColor m_color READ color WRITE setColor)
            QColor color() const;
            void setColor(const QColor & color);

            Q_PROPERTY(QFont m_font READ font WRITE setFont)
            QFont font() const;
            void setFont(const QFont & font);

            Q_PROPERTY(QStringList m_string_list READ text WRITE setText)
            QStringList text() const;
            void setText(const QStringList & textList);

            QString textMultiline() const;
            void setText(const QString & text);

            virtual QPainterPath itemShape() const;
            virtual QPainterPath itemOpaqueArea() const;
            virtual QPainterPath itemDrawArea() const;

            virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

            virtual QDomDocument toSvg() const;
            virtual QDomDocument toTemplateSvg() const;
            virtual QDomDocument svgVisibleArea() const;
            virtual QDomDocument svgTemplateArea() const;
            static TextItem * fromSvg(QDomElement & element);

            /// Returns item's property browser
            virtual QtAbstractPropertyBrowser * propertyBrowser();

        private:

            virtual void refreshItem();
            QPainterPath getLinePath(const QString & string);
            void setCursorPositionVisible(bool isVisible);
            void updateIcon();


            class TextItemPrivate
            {
                TextItemPrivate(TextItem * item) :
                    m_item(item),
                    m_cursorIsVisible(false),
                    m_cursor_row(0),
                    m_cursor_character(0),
                    command(0)
                {
                }

                void moveCursorLeft();
                void moveCursorRight();
                void moveCursorUp();
                void moveCursorDown();
                void moveCursorEnd();
                void moveCursorHome();
                void removeTextAfter();
                void removeTextBefore();
                void addNewLine();
                void addText(const QString & text);
                void addText(int row, int at, const QString & text);
                void removeText(int row, int at, int length);
                void closeEditor();

                TextItem * m_item;

                QStringList m_string_list;

                QPointF m_cursor_point;
                bool m_cursorIsVisible;
                int m_cursor_row;
                int m_cursor_character;

                QUndoCommand * command;

                friend class TextItem;
                friend class TextItemLoader;

                friend class TextChangeUndoCommand;
                friend class TextColorUndoCommand;
                friend class TextFontUndoCommand;
                friend class AddTextUndoCommand;
                friend class RemoveTextUndoCommand;
                friend class AddLineUndoCommand;
                friend class MergeLineUndoCommand;
            };
            TextItemPrivate * d;
            friend class TextItemPrivate;

            QColor m_color;
            QFont m_font;

            QPainterPath m_complete_path;
            QPainterPath m_text_path;
            QFontMetrics m_metrics;

        friend class TextItemLoader;

        friend class TextChangeUndoCommand;
        friend class TextColorUndoCommand;
        friend class TextFontUndoCommand;
        friend class AddTextUndoCommand;
        friend class RemoveTextUndoCommand;
        friend class AddLineUndoCommand;
        friend class MergeLineUndoCommand;
    };
}

#endif // TEXTITEM_H
