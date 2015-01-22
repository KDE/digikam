/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : base class for sidebar widgets
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

// QT includes

#include <QPixmap>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "album.h"
#include "imageinfo.h"
#include "statesavingobject.h"

namespace Digikam
{

/**
 * Abstract base class for widgets that are use in one of digikams's sidebars.
 *
 * @author jwienke
 */
class SidebarWidget : public QWidget, public StateSavingObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent of this widget, may be null
     */
    explicit SidebarWidget(QWidget* parent);

    /**
     * Destructor.
     */
    virtual ~SidebarWidget();

    /**
     * This method is called if the visible sidebar widget is changed.
     *
     * @param if true, this widget is the new active widget, if false another
     *        widget is active
     */
    virtual void setActive(bool active) = 0;

    /**
     * This method is invoked when the application settings should be (re-)
     * applied to this widget.
     */
    virtual void applySettings() = 0;

    /**
     * This is called on this widget when the history requires to move back to
     * the specified album
     */
    virtual void changeAlbumFromHistory(QList<Album*> album) = 0;

    /**
     * Must be implemented and return the icon that shall be visible for this
     * sidebar widget.
     *
     * @return pixmap icon
     */
    virtual const QIcon getIcon() = 0;

    /**
     * Must be implemented to return the title of this sidebar's tab.
     *
     * @return localized title string
     */
    virtual const QString getCaption() = 0;

Q_SIGNALS:

    /**
     * This signal can be emitted if this sidebar widget wants to be the one
     * that is active.
     */
    void requestActiveTab(SidebarWidget*);
};

} // namespace Digikam

#endif /* SIDEBARWIDGET_H */
