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

#ifndef TEMPLATE_H
#define TEMPLATE_H

// Qt includes

#include <QMetaType>
#include <QString>
#include <QStringList>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "digikam_export.h"

using namespace KExiv2Iface;

namespace Digikam
{

class TemplatePrivate;

class DIGIKAM_EXPORT Template
{
public:

    Template();
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

    void print() const;

    void setAuthors(const QStringList& authors);
    void setAuthorsPosition(const QString& authorPosition);
    void setCredit(const QString& credit);
    void setCopyright(const KExiv2::AltLangMap& copyright);
    void setRightUsageTerms(const KExiv2::AltLangMap& rightUsageTerms);
    void setSource(const QString& source);
    void setInstructions(const QString& instructions);

    QStringList        authors()         const;
    QString            authorsPosition() const;
    QString            credit()          const;
    KExiv2::AltLangMap copyright()       const;
    KExiv2::AltLangMap rightUsageTerms() const;
    QString            source()          const;
    QString            instructions()    const;

    static QString     removeTemplateTitle() { return QString("_REMOVE_TEMPLATE_"); };

protected:

    /** Template title used internaly. This value always exist and cannot be empty.
     */
    QString            m_templateTitle;

    /** List of author names.
     */
    QStringList        m_authors;

    /** Description of authors position.
     */
    QString            m_authorsPosition;

    /** Credit description.
     */
    QString            m_credit;

    /** Language alternative copyright notices.
     */
    KExiv2::AltLangMap m_copyright;

    /** Language alternative right term usages.
     */
    KExiv2::AltLangMap m_rightUsageTerms;

    /** Descriptions of contents source.
     */
    QString            m_source;

    /** Special instructions to process with contents.
     */
    QString            m_instructions;
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::Template)

#endif /* TEMPLATE_H */
