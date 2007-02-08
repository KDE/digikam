/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-08-02
 * Description : a stack of widgets to set image file save 
 *               options into image editor.
 *
 * Copyright 2007 by Gilles Caulier
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

#ifndef FILESAVEOPTIONSBOX_H
#define FILESAVEOPTIONSBOX_H

// KDE includes.

#include <qwidgetstack.h>
#include <qstring.h>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class FileSaveOptionsBoxPriv;

class DIGIKAM_EXPORT FileSaveOptionsBox : public QWidgetStack
{
Q_OBJECT

public:

    FileSaveOptionsBox(QWidget *parent=0);
    ~FileSaveOptionsBox();

    void applySettings();

public slots:

    void slotImageFileFormatChanged(const QString&);
    void slotImageFileSelected(const QString&);

private:

    void toggleFormatOptions(const QString& format);
    void readSettings();

private:

    FileSaveOptionsBoxPriv* d;
};

}  // namespace Digikam

#endif /* FILESAVEOPTIONSBOX_H */
