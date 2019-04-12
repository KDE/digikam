/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save JPEG image options.
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_JPEG_SETTINGS_H
#define DIGIKAM_JPEG_SETTINGS_H

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT JPEGSettings : public QWidget
{
    Q_OBJECT

public:

    explicit JPEGSettings(QWidget* const parent = nullptr);
    ~JPEGSettings();

    void setCompressionValue(int val);
    int  getCompressionValue() const;

    void setSubSamplingValue(int val);
    int  getSubSamplingValue() const;

    static int convertCompressionForLibJpeg(int value);

Q_SIGNALS:

    void signalSettingsChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_JPEG_SETTINGS_H
