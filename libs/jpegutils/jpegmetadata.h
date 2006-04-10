/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-06-13
 * Description : JPEG files metadata parser.
 * 
 * Copyright 2005 by Renchi Raju
 * Copyright 2006 by Caulier Gilles
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

#ifndef JPEGREADER_H
#define JPEGREADER_H

#include <qstring.h>
#include <qdatetime.h>

namespace Digikam
{
void readJPEGMetaData(const QString& filePath, QString& comments, 
                      QDateTime& datetime, int& rating);
}

#endif /* JPEGREADER_H */
