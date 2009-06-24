/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : identity list view.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "templatelist.h"
#include "templatelist.moc"

// Qt includes

#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "templatemanager.h"
#include "template.h"

namespace Digikam
{

TemplateListItem::TemplateListItem(QTreeWidget *parent, Template *t)
                : QTreeWidgetItem(parent), m_template(0)
{
    setTemplate(t);
}

TemplateListItem::~TemplateListItem()
{
    delete m_template;
}

void TemplateListItem::setTemplate(Template *t)
{
    if (m_template)
        delete m_template;

    m_template = new Template(*t);

    if (m_template)
    {
        setText(0, m_template->templateTitle());
        setText(1, m_template->author());
    }
}

Template* TemplateListItem::getTemplate() const
{
    return m_template;
}

// -------------------------------------------------------------------

TemplateList::TemplateList(QWidget* parent)
            : QTreeWidget(parent)
{
    setColumnCount(2);
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setWhatsThis(i18n("Here you can see the metadata template list managed by digiKam."));

    QStringList labels;
    labels.append( i18n("Title") );
    labels.append( i18n("Author") );
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
        QList<Template*>* list = tm->templateList();
        foreach (Template *t, *list)
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
            TemplateListItem *item = dynamic_cast<TemplateListItem*>(*it);
            if (item)
            {
                Template* t = item->getTemplate();
                if (t)
                {
                    tm->insert(new Template(*t));
                }
            }
            ++it;
        }
        tm->save();
    }
}

bool TemplateList::contains(const QString& title)
{
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        TemplateListItem *item = dynamic_cast<TemplateListItem*>(*it);
        if (item)
        {
            Template* t = item->getTemplate();
            if (t)
            {
                if (t->templateTitle() == title)
                    return true;
            }
        }
        ++it;
    }
    return false;
}

}  // namespace Digikam
