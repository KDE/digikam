/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef HTML_WIZARD_H
#define HTML_WIZARD_H

// Local includes

#include "dwizarddlg.h"
#include "gallerytheme.h"
#include "album.h"

namespace Digikam
{

class GalleryInfo;

/**
 * The wizard used by the user to select the various settings.
 */
class HTMLWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit HTMLWizard(QWidget* const parent);
    ~HTMLWizard();

    int parametersPageId()                             const;
    int imageSettingsPageId()                          const;

    GalleryInfo*      galleryInfo()                    const;
    GalleryTheme::Ptr theme()                          const;
    QWidget* parametersWidget(const QByteArray& iname) const;
    void updateSettings();

private Q_SLOTS:

    void slotThemeSelectionChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // HTML_WIZARD_H
