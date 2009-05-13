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

#include "kdatepickerpopup.h"
#include "kdatepickerpopup.moc"

// Qt includes

#include <QDateTime>
#include <QWidgetAction>

// KDE includes

#include <klocale.h>


namespace Digikam
{

class KDatePickerAction : public QWidgetAction
{
public:

    KDatePickerAction( KDatePicker *widget, QObject *parent )
        : QWidgetAction( parent ),
          mDatePicker( widget ), mOriginalParent( widget->parentWidget() )
    {
    }

protected:

    QWidget* createWidget( QWidget *parent )
    {
      mDatePicker->setParent( parent );
      return mDatePicker;
    }

    void deleteWidget( QWidget *widget )
    {
      if ( widget != mDatePicker )
        return;

      mDatePicker->setParent( mOriginalParent );
    }

private:

    KDatePicker *mDatePicker;
    QWidget *mOriginalParent;
};

KDatePickerPopup::KDatePickerPopup( Items items, const QDate& date, QWidget *parent )
                : KMenu( parent )
{
    mItems = items;

    mDatePicker = new KDatePicker( this );
    mDatePicker->setCloseButton( false );

    connect( mDatePicker, SIGNAL( dateEntered( const QDate& ) ),
            this, SLOT( slotDateChanged( const QDate& ) ) );

    connect( mDatePicker, SIGNAL( dateSelected( const QDate& ) ),
            this, SLOT( slotDateChanged( const QDate& ) ) );

    mDatePicker->setDate( date );

    buildMenu();
}

void KDatePickerPopup::buildMenu()
{
    if ( isVisible() ) return;
    clear();

    if ( mItems & DatePicker ) 
    {
        addAction( new KDatePickerAction( mDatePicker, this ) );

        if ( ( mItems & NoDate ) || ( mItems & Words ) )
            addSeparator();
    }

    if ( mItems & Words )
    {
        addAction( i18n("&Today"),       this, SLOT( slotToday() ) );
        addAction( i18n("To&morrow"),    this, SLOT( slotTomorrow() ) );
        addAction( i18n("Next &Week"),   this, SLOT( slotNextWeek() ) );
        addAction( i18n("Next M&onth"),  this, SLOT( slotNextMonth() ) );

        addAction( i18n("Y&esterday"),   this, SLOT( slotYesterday() ) );
        addAction( i18n("Last &Monday"), this, SLOT( slotPrevMonday() ) );
        addAction( i18n("Last &Friday"), this, SLOT( slotPrevFriday() ) );
        addAction( i18n("Last &Week"),   this, SLOT( slotPrevWeek() ) );
        addAction( i18n("Last M&onth"),  this, SLOT( slotPrevMonth() ) );

        if ( mItems & NoDate )
        addSeparator();
    }

    if ( mItems & NoDate )
        addAction( i18n("No Date"), this, SLOT( slotNoDate() ) );
}

KDatePicker *KDatePickerPopup::datePicker() const
{
    return mDatePicker;
}

void KDatePickerPopup::setDate( const QDate& date )
{
    mDatePicker->setDate( date );
}

#if 0
void KDatePickerPopup::setItems( int items )
{
  mItems = items;
  buildMenu();
}
#endif

void KDatePickerPopup::slotDateChanged( const QDate& date )
{
    emit dateChanged( date );
    hide();
}

void KDatePickerPopup::slotToday()
{
    emit dateChanged( QDate::currentDate() );
}

void KDatePickerPopup::slotTomorrow()
{
    emit dateChanged( QDate::currentDate().addDays( 1 ) );
}

void KDatePickerPopup::slotNoDate()
{
    emit dateChanged( QDate() );
}

void KDatePickerPopup::slotNextWeek()
{
    emit dateChanged( QDate::currentDate().addDays( 7 ) );
}

void KDatePickerPopup::slotNextMonth()
{
    emit dateChanged( QDate::currentDate().addMonths( 1 ) );
}

void KDatePickerPopup::slotYesterday()
{
    emit dateChanged( QDate::currentDate().addDays( -1 ) );
}

void KDatePickerPopup::slotPrevFriday()
{
    QDate date = QDate::currentDate();
    int day = date.dayOfWeek();
    if ( day < 6 )
        date = date.addDays( 5 - 7 - day );
    else
        date = date.addDays( 5 - day );

    emit dateChanged( date );
}

void KDatePickerPopup::slotPrevMonday()
{
    QDate date = QDate::currentDate();
    emit dateChanged( date.addDays( 1 - date.dayOfWeek() ) );
}

void KDatePickerPopup::slotPrevWeek()
{
    emit dateChanged( QDate::currentDate().addDays( -7 ) );
}

void KDatePickerPopup::slotPrevMonth()
{
    emit dateChanged( QDate::currentDate().addMonths( -1 ) );
}

}  // namespace Digikam
