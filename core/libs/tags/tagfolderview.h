/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : tags folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TAGFOLDERVIEW_H
#define TAGFOLDERVIEW_H

// Qt includes

#include <QTreeView>

// Local includes

#include "albumtreeview.h"

namespace Digikam
{

class ContextMenuHelper;

class TagFolderView: public TagTreeView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent for Qt's parent child mechanism
     * @param model tag model to display
     */
    TagFolderView(QWidget* const parent, Digikam::TagModel* const model);

    /**
     * Destructor.
     */
    virtual ~TagFolderView();

    /**
     * Define whether to show the "find duplicate" action in context menus
     * or not.
     *
     * @param show if <code>true</code> the action to find duplicate images in
     *             the tag album is displayed
     */
    void setShowFindDuplicateAction(bool show);

    /**
     * Define whether to show the "Delete People Tags" action in context menus
     * or not.
     *
     * @param show if <code>true</code> the action to delete people tags in
     *             the tag album is displayed
     */
    void setShowDeleteFaceTagsAction(bool show);

Q_SIGNALS:

    void signalFindDuplicates(QList<TAlbum*>);

protected:

    QString contextMenuTitle() const;

    /**
     * Hook method to add custom actions to the generated context menu.
     *
     * The default implementation adds actions to reset the tag icon and to
     * find duplicates in a tag album. If you want to use these actions,
     * remember to call this class' implementation of this method and
     * the handleCustomContextMenuAction in your derived class.
     *
     * @param cmh helper object to create the context menu
     * @param album tag on which the context menu will be created. May be null if
     *              it is requested on no tag entry
     */
    virtual void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album);

    /**
     * Hook method to handle the custom context menu actions that were added
     * with addCustomContextMenuActions.
     *
     * @param action the action that was chosen by the user, may be null if none
     *               of the custom actions were selected
     * @param album the tag on which the context menu was requested. May be null
     *              if there was no
     */
    virtual void handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album);

    /**
     * Reimplement contextMenuEvent from AbstractAlbumTree to support multiple
     * selection
     *
     * @param event context menu event triggered by right click
     */
    void contextMenuEvent(QContextMenuEvent* event);

    /**
     * Implementation of AddCustomContextMenuActions(see above) that handle
     * multiple selection. If only one element is selected, only
     * AddCustomContextMenuActions is called
     *
     * @param cmh       - helper object to create context menu
     * @param albums    - vector of selected albums to be used on menu actions
     */
    virtual void setContexMenuItems(ContextMenuHelper& cmh, QList< TAlbum* > albums);

private Q_SLOTS:

    void slotTagNewFromABCMenu(const QString& personName);

    /**
     * @brief slotExpandNode - expands recursively selected nodes
     */
    void slotExpandNode();

    /**
     * @brief slotCollapseNode - collapse recursively selected nodes
     */
    void slotCollapseNode();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TAGFOLDERVIEW_H
