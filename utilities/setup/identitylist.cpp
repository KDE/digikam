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

#include "identitylist.h"
#include "identitylist.moc"

// Qt includes

#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "photographerlist.h"
#include "photographer.h"

namespace Digikam
{

IndentityListItem::IndentityListItem(QTreeWidget *parent, Photographer *photographer)
                 : QTreeWidgetItem(parent), m_photographer(0)
{
    setPhotographer(photographer);
}

IndentityListItem::~IndentityListItem()
{
    delete m_photographer;
}

void IndentityListItem::setPhotographer(Photographer *photographer)
{
    if (m_photographer)
        delete m_photographer;

    m_photographer = new Photographer(*photographer);

    if (m_photographer)
    {
        setText(0, m_photographer->author());
        setText(1, m_photographer->authorPosition());
    }
}

Photographer* IndentityListItem::photographer() const
{
    return m_photographer;
}

// -------------------------------------------------------------------

IndentityList::IndentityList(QWidget* parent)
             : QTreeWidget(parent)
{
    setColumnCount(2);
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setWhatsThis(i18n("Here you can see the photographers list managed by digiKam."));

    QStringList labels;
    labels.append( i18n("Author") );
    labels.append( i18n("Author Position") );
    setHeaderLabels(labels);
    header()->setResizeMode(0, QHeaderView::ResizeToContents);
    header()->setResizeMode(1, QHeaderView::Stretch);
}

IndentityList::~IndentityList()
{
}

void IndentityList::readSettings()
{
    PhotographerList* plist = PhotographerList::defaultList();
    if (plist)
    {
        QList<Photographer*>* list = plist->photographerList();
        foreach (Photographer *photographer, *list)
        {
            new IndentityListItem(this, photographer);
        }
    }
}

void IndentityList::applySettings()
{
    PhotographerList* plist = PhotographerList::defaultList();
    if (plist)
    {
        plist->clear();

        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            IndentityListItem *item = dynamic_cast<IndentityListItem*>(*it);
            if (item)
            {
                Photographer* photographer = item->photographer();
                if (photographer)
                {
                    plist->insert(new Photographer(*photographer));
                }
            }
            ++it;
        }
        plist->save();
    }
}

}  // namespace Digikam
