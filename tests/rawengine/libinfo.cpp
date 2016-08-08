/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2013-09-07
 * @brief  a command line tool to show RawEngine info
 *
 * @author Copyright (C) 2013-2016 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QDebug>

// Local includes

#include "drawdecoder.h"

using namespace RawEngine;

int main(int /*argc*/, char** /*argv*/)
{
    qDebug() << "Libraw version    : " << DRawDecoder::librawVersion();
    qDebug() << "Use OpenMP        : " << DRawDecoder::librawUseGomp();
    qDebug() << "Raw files list    : " << DRawDecoder::rawFilesList();
    qDebug() << "Raw files version : " << DRawDecoder::rawFilesVersion();
    qDebug() << "Supported camera  : " << DRawDecoder::supportedCamera();

    return 0;
}
