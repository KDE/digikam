////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005 Tom Albers <tomalbers@kde.nl>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301  USA
//
//////////////////////////////////////////////////////////////////////////////


/** @file squeezedcombobox.cpp */

#include <qlistbox.h>
#include <qcombobox.h>
#include <qpair.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qtooltip.h>

#include "squeezedcombobox.h"

SqueezedComboBox::SqueezedComboBox( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    setMinimumWidth(100);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));
    connect(this, SIGNAL(activated( int )),
            SLOT(slotUpdateToolTip( int )));
}

SqueezedComboBox::~SqueezedComboBox()
{
    delete m_timer;
}

QSize SqueezedComboBox::sizeHint() const
{
    constPolish();
    QFontMetrics fm = fontMetrics();

    int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH = QMAX( fm.lineSpacing(), 14 ) + 2;

    return style().sizeFromContents(QStyle::CT_ComboBox, this,
    QSize(maxW, maxH)).
            expandedTo(QApplication::globalStrut());
}

void SqueezedComboBox::insertSqueezedItem(const QString& newItem, int index)
{
    m_OriginalItems.append( qMakePair(index, newItem) );
    insertItem( squeezeText(newItem), index );

    // if this is the first item, set the tooltip.
    if (index == 0)
        slotUpdateToolTip(0);
}

void SqueezedComboBox::resizeEvent ( QResizeEvent * )
{
    m_timer->start(200, true);
}

void SqueezedComboBox::slotTimeOut()
{
    QValueList< QPair<int, QString> >::iterator it;
    for (it = m_OriginalItems.begin() ; it != m_OriginalItems.end();
         ++it)
    {
        changeItem( squeezeText((*it).second), (*it).first );
    }
}

QString SqueezedComboBox::squeezeText( const QString& original)
{
    // not the complete widgetSize is usable. Need to compensate for that.
    int widgetSize = width()-20;
    QFontMetrics fm( fontMetrics() );

    // If we can fit the full text, return that.
    if (fm.width(original) < widgetSize)
        return(original);

    // We need to squeeze.
    QString sqItem = original; // prevent empty return value;
    widgetSize = widgetSize-fm.width("...");
    for (uint i = 0 ; i != original.length(); ++i)
    {
        if ( (int)fm.width(original.right(i)) > widgetSize)
        {
            i=i-3; // one to many and compensate for ...
            sqItem = QString("..." + original.right(i));
            break;
        }
    }
    return sqItem;
}

void SqueezedComboBox::slotUpdateToolTip( int index )
{
    QToolTip::remove(this);

    QValueList< QPair<int, QString> >::iterator it;
    for (it = m_OriginalItems.begin() ; it != m_OriginalItems.end();
         ++it)
    {
        if ((*it).first == index)
        {
            QToolTip::add(this, (*it).second);
            break;
        }
    }

}

#include "squeezedcombobox.moc"
