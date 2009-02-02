/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : dialog displayed at the first digiKam run
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier  <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAMFIRSTRUN_H
#define DIGIKAMFIRSTRUN_H

// KDE includes.

#include <kdialog.h>

// Local includes.

#include "digikam_export.h"

class KUrlRequester;

namespace Digikam
{

class DigikamFirstRunPriv;

class DigikamFirstRun : public KDialog
{
    Q_OBJECT

public:

    DigikamFirstRun(QWidget* parent=0);
    ~DigikamFirstRun();

protected slots:

    void slotButtonClicked(int button);
    void slotAlbumRootChanged(const KUrl &url);
    void slotDbPathChanged(const KUrl &url);

private:

    void saveSettings(const QString& rootAlbumFolder, const QString& dbFolder);
    bool checkRootAlbum(QString& rootAlbumFolder);
    bool checkDatabase(QString& dbFolder);

private:

    DigikamFirstRunPriv* const d;
};

}  // namespace Digikam

#endif // DIGIKAMFIRSTRUN_H
