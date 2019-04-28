/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_IMAGE_SHACK_WIDGET_PRIVATE_H
#define DIGIKAM_IMAGE_SHACK_WIDGET_PRIVATE_H

#include "imageshackwidget.h"

// Qt includes

#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QStringList>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QApplication>
#include <QPushButton>
#include <QMimeDatabase>
#include <QMimeType>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "ditemslist.h"
#include "imageshacksession.h"

using namespace Digikam;

namespace DigikamGenericImageShackPlugin
{

class Q_DECL_HIDDEN ImageShackWidget::Private
{
public:

    Private()
    {
        imgList            = nullptr;
        iface              = nullptr;
        session            = nullptr;
        headerLbl          = nullptr;
        accountNameLbl     = nullptr;
        tagsFld            = nullptr;
        privateImagesChb   = nullptr;
        remBarChb          = nullptr;
        chgRegCodeBtn      = nullptr;
        reloadGalleriesBtn = nullptr;
        galleriesCob       = nullptr;
        progressBar        = nullptr;
    }

    DItemsList*       imgList;
    DInfoInterface*    iface;
    ImageShackSession* session;

    QLabel*            headerLbl;
    QLabel*            accountNameLbl;

    QLineEdit*         tagsFld;

    QCheckBox*         privateImagesChb;
    QCheckBox*         remBarChb;

    QPushButton*       chgRegCodeBtn;
    QPushButton*       reloadGalleriesBtn;

    QComboBox*         galleriesCob;

    DProgressWdg*      progressBar;
};

} // namespace DigikamGenericImageShackPlugin

#endif // DIGIKAM_IMAGE_SHACK_WIDGET_PRIVATE_H
