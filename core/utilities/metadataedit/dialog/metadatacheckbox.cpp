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

#include "metadatacheckbox.h"

namespace Digikam
{

MetadataCheckBox::MetadataCheckBox(const QString& text, QWidget* const parent)
    : QCheckBox(text, parent)
{
    m_valid = true;

    connect(this, SIGNAL(toggled(bool)),
            this, SLOT(slotValid()));
}

MetadataCheckBox::~MetadataCheckBox()
{
}

void MetadataCheckBox::setValid(bool v)
{
    m_valid = v;
}

bool MetadataCheckBox::isValid() const
{
    return m_valid;
}

void MetadataCheckBox::slotValid()
{
    setValid(true);
}

}  // namespace Digikam
