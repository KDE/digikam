/* ============================================================
 * File  : exifproposplugin.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-13
 * Description : an image exif viewer dialog.
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef EXIFPROPSPLUGIN_H
#define EXIFPROPSPLUGIN_H

// Qt includes.

// KDE includes.

#include <kpropertiesdialog.h>

class KExifData;

class ExifPropsPlugin : public KPropsDlgPlugin
{
   Q_OBJECT

public:

   ExifPropsPlugin( KPropertiesDialog *propsDlg, QString imageFile);
   ~ExifPropsPlugin();

public slots:    
    
        
private:
    
    KExifData   *mExifData;

    void setupGui(KPropertiesDialog *dialog);

private slots:


};


#endif /* EXIFPROPSPLUGIN_H */
