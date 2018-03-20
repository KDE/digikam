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

#ifndef TEMPLATE_H
#define TEMPLATE_H

// Qt includes

#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QDebug>

// Local includes

#include "metadatainfo.h"
#include "digikam_export.h"
#include "metaengine.h"

namespace Digikam
{

class TemplatePrivate;

class DIGIKAM_EXPORT Template
{
public:

    explicit Template();
    ~Template();

    /** Return true is Template title is null
     */
    bool isNull() const;

    /** Return true is Template contents is empty
     */
    bool isEmpty() const;

    /** Compare for metadata equality, not including "templateTitle" value.
     */
    bool operator==(const Template& t) const;

    void setTemplateTitle(const QString& title);
    QString templateTitle() const;

    void setAuthors(const QStringList& authors);
    void setAuthorsPosition(const QString& authorPosition);
    void setCredit(const QString& credit);
    void setCopyright(const MetaEngine::AltLangMap& copyright);
    void setRightUsageTerms(const MetaEngine::AltLangMap& rightUsageTerms);
    void setSource(const QString& source);
    void setInstructions(const QString& instructions);
    void setLocationInfo(const IptcCoreLocationInfo& inf);
    void setContactInfo(const IptcCoreContactInfo& inf);
    void setIptcSubjects(const QStringList& subjects);

    QStringList            authors()         const;
    QString                authorsPosition() const;
    QString                credit()          const;
    MetaEngine::AltLangMap copyright()       const;
    MetaEngine::AltLangMap rightUsageTerms() const;
    QString                source()          const;
    QString                instructions()    const;
    IptcCoreLocationInfo   locationInfo()    const;
    IptcCoreContactInfo    contactInfo()     const;
    QStringList            IptcSubjects()    const;

    static QString removeTemplateTitle()
    {
        return QLatin1String("_REMOVE_TEMPLATE_");
    };

protected:

    /** Template title used internaly. This value always exist and cannot be empty.
     */
    QString                  m_templateTitle;

    /** List of author names.
     */
    QStringList              m_authors;

    /** Description of authors position.
     */
    QString                  m_authorsPosition;

    /** Credit description.
     */
    QString                  m_credit;

    /** Language alternative copyright notices.
     */
    MetaEngine::AltLangMap   m_copyright;

    /** Language alternative right term usages.
     */
    MetaEngine::AltLangMap   m_rightUsageTerms;

    /** Descriptions of contents source.
     */
    QString                  m_source;

    /** Special instructions to process with contents.
     */
    QString                  m_instructions;

    /** IPTC Location Information.
     */
    IptcCoreLocationInfo     m_locationInfo;

    /** IPTC Contact Information.
     */
    IptcCoreContactInfo      m_contactInfo;

    /** IPTC Subjects Information.
     */
    QStringList              m_subjects;
};

//! qDebug() stream operator. Writes property @a t to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const Template& t);

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::Template)

#endif // TEMPLATE_H
