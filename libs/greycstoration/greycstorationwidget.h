/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-09-13
 * Description : Greycstoration settings widgets
 *
 * Copyright 2007 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GREYCSTORATION_WIDGET_H
#define GREYCSTORATION_WIDGET_H

// Qt includes.

#include <qobject.h>
#include <qfile.h>
#include <qstring.h>

// Local Includes.

#include "digikam_export.h"
#include "greycstorationsettings.h"

class QTabWidget;

namespace Digikam
{

class GreycstorationWidgetPriv;

class DIGIKAM_EXPORT GreycstorationWidget : public QObject
{
    Q_OBJECT

public:

    GreycstorationWidget(QTabWidget *parent);
    ~GreycstorationWidget();

    void setSettings(GreycstorationSettings settings);
    GreycstorationSettings getSettings();

    bool loadSettings(QFile& file, const QString& header);
    void saveSettings(QFile& file, const QString& header);

    void setEnabled(bool);

private:

    GreycstorationWidgetPriv* d;
};

} // NameSpace Digikam

#endif /* GREYCSTORATION_WIDGET_H */
