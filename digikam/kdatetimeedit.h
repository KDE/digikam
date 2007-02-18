/*
    Copyright (c) 2005 Tom Albers <tomalbers@kde.nl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor, 
    Boston, MA  02110-1301  USA
*/

/** @file kdatetimeedit.h **/

#ifndef KDATETIMEEDIT_H
#define KDATETIMEEDIT_H

// Qt includes.

#include <qhbox.h>

class QTimeEdit;

namespace Digikam
{

class KDateEdit;

/**
 * @class KDateTimeEdit
 * This class is basically the same as the KDateTime class
 * with the exception that a QTimeEdit is placed directly
 * besides it.
 *
 * @image html kdatetimeedit.png "This is how it looks"
 * @author Tom Albers
 */
class KDateTimeEdit : public QHBox
{
    Q_OBJECT

public:
 
    /**
     * constructor
     * @param parent the parent widget
     * @param name the name of the widget
     */
    KDateTimeEdit(QWidget *parent, const char *name);

     /**
      * destructor
      */
     ~KDateTimeEdit();

     /**
      * returns the date and time
      * @return a QDateTime with the currently chosen date and time
      */
     QDateTime dateTime();

     /**
      * Sets the date and the time of this widget.
      */
     void setDateTime(const QDateTime dateTime);

signals:

    /**
     *  This signal is emitted whenever the user modifies the date or time.
     *  The passed date and time can be invalid.
     */
    void dateTimeChanged( const QDateTime &dateTime );

private:

    KDateEdit* m_datePopUp;
    QTimeEdit* m_timePopUp;

private slots:

    void slotDateTimeChanged();
};

}  // namespace Digikam

#endif // KDATETIMEEDIT_H
