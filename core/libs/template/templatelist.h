/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : template list view.
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

#ifndef TEMPLATELIST_H
#define TEMPLATELIST_H

// Qt includes

#include <QTreeWidget>

// Local includes

#include "template.h"

namespace Digikam
{

class TemplateListItem : public QTreeWidgetItem
{

public:

    TemplateListItem(QTreeWidget* const parent, const Template& t);
    ~TemplateListItem();

    void     setTemplate(const Template& t);
    Template getTemplate() const;

private:

    Template m_template;
};

// -------------------------------------------------------------------

class TemplateList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit TemplateList(QWidget* const parent=0);
    ~TemplateList();

    TemplateListItem* find(const QString& title);

    void readSettings();
    void applySettings();
};

}  // namespace Digikam

#endif // TEMPLATELIST_H
