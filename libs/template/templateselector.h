/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-23
 * Description : a widget to select metadata template.
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

#ifndef TEMPLATESELECTOR_H
#define TEMPLATESELECTOR_H

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"

namespace Digikam
{

class Template;

class DIGIKAM_EXPORT TemplateSelector : public DHBox
{
    Q_OBJECT

public:

    enum SelectorItems
    {
        REMOVETEMPLATE = 0,
        DONTCHANGE     = 1
    };

public:

    explicit TemplateSelector(QWidget* const parent=0);
    virtual ~TemplateSelector();

    Template  getTemplate() const;
    void      setTemplate(const Template& t);

    int  getTemplateIndex() const;
    void setTemplateIndex(int i);

Q_SIGNALS:

    void signalTemplateSelected();

private Q_SLOTS:

    void slotOpenSetup();
    void slotTemplateListChanged();

private:

    void populateTemplates();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // TEMPLATESELECTOR_H
