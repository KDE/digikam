/*
 * SideBarWidget.h
 *
 *  Created on: 14.11.2009
 *      Author: languitar
 */

#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

// QT includes
#include <qwidget.h>
#include <qpixmap.h>

// KDE includes
#include <kconfiggroup.h>
#include <kiconloader.h>

// Local includes
#include "album.h"
#include "imageinfo.h"

namespace Digikam
{

/**
 * Abstract base class for widgets that are use in one of digikams's sidebars.
 *
 * @author jwienke
 */
class SideBarWidget: public QWidget
{
Q_OBJECT
public:

    /**
     * Constructor.
     *
     * @param parent parent of this widget, may be null
     */
    SideBarWidget(QWidget *parent);

    /**
     * Destructor.
     */
    virtual ~SideBarWidget();

    /**
     * This method is called if the visible sidebar widget is changed.
     *
     * @param if true, this widget is the new active widget, if false another
     *        widget is active
     */
    virtual void setActive(bool active) = 0;

    /**
     * This method must be implemented to restore the last state of the sidebar
     * widget from the config.
     *
     * @param group config group to use to restore from
     */
    virtual void loadViewState(KConfigGroup &group) = 0;

    /**
     * This method must be implemented to store the current state.
     *
     * @param group config croup to store state to
     */
    virtual void saveViewState(KConfigGroup &group) = 0;

    /**
     * This is called on this widget when the history requires to move back to
     * the specified album
     */
    virtual void changeAlbumFromHistory(Album *album) = 0;

    /**
     * This method is called when the user wants to go to a special album and
     * item in this. The widget can react on this.
     *
     * @param info requested image and album
     */
    virtual void gotoAlbumAndItem(const ImageInfo &info) = 0;

    /**
     * Must be implemented and return the icon that shall be visible for this
     * sidebar widget.
     *
     * @return pixmap icon
     */
    virtual QPixmap getIcon() = 0;

    /**
     * Must be implemented to return the title of this sidebar's tab.
     *
     * @return localized title string
     */
    virtual QString getCaption() = 0;

};

}

#endif /* SIDEBARWIDGET_H */
