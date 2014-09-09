/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-04-21
 * Description : a menu widget to pick a date.
 *               this widget come from libkdepim.
 *
 * Copyright (C) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>
 * Copyright (C) 2006 Mikolaj Machowski <mikmach@wp.pl>
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DDATEPICKERPOPUP_H
#define DDATEPICKERPOPUP_H

// Qt includes

#include <QDateTime>

// KDE includes

#include <kdatepicker.h>
#include <kmenu.h>

namespace Digikam
{

/**
   @short This menu helps the user to select a date quickly.

   This menu helps the user to select a date quickly. It offers various ways of selecting, e.g. with a KDatePicker or with words like "Tomorrow".

   The available items are:

   @li NoDate: A menu-item with "No Date". If chosen, the datepicker will emit a null QDate.
   @li DatePicker: Show a KDatePicker-widget.
   @li Words: Show items like "Today", "Tomorrow" or "Next Week".

   When supplying multiple items, separate each item with a bitwise OR.

   @author Bram Schoenmakers <bram_s@softhome.net>
*/
class DDatePickerPopup: public KMenu
{
    Q_OBJECT

public:

    enum ItemFlag
    {
        NoDate     = 1,
        DatePicker = 2,
        Words      = 4
    };

    Q_DECLARE_FLAGS( Items, ItemFlag )

    /**
       A constructor for the DDatePickerPopup.

       @param items List of all desirable items, separated with a bitwise OR.
       @param date Initial date of datepicker-widget.
       @param parent The object's parent.
    */
    explicit DDatePickerPopup( Items items = DatePicker, const QDate& date = QDate::currentDate(),
                               QWidget* parent = 0 );
    virtual ~DDatePickerPopup();

    /**
       @return A pointer to the private variable mDatePicker, an instance of
       KDatePicker.
    */
    KDatePicker* datePicker() const;

    void setDate( const QDate& date );

#if 0
    /** Set items which should be shown and rebuilds the menu afterwards. Only if the menu is not visible.
    @param items List of all desirable items, separated with a bitwise OR.
    */
    void setItems( int items = 1 );
#endif

    /** @return Returns the bitwise result of the active items in the popup. */
    int items() const;

Q_SIGNALS:

    /**
      This signal emits the new date (selected with datepicker or other
      menu-items).
    */
    void dateChanged (const QDate&);

protected Q_SLOTS:

    void slotDateChanged(const QDate&);
    void slotToday();
    void slotTomorrow();
    void slotNextWeek();
    void slotNextMonth();

    void slotYesterday();
    void slotPrevMonday();
    void slotPrevFriday();
    void slotPrevWeek();
    void slotPrevMonth();

    void slotNoDate();

private:

    void buildMenu();

private:

    class DDatePickerPopupPriv;
    DDatePickerPopupPriv* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( DDatePickerPopup::Items )

}  // namespace Digikam

#endif // DDATEPICKERPOPUP_H
