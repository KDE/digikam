/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class that holds list of applied filters to image
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "filteraction.h"

namespace Digikam
{

FilterAction::FilterAction()
    : m_category(ReproducibleFilter),
      m_version(0)
{
}

FilterAction::FilterAction(const QString& identifier, int version, FilterAction::Category category)
    : m_category(category),
      m_identifier(identifier),
      m_version(version)
{
}

bool FilterAction::isNull() const
{
    return m_identifier.isEmpty();
}

bool FilterAction::operator==(const FilterAction& other) const
{
    return m_identifier      == other.m_identifier             &&
           m_version         == other.m_version                &&
           m_category        == other.m_category               &&
           m_description     == other.m_description            &&
           m_displayableName == other.m_displayableName        &&
           m_params          == other.m_params;
}

FilterAction::Category FilterAction::category() const
{
    return m_category;
}

QString FilterAction::identifier() const
{
    return m_identifier;
}

int FilterAction::version() const
{
    return m_version;
}

QString FilterAction::description() const
{
    return m_description;
}

void FilterAction::setDescription(const QString& description)
{
    m_description = description;
}

QString FilterAction::displayableName() const
{
    return m_displayableName;
}

void FilterAction::setDisplayableName(const QString& displayableName)
{
    m_displayableName = displayableName;
}

FilterAction::Flags FilterAction::flags() const
{
    return m_flags;
}

void FilterAction::setFlags(Flags flags)
{
    m_flags = flags;
}

void FilterAction::addFlag(Flags flags)
{
    m_flags |= flags;
}

void FilterAction::removeFlag(Flags flags)
{
    m_flags &= ~flags;
}

bool FilterAction::hasParameters() const
{
    return !m_params.isEmpty();
}

const QHash<QString,QVariant> &FilterAction::parameters() const
{
    return m_params;
}

QHash<QString, QVariant> &FilterAction::parameters()
{
    return m_params;
}

bool FilterAction::hasParameter(const QString& key) const
{
    return m_params.contains(key);
}

const QVariant FilterAction::parameter(const QString& key) const
{
    return m_params.value(key);
}

QVariant& FilterAction::parameter(const QString& key)
{
    return m_params[key];
}

void FilterAction::setParameter(const QString& key, const QVariant& value)
{
    m_params.insert(key, value);
}

void FilterAction::addParameter(const QString& key, const QVariant& value)
{
    m_params.insertMulti(key, value);
}

void FilterAction::addParameters(const QHash<QString, QVariant>& params)
{
    m_params = m_params.unite(params);
}

void FilterAction::setParameters(const QHash<QString, QVariant>& params)
{
    m_params = params;
}

void FilterAction::removeParameters(const QString& key)
{
    m_params.remove(key);
}

void FilterAction::clearParameters()
{
    m_params.clear();
}

} // namespace Digikam
