/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-11
 * Description : a widget to customize album name created by
 *               camera interface.
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMCUSTOMIZER_H
#define ALBUMCUSTOMIZER_H

// Qt includes

#include <QWidget>

// KDE includes

#include <kconfig.h>

namespace Digikam
{

class AlbumCustomizer : public QWidget
{
    Q_OBJECT

public:

    enum DateFormatOptions
    {
        IsoDateFormat=0,
        TextDateFormat,
        LocalDateFormat
    };

public:

    AlbumCustomizer(QWidget* parent=0);
    ~AlbumCustomizer();

    void readSettings(KConfigGroup& group);
    void saveSettings(KConfigGroup& group);

    bool autoAlbumDateEnabled() const;
    bool autoAlbumExtEnabled() const;
    int  folderDateFormat() const;

private:

    class AlbumCustomizerPriv;
    AlbumCustomizerPriv* const d;
};

}  // namespace Digikam

#endif /* ALBUMCUSTOMIZER_H */
