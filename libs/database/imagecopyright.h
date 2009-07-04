/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-12
 * Description : Access to copyright info of an image in the database
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGECOPYRIGHT_H
#define IMAGECOPYRIGHT_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QList>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "template.h"
#include "digikam_export.h"

namespace Digikam
{

class CopyrightInfo;

class DIGIKAM_DATABASE_EXPORT ImageCopyright
{

public:

    ImageCopyright(qlonglong imageid);

    /** Create a null ImageCopyright object */
    ImageCopyright();

    enum ReplaceMode
    {
        /// Remove entries for all languages and add one new entry
        ReplaceAllEntries,
        /// Only replace the entry with the given language
        ReplaceLanguageEntry,
        /// No constraints on adding the entry
        AddEntryToExisting
    };

    /** Returns the author/creator/byline.
     *  This is Photoshop Author.
     *  This is IPTC By-line.
     *  This is DC creator.
     *  This is dc:creator in XMP.
     *  "Contains preferably the name of the person who created the content of this news object, a
     *  photographer for photos, a graphic artist for graphics, or a writer for textual news. If it is not
     *  appropriate to add the name of a person the name of a company or organization could be
     *  applied as well.
     *  Aligning with IIM notions IPTC Core intents to have only one creator for this news object
     *  despite the underlying XMP property dc:creator allows for more than one item to be
     *  included. If there are more than one item in this array the first one should be considered as
     *  the IPTC Core Creator value."
     */
     QStringList creator();
     QStringList author() { return creator(); }
     QStringList byLine() { return creator(); }

    /** Sets the creator.
     *  If you want to specify only one creator, set the replace mode to ReplaceAllEntries.
     *  If you want to add it to a list of existing entries, pass AddEntryToExisting.
     *  You shall not use ReplaceLanguageEntry for this method, creators have no language associated.
     */
    void setCreator(const QString& creator, ReplaceMode mode = ReplaceAllEntries);
    void setAuthor(const QString& author, ReplaceMode mode = ReplaceAllEntries) { setCreator(author, mode); }
    void setByLine(const QString& byline, ReplaceMode mode = ReplaceAllEntries) { setCreator(byline, mode); }

    void removeCreators();

    /** Returns the credit/provider.
     *  This is Photoshop Credit.
     *  This is IPTC Credit.
     *  This is photoshop:Credit in XMP
     *  "Identifies the provider of the news object, who is not necessarily the owner/creator."
     */
    QString provider();
    QString credit() { return provider(); }

    void setProvider(const QString& provider);
    void setCredit(const QString& credit) { setProvider(credit); }

    void removeProvider();

    /** Returns the copyright notice.
     *  This is Photoshop Copyright Notice.
     *  This is IPTC Copyright Notice.
     *  This is DC Rights.
     *  This is dc:rights in XMP.
     *  " Contains any necessary copyright notice for claiming the intellectual property for this news
     *    object and should identify the current owner of the copyright for the news object. Other
     *    entities like the creator of the news object may be added. Notes on usage rights should be
     *    provided in "Rights usage terms". "
     *  Note on language matching:
     *  You can specify a language code.
     *  If the requested language is not available, the entry with default
     *  language code is returned.
     *  If a default-language entry is not available, the first entry is returned.
     *  If you pass a null string as languageCode, the local language is returned.
     */
    QString copyrightNotice(const QString& languageCode = QString());
    QString rights(const QString& languageCode = QString()) { return copyrightNotice(languageCode); }

    KExiv2Iface::KExiv2::AltLangMap allCopyrightNotices();

    /** Sets the copyright notice. If you supply a null QString as language code,
     *  this is regarded as an entry for the default language ("x-default").
     *  The ReplaceMode determines how existing entries are handled.
     */
    void setCopyrightNotice(const QString& notice, const QString& languageCode = QString(),
                            ReplaceMode mode = ReplaceLanguageEntry);
    void setRights(const QString& notice, const QString& languageCode = QString(),
                   ReplaceMode mode = ReplaceLanguageEntry)
        { setCopyrightNotice(notice, languageCode, mode); }

    void removeCopyrightNotices();

    /** Returns the right usage terms.
     *  This has no equivalent in Photoshop, IPTC, or DC.
     *  This is xmpRights:UsageTerms in XMP.
     *  Language matching is done as with copyrightNotice().
     *  "Free text instructions on how this news object can be legally used."
     */
    QString rightsUsageTerms(const QString& languageCode = QString());
    KExiv2Iface::KExiv2::AltLangMap allRightsUsageTerms();

    void setRightsUsageTerms(const QString& term, const QString& languageCode = QString(),
                             ReplaceMode mode = ReplaceLanguageEntry);

    void removeRightsUsageTerms();

    /** Returns the source.
     *  This is Photoshop Source.
     *  This is IPTC Source.
     *  This is photoshop::Source in XMP.
     *  " Identifies the original owner of the copyright for the intellectual content of the news object.
     *    This could be an agency, a member of an agency or an individual. Source could be
     *    different from Creator and from the entities in the CopyrightNotice.
     *    As the original owner can not change the content of this property should never be changed
     *    or deleted after the information is entered following the news object's initial creation."
     */
    QString source();
    void setSource(const QString& source);

    void removeSource();

    /** Returns the creator's job title.
     *  This is Photoshop AuthorsPosition.
     *  This is IPTC By-line Title.
     *  This is photoshop:AuthorsPosition in XMP.
     *  " Contains the job title of the person who created the content of this news object. As this is
     *    sort of a qualifier the Creator element has to be filled in as mandatory prerequisite for
     *    using Creator's Jobtitle."
     */
    QString creatorJobTitle();
    QString authorsPosition() { return creatorJobTitle(); }
    QString byLineTitle()     { return creatorJobTitle(); }

    void setCreatorJobTitle(const QString& title);
    void setAuthorsPosition(const QString& position) { setCreatorJobTitle(position); }
    void setByLineTitle(const QString& title)        { setCreatorJobTitle(title); }

    void removeCreatorJobTitle();

    /** Returns the instructions.
     *  This is Photoshop Instructions.
     *  This is IPTC Special Instruction.
     *  This is photoshop:Instructions in XMP.
     *  " Any of a number of instructions from the provider or creator to the receiver of the news
     *    object which might include any of the following: embargoes (NewsMagazines OUT) and
     *    other restrictions not covered by the "Rights Usage Terms" field; information regarding the
     *    original means of capture (scanning notes, colorspace info) or other specific text
     *    information that the user may need for accurate reproduction; additional permissions or
     *    credits required when publishing. "
     */
    QString instructions();
    void setInstructions(const QString& instructions);
    void removeInstructions();

    IptcCoreContactInfo contactInfo();
    void setContactInfo(const IptcCoreContactInfo &info);
    void removeContactInfo();

    /**
     * Returns all entries of the given type in a Template container.
     */
    Template toMetadataTemplate();

protected:

    QString readSimpleProperty(const QString& property);
    void    setSimpleProperty(const QString& property, const QString& value);
    QString readLanguageProperty(const QString& property, const QString& languageCode);
    KExiv2Iface::KExiv2::AltLangMap readLanguageProperties(const QString& property);
    void    setLanguageProperty(const QString& property, const QString& value, const QString& languageCode, ReplaceMode mode);
    void    removeProperties(const QString &property);
    void    removeLanguageProperty(const QString &property, const QString &languageCode);
    int     languageMatch(const QList<CopyrightInfo> infos, const QString& languageCode) const;

protected:

    qlonglong m_id;
};

} // namespace Digikam

#endif // IMAGECOPYRIGHT_H
