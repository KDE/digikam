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

/** @file squeezedcombobox.h */

#ifndef SQUEEZEDCOMBOBOX_H
#define SQUEEZEDCOMBOBOX_H

class QListBox;
class QTimer;

#include <qcombobox.h>
#include <qstring.h>

/** @class SqueezedComboBox
 *
 * This widget is a @ref QComboBox, but then a little bit
 * different. It only shows the right part of the items
 * depending on de size of the widget. When it is not
 * possible to show the complete item, it will be shortened
 * and "..." will be prepended.
 *
 * @author Tom Albers
 */
class SqueezedComboBox : public QComboBox
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent parent widget
     * @param name name to give to the widget
     */
    SqueezedComboBox(QWidget *parent = 0, const char *name = 0 );

    /**
     * destructor
     */
    virtual ~SqueezedComboBox();

    /**
     * This inserts a item to the list. See QComboBox::insertItem()
     * for detaills. Please do not use QComboBox::insertItem() to this
     * widget, as that will fail.
     * @param newItem the original (long version) of the item which needs
     *                to be added to the combobox
     * @param index the position in the widget.
     */
    void insertSqueezedItem(const QString& newItem, int index);

    
    /**
     * Sets the sizeHint() of this widget.
     */
    virtual QSize sizeHint() const;

private slots:
    void slotTimeOut();
    void slotUpdateToolTip( int index );

private:
    void resizeEvent ( QResizeEvent * );
    QString squeezeText( const QString& original);

    QValueList< QPair<int,QString> >  m_OriginalItems;
    QTimer*                           m_timer;
};

#endif
