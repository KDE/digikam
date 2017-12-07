/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : Template information container.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

namespace Digikam
{

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
    bool b1  = m_authors         == t.m_authors;
    bool b2  = m_authorsPosition == t.m_authorsPosition;
    bool b3  = m_credit          == t.m_credit;
    bool b4  = m_copyright       == t.m_copyright;
    bool b5  = m_rightUsageTerms == t.m_rightUsageTerms;
    bool b6  = m_source          == t.m_source;
    bool b7  = m_instructions    == t.m_instructions;
    bool b8  = m_locationInfo    == t.m_locationInfo;
    bool b9  = m_contactInfo     == t.m_contactInfo;
    bool b10 = m_subjects        == t.m_subjects;
    /*
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.authors()         << m_authors         << b1;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.authorsPosition() << m_authorsPosition << b2;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.credit()          << m_credit          << b3;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.copyright()       << m_copyright       << b4;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.rightUsageTerms() << m_rightUsageTerms << b5;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.source()          << m_source          << b6;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.instructions()    << m_instructions    << b7;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.locationInfo()    << m_locationInfo    << b8;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.contactInfo()     << m_contactInfo     << b9;
        qCDebug(DIGIKAM_METAENGINE_LOG) << t.IptcSubjects()    << m_subjects        << b10;
    */
    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10;
}

bool Template::isEmpty() const
{
    return (m_authors.isEmpty()         &&
            m_authorsPosition.isEmpty() &&
            m_credit.isEmpty()          &&
            m_copyright.isEmpty()       &&
            m_rightUsageTerms.isEmpty() &&
            m_source.isEmpty()          &&
            m_instructions.isEmpty()    &&
            m_locationInfo.isEmpty()    &&
            m_contactInfo.isEmpty()     &&
            m_subjects.isEmpty()
           );
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

void Template::setCopyright(const MetaEngine::AltLangMap& copyright)
{
    m_copyright = copyright;
}

void Template::setRightUsageTerms(const MetaEngine::AltLangMap& rightUsageTerms)
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

void Template::setLocationInfo(const IptcCoreLocationInfo& inf)
{
    m_locationInfo = inf;
}

void Template::setContactInfo(const IptcCoreContactInfo& inf)
{
    m_contactInfo = inf;
}

void Template::setIptcSubjects(const QStringList& subjects)
{
    m_subjects = subjects;
    m_subjects.sort();
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

MetaEngine::AltLangMap Template::copyright() const
{
    return m_copyright;
}

MetaEngine::AltLangMap Template::rightUsageTerms() const
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

IptcCoreLocationInfo Template::locationInfo() const
{
    return m_locationInfo;
}

IptcCoreContactInfo Template::contactInfo() const
{
    return m_contactInfo;
}

QStringList Template::IptcSubjects() const
{
    return m_subjects;
}

QDebug operator<<(QDebug dbg, const Template& t)
{
    dbg.nospace() << "Template::title: "
                  << t.templateTitle() << ", ";
    dbg.nospace() << "Template::authors: "
                  << t.authors() << ", ";
    dbg.nospace() << "Template::authorsPosition: "
                  << t.authorsPosition() << ", ";
    dbg.nospace() << "Template::credit: "
                  << t.credit() << ", ";
    dbg.nospace() << "Template::copyright: "
                  << t.copyright() << ", ";
    dbg.nospace() << "Template::rightUsageTerms: "
                  << t.rightUsageTerms() << ", ";
    dbg.nospace() << "Template::source: "
                  << t.source() << ", ";
    dbg.nospace() << "Template::instructions: "
                  << t.instructions() << ", ";
    dbg.nospace() << "Template::locationinfo: "
                  << t.locationInfo() << ", ";
    dbg.nospace() << "Template::contactinfo: "
                  << t.contactInfo();
    dbg.nospace() << "Template::IptcSubjects: "
                  << t.IptcSubjects();
    return dbg.space();
}

}  // namespace Digikam
