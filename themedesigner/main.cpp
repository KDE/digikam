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

// Qt includes

#include <QString>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>

// Local includes

#include "daboutdata.h"
#include "mainwindow.h"
#include "version.h"

int main(int argc, char** argv)
{
    KAboutData aboutData("digikamthemedesigner", "digikam",
                         ki18n("digiKam Theme Designer"),
                         Digikam::digiKamVersion().toAscii(),
                         Digikam::digiKamSlogan(),
                         KAboutData::License_GPL,
                         Digikam::copyright(),
                         KLocalizedString(),
                         Digikam::webProjectUrl().url().toUtf8());

    Digikam::authorsRegistration(aboutData);

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
