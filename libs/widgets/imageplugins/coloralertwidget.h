/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-11-01
 * Description : a settings widget to handle pure white and 
 *               black color alert.
 * 
 * Copyright 2007 Gilles Caulier
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

#ifndef COLORALERTWIDGET_H
#define COLORALERTWIDGET_H

// Qt includes.

#include <qcolor.h>
#include <qhbox.h>
#include <qstring.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ColorAlertWidgetPriv;

class DIGIKAM_EXPORT ColorAlertWidget : public QHBox
{
Q_OBJECT

public:

    ColorAlertWidget(const QString& settingsSection, QWidget *parent=0);
    ~ColorAlertWidget();

    bool   whiteAlertIsChecked();
    bool   blackAlertIsChecked();

    QColor whiteAlertColor();
    QColor blackAlertColor();
           
signals:

    void signalWhiteAlertToggled(bool);
    void signalBlackAlertToggled(bool);
 
    void signalWhiteAlertColorChanged(const QColor&); 
    void signalBlackAlertColorChanged(const QColor&); 

private:
        
    void readSettings(void);
    void writeSettings(void);

private:

    ColorAlertWidgetPriv* d;        
};

}  // NameSpace Digikam

#endif /* COLORALERTWIDGET_H */
