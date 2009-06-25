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

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class TemplatePrivate;

class DIGIKAM_EXPORT Template
{
public:

    Template();
    ~Template();

    bool isNull() const;

    /** Compare for metadata equality, not including "templateTitle"
     */
    bool operator==(const Template& t) const;

    void setTemplateTitle(const QString& title);
    QString templateTitle() const;

    void setAuthors(const QStringList& authors);
    void setAuthorsPosition(const QString& authorPosition);
    void setCredit(const QString& credit);
    void setCopyright(const QString& copyright);
    void setRightUsageTerms(const QString& rightUsageTerms);
    void setSource(const QString& source);
    void setInstructions(const QString& instructions);

    QStringList authors()         const;
    QString     authorsPosition() const;
    QString     credit()          const;
    QString     copyright()       const;
    QString     rightUsageTerms() const;
    QString     source()          const;
    QString     instructions()    const;

protected:

    // template title used internaly
    QString     m_templateTitle;

    // Metadata strings recorded to DB and XMP
    QStringList m_authors;
    QString     m_authorsPosition;
    QString     m_credit;
    QString     m_copyright;
    QString     m_rightUsageTerms;
    QString     m_source;
    QString     m_instructions;
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::Template)

#endif /* TEMPLATE_H */
