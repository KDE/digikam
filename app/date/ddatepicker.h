/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : A date selection widget.
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 1997      by Tim D. Gilman <tdgilman at best dot org>
 * Copyright (C) 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * Copyright (C) 2007      by John Layt <john at layt dot net>
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

#ifndef DDATE_PICKER_H
#define DDATE_PICKER_H

// Qt includes

#include <QDateTime>
#include <QFrame>

// Local includes

#include "digikam_export.h"

class QLineEdit;

namespace Digikam
{

class DDateTable;

/**
 * Provides a widget for calendar date input.
 *
 */
class DIGIKAM_EXPORT DDatePicker: public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged USER true)
    Q_PROPERTY(bool closeButton READ hasCloseButton WRITE setCloseButton)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize)

public:

    /**
     * The constructor. The current date will be displayed initially.
     */
    explicit DDatePicker(QWidget* const parent = 0);

    /**
     * The constructor. The given date will be displayed initially.
     */
    explicit DDatePicker(const QDate& dt, QWidget* const parent = 0);

    /**
     * The destructor.
     */
    virtual ~DDatePicker();

    /** The size hint for date pickers. The size hint recommends the
     *  minimum size of the widget so that all elements may be placed
     *  without clipping. This sometimes looks ugly, so when using the
     *  size hint, try adding 28 to each of the reported numbers of
     *  pixels.
     */
    QSize sizeHint() const Q_DECL_OVERRIDE;

    /**
     * Sets the date.
     *
     *  @returns @p false and does not change anything if the date given is invalid.
     */
    bool setDate(const QDate& date);

    /**
     * @returns the selected date.
     */
    const QDate& date() const;

    /**
     * @returns the DDateTable widget child of this DDatePicker
     * widget.
     */
    DDateTable* dateTable() const;

    /**
     * Sets the font size of the widgets elements.
     **/
    void setFontSize(int);

    /**
     * Returns the font size of the widget elements.
     */
    int fontSize() const;

    /**
     * By calling this method with @p enable = true, DDatePicker will show
     * a little close-button in the upper button-row. Clicking the
     * close-button will cause the DDatePicker's topLevelWidget()'s close()
     * method being called. This is mostly useful for toplevel datepickers
     * without a window manager decoration.
     * @see hasCloseButton
     */
    void setCloseButton(bool enable);

    /**
     * @returns true if a DDatePicker shows a close-button.
     * @see setCloseButton
     */
    bool hasCloseButton() const;

protected:

    /// to catch move keyEvents when QLineEdit has keyFocus
    bool eventFilter(QObject*, QEvent*) Q_DECL_OVERRIDE;

    /// the resize event
    void resizeEvent(QResizeEvent*) Q_DECL_OVERRIDE;
    void changeEvent(QEvent*) Q_DECL_OVERRIDE;

protected Q_SLOTS:

    void dateChangedSlot(const QDate& date);
    void tableClickedSlot();
    void monthForwardClicked();
    void monthBackwardClicked();
    void yearForwardClicked();
    void yearBackwardClicked();
    void selectMonthClicked();
    void selectYearClicked();
    void uncheckYearSelector();
    void lineEnterPressed();
    void todayButtonClicked();
    void weekSelected(int);

Q_SIGNALS:

    /** This signal is emitted each time the selected date is changed.
     *  Usually, this does not mean that the date has been entered,
     *  since the date also changes, for example, when another month is
     *  selected.
     *  @see dateSelected
     */
    void dateChanged(const QDate& date);

    /** This signal is emitted each time a day has been selected by
     *  clicking on the table (hitting a day in the current month). It
     *  has the same meaning as dateSelected() in older versions of
     *  DDatePicker.
     */
    void dateSelected(const QDate& date);

    /** This signal is emitted when enter is pressed and a VALID date
     *  has been entered before into the line edit. Connect to both
     *  dateEntered() and dateSelected() to receive all events where the
     *  user really enters a date.
     */
    void dateEntered(const QDate& date);

    /** This signal is emitted when the day has been selected by
     *  clicking on it in the table.
     */
    void tableClicked();

private:

    void initWidget(const QDate& date);

private:

    class Private;
    Private *const d;

    friend class Private;
};


}  // namespace Digikam

#endif // DDATE_PICKER_H
