/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-31-01
 * Description : a tool to print images
 *
 * Copyright (C) 2003      by Todd Shoemaker <todd at theshoemakers dot net>
 * Copyright (C) 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ADV_PRINT_UTILS_H
#define ADV_PRINT_UTILS_H

// Qt includes

#include <QString>

class QWidget;
class QStringList;

namespace Digikam
{

int  AdvPrintNint(double n);
bool AdvPrintLaunchExternalApp(const QString& program, const QStringList& args);
bool AdvPrintCheckTempPath(QWidget* const parent, const QString& tempPath);

}  // Namespace Digikam

#endif // ADV_PRINT_UTILS_H
