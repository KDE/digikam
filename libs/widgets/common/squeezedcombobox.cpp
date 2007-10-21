/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : a combo box with a width not depending of text 
 *               content size
 * 
 * Copyright (C) 2005 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

/** @file squeezedcombobox.cpp */

// Qt includes.

#include <qlistbox.h>
#include <qcombobox.h>
#include <qpair.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qmap.h>

// Local includes.

#include "squeezedcombobox.h"
#include "squeezedcombobox.moc"

namespace Digikam
{

class SqueezedComboBoxPriv
{
public:

    SqueezedComboBoxPriv()
    {
        timer   = 0;
        tooltip = 0;
    }

    QMap<int, QString>   originalItems;

    QTimer              *timer;

    SqueezedComboBoxTip *tooltip;
};

SqueezedComboBox::SqueezedComboBox(QWidget *parent, const char *name)
                : QComboBox(parent, name)
{
    d = new SqueezedComboBoxPriv;
    d->timer = new QTimer(this);

    // See B.K.O #138747 : always for QComboBox instance to use a QListbox to 
    // render content independently of Widget style used. 
    setListBox(new QListBox(this));

    d->tooltip = new SqueezedComboBoxTip(listBox()->viewport(), this);
    setMinimumWidth(100);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    connect(this, SIGNAL(activated( int )),
            this, SLOT(slotUpdateToolTip( int )));
}

SqueezedComboBox::~SqueezedComboBox()
{
    delete d->tooltip;
    delete d->timer;
    delete d;
}

QSize SqueezedComboBox::sizeHint() const
{
    constPolish();
    QFontMetrics fm = fontMetrics();

    int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH = QMAX( fm.lineSpacing(), 14 ) + 2;

    return style().sizeFromContents(QStyle::CT_ComboBox, this,
    QSize(maxW, maxH)).expandedTo(QApplication::globalStrut());
}

void SqueezedComboBox::insertSqueezedItem(const QString& newItem, int index)
{
    d->originalItems[index] = newItem;
    insertItem( squeezeText(newItem), index );

    // if this is the first item, set the tooltip.
    if (index == 0)
        slotUpdateToolTip(0);
}

void SqueezedComboBox::insertSqueezedList(const QStringList& newItems, int index)
{
    for(QStringList::const_iterator it = newItems.begin() ; it != newItems.end() ; ++it)
    {
        insertSqueezedItem(*it, index);
        index++;
    }
}

void SqueezedComboBox::resizeEvent(QResizeEvent *)
{
    d->timer->start(200, true);
}

void SqueezedComboBox::slotTimeOut()
{
    QMapIterator<int,QString> it;
    for (it = d->originalItems.begin() ; it != d->originalItems.end();
         ++it)
    {
        changeItem(squeezeText(it.data()), it.key());
    }
}

QString SqueezedComboBox::squeezeText(const QString& original)
{
    // not the complete widgetSize is usable. Need to compensate for that.
    int widgetSize = width()-30;
    QFontMetrics fm(fontMetrics());

    // If we can fit the full text, return that.
    if (fm.width(original) < widgetSize)
        return(original);

    // We need to squeeze.
    QString sqItem = original; // prevent empty return value;
    widgetSize = widgetSize-fm.width("...");
    for (uint i = 0 ; i != original.length(); ++i)
    {
        if ((int)fm.width(original.right(i)) > widgetSize)
        {
            sqItem = QString(original.left(i) + "...");
            break;
        }
    }
    return sqItem;
}

void SqueezedComboBox::slotUpdateToolTip(int index)
{
    QToolTip::remove(this);
    QToolTip::add(this, d->originalItems[index]);
}

QString SqueezedComboBox::itemHighlighted()
{
    int curItem = listBox()->currentItem();
    return d->originalItems[curItem];
}

// ------------------------------------------------------------------------

SqueezedComboBoxTip::SqueezedComboBoxTip(QWidget *parent, SqueezedComboBox *name)
                   : QToolTip( parent )
{
    m_originalWidget = name;
}

void SqueezedComboBoxTip::maybeTip(const QPoint &pos)
{
    QListBox* listBox = m_originalWidget->listBox();
    if (!listBox)
        return;

    QListBoxItem* selectedItem = listBox->itemAt( pos );
    if (selectedItem)
    {
        QRect positionToolTip = listBox->itemRect(selectedItem);
        QString toolTipText = m_originalWidget->itemHighlighted();
        if (!toolTipText.isNull())
            tip(positionToolTip, toolTipText);
    }
}

}  // namespace Digikam
