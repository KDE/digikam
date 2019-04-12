/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-12
 * Description : Access to copy-right info of an item in the database
 *
 * Copyright (C) 2008-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemcopyright.h"

// Qt includes

#include <QLocale>

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "itemscanner.h"
#include "template.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemCopyrightCache
{
public:

    explicit ItemCopyrightCache(ItemCopyright* const object)
        : object(object)
    {
        // set this as cache
        object->m_cache = this;
        // read all properties
        infos = CoreDbAccess().db()->getItemCopyright(object->m_id, QString());
    }

    ~ItemCopyrightCache()
    {
        object->m_cache = nullptr;
    }

    QList<CopyrightInfo> infos;

private:

    ItemCopyright* object;
};

// -------------------------------------------------------------------------------------------

ItemCopyright::ItemCopyright(qlonglong imageid)
    : m_id(imageid),
      m_cache(nullptr)
{
}

ItemCopyright::ItemCopyright()
    : m_id(0),
      m_cache(nullptr)
{
}

ItemCopyright::ItemCopyright(const ItemCopyright& other)
    : m_id(other.m_id),
      m_cache(nullptr)
{
    // the cache is only short-lived, to keep complexity low
}

ItemCopyright::~ItemCopyright()
{
    delete m_cache;
    m_cache = nullptr;
}

ItemCopyright& ItemCopyright::operator=(const ItemCopyright& other)
{
    delete m_cache;
    m_cache = nullptr;

    m_id = other.m_id;
    return *this;
}

void ItemCopyright::replaceFrom(const ItemCopyright& source)
{
    if (!m_id)
    {
        return;
    }

    CoreDbAccess access;
    access.db()->removeItemCopyrightProperties(m_id);

    if (!source.m_id)
    {
        return;
    }

    QList<CopyrightInfo> infos = access.db()->getItemCopyright(source.m_id, QString());

    foreach (const CopyrightInfo& info, infos)
    {
        access.db()->setItemCopyrightProperty(m_id, info.property, info.value,
                                               info.extraValue, CoreDB::PropertyNoConstraint);
    }
}

QStringList ItemCopyright::creator() const
{
    QList<CopyrightInfo> infos = copyrightInfos(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreator));
    QStringList list;

    foreach (const CopyrightInfo& info, infos)
    {
        list << info.value;
    }

    return list;
}

void ItemCopyright::setCreator(const QString& creator, ReplaceMode mode)
{
    CoreDB::CopyrightPropertyUnique uniqueness;

    if (mode == ReplaceAllEntries)
    {
        uniqueness = CoreDB::PropertyUnique;
    }
    else
    {
        uniqueness = CoreDB::PropertyNoConstraint;
    }

    CoreDbAccess().db()->setItemCopyrightProperty(m_id, ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreator),
                                                   creator, QString(), uniqueness);
}

void ItemCopyright::removeCreators()
{
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreator));
}

QString ItemCopyright::provider() const
{
    return readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvider));
}

void ItemCopyright::setProvider(const QString& provider)
{
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvider), provider);
}

void ItemCopyright::removeProvider()
{
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvider));
}

QString ItemCopyright::copyrightNotice(const QString& languageCode)
{
    return readLanguageProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCopyrightNotice), languageCode);
}

MetaEngine::AltLangMap ItemCopyright::allCopyrightNotices()
{
    return readLanguageProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCopyrightNotice));
}

void ItemCopyright::setCopyrightNotice(const QString& notice, const QString& languageCode, ReplaceMode mode)
{
    setLanguageProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCopyrightNotice), notice, languageCode, mode);
}

void ItemCopyright::removeCopyrightNotices()
{
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCopyrightNotice));
}

QString ItemCopyright::rightsUsageTerms(const QString& languageCode)
{
    return readLanguageProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreRightsUsageTerms), languageCode);
}

MetaEngine::AltLangMap ItemCopyright::allRightsUsageTerms()
{
    return readLanguageProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreRightsUsageTerms));
}

void ItemCopyright::setRightsUsageTerms(const QString& term, const QString& languageCode, ReplaceMode mode)
{
    setLanguageProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreRightsUsageTerms), term, languageCode, mode);
}

void ItemCopyright::removeRightsUsageTerms()
{
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreRightsUsageTerms));
}

QString ItemCopyright::source()
{
    return readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSource));
}

void ItemCopyright::setSource(const QString& source)
{
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSource), source);
}

void ItemCopyright::removeSource()
{
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSource));
}

QString ItemCopyright::creatorJobTitle() const
{
    return readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreatorJobTitle));
}

void ItemCopyright::setCreatorJobTitle(const QString& title)
{
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreatorJobTitle), title);
}

void ItemCopyright::removeCreatorJobTitle()
{
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCreatorJobTitle));
}

QString ItemCopyright::instructions()
{
    return readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreInstructions));
}

void ItemCopyright::setInstructions(const QString& instructions)
{
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreInstructions), instructions);
}

void ItemCopyright::removeInstructions()
{
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreInstructions));
}

IptcCoreContactInfo ItemCopyright::contactInfo()
{
    IptcCoreContactInfo info;
    info.city          = readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoCity));
    info.country       = readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoCountry));
    info.address       = readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoAddress));
    info.postalCode    = readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoPostalCode));
    info.provinceState = readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoProvinceState));
    info.email         = readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoEmail));
    info.phone         = readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoPhone));
    info.webUrl        = readSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoWebUrl));

    return info;
}

void ItemCopyright::setContactInfo(const IptcCoreContactInfo& info)
{
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoCity), info.city);
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoCountry), info.country);
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoAddress), info.address);
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoPostalCode), info.postalCode);
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoProvinceState), info.provinceState);
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoEmail), info.email);
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoPhone), info.phone);
    setSimpleProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoWebUrl), info.webUrl);
}

void ItemCopyright::removeContactInfo()
{
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoCity));
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoCountry));
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoAddress));
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoPostalCode));
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoProvinceState));
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoEmail));
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoPhone));
    removeProperties(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreContactInfoWebUrl));
}

void ItemCopyright::fillTemplate(Template& t)
{
    ItemCopyrightCache cache(this);

    t.setAuthors(author());
    t.setAuthorsPosition(authorsPosition());
    t.setCredit(credit());
    t.setCopyright(allCopyrightNotices());
    t.setRightUsageTerms(allRightsUsageTerms());
    t.setSource(source());
    t.setInstructions(instructions());
    t.setContactInfo(contactInfo());
}

void ItemCopyright::setFromTemplate(const Template& t)
{
    foreach (const QString& author, t.authors()) // krazy:exclude=foreach
    {
        setAuthor(author, ItemCopyright::AddEntryToExisting);
    }

    setCredit(t.credit());

    MetaEngine::AltLangMap copyrights = t.copyright();
    MetaEngine::AltLangMap::const_iterator it;

    for (it = copyrights.constBegin() ; it != copyrights.constEnd() ; ++it)
    {
        setRights(it.value(), it.key(), ItemCopyright::AddEntryToExisting);
    }

    MetaEngine::AltLangMap usages = t.rightUsageTerms();
    MetaEngine::AltLangMap::const_iterator it2;

    for (it2 = usages.constBegin() ; it2 != usages.constEnd() ; ++it2)
    {
        setRightsUsageTerms(it2.value(), it2.key(), ItemCopyright::AddEntryToExisting);
    }

    setSource(t.source());
    setAuthorsPosition(t.authorsPosition());
    setInstructions(t.instructions());
    setContactInfo(t.contactInfo());
}

void ItemCopyright::removeAll()
{
    ItemCopyrightCache cache(this);

    removeCreators();
    removeProvider();
    removeCopyrightNotices();
    removeRightsUsageTerms();
    removeSource();
    removeCreatorJobTitle();
    removeInstructions();
    removeContactInfo();
}

CopyrightInfo ItemCopyright::copyrightInfo(const QString& property) const
{
    if (m_cache)
    {
        foreach (const CopyrightInfo& info, m_cache->infos)
        {
            if (info.property == property)
            {
                return info;
            }
        }
    }
    else
    {
        QList<CopyrightInfo> infos = CoreDbAccess().db()->getItemCopyright(m_id, property);

        if (!infos.isEmpty())
        {
            return infos.first();
        }
    }

    return CopyrightInfo();
}

QList<CopyrightInfo> ItemCopyright::copyrightInfos(const QString& property) const
{
    if (m_cache)
    {
        QList<CopyrightInfo> infos;

        foreach (const CopyrightInfo& info, m_cache->infos)
        {
            if (info.property == property)
            {
                infos << info;
            }
        }

        return infos;
    }
    else
    {
        return CoreDbAccess().db()->getItemCopyright(m_id, property);
    }
}

QString ItemCopyright::readSimpleProperty(const QString& property) const
{
    return copyrightInfo(property).value;
}

void ItemCopyright::setSimpleProperty(const QString& property, const QString& value)
{
    CoreDbAccess().db()->setItemCopyrightProperty(m_id, property, value, QString(), CoreDB::PropertyUnique);
}

QString ItemCopyright::readLanguageProperty(const QString& property, const QString& languageCode)
{
    QList<CopyrightInfo> infos = copyrightInfos(property);
    int index                  = languageMatch(infos, languageCode);

    if (index == -1)
    {
        return QString();
    }
    else
    {
        return infos.at(index).value;
    }
}

MetaEngine::AltLangMap ItemCopyright::readLanguageProperties(const QString& property)
{
    MetaEngine::AltLangMap map;
    QList<CopyrightInfo> infos = copyrightInfos(property);

    foreach (const CopyrightInfo& info, infos)
    {
        map[info.extraValue] = info.value;
    }

    return map;
}

void ItemCopyright::setLanguageProperty(const QString& property, const QString& value,
                                         const QString& languageCode, ReplaceMode mode)
{
    CoreDB::CopyrightPropertyUnique uniqueness;

    if (mode == ReplaceAllEntries)
    {
        uniqueness = CoreDB::PropertyUnique;
    }
    else if (mode == ReplaceLanguageEntry)
    {
        uniqueness = CoreDB::PropertyExtraValueUnique;
    }
    else
    {
        uniqueness = CoreDB::PropertyNoConstraint;
    }

    QString language = languageCode;

    if (language.isNull())
    {
        language = QLatin1String("x-default");
    }

    CoreDbAccess().db()->setItemCopyrightProperty(m_id, property, value, language, uniqueness);
}

void ItemCopyright::removeProperties(const QString& property)
{
    // if we have a cache, find out if anything need to be done at all
    if (m_cache && copyrightInfo(property).isNull())
    {
        return;
    }

    CoreDbAccess().db()->removeItemCopyrightProperties(m_id, property);
}

void ItemCopyright::removeLanguageProperty(const QString& property, const QString& languageCode)
{
    if (m_cache && copyrightInfo(property).isNull())
    {
        return;
    }

    CoreDbAccess().db()->removeItemCopyrightProperties(m_id, property, languageCode);
}

int ItemCopyright::languageMatch(const QList<CopyrightInfo> infos, const QString& languageCode) const
{
    QString langCode;
    QString fullCode = languageCode;

    if (languageCode.isNull())
    {
        // find local language

        QString spec     = QLocale().name().toLower();
        QString langCode = spec.left(spec.indexOf(QLatin1Char('_'))) + QLatin1Char('-');
        QString fullCode = spec.replace(QLatin1Char('_'), QLatin1Char('-'));
    }
    else if (languageCode == QLatin1String("x-default"))
    {
        langCode = languageCode;
    }
    else
    {
        // en-us => en-
        langCode = languageCode.section(QLatin1Char('-'), 0, 0, QString::SectionIncludeTrailingSep);
    }

    int fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch;
    fullCodeMatch    = -1;
    langCodeMatch    = -1;
    defaultCodeMatch = -1;
    firstMatch       = -1;

    (void)firstMatch; // Remove clang warning.

    if (infos.isEmpty())
    {
        return -1;
    }
    else
    {
        firstMatch = 0; // index of first entry - at least we have one
    }

    // First we search for a full match
    // Second for a match of the language code
    // Third for the default code
    // Fourth we return the first comment

    QLatin1String defaultCode("x-default");

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        const CopyrightInfo& info = infos.at(i);

        if (info.extraValue == fullCode)
        {
            fullCodeMatch = i;
            break;
        }
        else if (info.extraValue.startsWith(langCode) && langCodeMatch == -1)
        {
            langCodeMatch = i;
        }
        else if (info.extraValue == defaultCode)
        {
            defaultCodeMatch = i;
        }
    }

    int chosen = fullCodeMatch;

    if (chosen == -1)
    {
        chosen = langCodeMatch;
    }

    if (chosen == -1)
    {
        chosen = defaultCodeMatch;
    }

    if (chosen == -1)
    {
        chosen = firstMatch;
    }

    return chosen;
}

} // namespace Digikam
