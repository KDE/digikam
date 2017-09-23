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

#include "iccprofilescombobox.h"

// Qt includes

#include <QFileInfo>
#include <QSignalMapper>
#include <QSet>
#include <QMenu>
#include <QIcon>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "icctransform.h"

namespace Digikam
{

IccProfilesComboBox::IccProfilesComboBox(QWidget* const parent)
    : SqueezedComboBox( parent )
{
}

IccProfilesComboBox::~IccProfilesComboBox()
{
}

bool iccProfileLessThan(IccProfile a, IccProfile b)
{
    return a.description() < b.description();
}

// if needed outside this class, make it a public static method in a namespace
static QString profileUserString(const IccProfile& p)
{
    IccProfile profile(p);
    QFileInfo info(profile.filePath());
    QString fileName = info.fileName();

    QString description = profile.description();

    if (!description.isEmpty() && !fileName.isEmpty())
    {
        return i18nc("<Profile Description> (<File Name>)", "%1 (%2)", description, fileName);
    }
    else if (!fileName.isEmpty())
    {
        return fileName;
    }
    else
    {
        return QString();
    }
}

// if needed outside this class, make it a public static method in a namespace
static void formatProfiles(const QList<IccProfile>& givenProfiles, QList<IccProfile>* const returnedProfiles, QStringList* const userText)
{
    QList<IccProfile> profiles;
    QSet<QString>     filePaths;

    foreach(IccProfile profile, givenProfiles) // krazy:exclude=foreach
    {
        QString filePath = profile.filePath();

        if (!profile.description().isNull() && (filePath.isNull() || !filePaths.contains(filePath)) )
        {
            profiles << profile;
            filePaths << filePath;
        }
    }

    std::sort(profiles.begin(), profiles.end(), iccProfileLessThan);

    foreach(IccProfile profile, profiles) // krazy:exclude=foreach
    {
        QString description = profileUserString(profile);

        if (description.isNull())
        {
            continue;
        }

        *returnedProfiles << profile;
        *userText << description;
    }
}

void IccProfilesComboBox::addProfilesSqueezed(const QList<IccProfile>& givenProfiles)
{
    QList<IccProfile> profiles;
    QStringList       userDescription;
    formatProfiles(givenProfiles, &profiles, &userDescription);

    for (int i=0; i<profiles.size(); ++i)
    {
        addSqueezedItem(userDescription.at(i), QVariant::fromValue(profiles.at(i)));
    }
}

void IccProfilesComboBox::addProfileSqueezed(const IccProfile& profile, const QString& d)
{
    QString description = d;

    if (description.isNull())
    {
        description = profileUserString(profile);
    }

    addSqueezedItem(description, QVariant::fromValue(profile));
}

void IccProfilesComboBox::replaceProfilesSqueezed(const QList<IccProfile>& profiles)
{
    IccProfile current = currentProfile();
    clear();
    addProfilesSqueezed(profiles);
    setCurrentProfile(current);
}

void IccProfilesComboBox::setNoProfileIfEmpty(const QString& message)
{
    if (count() == 0)
    {
        setEnabled(false);
        addSqueezedItem(message);
        setCurrentIndex(0);
    }
}

IccProfile IccProfilesComboBox::currentProfile() const
{
    return itemData(currentIndex()).value<IccProfile>();
}

void IccProfilesComboBox::setCurrentProfile(const IccProfile& profile)
{
    if (profile.isNull())
    {
        setCurrentIndex(-1);
        return;
    }

    const int size = count();

    for (int i = 0; i < size; ++i)
    {
        if (itemData(i).value<IccProfile>() == profile)
        {
            setCurrentIndex(i);
            return;
        }
    }

    setCurrentIndex(-1);
}

// ------------------------------------------------------------------------------------------

IccProfilesMenuAction::IccProfilesMenuAction(const QIcon& icon, const QString& text, QObject* const parent)
    : QMenu(text),
      m_parent(parent)
{
    setIcon(icon);
    m_mapper = new QSignalMapper(this);

    connect(m_mapper, SIGNAL(mapped(QObject*)),
            this, SLOT(slotTriggered(QObject*)));
}

IccProfilesMenuAction::IccProfilesMenuAction(const QString& text, QObject* const parent)
    : QMenu(text),
      m_parent(parent)
{
    m_mapper = new QSignalMapper(this);

    connect(m_mapper, SIGNAL(mapped(QObject*)),
            this, SLOT(slotTriggered(QObject*)));
}

void IccProfilesMenuAction::replaceProfiles(const QList<IccProfile>& profiles)
{
    clear();
    addProfiles(profiles);
}

void IccProfilesMenuAction::addProfiles(const QList<IccProfile>& givenProfiles)
{
    QList<IccProfile> profiles;
    QStringList userDescription;
    formatProfiles(givenProfiles, &profiles, &userDescription);

    for (int i=0; i<profiles.size(); ++i)
    {
        addProfile(profiles.at(i), userDescription.at(i));
    }
}

void IccProfilesMenuAction::addProfile(const IccProfile& profile, const QString& d)
{
    QString description = d;

    if (description.isNull())
    {
        description = profileUserString(profile);
    }

    QAction* const action = new QAction(d.left(50), m_parent);
    action->setData(QVariant::fromValue(profile));
    addAction(action);

    connect(action, SIGNAL(triggered()),
            m_mapper, SLOT(map()));

    m_mapper->setMapping(action, action);
}

void IccProfilesMenuAction::disableIfEmpty()
{
    if (isEmpty())
    {
        setEnabled(false);
    }
}

void IccProfilesMenuAction::slotTriggered(QObject* obj)
{
    QAction* const action = static_cast<QAction*>(obj);
    IccProfile profile    = action->data().value<IccProfile>();

    if (!profile.isNull())
    {
        emit triggered(profile);
    }
}

// ------------------------------------------------------------------------------------------

IccRenderingIntentComboBox::IccRenderingIntentComboBox(QWidget* const parent)
    : QComboBox(parent)
{
    addItem(QLatin1String("Perceptual"), IccTransform::Perceptual);
    addItem(QLatin1String("Relative Colorimetric"), IccTransform::RelativeColorimetric);
    addItem(QLatin1String("Absolute Colorimetric"), IccTransform::AbsoluteColorimetric);
    addItem(QLatin1String("Saturation"), IccTransform::Saturation);
    setWhatsThis( i18n("<ul><li><p><b>Perceptual intent</b> causes the full gamut of the image to be "
                       "compressed or expanded to fill the gamut of the destination device, so that gray balance is "
                       "preserved but colorimetric accuracy may not be preserved.</p>"
                       "<p>In other words, if certain colors in an image fall outside of the range of colors that the output "
                       "device can render, the image intent will cause all the colors in the image to be adjusted so that "
                       "the every color in the image falls within the range that can be rendered and so that the relationship "
                       "between colors is preserved as much as possible.</p>"
                       "<p>This intent is most suitable for display of photographs and images, and is the default intent.</p></li>"
                       "<li><p><b>Absolute Colorimetric intent</b> causes any colors that fall outside the range that the output device "
                       "can render to be adjusted to the closest color that can be rendered, while all other colors are "
                       "left unchanged.</p>"
                       "<p>This intent preserves the white point and is most suitable for spot colors (Pantone, TruMatch, "
                       "logo colors, ....)</p></li>"
                       "<li><p><b>Relative Colorimetric intent</b> is defined such that any colors that fall outside the range that the "
                       "output device can render are adjusted to the closest color that can be rendered, while all other colors "
                       "are left unchanged. Proof intent does not preserve the white point.</p></li>"
                       "<li><p><b>Saturation intent</b> preserves the saturation of colors in the image at the possible expense of "
                       "hue and lightness.</p>"
                       "<p>Implementation of this intent remains somewhat problematic, and the ICC is still working on methods to "
                       "achieve the desired effects.</p>"
                       "<p>This intent is most suitable for business graphics such as charts, where it is more important that the "
                       "colors be vivid and contrast well with each other rather than a specific color.</p></li></ul>"));
}

void IccRenderingIntentComboBox::setIntent(int intent)
{
    const int size = count();

    for (int i=0; i<size; ++i)
    {
        if (itemData(i).toInt() == intent)
        {
            setCurrentIndex(i);
            return;
        }
    }

    setCurrentIndex(-1);
}

int IccRenderingIntentComboBox::intent() const
{
    return itemData(currentIndex()).toInt();
}

} // namespace Digikam
