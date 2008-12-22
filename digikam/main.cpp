/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-07-28
 * Description : main program from digiKam
 *
 * Copyright (C) 2002-2006 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes.

#include <qstring.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qfile.h>

// KDE includes.

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <ktip.h>
#include <kdeversion.h>
#include <kmessagebox.h>

// KIPI includes.

#include <libkipi/version.h>
#include <libkipi/interface.h>

// Local includes.

#include "daboutdata.h"
#include "albumdb.h"
#include "digikamapp.h"
#include "digikamfirstrun.h"

static KCmdLineOptions options[] =
{
    { "detect-camera", I18N_NOOP("Automatically detect and open camera"), 0 },
    { "download-from <path>", I18N_NOOP("Open camera dialog at <path>"), 0 },
    KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    QString libInfo     = Digikam::libraryInfo();

    QString description = Digikam::digiKamDescription();

    KAboutData aboutData( "digikam",
                          I18N_NOOP("digiKam"),
                          digikam_version,
                          description.latin1(),
                          KAboutData::License_GPL,
                          Digikam::copyright(),
                          0,
                          Digikam::webProjectUrl());

    aboutData.setOtherText(libInfo.latin1());

    Digikam::authorsRegistration(aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KConfig* config = KGlobal::config();
    config->setGroup("General Settings");
    QString version = config->readEntry("Version");

    config->setGroup("Album Settings");
    QString albumPath = config->readPathEntry("Album Path");
    QFileInfo dirInfo(albumPath);

    // version 0.6 was the version when the new Albums Library
    // storage was implemented

    if (version.startsWith("0.5") ||
        !dirInfo.exists() ||
        !dirInfo.isDir())
    {
        // Run the first run
        Digikam::DigikamFirstRun *firstRun = new Digikam::DigikamFirstRun(config);
        app.setMainWidget(firstRun);
        firstRun->show();
        return app.exec();
    }

    Digikam::DigikamApp *digikam = new Digikam::DigikamApp();

    app.setMainWidget(digikam);
    digikam->show();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (args && args->isSet("detect-camera"))
        digikam->autoDetect();
    else if (args && args->isSet("download-from"))
        digikam->downloadFrom(args->getOption("download-from"));

#if KDE_IS_VERSION(3,2,0)
    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");

    KGlobal::locale()->insertCatalogue("kipiplugins");
    KGlobal::locale()->insertCatalogue("libkdcraw");

    KTipDialog::showMultiTip(0, tipsFiles, false);
#else
    KTipDialog::showTip(0, "digikam/tips", false);
#endif

    return app.exec();
}
