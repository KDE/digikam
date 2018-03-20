/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-29
 * Description : metadata template viewer.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TEMPLATEVIEWER_H
#define TEMPLATEVIEWER_H

// Qt includes

#include <QString>

// Local includes

#include "dexpanderbox.h"

namespace Digikam
{

class Template;

class TemplateViewer : public DExpanderBox
{
    Q_OBJECT

public:

    explicit TemplateViewer(QWidget* const parent=0);
    virtual ~TemplateViewer();

    void setTemplate(const Template& t);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // TEMPLATEVIEWER_H
