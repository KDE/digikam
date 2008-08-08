/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : main program from digiKam theme designer
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>

// Local includes.

#include "version.h"
#include "mainwindow.h"

static const char *description = I18N_NOOP("A Color Theme Designer for digiKam");

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char** argv)
{
    KAboutData aboutData("digikamthemedesigner",
                     I18N_NOOP("digiKam Theme Designer"),
                     digikam_version,
                     description,
                     KAboutData::License_GPL,
                     I18N_NOOP("(c) 2002-2008, digiKam developers team"),
                     0,
                     "http://www.digikam.org");

    aboutData.addAuthor ( "Caulier Gilles",
                          I18N_NOOP("Main developer and coordinator"),
                          "caulier dot gilles at gmail dot com",
                          "http://www.digikam.org/?q=blog/3");

    aboutData.addAuthor ( "Marcel Wiesweg",
                          I18N_NOOP("Developer"),
                          "marcel dot wiesweg at gmx dot de",
                          "http://www.digikam.org/?q=blog/8");

    aboutData.addAuthor ( "Renchi Raju",
                          I18N_NOOP("Developer"),
                          "renchi at pooh dot tam dot uiuc dot edu",
                          0);

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);

    KGlobal::locale()->setMainCatalogue( "digikam" );

    KApplication app;
    Digikam::MainWindow *im = new Digikam::MainWindow();
    app.setMainWidget(im);
    im->resize(800, 600);
    im->show();

    return app.exec();
}
