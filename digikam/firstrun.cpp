/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-25
 * Description : a widget to use in first run dialog
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kseparator.h>

// Local includes.

#include "firstrun.h"
#include "firstrun.moc"

namespace Digikam
{

FirstRunWidget::FirstRunWidget( QWidget* parent )
              : QWidget( parent )
{
    setObjectName("FirstRunWidget");

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    m_textLabel2         = new QLabel(this);
    m_textLabel2->setText(i18n( "<b>Local Root Collection Path</b>"));

    KSeparator *line1    = new KSeparator(Qt::Horizontal, this);
    QGridLayout *grid    = new QGridLayout();
    m_pixLabel           = new QLabel(this);
    m_pixLabel->setAlignment(Qt::AlignTop);

    m_path       = new KUrlRequester(this);
    m_path->setMode(KFile::LocalOnly | KFile::Directory);

    m_textLabel1 = new QLabel(this);
    m_textLabel1->setAlignment(Qt::AlignVCenter);
    m_textLabel1->setWordWrap(true);
    m_textLabel1->setText(i18n("<p>digiKam use root collection path to store the photo albums "
                               "which you create in <b>My Albums</b> view from left side-bar. "
                               "Below, please select which folder you would like "
                               "digiKam to use as first root collection path from your local "
                               "file system.</p>" 
                               "<p><b>Note: you can set other root collection path later using "
                               "settings panel. Removable media and shared file system are "
                               "supported.</b></p>") );

    grid->addWidget(m_pixLabel,   0, 0, 2, 1);
    grid->addWidget(m_textLabel1, 0, 1, 1, 1);
    grid->addWidget(m_path,       1, 1, 1, 1);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    vlayout->addWidget(m_textLabel2);
    vlayout->addWidget(line1);
    vlayout->addLayout(grid);
    vlayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                     QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    vlayout->setMargin(0);
    vlayout->setSpacing(KDialog::spacingHint());
}

FirstRunWidget::~FirstRunWidget()
{
}

}  // namespace Digikam
