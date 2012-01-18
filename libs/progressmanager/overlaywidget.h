/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : progress manager
 *
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2004 David Faure <faure at kde dot org>
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

#ifndef OVERLAY_WIDGET_H
#define OVERLAY_WIDGET_H

#include <KHBox>

class QResizeEvent;
class QEvent;

namespace Digikam
{

/**
 * This is a widget that can align itself with another one, without using a layout,
 * so that it can actually be on top of other widgets.
 * Currently the only supported type of alignment is "right aligned, on top of the other widget".
 *
 * OverlayWidget inherits KHBox for convenience purposes (layout, and frame)
 */
class OverlayWidget : public KHBox
{
    Q_OBJECT

public:
    
    OverlayWidget(QWidget* alignWidget, QWidget* parent, const char* name = 0);
    ~OverlayWidget();

    QWidget* alignWidget() const { return mAlignWidget; }
    void setAlignWidget(QWidget* alignWidget);

protected:
    
    void resizeEvent(QResizeEvent* ev);
    bool eventFilter(QObject* o, QEvent* e);

private:
    
    void reposition();

private:
    
    QWidget* mAlignWidget;
};

} // namespace Digikam

#endif /* OVERLAY_WIDGET_H */
