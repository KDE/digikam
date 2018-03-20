/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : a command line tool to encode images as a
 *               video stream.
 *
 * Copyright (C) 2017 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

#include <QDebug>
#include <QApplication>

// Local includes

#include "metaengine.h"
#include "vidslidethread.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // ---------------------------------------------
    // Get list of image files from CLI

    VidSlideSettings settings;
    settings.transition = TransitionMngr::HorizontalLines;

    if (argc > 1)
    {
        for (int i = 1 ; i < argc ; i++)
        {
            settings.inputImages << QUrl::fromLocalFile(QString::fromUtf8(argv[i]));
        }
    }
    else
    {
        qDebug() << "framesencoder - images to encode as video stream";
        qDebug() << "Usage: <list of image files>";
        return -1;
    }

    // ---------------------------------------------
    // Common settings

    MetaEngine::initializeExiv2();

    VidSlideThread* const encoder = new VidSlideThread(&app);
    encoder->processStream(&settings);
    encoder->start();

    QObject::connect(encoder, SIGNAL(signalDone(bool)),
                     &app, SLOT(quit()));

    app.exec();

    MetaEngine::cleanupExiv2();

    return 0;
}
