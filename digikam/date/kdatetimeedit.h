/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a widget to edit time stamp.
 *
 * Copyright (C) 2005 Tom Albers <tomalbers@kde.nl>
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

#ifndef KDATETIMEEDIT_H
#define KDATETIMEEDIT_H

// Qt includes

#include <QDateTime>

// KDE includes

#include <khbox.h>

class QTimeEdit;

namespace Digikam
{

class DDateEdit;

/**
 * @class KDateTimeEdit
 * This class is basically the same as the KDateTime class
 * with the exception that a QTimeEdit is placed directly
 * besides it.
 *
 * @image html kdatetimeedit.png "This is how it looks"
 * @author Tom Albers
 */
class KDateTimeEdit : public KHBox
{
    Q_OBJECT

public:

    /**
     * constructor
     * @param parent the parent widget
     * @param name the name of the widget
     */
    KDateTimeEdit(QWidget* parent, const char* name);

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

Q_SIGNALS:

    /**
     *  This signal is emitted whenever the user modifies the date or time.
     *  The passed date and time can be invalid.
     */
    void dateTimeChanged( const QDateTime& dateTime );

private:

    QTimeEdit* m_timePopUp;

    DDateEdit* m_datePopUp;

private Q_SLOTS:

    void slotDateTimeChanged();
};

}  // namespace Digikam

#endif // KDATETIMEEDIT_H
