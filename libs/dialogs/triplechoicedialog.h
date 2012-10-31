/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-03
 * Description : dialog which provides at least three choices, plus a cancel button
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef TRIPLECHOICEDIALOG_H
#define TRIPLECHOICEDIALOG_H

// Qt includes

// KDE includes

#include <kdialog.h>

// Local includes

#include "digikam_export.h"

class QToolButton;

namespace Digikam
{

class DIGIKAM_EXPORT TripleChoiceDialog : public KDialog
{
    Q_OBJECT

public:

    /**
     * This dialog provides a widget containing a number of buttons.
     * For two options and less, traditional Ok | Apply | Cancel is sufficient,
     * but with one more option Ok | Apply | User1 and Cancel, there is one
     * button too much for the traditional layout.
     *
     * Note that per default, the buttonContainer() widget is not added anywhere.
     * Instead, add it to the layout when you setup the dialog's main widget.
     */

    explicit TripleChoiceDialog(QWidget* const parent = 0);
    ~TripleChoiceDialog();

    /**
     * Sets the icon size of the added buttons.
     * Default: KIconLoader::SizeMedium.
     * Note: Call before addButton() or buttonContainer().
     */
    void setIconSize(int iconSize);
    int iconSize() const;

    /**
     * Change the visibility of the Cancel button. Default: True.
     * The Cancel button is at the normal position in the dialog.
     */
    void setShowCancelButton(bool show);

    /**
     * Add a button to the dialog.
     * This button is not positioned like a normal dialog button
     * at the bottom, but in a central area.
     * Each added button has a unique, associated keycode (see KDialog)
     * Note: The Ok button is automatically bound to the Return key press.
     */
    QToolButton* addChoiceButton(int key, const QString& iconName, const QString& text);
    QToolButton* addChoiceButton(int key, const QString& text);
    QToolButton* choiceButton(int key) const;

    /**
     * Returns the button that was clicked
     */
    int clickedButton() const;

protected:

    /**
     * Handles button presses.
     * The default implementation, emits buttonClicked(),
     * rejects in case of cancel, otherwise accepts.
     * Note: Normal KDialog events, for the Cancel button or pressing
     * the Esc or Return keys, arrive here too.
     */
    virtual void slotButtonClicked(int button);

    /**
     * Returns the widget that contains the buttons.
     * You must place this widget in your dialogs layout.
     */
    QWidget* buttonContainer() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TRIPLECHOICEDIALOG_H
