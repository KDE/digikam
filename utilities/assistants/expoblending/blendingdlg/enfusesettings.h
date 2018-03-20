/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
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

#ifndef ENFUSESETTINGS_H
#define ENFUSESETTINGS_H

// Qt includes

#include <QUrl>
#include <QWidget>

// Locale includes

#include "dsavesettingswidget.h"

class KConfigGroup;

namespace Digikam
{

class EnfuseSettings
{
public:

    EnfuseSettings()
        : autoLevels(true),
          hardMask(false),
          ciecam02(false),
          levels(20),
          exposure(1.0),
          saturation(0.2),
          contrast(0.0),
          outputFormat(DSaveSettingsWidget::OUTPUT_PNG)
    {
    }

    ~EnfuseSettings()
    {
    }

    QString asCommentString() const;

    QString inputImagesList() const;

public:

    bool                              autoLevels;
    bool                              hardMask;
    bool                              ciecam02;

    int                               levels;

    double                            exposure;
    double                            saturation;
    double                            contrast;

    QString                           targetFileName;

    QList<QUrl>                       inputUrls;
    QUrl                              previewUrl;

    DSaveSettingsWidget::OutputFormat outputFormat;
};

// ------------------------------------------------------------------------


class EnfuseSettingsWidget : public QWidget
{
    Q_OBJECT

public:

    EnfuseSettingsWidget(QWidget* const parent);
    ~EnfuseSettingsWidget();

    void           setSettings(const EnfuseSettings& settings);
    EnfuseSettings settings() const;

    void           resetToDefault();

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ENFUSESETTINGS_H
