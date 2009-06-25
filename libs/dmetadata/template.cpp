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
        : d(new TemplatePrivate)
{
}

Template::~Template()
{
    delete d;
}

Template::Template(const Template& t)
        : d(new TemplatePrivate)
{
    setTemplateTitle(t.d->templateTitle);
    setAuthors(t.d->authors);
    setAuthorsPosition(t.d->authorsPosition);
    setCredit(t.d->credit);
    setCopyright(t.d->copyright);
    setRightUsageTerms(t.d->rightUsageTerms);
    setSource(t.d->source);
    setInstructions(t.d->instructions);
}

Template& Template::operator=(const Template& t)
{
    if (this != &t)
    {
        setTemplateTitle(t.d->templateTitle);
        setAuthors(t.d->authors);
        setAuthorsPosition(t.d->authorsPosition);
        setCredit(t.d->credit);
        setCopyright(t.d->copyright);
        setRightUsageTerms(t.d->rightUsageTerms);
        setSource(t.d->source);
        setInstructions(t.d->instructions);
    }
    return *this;
}

bool Template::operator==(const Template& t) const
{
    return d->authors          == t.d->authors
        && d->authorsPosition  == t.d->authorsPosition
        && d->credit           == t.d->credit
        && d->copyright        == t.d->copyright
        && d->rightUsageTerms  == t.d->rightUsageTerms
        && d->source           == t.d->source
        && d->instructions     == t.d->instructions
      ;
}

void Template::setTemplateTitle(const QString& title)
{
    d->templateTitle = title;
}

QString Template::templateTitle() const
{
    return d->templateTitle;
}

void Template::setAuthors(const QStringList& authors)
{
    d->authors = authors;
    d->authors.sort();
}

void Template::setAuthorsPosition(const QString& authorsPosition)
{
    d->authorsPosition = authorsPosition;
}

void Template::setCredit(const QString& credit)
{
    d->credit = credit;
}

void Template::setCopyright(const QString& copyright)
{
    d->copyright = copyright;
}

void Template::setRightUsageTerms(const QString& rightUsageTerms)
{
    d->rightUsageTerms = rightUsageTerms;
}

void Template::setSource(const QString& source)
{
    d->source = source;
}

void Template::setInstructions(const QString& instructions)
{
    d->instructions = instructions;
}

QStringList Template::authors() const
{
    return d->authors;
}

QString Template::authorsPosition() const
{
    return d->authorsPosition;
}

QString Template::credit() const
{
    return d->credit;
}

QString Template::copyright() const
{
    return d->copyright;
}

QString Template::rightUsageTerms() const
{
    return d->rightUsageTerms;
}

QString Template::source() const
{
    return d->source;
}

QString Template::instructions() const
{
    return d->instructions;
}

}  // namespace Digikam
