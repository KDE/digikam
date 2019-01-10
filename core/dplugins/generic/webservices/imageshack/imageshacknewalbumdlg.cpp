/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-01
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2010      by Jens Mueller <tschenser at gmx dot de>
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

#include "imageshacknewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QIcon>
#include <QApplication>
#include <QPushButton>

namespace Digikam
{

ImageShackNewAlbumDlg::ImageShackNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300, 0);
}

ImageShackNewAlbumDlg::~ImageShackNewAlbumDlg()
{
}

QString ImageShackNewAlbumDlg::getAlbumTitle()
{
    return getTitleEdit()->text();
}

} // namespace Digikam
