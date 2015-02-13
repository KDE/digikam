/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-07-31
 * Description : Tag List implementation as Quick Access for various
 *               subtrees in Tag Manager
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

#ifndef TAGLIST_H
#define TAGLIST_H

// Qt includes

#include <QWidget>

namespace Digikam
{

class TagMngrTreeView;
class Album;

class TagList : public QWidget
{
    Q_OBJECT

public:

    TagList(TagMngrTreeView* const treeView, QWidget* const parent);
    ~TagList();

    /**
     * @brief saveSettings   - save settings to digiKam_tagsmanagerrc KConfig
     */
    void saveSettings();

    /**
     * @brief restoreSettings - read settings from digikam_tagsmanagerrc
     *                          config and populate model with data
     */
    void restoreSettings();

    /**
     * @brief enableAddButton - disable Add Button when selection is empty
     *                           or only root tag is selected
     */
    void enableAddButton(bool value);

private Q_SLOTS:

    void slotAddPressed();

    void slotSelectionChanged();

    void slotTagDeleted(Album* album);

    void slotDeleteSelected();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TAGLIST_H
