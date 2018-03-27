/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-12
 * Description : Access to copy-right info of an image in the database
 *
 * Copyright (C) 2008-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGE_COPYRIGHT_H
#define IMAGE_COPYRIGHT_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QList>

// Local includes

#include "metaengine.h"
#include "metadatainfo.h"
#include "digikam_export.h"

namespace Digikam
{

class CopyrightInfo;
class ImageCopyrightCache;
class Template;

class DIGIKAM_DATABASE_EXPORT ImageCopyright
{
public:

    enum ReplaceMode
    {
        /// Remove entries for all languages and add one new entry
        ReplaceAllEntries,
        /// Only replace the entry with the given language
        ReplaceLanguageEntry,
        /// No constraints on adding the entry
        AddEntryToExisting
    };

public:

    ImageCopyright(qlonglong imageid);

    /** Create a null ImageCopyright object */
    ImageCopyright();

    ImageCopyright(const ImageCopyright& other);
    ~ImageCopyright();

    ImageCopyright& operator=(const ImageCopyright& other);

    /** Returns the author/creator/byline.
     *  This is Photoshop Author.
     *  This is IPTC By-line.
     *  This is DC creator.
     *  This is dc:creator in XMP.
     *  &ldquo;Contains preferably the name of the person who created the content of this news object, a
     *  photographer for photos, a graphic artist for graphics, or a writer for textual news. If it is not
     *  appropriate to add the name of a person the name of a company or organization could be
     *  applied as well.
     *  Aligning with IIM notions IPTC Core intents to have only one creator for this news object
     *  despite the underlying XMP property dc:creator allows for more than one item to be
     *  included. If there are more than one item in this array the first one should be considered as
     *  the IPTC Core Creator value.&rdquo;
     */
    QStringList creator() const;

    QStringList author() const
    {
        return creator();
    }

    QStringList byLine() const
    {
        return creator();
    }

    /** Sets the creator.
     *  If you want to specify only one creator, set the replace mode to ReplaceAllEntries.
     *  If you want to add it to a list of existing entries, pass AddEntryToExisting.
     *  You shall not use ReplaceLanguageEntry for this method, creators have no language associated.
     */
    void setCreator(const QString& creator, ReplaceMode mode = ReplaceAllEntries);

    void setAuthor(const QString& author, ReplaceMode mode = ReplaceAllEntries)
    {
        setCreator(author, mode);
    }

    void setByLine(const QString& byline, ReplaceMode mode = ReplaceAllEntries)
    {
        setCreator(byline, mode);
    }

    void removeCreators();

    /** Returns the credit/provider.
     *  This is Photoshop Credit.
     *  This is IPTC Credit.
     *  This is photoshop:Credit in XMP
     *  &ldquo;Identifies the provider of the news object, who is not necessarily the owner/creator.&rdquo;
     */
    QString provider() const;

    QString credit() const
    {
        return provider();
    }

    void setProvider(const QString& provider);

    void setCredit(const QString& credit)
    {
        setProvider(credit);
    }

    void removeProvider();

    /** Returns the copyright notice.
     *  This is Photoshop Copyright Notice.
     *  This is IPTC Copyright Notice.
     *  This is DC Rights.
     *  This is dc:rights in XMP.
     *  &ldquo; Contains any necessary copyright notice for claiming the intellectual property for this news
     *    object and should identify the current owner of the copyright for the news object. Other
     *    entities like the creator of the news object may be added. Notes on usage rights should be
     *    provided in &lsquo;Rights usage terms&rsquo;. &rdquo;
     *  Note on language matching:
     *  You can specify a language code.
     *  If the requested language is not available, the entry with default
     *  language code is returned.
     *  If a default-language entry is not available, the first entry is returned.
     *  If you pass a null string as languageCode, the local language is returned.
     */
    QString copyrightNotice(const QString& languageCode = QString());

    QString rights(const QString& languageCode = QString())
    {
        return copyrightNotice(languageCode);
    }

    MetaEngine::AltLangMap allCopyrightNotices();

    /** Sets the copyright notice. If you supply a null QString as language code,
     *  this is regarded as an entry for the default language (&ldquo;x-default&rdquo;).
     *  The ReplaceMode determines how existing entries are handled.
     */
    void setCopyrightNotice(const QString& notice, const QString& languageCode = QString(),
                            ReplaceMode mode = ReplaceLanguageEntry);

    void setRights(const QString& notice, const QString& languageCode = QString(),
                   ReplaceMode mode = ReplaceLanguageEntry)
    {
        setCopyrightNotice(notice, languageCode, mode);
    }

    void removeCopyrightNotices();

    /** Returns the right usage terms.
     *  This has no equivalent in Photoshop, IPTC, or DC.
     *  This is xmpRights:UsageTerms in XMP.
     *  Language matching is done as with copyrightNotice().
     *  &ldquo;Free text instructions on how this news object can be legally used.&rdquo;
     */
    QString rightsUsageTerms(const QString& languageCode = QString());
    MetaEngine::AltLangMap allRightsUsageTerms();

    void setRightsUsageTerms(const QString& term, const QString& languageCode = QString(),
                             ReplaceMode mode = ReplaceLanguageEntry);

    void removeRightsUsageTerms();

    /** Returns the source.
     *  This is Photoshop Source.
     *  This is IPTC Source.
     *  This is photoshop::Source in XMP.
     *  &ldquo; Identifies the original owner of the copyright for the intellectual content of the news object.
     *    This could be an agency, a member of an agency or an individual. Source could be
     *    different from Creator and from the entities in the CopyrightNotice.
     *    As the original owner can not change the content of this property should never be changed
     *    or deleted after the information is entered following the news object's initial creation.&rdquo;
     */
    QString source();
    void setSource(const QString& source);

    void removeSource();

    /** Returns the creator's job title.
     *  This is Photoshop AuthorsPosition.
     *  This is IPTC By-line Title.
     *  This is photoshop:AuthorsPosition in XMP.
     *  &ldquo; Contains the job title of the person who created the content of this news object. As this is
     *    sort of a qualifier the Creator element has to be filled in as mandatory prerequisite for
     *    using Creator's Jobtitle.&rdquo;
     */
    QString creatorJobTitle() const;

    QString authorsPosition() const
    {
        return creatorJobTitle();
    }

    QString byLineTitle() const
    {
        return creatorJobTitle();
    }

    void setCreatorJobTitle(const QString& title);

    void setAuthorsPosition(const QString& position)
    {
        setCreatorJobTitle(position);
    }

    void setByLineTitle(const QString& title)
    {
        setCreatorJobTitle(title);
    }

    void removeCreatorJobTitle();

    /** Returns the instructions.
     *  This is Photoshop Instructions.
     *  This is IPTC Special Instruction.
     *  This is photoshop:Instructions in XMP.
     *  &ldquo; Any of a number of instructions from the provider or creator to the receiver of the news
     *    object which might include any of the following: embargoes (NewsMagazines OUT) and
     *    other restrictions not covered by the &lsquo;Rights Usage Terms&rsquo; field; information regarding the
     *    original means of capture (scanning notes, colorspace info) or other specific text
     *    information that the user may need for accurate reproduction; additional permissions or
     *    credits required when publishing. &rdquo;
     */
    QString instructions();
    void setInstructions(const QString& instructions);
    void removeInstructions();

    /** Returns the creator's contact info.
     *  This is Iptc4xmpCore:CreatorContactInfo in XMP.
     *  &ldquo;  The creator's contact information provides all necessary information to get in contact with
     *     the creator of this news object and comprises a set of sub-properties for proper addressing.&rdquo;
     */
    IptcCoreContactInfo contactInfo();
    void setContactInfo(const IptcCoreContactInfo& info);
    void removeContactInfo();

    /** Fills the information fields in template concerning copyright info
     *  (note there are other fields in the a Template. There will not be touched)
     */
    void fillTemplate(Template& t);

    /** Sets all database copyright fields from the template.
     *  This does not clear any fields before. */
    void setFromTemplate(const Template& t);

    /** Calls all remove...() methods in this class */
    void removeAll();

    /** Removes all entries and replaces them with the entries from source. */
    void replaceFrom(const ImageCopyright& source);

protected:

    CopyrightInfo copyrightInfo(const QString& property)                                 const;
    QList<CopyrightInfo> copyrightInfos(const QString& property)                         const;
    QString readSimpleProperty(const QString& property)                                  const;
    int     languageMatch(const QList<CopyrightInfo> infos, const QString& languageCode) const;

    void    setSimpleProperty(const QString& property, const QString& value);
    QString readLanguageProperty(const QString& property, const QString& languageCode);
    MetaEngine::AltLangMap readLanguageProperties(const QString& property);
    void    setLanguageProperty(const QString& property, const QString& value, const QString& languageCode, ReplaceMode mode);
    void    removeProperties(const QString& property);
    void    removeLanguageProperty(const QString& property, const QString& languageCode);

protected:

    friend class ImageCopyrightCache;

    qlonglong            m_id;
    ImageCopyrightCache* m_cache;
};

} // namespace Digikam

#endif // IMAGE_COPYRIGHT_H
