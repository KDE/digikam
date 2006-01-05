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

// Qt includes.

#include <qdatetimeedit.h>

// KDE includes.

#include "kdatetimeedit.h"
#include "kdateedit.h"

namespace Digikam
{

KDateTimeEdit::KDateTimeEdit(QWidget *parent, const char *name)
    : QHBox(parent, name)
{
    m_datePopUp = new KDateEdit(this, "datepopup");
    m_timePopUp = new QTimeEdit(QTime::currentTime(), this);

    connect(m_datePopUp, SIGNAL(dateChanged(const QDate& )),
            SLOT(slotDateTimeChanged()) );
    connect(m_timePopUp, SIGNAL(valueChanged(const QTime& )),
            SLOT(slotDateTimeChanged()) );
}

KDateTimeEdit::~KDateTimeEdit()
{
    delete m_datePopUp;
    m_datePopUp = 0;

    delete m_timePopUp;
    m_timePopUp = 0;
}

QDateTime KDateTimeEdit::dateTime()
{
    return QDateTime(m_datePopUp->date(), m_timePopUp->time());
}

void KDateTimeEdit::setDateTime(const QDateTime dateTime)
{
    m_datePopUp->setDate(dateTime.date());
    m_timePopUp->setTime(dateTime.time());
}

void KDateTimeEdit::slotDateTimeChanged()
{
    emit dateTimeChanged(dateTime());
}

}  // namespace Digikam

#include "kdatetimeedit.moc"
