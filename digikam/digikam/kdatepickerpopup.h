/*
  This file is part of libkdepim.

  Copyright (c) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>

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
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/
#ifndef KDATEPICKERPOPUP_H
#define KDATEPICKERPOPUP_H

#include <qdatetime.h>
#include <qpopupmenu.h>

#include <kdepimmacros.h>
#include <kdatepicker.h>

/**
   @short This menu helps the user to select a date quickly.

   This menu helps the user to select a date quicly. It offers various ways of selecting, e.g. with a KDatePicker or with words like "Tomorrow".

   The available items are:

   @li NoDate: A menu-item with "No Date". If choosen, the datepicker will emit a null QDate.
   @li DatePicker: Show a KDatePicker-widget.
   @li Words: Show items like "Today", "Tomorrow" or "Next Week".

   When supplying multiple items, separate each item with a bitwise OR.

   @author Bram Schoenmakers <bram_s@softhome.net>
*/
class KDE_EXPORT KDatePickerPopup: public QPopupMenu
{
    Q_OBJECT
  public:
    enum { NoDate = 1, DatePicker = 2, Words = 4 };

    /**
       A constructor for the KDatePickerPopup.

       @param items List of all desirable items, separated with a bitwise OR.
       @param date Initial date of datepicker-widget.
       @param parent The object's parent.
       @param name The object's name.
    */
    KDatePickerPopup( int items = 2, const QDate &date = QDate::currentDate(),
                      QWidget *parent = 0, const char *name = 0 );

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
    void slotDateChanged ( QDate );

    void slotToday();
    void slotTomorrow();
    void slotFriday();
    void slotSunday();
    void slotNextWeek();
    void slotNextMonth();
    void slotNoDate();

  private:
    void buildMenu();

    KDatePicker *mDatePicker;
    int mItems;
};

#endif
