/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : a checkbox with a boolean valid parameter.
 *               The boolean statement is used to check if
 *               a metadata value from a picture have a know
 *               value registered by EXIF/IPTC spec.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATA_CHECKBOX_H
#define METADATA_CHECKBOX_H

// Qt includes

#include <QCheckBox>

namespace Digikam
{

class MetadataCheckBox : public QCheckBox
{
    Q_OBJECT

public:

    MetadataCheckBox(const QString& text, QWidget* const parent);
    ~MetadataCheckBox();

    void setValid(bool v);
    bool isValid() const;

private Q_SLOTS:

    void slotValid();

private:

    bool m_valid;
};

}  // namespace Digikam

#endif // METADATA_CHECKBOX_H
