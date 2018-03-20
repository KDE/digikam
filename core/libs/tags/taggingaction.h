/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Action when adding a tag
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef TAGGINGACTION_H
#define TAGGINGACTION_H

// Qt includes

#include <QString>
#include <QMetaType>

namespace Digikam
{

class TaggingAction
{
public:

    /** Describes two possible actions:
     *  Assigning an existing tag, known by tag id,
     *  or creation of a new tag, with a given tag name and a parent tag.
     */
    enum Type
    {
        NoAction,
        AssignTag,
        CreateNewTag
    };

public:

    /** Create a NoAction
     */
    TaggingAction();

    /** Assign the existing tag with given id
     */
    explicit TaggingAction(int tagId);

    /** Create a new tag with the given name.
     *  The parent shall be the tag with the given id,
     *  or 0 for a toplevel tag.
     */
    TaggingAction(const QString& name, int parentTagId);

    bool operator==(const TaggingAction& other) const;

    Type type()              const;
    bool isValid()           const;
    bool shallAssignTag()    const;
    bool shallCreateNewTag() const;

    /// If shallAssignTag(), returns the tag id
    int tagId() const;

    /// If shallCreateNewTag(), returns the tag name and the parent tag id, 0 for toplevel tag
    QString newTagName()  const;
    int     parentTagId() const;

protected:

    Type    m_type;
    int     m_tagId;
    QString m_tagName;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::TaggingAction)

#endif // TAGGINGACTION_H
