/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GALLERY_THEME_H
#define GALLERY_THEME_H

// Qt includes

#include <QString>
#include <QList>
#include <QSharedPointer>

namespace Digikam
{

class AbstractThemeParameter;

/**
 * An HTML theme. This class gives access to the various theme properties,
 * including the theme parameters.
 */
class GalleryTheme
{
public:

    typedef QSharedPointer<GalleryTheme>   Ptr;
    typedef QList<Ptr>                     List;
    typedef QList<AbstractThemeParameter*> ParameterList;

public:

    ~GalleryTheme();

    // Internal theme name == name of theme folder
    QString internalName()          const;
    QString name()                  const;
    QString comment()               const;

    QString authorName()            const;
    QString authorUrl()             const;

    QString previewName()           const;
    QString previewUrl()            const;
    bool allowNonsquareThumbnails() const;

    /**
     * Theme directory on hard disk
     */
    QString directory()             const;

    ParameterList parameterList()   const;

    /**
     * Returns the list of available themes
     */
    static const List& getList();
    static Ptr findByInternalName(const QString& internalName);

private:

    GalleryTheme();

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GALLERY_THEME_H
