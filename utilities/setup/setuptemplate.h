/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-04
 * Description : metadata template setup page.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUP_TEMPLATE_H
#define SETUP_TEMPLATE_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "template.h"

namespace Digikam
{

class SetupTemplate : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupTemplate(QWidget* const parent = 0);
    ~SetupTemplate();

    void applySettings();
    void setTemplate(const Template& t);

private:

    void readSettings();
    void populateTemplate(const Template& t);

private Q_SLOTS:

    void slotSelectionChanged();
    void slotAddTemplate();
    void slotDelTemplate();
    void slotRepTemplate();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SETUP_TEMPLATE_H
