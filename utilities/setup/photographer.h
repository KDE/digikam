/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : Photographer information container.
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

#ifndef PHOTOGRAPHER_H
#define PHOTOGRAPHER_H

// Qt includes

#include <QString>

namespace Digikam
{

class PhotographerPrivate;

class Photographer
{
public:

    Photographer();
    ~Photographer();

    Photographer(const Photographer& photographer);
    Photographer& operator=(const Photographer& photographer);

    void setAuthor(const QString& author);
    void setAuthorPosition(const QString& authorPosition);
    void setCredit(const QString& credit);
    void setCopyright(const QString& copyright);
    void setRightUsageTerms(const QString& rightUsageTerms);
    void setSource(const QString& source);
    void setInstructions(const QString& instructions);
    void setValid(bool valid);

    QString author()          const;
    QString authorPosition()  const;
    QString credit()          const;
    QString copyright()       const;
    QString rightUsageTerms() const;
    QString source()          const;
    QString instructions()    const;
    bool    valid()           const;

private:

    PhotographerPrivate* const d;
};

}  // namespace Digikam

#endif /* PHOTOGRAPHER_H */
