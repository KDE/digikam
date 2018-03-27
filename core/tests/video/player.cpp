/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a stand alone tool to play a video file.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QApplication>
#include <QUrl>
#include <QDebug>

// Local includes

#include "vidplayerdlg.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    if (argc == 1)
    {
        qDebug() << "player - video file to play";
        qDebug() << "Usage: video files";
        return -1;
    }

    VidPlayerDlg player(QString::fromUtf8(argv[1]));
    player.show();
    player.resize(800, 600);

    return a.exec();
}
