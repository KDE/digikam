/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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
#include "dimageslist.h"
#include "imageshacksession.h"

namespace Digikam
{

class ImageShackWidget::Private
{
public:

    Private()
    {
        imgList            = 0;
        iface              = 0;
        session            = 0;
        headerLbl          = 0;
        accountNameLbl     = 0;
        tagsFld            = 0;
        privateImagesChb   = 0;
        remBarChb          = 0;
        chgRegCodeBtn      = 0;
        reloadGalleriesBtn = 0;
        galleriesCob       = 0;
        progressBar        = 0;
    }

    DImagesList*       imgList;
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

}  // namespace Digikam
