/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-28
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#ifndef CAMERAINFODIALOG_H
#define CAMERAINFODIALOG_H

#include <kdialogbase.h>

class QString;

namespace Digikam
{

class CameraInfoDialog : public KDialogBase
{
public:

    CameraInfoDialog(const QString& summary,
                     const QString& manual,
                     const QString& about);
    ~CameraInfoDialog();
};

} // namespace Digikam

#endif /* CAMERAINFODIALOG_H */
