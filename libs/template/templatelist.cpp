/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : identity list view.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "templatelist.moc"

// Qt includes

#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

// KDE includes

#include <klocale.h>

// Local includes

#include "templatemanager.h"
#include "template.h"

namespace Digikam
{

TemplateListItem::TemplateListItem(QTreeWidget* const parent, const Template& t)
    : QTreeWidgetItem(parent)
{
    setTemplate(t);
}

TemplateListItem::~TemplateListItem()
{
}

void TemplateListItem::setTemplate(const Template& t)
{
    m_template = t;

    if (!m_template.isNull())
    {
        setText(0, m_template.templateTitle());
        setText(1, m_template.authors().join(";"));
    }
}

Template TemplateListItem::getTemplate() const
{
    return m_template;
}

// -------------------------------------------------------------------

TemplateList::TemplateList(QWidget* const parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setWhatsThis(i18n("Here you can see the metadata template list managed by digiKam."));

    QStringList labels;
    labels.append(i18n("Title"));
    labels.append(i18n("Authors"));
    setHeaderLabels(labels);
    header()->setResizeMode(0, QHeaderView::ResizeToContents);
    header()->setResizeMode(1, QHeaderView::Stretch);
}

TemplateList::~TemplateList()
{
}

void TemplateList::readSettings()
{
    TemplateManager* tm = TemplateManager::defaultManager();

    if (tm)
    {
        QList<Template> list = tm->templateList();
        foreach(const Template& t, list)
        {
            new TemplateListItem(this, t);
        }
    }
}

void TemplateList::applySettings()
{
    TemplateManager* tm = TemplateManager::defaultManager();

    if (tm)
    {
        tm->clear();

        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            TemplateListItem* item = dynamic_cast<TemplateListItem*>(*it);

            if (item)
            {
                Template t = item->getTemplate();

                if (!t.isNull())
                {
                    tm->insert(t);
                }
            }

            ++it;
        }

        tm->save();
    }
}

TemplateListItem* TemplateList::find(const QString& title)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        TemplateListItem* item = dynamic_cast<TemplateListItem*>(*it);

        if (item)
        {
            Template t = item->getTemplate();

            if (!t.isNull())
            {
                if (t.templateTitle() == title)
                {
                    return item;
                }
            }
        }

        ++it;
    }

    return 0;
}

}  // namespace Digikam
