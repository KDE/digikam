/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save TIFF image options.
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TIFFSETTINGS_H
#define TIFFSETTINGS_H

// KDE includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class TIFFSettingsPriv;

class DIGIKAM_EXPORT TIFFSettings : public QWidget
{
    Q_OBJECT

public:

    TIFFSettings(QWidget* parent = 0);
    ~TIFFSettings();

    void setCompression(bool b);
    bool getCompression();

Q_SIGNALS:

    void signalSettingsChanged();

private:

    TIFFSettingsPriv* const d;
};

}  // namespace Digikam

#endif /* TIFFSETTINGS_H */
