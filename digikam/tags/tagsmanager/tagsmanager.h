/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
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
#include <kmainwindow.h>
#include <QPointer>

namespace Digikam
{

class TagModel;
class TAlbum;

class TagsManager : public KMainWindow
{
    Q_OBJECT

public:
    TagsManager();
    ~TagsManager();

    /**
     * @brief setupUi   setup all gui elements for Tag Manager
     * @param Dialog    parent dialog
     */
    void setupUi(KMainWindow *Dialog);

    static QPointer<TagsManager> internalPtr;
    static TagsManager* instance();

Q_SIGNALS:
    void signalSelectionChanged(TAlbum* album);

private Q_SLOTS:

    /**
     * @brief slotOpenProperties - open tag properties option when
     *                             activating Tag Properties from right sidebar
     */
    void slotOpenProperties();

    /**
     * @brief slotSelectionChanged - update tag properties in tagPropWidget when
     *                               different item is selected
     */
    void slotSelectionChanged();

    /**
     * Not used yet
     */
    void slotItemChanged();

    /**
     * @brief slotAddAction     - add new tag when addAction(+) is triggered
     */
    void slotAddAction();

    /**
     * @brief slotDeleteAction  - delete tag/tags when delAction is triggered
     */
    void slotDeleteAction();

    /**
     * @brief slotResetTagIcon  - connected to resetTagIcon action and
     *                            will reset icon to all selected tags
     */
    void slotResetTagIcon();

    /**
     * @brief slotCreateTagAddr - connected to createTagAddr action and
     *                            will create tags from Adressbook
     */
    void slotCreateTagAddr();

    /**
     * @brief slotInvertSel     - connected to invSel action and will
     *                            invert selection of current items
     */
    void slotInvertSel();

    /**
     * @brief slotWriteToImg     - connected to wrDbImg action and will
     *                             write all metadata from database to images
     */
    void slotWriteToImg();

    /**
     * @brief slotReadFromImg     - coonected to readTags action and will
     *                              reread all images metadata into database
     */
    void slotReadFromImg();

    /**
     * @brief slotWipeAll         - connected to wipeAll action and will
     *                              wipe all tag related data from database
     *                              and reread from image's metadata
     */
    void slotWipeAll();

    /**
     * @brief slotNepomukToDb     - coonected to syncNepomuk action and
     *                               will sync all Database tags with nepomuk
     *                               interface
     */
    void slotNepomukToDb();

    void slotDbToNepomuk();

    void slotForkTags();

private:

    void setupActions();
    class PrivateTagMngr;
    PrivateTagMngr* d;
};

}
