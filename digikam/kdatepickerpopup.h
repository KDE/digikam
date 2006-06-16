/* ============================================================
 * Authors: bram Schoenmakers <bramschoenmakers@kde.nl>
 *          Mikolaj Machowski <mikmach@wp.pl>
 * Date   : 2004-04-21
 * Description : a menu widget to pick a date.
 *               this widget come from libkdepim.
 *
 * Copyright (c) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>
 *           (c) 2006 Mikolaj Machowski <mikmach@wp.pl>
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

#ifndef KDATEPICKERPOPUP_H
#define KDATEPICKERPOPUP_H

// Qt includes.

#include <qdatetime.h>
#include <qpopupmenu.h>

// KDE includes.

#include <kdatepicker.h>

namespace Digikam
{

/**
   @short This menu helps the user to select a past date quickly.

   This menu helps the user to select a past date quickly. It offers various ways of selecting, e.g. with a KDatePicker or with words like "Yesterday".

   The available items are:

   @li NoDate: A menu-item with "No Date". If choosen, the datepicker will emit a null QDate.
   @li DatePicker: Show a KDatePicker-widget.
   @li Words: Show items like "Today", "Yesterday" or "Previous Week".

   When supplying multiple items, separate each item with a bitwise OR.

   @author Bram Schoenmakers <bram_s@softhome.net>, Mikolaj Machowski <mikmach@wp.pl>
*/
class KDatePickerPopup: public QPopupMenu
{
    Q_OBJECT

public:

    enum 
    {
    NoDate = 1,
    DatePicker = 2,
    Words = 4
    };

    /**
       A constructor for the KDatePickerPopup.

       @param items List of all desirable items, separated with a bitwise OR.
       @param date Initial date of datepicker-widget.
       @param parent The object's parent.
       @param name The object's name.
    */
    KDatePickerPopup(int items = 2, const QDate &date = QDate::currentDate(),
                     QWidget *parent = 0, const char *name = 0);

    /**
       @return A pointer to the private variable mDatePicker, an instance of
       KDatePicker.
    */
    KDatePicker *datePicker() const;

    void setDate( const QDate &date );

#if 0
    /** Set items which should be shown and rebuilds the menu afterwards. Only if the menu is not visible.
    @param items List of all desirable items, separated with a bitwise OR.
    */
    void setItems( int items = 1 );
#endif
    /** @return Returns the bitwise result of the active items in the popup. */
    int items() const { return mItems; }

  signals:

    /**
      This signal emits the new date (selected with datepicker or other
      menu-items).
    */
    void dateChanged ( QDate );

protected slots:

    void slotDateChanged(QDate);
    void slotToday();
    void slotYesterday();
    void slotPrevMonday();
    void slotPrevFriday();
    void slotPrevWeek();
    void slotPrevMonth();
    void slotNoDate();

private:

    void buildMenu();

private:

    int          mItems;

    KDatePicker *mDatePicker;
};

}  // namespace Digikam

#endif // KDATEPICKERPOPUP_H
