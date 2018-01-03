/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : JPEG image Converter batch tool.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CONVERT2JPEG_H
#define CONVERT2JPEG_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class JPEGSettings;

class Convert2JPEG : public BatchTool
{
    Q_OBJECT

public:

    explicit Convert2JPEG(QObject* const parent = 0);
    ~Convert2JPEG();

    QString outputSuffix() const;
    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new Convert2JPEG(parent); };

    void registerSettingsWidget();

private Q_SLOTS:

    void slotSettingsChanged();
    void slotAssignSettings2Widget();

private:

    bool toolOperations();

private:

    JPEGSettings* m_settings;
    bool          m_changeSettings;
};

}  // namespace Digikam

#endif /* CONVERT2JPEG_H */
