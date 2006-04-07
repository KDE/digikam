/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-19
 * Description : Gphoto2 interface
 * 
 * Copyright 2003 by Renchi Raju
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

#ifndef GPIFACE_H
#define GPIFACE_H

class QString;
class QStringList;

namespace Digikam
{

class GPIface
{
public:

    static int autoDetect(QString& model, QString& port);
    static void getSupportedCameras(int& count, QStringList& clist);
    static void getSupportedPorts(QStringList& plist);
    static void getCameraSupportedPorts(const QString& model, QStringList& plist);
};

}  // namespace Digikam

#endif /* GPIFACE_H */
