/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : a combo box containing ICC profiles
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ICCPROFILESCOMBOBOX_H
#define ICCPROFILESCOMBOBOX_H

// Qt includes

#include <QComboBox>
#include <QIcon>
#include <QMenu>

// Local includes

#include "squeezedcombobox.h"
#include "digikam_export.h"
#include "iccprofile.h"

class QSignalMapper;

namespace Digikam
{

class DIGIKAM_EXPORT IccProfilesComboBox : public SqueezedComboBox
{
    Q_OBJECT

public:

    explicit IccProfilesComboBox(QWidget* const parent = 0);
    ~IccProfilesComboBox();

    /**
     * Checks the given profiles for validity, creates a suitable description (ICC profile description, file path),
     * removes duplicates by file path, sorts them and adds them in sorted order.
     */
    void addProfilesSqueezed(const QList<IccProfile>& profiles);
    /**
     * Add the given profile with the given description, or, if null, a standard description.
     * Does not test for duplicity, does not sort into existing profiles.
     */
    void addProfileSqueezed(const IccProfile& profile, const QString& description = QString());
    /**
     * Clears, does the same as addProfilesSqueezed, and restores the current entry if possible.
     */
    void replaceProfilesSqueezed(const QList<IccProfile>& profiles);
    /**
     * Sets a message the is displayed in the combo box and disables the combo box,
     * if the combo box is currently empty
     */
    void setNoProfileIfEmpty(const QString& message);

    /** Retrieves the current profile, or a null profile if none is selected.
     */
    IccProfile currentProfile() const;

    /** Sets the current profile. If profile is not in the list, sets no current item (-1)
     */
    void setCurrentProfile(const IccProfile& profile);

    /// Use the signal currentIndexChanged(int) for change notification
};

// ----------------------------------------------------------------------------

class DIGIKAM_EXPORT IccRenderingIntentComboBox : public QComboBox
{
public:

    explicit IccRenderingIntentComboBox(QWidget* const parent = 0);

    void setIntent(int intent);
    int  intent() const;
};

// ----------------------------------------------------------------------------

class DIGIKAM_EXPORT IccProfilesMenuAction : public QMenu
{
    Q_OBJECT

public:

    IccProfilesMenuAction(const QIcon& icon, const QString& text, QObject* const parent);
    IccProfilesMenuAction(const QString& text, QObject* const parent);

    /**
     * Checks the given profiles for validity, creates a suitable description (ICC profile description, file path),
     * removes duplicates (in newly added list) by file path, sorts them and adds them in sorted order.
     */
    void addProfiles(const QList<IccProfile>& profile);

    /**
     * Add the given profile with the given description, or, if null, a standard description.
     * Does not test for duplicity, does not sort into existing profiles.
     */
    void addProfile(const IccProfile& profile, const QString& description = QString());

    /**
     * Equivalent to calling clear() and addProfiles().
     */
    void replaceProfiles(const QList<IccProfile>& profile);

    /**
     * Disables if the menu is currently empty.
     */
    void disableIfEmpty();

Q_SIGNALS:

    void triggered(const IccProfile& profile);

protected Q_SLOTS:

    void slotTriggered(QObject*);

protected:

    QObject*       m_parent;
    QSignalMapper* m_mapper;
};

} // namespace Digikam

#endif // ICCPROFILESCOMBOBOX_H
