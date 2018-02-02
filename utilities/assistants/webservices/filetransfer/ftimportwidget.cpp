/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 04.10.2009
 * Description : main widget of the import dialog
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "ftimportwidget.h"

// Qt includes

#include <QBoxLayout>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimageslist.h"

namespace Digikam
{

FTImportWidget::FTImportWidget(QWidget* const parent, DInfoInterface* const iface)
    : QWidget(parent)
{
    // setup image list
    m_imageList = new DImagesList(this);
    m_imageList->setAllowRAW(true);
    m_imageList->setIface(iface);
    m_imageList->listView()->setWhatsThis(i18n("This is the list of images to import "
                                               "into the current album."));

    // setup upload widget
    m_uploadWidget            = iface->albumSelector(this);

    // layout dialog
    QVBoxLayout* const layout = new QVBoxLayout(this);

    layout->addWidget(m_imageList);
    layout->addWidget(m_uploadWidget);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
}

FTImportWidget::~FTImportWidget()
{
}

DImagesList* FTImportWidget::imagesList() const
{
    return m_imageList;
}

QWidget* FTImportWidget::uploadWidget() const
{
    return m_uploadWidget;
}

QList<QUrl> FTImportWidget::sourceUrls() const
{
    return m_imageList->imageUrls();
}

} // namespace Digikam
