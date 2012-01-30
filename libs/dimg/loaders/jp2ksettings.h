/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save JPEG 2000 image options.
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef JP2KSETTINGS_H
#define JP2KSETTINGS_H

// KDE includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class JP2KSettingsPriv;

class DIGIKAM_EXPORT JP2KSettings : public QWidget
{
    Q_OBJECT

public:

    JP2KSettings(QWidget* parent = 0);
    ~JP2KSettings();

    void setCompressionValue(int val);
    int  getCompressionValue();

    void setLossLessCompression(bool b);
    bool getLossLessCompression();

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotToggleJPEG2000LossLess(bool);

private:

    JP2KSettingsPriv* const d;
};

}  // namespace Digikam

#endif /* JP2KSETTINGS_H */
