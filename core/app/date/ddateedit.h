/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-01-10
 * Description : a combo box to list date.
 *               this widget come from libkdepim.
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2002      by Cornelius Schumacher <schumacher at kde dot org>
 * Copyright (C) 2003-2004 by Reinhold Kainhofer <reinhold at kainhofer dot com>
 * Copyright (C) 2004      by Tobias Koenig <tokoe at kde dot org>
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

#ifndef DDATEEDIT_H
#define DDATEEDIT_H

// Qt includes

#include <QMouseEvent>
#include <QEvent>
#include <QComboBox>

namespace Digikam
{

/**
  A date editing widget that consists of an editable combo box.
  The combo box contains the date in text form, and clicking the combo
  box arrow will display a 'popup' style date picker.

  This widget also supports advanced features like allowing the user
  to type in the day name to get the date. The following keywords
  are supported (in the native language): tomorrow, yesterday, today,
  Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday.

  @image html kdateedit.png "This is how it looks"

  @author Cornelius Schumacher <schumacher@kde.org>
  @author Mike Pilone <mpilone@slac.com>
  @author David Jarvie <software@astrojar.org.uk>
  @author Tobias Koenig <tokoe@kde.org>
*/
class DDateEdit : public QComboBox
{
    Q_OBJECT

public:

    explicit DDateEdit(QWidget* const parent=0, const QString& name=QString());
    virtual ~DDateEdit();

    /**
      @return The date entered. This date could be invalid,
              you have to check validity yourself.
     */
    QDate date() const;

    /**
      Sets whether the widget is read-only for the user. If read-only,
      the date picker pop-up is inactive, and the displayed date cannot be edited.

      @param readOnly True to set the widget read-only, false to set it read-write.
     */
    void setReadOnly(bool readOnly);

    /**
      @return True if the widget is read-only, false if read-write.
     */
    bool isReadOnly() const;

    virtual void showPopup();

Q_SIGNALS:

    /**
      This signal is emitted whenever the user modifies the date.
      The passed date can be invalid.
     */
    void dateChanged(const QDate& date);

public Q_SLOTS:

    /**
      Sets the date.

      @param date The new date to display. This date must be valid or
                  it will not be set
     */
    void setDate(const QDate& date);

protected Q_SLOTS:

    void lineEnterPressed();
    void slotTextChanged(const QString&);
    void dateEntered(const QDate&);
    void dateSelected(const QDate&);

protected:

    virtual bool eventFilter(QObject*, QEvent*);
    virtual void mousePressEvent(QMouseEvent*);

    /**
      Sets the date, without altering the display.
      This method is used internally to set the widget's date value.
      As a virtual method, it allows derived classes to perform additional validation
      on the date value before it is set. Derived classes should return true if
      QDate::isValid(@p date) returns false.

      @param date The new date to set.
      @return True if the date was set, false if it was considered invalid and
              remains unchanged.
     */
    virtual bool assignDate(const QDate& date);

    /**
      Fills the keyword map. Re-implement it if you want additional
      keywords.
     */
    void setupKeywords();

private:

    QDate parseDate(bool* = 0) const;
    void updateView();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DDATEEDIT_H
