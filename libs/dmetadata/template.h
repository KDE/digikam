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

#include <QString>

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

    Template(const Template& t);
    Template& operator=(const Template& t);

    /** Compare for equality */
    bool operator==(const Template& t) const;

    void setAuthor(const QString& author);
    void setAuthorPosition(const QString& authorPosition);
    void setCredit(const QString& credit);
    void setCopyright(const QString& copyright);
    void setRightUsageTerms(const QString& rightUsageTerms);
    void setSource(const QString& source);
    void setInstructions(const QString& instructions);

    QString author()          const;
    QString authorPosition()  const;
    QString credit()          const;
    QString copyright()       const;
    QString rightUsageTerms() const;
    QString source()          const;
    QString instructions()    const;

private:

    TemplatePrivate* const d;
};

}  // namespace Digikam

#endif /* TEMPLATE_H */
