/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creatingtags
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

#include "taggingaction.h"

namespace Digikam
{

TaggingAction::TaggingAction()
    : m_type(NoAction),
      m_tagId(-1)
{
}

TaggingAction::TaggingAction(int tagId)
    : m_type(AssignTag),
      m_tagId(tagId)
{
}

TaggingAction::TaggingAction(const QString& name, int parentTagId)
    : m_type(CreateNewTag),
      m_tagId(parentTagId),
      m_tagName(name)
{
}

bool TaggingAction::operator==(const TaggingAction& other) const
{
    return (m_type    == other.m_type)    &&
           (m_tagId   == other.m_tagId)   &&
           (m_tagName == other.m_tagName);
}

TaggingAction::Type TaggingAction::type() const
{
    return m_type;
}

bool TaggingAction::isValid() const
{
    return m_type != NoAction;
}

bool TaggingAction::shallAssignTag() const
{
    return m_type == AssignTag;
}

bool TaggingAction::shallCreateNewTag() const
{
    return m_type == CreateNewTag;
}

int TaggingAction::tagId() const
{
    return m_tagId;
}

QString TaggingAction::newTagName() const
{
    return m_tagName;
}

int TaggingAction::parentTagId() const
{
    return m_tagId;
}

} // namespace Digikam
