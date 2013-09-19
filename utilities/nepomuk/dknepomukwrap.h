/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-17
 * Description : Wrapper class that will provide get/set methods to
 *               communicate with Nepomuk. Since Nepomuk can change it's api
 *               please keep all Nepomuk related code in this class.
 *               DkNepomukService should be as clean as possible.
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
#ifndef DKNEPOMUKWRAP_H
#define DKNEPOMUKWRAP_H

#include <Nepomuk2/Tag>

namespace Digikam
{

/**
 * @brief DkNepomukService - wrapper class aroud Nepomuk Api, to protect
 *                           code from DkNepomukService from Nepomuk Api changes
 */
class DkNepomukWrap
{

public:
    DkNepomukWrap();

    /**
     * @brief digikamToNepomukTag - get corresponding Nepomuk Tag given Digikam
     *                              tag id
     * @param tagId               - tag id from digiKam's database
     */
    static Nepomuk2::Tag digikamToNepomukTag(int tagId);

    /**
     * @brief renameNepomukTag  - will rename a nepomuk Tag when it's corresponding
     *                            digiKam tag is renamed
     * @param oldName           - tag's old name to be identified in Nepomuk base
     * @param newName           - tag's new name
     */
    static void renameNepomukTag(QString oldName, QString newName);

    /**
     * @brief setUnsetTag   - used to add or remove a tag from specified resource
     * @param res           - resource to which tag will be asigned or deleted
     * @param tagToSet      - tag to be added or removed from resource
     * @param toSet         - true if tag will be set and false to remove
     */
    static void setUnsetTag(Nepomuk2::Resource res, Nepomuk2::Tag tagToSet,
                            bool toSet);

};

}

#endif // DKNEPOMUKWRAP_H