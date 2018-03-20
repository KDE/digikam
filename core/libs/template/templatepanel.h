/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-06
 * Description : metadata template settings panel.
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

#ifndef TEMPLATE_PANEL_H
#define TEMPLATE_PANEL_H

// Qt includes

#include <QTabWidget>

// Local includes

#include "template.h"

namespace Digikam
{

class TemplatePanel : public QTabWidget
{
public:

    enum TemplateTab
    {
        RIGHTS=0,
        LOCATION,
        CONTACT,
        SUBJECTS
    };

public:

    explicit TemplatePanel(QWidget* const parent = 0);
    ~TemplatePanel();

    void     setTemplate(const Template& t);
    Template getTemplate() const;

    void     apply();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // TEMPLATE_PANEL_H
