/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : Template information container.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "template.h"

namespace Digikam
{

class TemplatePrivate
{
public:

    TemplatePrivate(){}

    // template title used internaly
    QString     templateTitle;

    // Metadata strings recorded to DB and XMP
    QStringList authors;
    QString     authorsPosition;
    QString     credit;
    QString     copyright;
    QString     rightUsageTerms;
    QString     source;
    QString     instructions;
};

Template::Template()
{
}

Template::~Template()
{
}

bool Template::isNull() const
{
    return m_templateTitle.isNull();
}

bool Template::operator==(const Template& t) const
{
    return m_authors          == t.m_authors
        && m_authorsPosition  == t.m_authorsPosition
        && m_credit           == t.m_credit
        && m_copyright        == t.m_copyright
        && m_rightUsageTerms  == t.m_rightUsageTerms
        && m_source           == t.m_source
        && m_instructions     == t.m_instructions
      ;
}

void Template::setTemplateTitle(const QString& title)
{
    m_templateTitle = title;
}

QString Template::templateTitle() const
{
    return m_templateTitle;
}

void Template::setAuthors(const QStringList& authors)
{
    m_authors = authors;
    m_authors.sort();
}

void Template::setAuthorsPosition(const QString& authorsPosition)
{
    m_authorsPosition = authorsPosition;
}

void Template::setCredit(const QString& credit)
{
    m_credit = credit;
}

void Template::setCopyright(const QString& copyright)
{
    m_copyright = copyright;
}

void Template::setRightUsageTerms(const QString& rightUsageTerms)
{
    m_rightUsageTerms = rightUsageTerms;
}

void Template::setSource(const QString& source)
{
    m_source = source;
}

void Template::setInstructions(const QString& instructions)
{
    m_instructions = instructions;
}

QStringList Template::authors() const
{
    return m_authors;
}

QString Template::authorsPosition() const
{
    return m_authorsPosition;
}

QString Template::credit() const
{
    return m_credit;
}

QString Template::copyright() const
{
    return m_copyright;
}

QString Template::rightUsageTerms() const
{
    return m_rightUsageTerms;
}

QString Template::source() const
{
    return m_source;
}

QString Template::instructions() const
{
    return m_instructions;
}

}  // namespace Digikam
