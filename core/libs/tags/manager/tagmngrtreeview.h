/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-05
 * Description : Tag Manager Tree View derived from TagsFolderView to implement
 *               a custom context menu and some batch view options, such as
 *               expanding multiple items
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef DIGIKAM_TAG_MNGR_TREE_VIEW_H
#define DIGIKAM_TAG_MNGR_TREE_VIEW_H

// Local includes

#include "tagfolderview.h"

namespace Digikam
{

class TagsManager;

class TagMngrTreeView : public TagFolderView
{
    Q_OBJECT

public:

    explicit TagMngrTreeView(TagsManager* const parent, TagModel* const model);
    virtual ~TagMngrTreeView();

    /**
     * @brief setAlbumFilterModel - reimplement from AbstractAlbumTree
     */
    void setAlbumFilterModel(TagsManagerFilterModel* const filteredModel,
                             CheckableAlbumFilterModel* const filterModel);

    TagsManagerFilterModel* getFilterModel() const
    {
        return m_tfilteredModel;
    }

protected:

    /**
     * @brief setContexMenuItems -  Reimplemented method from TagsFolderView.
     *                              Will set custom actions for Tags Manager.
     *                              Some actions are also available in toolbar
     *
     * @param chm                - ContextMenuHelper class to help setting some
     *                             basic actions
     * @param albums             - List of currently selected albums
     */
    virtual void setContexMenuItems(ContextMenuHelper& cmh, const QList<TAlbum*>& albums);

    /**
     * @brief contextMenuEvent   - Reimplement contextMenuEvent from AbstractAlbumTree
     *                             to support multiple selection
     *
     * @param event context menu event triggered by right click
     */
    void contextMenuEvent(QContextMenuEvent* event) override;

protected:

    TagsManagerFilterModel* m_tfilteredModel;

public Q_SLOTS:

    /**
     * @brief slotExpandTree    - connected to expandTree action and will
     *                            expand tree by one level
     */
    void slotExpandTree();

    /**
     * @brief slotExpandSelected - connected to expandSel action and will
     *                             expand selected nodes by one level
     */
    void slotExpandSelected();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TAG_MNGR_TREE_VIEW_H
