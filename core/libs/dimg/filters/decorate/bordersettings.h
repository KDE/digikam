/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-17
 * Description : Border settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BORDERSETTINGS_H
#define BORDERSETTINGS_H

// Local includes

#include <QWidget>
#include <QColor>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"
#include "borderfilter.h"

namespace Digikam
{

class Private;

class DIGIKAM_EXPORT BorderSettings : public QWidget
{
    Q_OBJECT

public:

    explicit BorderSettings(QWidget* parent);
    virtual ~BorderSettings();

    BorderContainer defaultSettings() const;
    void resetToDefault();

    BorderContainer settings() const;
    void setSettings(const BorderContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    static QString getBorderPath(int border);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotBorderTypeChanged(int borderType);
    void slotPreserveAspectRatioToggled(bool);
    void slotColorForegroundChanged(const QColor& color);
    void slotColorBackgroundChanged(const QColor& color);

private:

    void toggleBorderSlider(bool b);

private:

    Private* const d;
};

}  // namespace Digikam

#endif /* BORDERSETTINGS_H */
