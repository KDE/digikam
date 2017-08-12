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

#ifndef ABSTRACT_ITEMS_TOOL_H
#define ABSTRACT_ITEMS_TOOL_H

#include <QWidget>

#include "AbstractTool.h"

namespace PhotoLayoutsEditor
{

class AbstractPhoto;
class EffectsListView;
class ToolsDockWidget;

class AbstractItemsTool : public AbstractTool
{
    Q_OBJECT

private:

    AbstractPhoto* m_photo;
    QPointF        m_point;

public:

    AbstractItemsTool(Scene * scene, Canvas::SelectionMode selectionMode, QWidget * parent = 0);

    /** Current photo property
    * This property holds an information which item is currently editing.
    */
    Q_PROPERTY(AbstractPhoto * m_photo READ currentItem WRITE setCurrentItem)

    AbstractPhoto * currentItem();
    void setCurrentItem(AbstractPhoto * photo);

    QPointF mousePosition();
    void setMousePosition(const QPointF & position);

Q_SIGNALS:

    void itemCreated(AbstractPhoto * item);

public Q_SLOTS:

    /** This slot is called before current item change
    * It gives a chanse to save changes of currently edited item.
    */
    virtual void currentItemAboutToBeChanged() = 0;

    /** This slot is called after current item changed.
    * This is a notification to open editors/tools and configure it for new item.
    */
    virtual void currentItemChanged() = 0;

    /** This slot is called before current mouse position change.
    * This is a notification for the editor/tool to clear it's drawing on the current
    * position.
    */
    virtual void positionAboutToBeChanged() = 0;

    /** This slot is called after current mouse position changed.
    * This is a notification for the editor/tool to draw it's data on the new position.
    */
    virtual void positionChanged() = 0;
};

} // namespace PhotoLayoutsEditor

#endif // ABSTRACT_ITEMS_TOOL_H
