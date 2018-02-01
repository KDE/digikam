/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : A combobox which also has an intermediate state.
 *
 * Copyright (C) 2009 by Pieter Edelman <pieter dot edelman at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "comboboxintermediate.h"

// Qt includes

#include <QComboBox>
#include <QString>
#include <QVariant>

namespace Digikam
{

WSComboBoxIntermediate::WSComboBoxIntermediate(QWidget* const parent, const QString& text)
    : QComboBox(parent),
      m_isIntermediate(false),
      m_intermediateText(text)
{
    // Whenever the signal changes, there's a chance that the combobox should
    // be changed from intermediate to normal.
    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged(int)));
}

WSComboBoxIntermediate::~WSComboBoxIntermediate()
{
}

void WSComboBoxIntermediate::setIntermediate(bool state)
{
    if ((state) && (!m_isIntermediate))
    {
        // If the combobox should be set to intermediate and is not yet done so,
        // append a separator and the intermediate text.
        insertSeparator(count());
        addItem(m_intermediateText, QVariant(-1));

        // Set the combobox to the intermediate index, while avoiding that it is
        // directly unset by the currentIndexChanged signal.
        blockSignals(true);
        setCurrentIndex(count() - 1);
        blockSignals(false);

        m_isIntermediate = true;
    }
    else if ((!state) && (m_isIntermediate))
    {
        // If the intermediate state should be removed, simply remove the latest
        // two items, the intermediate text and the separator.
        removeItem(count() - 1);
        removeItem(count() - 1);
        m_isIntermediate = false;
    }
}

void WSComboBoxIntermediate::slotIndexChanged(int)
{
    if (m_isIntermediate)
    {
        setIntermediate(false);
    }
}

} // namespace Digikam
