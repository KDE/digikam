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

#include <QComboBox>
#include <QPair>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QToolTip>
#include <QResizeEvent>

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
        timer = 0;
    }

    QMap<int, QString>  originalItems;

    QTimer             *timer;
};

SqueezedComboBox::SqueezedComboBox(QWidget *parent, const char *name)
                : QComboBox(parent)
{
    d = new SqueezedComboBoxPriv;
    setObjectName(name);
    setMinimumWidth(100);
    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    connect(this, SIGNAL(activated(int)),
            SLOT(slotUpdateToolTip(int)));
}

SqueezedComboBox::~SqueezedComboBox()
{
    delete d->timer;
    delete d;
}

bool SqueezedComboBox::contains(const QString& text) const
{
    if (text.isEmpty())
        return false;

    for (QMap<int, QString>::const_iterator it = d->originalItems.begin() ; it != d->originalItems.end();
         ++it)
    {
        if (it.value() == text)
            return true;
    }

    return false;
}

QSize SqueezedComboBox::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();

    int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH = qMax( fm.lineSpacing(), 14 ) + 2;

    QStyleOptionComboBox options;
    options.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_ComboBox, &options,
                                     QSize(maxW, maxH), this).expandedTo(QApplication::globalStrut());
}

void SqueezedComboBox::insertSqueezedItem(const QString& newItem, int index, 
                                          const QVariant& userData)
{
    d->originalItems[index] = newItem;
    QComboBox::insertItem(index, squeezeText(newItem), userData);

    // if this is the first item, set the tooltip.
    if (index == 0)
        slotUpdateToolTip(0);
}

void SqueezedComboBox::addSqueezedItem(const QString& newItem, 
                                       const QVariant& userData)
{
    insertSqueezedItem(newItem, count(), userData);
}

void SqueezedComboBox::setCurrent(const QString& itemText)
{
    QString squeezedText = squeezeText(itemText);
    qint32 itemIndex = findText(squeezedText);
    if (itemIndex >= 0) 
        setCurrentIndex(itemIndex);
}

void SqueezedComboBox::resizeEvent(QResizeEvent *)
{
    d->timer->start(200);
}

void SqueezedComboBox::slotTimeOut()
{
    for (QMap<int, QString>::iterator it = d->originalItems.begin() ; 
         it != d->originalItems.end(); ++it)
    {
        setItemText( it.key(), squeezeText( it.value() ) );
    }
}

QString SqueezedComboBox::squeezeText(const QString& original)
{
    // not the complete widgetSize is usable. Need to compensate for that.
    int widgetSize = width()-30;
    QFontMetrics fm( fontMetrics() );

    // If we can fit the full text, return that.
    if (fm.width(original) < widgetSize)
        return(original);

    // We need to squeeze.
    QString sqItem = original; // prevent empty return value;
    widgetSize = widgetSize-fm.width("...");
    for (int i = 0 ; i != original.length(); ++i)
    {
        if ( (int)fm.width(original.right(i)) > widgetSize)
        {
            sqItem = QString("..." + original.right(--i));
            break;
        }
    }
    return sqItem;
}

void SqueezedComboBox::slotUpdateToolTip(int index)
{
     setToolTip(d->originalItems[index]);
}

QString SqueezedComboBox::itemHighlighted()
{
    int curItem = currentIndex();
    return d->originalItems[curItem];
}

QString SqueezedComboBox::item(int index)
{
    return d->originalItems[index];
}

}  // namespace Digikam
