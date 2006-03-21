/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-21
 * Description : a generic list view widget to 
 *               display metadata
 * 
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qtimer.h>
#include <qptrlist.h>
#include <qpalette.h>
#include <qheader.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "mdkeylistviewitem.h"
#include "metadatalistviewitem.h"
#include "metadatalistview.h"

namespace Digikam
{

MetadataListView::MetadataListView(QWidget* parent)
                : KListView(parent)
{
    header()->hide();
    addColumn("Name");    // No need i18n here.
    addColumn("Value");   // No need i18n here.
    setTreeStepSize(0);
    setItemMargin(0);
    setAllColumnsShowFocus(true);
    setResizeMode(KListView::AllColumns);
    setVScrollBarMode(QScrollView::AlwaysOn);
    
    m_parent = dynamic_cast<MetadataWidget *>(parent);
    
    connect(this, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(slotSelectionChanged(QListViewItem*)));
}

MetadataListView::~MetadataListView()
{
}

QString MetadataListView::getCurrentItemKey()
{
    if (currentItem())
    {
        if (currentItem()->isSelectable())
        {
            MetadataListViewItem *item = static_cast<MetadataListViewItem *>(currentItem());
            return item->getKey();
        }
    }
    
    return QString();
}

void MetadataListView::setCurrentItemByKey(QString itemKey)
{
    if (itemKey.isNull())
        return;

    QListViewItemIterator it(this);
    while ( it.current() )
    {
        if ( it.current()->isSelectable() )
        {
            MetadataListViewItem *item = dynamic_cast<MetadataListViewItem *>(it.current());
            
            if (item->getKey() == itemKey)
            {
                setSelected(item, true);
                ensureItemVisible(item);
                m_selectedItemKey = itemKey;
                return;
            }
        }
        
        ++it;
    }
}

void MetadataListView::slotSelectionChanged(QListViewItem *item)
{
    if (!item)
        return;

    MetadataListViewItem* viewItem = static_cast<MetadataListViewItem *>(item);
    m_selectedItemKey = viewItem->getKey();
    QString tagTitle  = m_parent->getTagTitle(m_selectedItemKey);
    QString tagDesc   = m_parent->getTagDescription(m_selectedItemKey);
    
    QWhatsThis::add( this, i18n("<b>Title: </b><p>%1<p>"
                                "<b>Value: </b><p>%2<p>"
                                "<b>Description: </b><p>%3")
                           .arg(tagTitle)
                           .arg(viewItem->getValue())
                           .arg(tagDesc) );
}

void MetadataListView::setIfdList(MetadataWidget::MetaDataMap ifds, const QStringList& tagsfilter)
{
    clear();
    
    uint subItems = 0;
    QString ifDItemName;
    MdKeyListViewItem *parentifDItem = 0;

    for (MetadataWidget::MetaDataMap::iterator it = ifds.begin(); it != ifds.end(); ++it)
    {
        // We checking if we have changed of ifDName
        QString currentIfDName = it.key().section('.', 1, 1);
        
        if ( currentIfDName != ifDItemName )
        {
            ifDItemName = currentIfDName;

            // Check if the current IfD have any items. If no remove it before to toogle to the next IfD.
            if ( subItems == 0 && parentifDItem)
                delete parentifDItem;

            parentifDItem = new MdKeyListViewItem(this, currentIfDName);
            subItems = 0;
        }

        // We ignore all unknown tags if necessary.
        if (!it.key().section('.', 2, 2).startsWith("0x"))
        {
            if (!tagsfilter.isEmpty())
            {
                // We using the filter to make a more user friendly output (Simple Mode)

                if (tagsfilter.contains(it.key().section('.', 2, 2)))
                {
                    QString tagTitle = m_parent->getTagTitle(it.key());
                    new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.data());
                    subItems++;
                }
            }
            else
            {
                // We don't filter the ouput (Complete Mode)
            
                QString tagTitle = m_parent->getTagTitle(it.key());
                new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.data());
                subItems++;
            }
        }
    }

    // To check if the last IfD have any items...
    if ( subItems == 0 && parentifDItem)
        delete parentifDItem;

    setCurrentItemByKey(m_selectedItemKey);
    QTimer::singleShot( 0, this, SLOT( triggerUpdate() ) );
}

void MetadataListView::setIfdList(MetadataWidget::MetaDataMap ifds, QStringList keysFilter,
                                  const QStringList& tagsFilter)
{
    clear();
    
    uint               subItems = 0;
    MdKeyListViewItem *parentifDItem = 0;

    for (QStringList::iterator itKeysFilter = keysFilter.begin();
         itKeysFilter != keysFilter.end();
         ++itKeysFilter)
    {
        subItems = 0;
        parentifDItem = new MdKeyListViewItem(this, *itKeysFilter);
        
        MetadataWidget::MetaDataMap::iterator it = ifds.end(); 

        while(1)   
        {
            if ( *itKeysFilter == it.key().section('.', 1, 1) )
            {
                // We ignore all unknown tags if necessary.
                if (!it.key().section('.', 2, 2).startsWith("0x"))
                {
                    if (!tagsFilter.isEmpty())
                    {
                        // We using the filter to make a more user friendly output (Simple Mode)
        
                        if (tagsFilter.contains(it.key().section('.', 2, 2)))
                        {
                            QString tagTitle = m_parent->getTagTitle(it.key());
                            new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.data());
                            subItems++;
                        }
                    }
                    else
                    {
                        // We don't filter the ouput (Complete Mode)
                    
                        QString tagTitle = m_parent->getTagTitle(it.key());
                        new MetadataListViewItem(parentifDItem, it.key(), tagTitle, it.data());
                        subItems++;
                    }
                }
            }
            
            if (it == ifds.begin()) break;
            --it;
        }

        // We checking if the last IfD have any items. If no, we remove it.
        if ( subItems == 0 && parentifDItem)
            delete parentifDItem;
    }

    setCurrentItemByKey(m_selectedItemKey);
    QTimer::singleShot( 0, this, SLOT( triggerUpdate() ) );
}

void MetadataListView::viewportResizeEvent(QResizeEvent* e)
{
    KListView::viewportResizeEvent(e);
    QTimer::singleShot( 0, this, SLOT( triggerUpdate() ) );
}

}  // namespace Digikam

#include "metadatalistview.moc"
