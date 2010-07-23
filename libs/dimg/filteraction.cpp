/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class that holds list of applied filters to image
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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
{
  
}
FilterAction::FilterAction(const QString& identifier, int version, FilterAction::Category category)
{
  m_identifier = identifier;
  m_version = version;
  m_category = category; 
}

bool FilterAction::isNull() const 
{
  return m_identifier.isEmpty();
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

bool FilterAction::hasParameter(const QString& key) const
{
    return m_params.contains(key);
}

const QVariant FilterAction::parameter(const QString& key) const
{
    return m_params.value(key);
}

QVariant &FilterAction::parameter(const QString& key)
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

void FilterAction::removeParameters(const QString& key)
{
    m_params.remove(key);
}

void FilterAction::clearParameters()
{
  m_params.clear();
}


const QHash<QString,QVariant> &FilterAction::parameters() const
{
    return m_params;
}

QHash<QString,QVariant> &FilterAction::parameters()
{
    return m_params;
}

void Digikam::FilterAction::addSetOfParameters(const QHash< QString, QVariant > values)
{
    m_params = values;
}

bool FilterAction::isEmpty() const
{
    return m_params.isEmpty();
}


}