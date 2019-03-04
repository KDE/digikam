/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to convert to color space
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "profileconversiontoolplugin.h"

// Qt includes

#include <QPointer>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "profileconversiontool.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "imageiface.h"

namespace DigikamEditorProfileConversionToolPlugin
{

ProfileConversionToolPlugin::ProfileConversionToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

ProfileConversionToolPlugin::~ProfileConversionToolPlugin()
{
}

QString ProfileConversionToolPlugin::name() const
{
    return i18n("Color Profile Conversion");
}

QString ProfileConversionToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ProfileConversionToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("preferences-desktop-display-color"));
}

QString ProfileConversionToolPlugin::description() const
{
    return i18n("A tool to convert image to a color space");
}

QString ProfileConversionToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can convert image to a different color space.</p>");
}

QList<DPluginAuthor> ProfileConversionToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2009-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2019"))
            ;
}

void ProfileConversionToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    m_profileMenuAction     = new IccProfilesMenuAction(icon(), QString(), parent);

    connect(m_profileMenuAction, SIGNAL(triggered(IccProfile)),
            this, SLOT(slotConvertToColorSpace(IccProfile)));

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotUpdateColorSpaceMenu()));

    ac->setMenu(m_profileMenuAction);
    ac->setText(i18n("Color Spaces"));
    ac->setObjectName(QLatin1String("editorwindow_colormanagement"));
    ac->setActionCategory(DPluginAction::EditorColors);

    addAction(ac);

    DPluginAction* const ac2 = new DPluginAction(parent);
    ac2->setIcon(icon());
    ac2->setText(i18nc("@action", "Color Space Converter..."));
    ac2->setObjectName(QLatin1String("editorwindow_color_spaceconverter"));
    ac2->setActionCategory(DPluginAction::EditorColors);

    connect(ac2, SIGNAL(triggered(bool)),
            this, SLOT(slotProfileConversionTool()));

    m_colorSpaceConverter = ac2;

    addAction(ac2);

    slotUpdateColorSpaceMenu();
}

void ProfileConversionToolPlugin::slotConvertToColorSpace(const IccProfile& profile)
{
    ImageIface iface;

    if (iface.originalIccProfile().isNull())
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("This image is not color managed."));
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    ProfileConversionTool::fastConversion(profile);
    qApp->restoreOverrideCursor();
}

void ProfileConversionToolPlugin::slotUpdateColorSpaceMenu()
{
    m_profileMenuAction->clear();

    EditorWindow* const editor = dynamic_cast<EditorWindow*>(m_profileMenuAction->parentObject());

    if (!IccSettings::instance()->isEnabled())
    {
        QAction* const action = new QAction(i18n("Color Management is disabled..."), this);
        m_profileMenuAction->addAction(action);

        if (editor)
        {
            connect(action, SIGNAL(triggered()),
                    editor, SLOT(slotSetupICC()));
        }
    }
    else
    {
        ICCSettingsContainer settings = IccSettings::instance()->settings();

        QList<IccProfile> standardProfiles, favoriteProfiles;
        QSet<QString> standardProfilePaths, favoriteProfilePaths;
        standardProfiles << IccProfile::sRGB()
                         << IccProfile::adobeRGB()
                         << IccProfile::wideGamutRGB()
                         << IccProfile::proPhotoRGB();

        foreach (IccProfile profile, standardProfiles) // krazy:exclude=foreach
        {
            m_profileMenuAction->addProfile(profile, profile.description());
            standardProfilePaths << profile.filePath();
        }

        m_profileMenuAction->addSeparator();

        favoriteProfilePaths  = QSet<QString>::fromList(ProfileConversionTool::favoriteProfiles());
        favoriteProfilePaths -= standardProfilePaths;

        foreach (const QString& path, favoriteProfilePaths)
        {
            favoriteProfiles << IccProfile(path);
        }

        m_profileMenuAction->addProfiles(favoriteProfiles);
    }

    m_profileMenuAction->addSeparator();
    m_profileMenuAction->addAction(m_colorSpaceConverter);

    if (editor)
    {
        m_colorSpaceConverter->setEnabled(editor->actionEnabledState() &&
                                          IccSettings::instance()->isEnabled());
    }
}

void ProfileConversionToolPlugin::slotProfileConversionTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ProfileConversionTool* const tool = new ProfileConversionTool(this);
        tool->setPlugin(this);

        connect(tool, SIGNAL(okClicked()),
                this, SLOT(slotUpdateColorSpaceMenu()));

        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorProfileConversionToolPlugin
