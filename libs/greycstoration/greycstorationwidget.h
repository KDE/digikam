/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-13
 * Description : Greycstoration settings widgets
 *
 * Copyright  (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QString>

// Local includes

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
    void setDefaultSettings(GreycstorationSettings settings);
    GreycstorationSettings getSettings();

    bool loadSettings(QFile& file, const QString& header);
    void saveSettings(QFile& file, const QString& header);

    void setEnabled(bool);

private:

    GreycstorationWidgetPriv* const d;
};

} // namespace Digikam

#endif /* GREYCSTORATION_WIDGET_H */
