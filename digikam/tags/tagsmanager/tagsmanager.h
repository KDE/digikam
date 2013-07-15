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
#include <kdialog.h>

namespace Digikam
{

class TagModel;
class TAlbum;

class TagsManager : public KDialog
{
    Q_OBJECT

public:
    TagsManager(TagModel* model = 0);
    ~TagsManager();

    /**
     * @brief setupUi   setup all gui elements for Tag Manager
     * @param Dialog    parent dialog
     */
    void setupUi(KDialog *Dialog);


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

private:
    class PrivateTagMngr;
    PrivateTagMngr* d;
};

}
