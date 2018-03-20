/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : Red Eyes auto correction settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#ifndef REDEYECORRECTIONSETTINGS_H
#define REDEYECORRECTIONSETTINGS_H

// Local includes

#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"
#include "redeyecorrectionfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT RedEyeCorrectionSettings : public QWidget
{
    Q_OBJECT

public:

    explicit RedEyeCorrectionSettings(QWidget* const parent=0);
    ~RedEyeCorrectionSettings();

    RedEyeCorrectionContainer defaultSettings() const;
    void resetToDefault();

    RedEyeCorrectionContainer settings() const;
    void setSettings(const RedEyeCorrectionContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* REDEYECORRECTIONSETTINGS_H */
