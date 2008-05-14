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

#include <QString>

// KDE includes.

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "mainwindow.h"

int main(int argc, char** argv)
{
    KAboutData aboutData("digikamthemedesigner", 0,
                         ki18n("digiKam Theme Designer"),
                         digiKamVersion().toAscii(),
                         ki18n("A Color Theme Designer for digiKam"),
                         KAboutData::License_GPL,
                         ki18n("(c) 2002-2008, digiKam developers team"),
                         KLocalizedString(),
                         "http://www.digikam.org");

    aboutData.addAuthor(ki18n("Caulier Gilles"),
                        ki18n("Main developer and coordinator"),
                        "caulier dot gilles at gmail dot com",
                        "http://www.digikam.org/?q=blog/3");

    aboutData.addAuthor(ki18n("Marcel Wiesweg"),
                        ki18n("Developer"),
                        "marcel dot wiesweg at gmx dot de",
                        "http://www.digikam.org/?q=blog/8");

    aboutData.addAuthor(ki18n("Renchi Raju"),
                        ki18n("Developer"),
                        "renchi at pooh dot tam dot uiuc dot edu");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[URL]", ki18n("Theme color scheme file to open."));
    KCmdLineArgs::addCmdLineOptions(options);

    KGlobal::locale()->setMainCatalog( "digikam" );

    KApplication app;
    Digikam::MainWindow *im = new Digikam::MainWindow();
    app.setTopWidget(im);
    im->resize(800, 600);
    im->show();

    app.exec();
}
