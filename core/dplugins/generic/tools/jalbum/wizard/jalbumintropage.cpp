/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
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

#include "jalbumintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QGroupBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dbinarysearch.h"
#include "digikam_debug.h"
#include "jalbumjar.h"
#include "jalbumjava.h"
#include "jalbumwizard.h"
#include "jalbumsettings.h"
#include "dlayoutbox.h"

namespace DigikamGenericJAlbumPlugin
{

class Q_DECL_HIDDEN JAlbumIntroPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : imageGetOption(nullptr),
        hbox(nullptr),
        wizard(nullptr),
        settings(nullptr),
        iface(nullptr),
        binSearch(nullptr)
    {
        wizard = dynamic_cast<JAlbumWizard*>(dialog);

        if (wizard)
        {
            settings  = wizard->settings();
            iface = settings->m_iface;
        }
    }

    QComboBox*       imageGetOption;
    DHBox*           hbox;
    JAlbumWizard*    wizard;
    JAlbumSettings*  settings;
    DInfoInterface*  iface;
    DBinarySearch*   binSearch;
    JalbumJar        jalbumBin;
    JalbumJava       jalbumJava;
};

JAlbumIntroPage::JAlbumIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                       "<p><h1><b>Welcome to jAlbum album tool</b></h1></p>"
                       "<p>This assistant will guide you to export quickly</p><p></p>"
                       "<p>your images as a jAlbum project.</p>"
                       "</qt>"));

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(JAlbumSettings::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(JAlbumSettings::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    // --------------------

    QGroupBox* const binaryBox      = new QGroupBox(vbox);
    QGridLayout* const binaryLayout = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "jAlbum Binaries"));
    d->binSearch                    = new DBinarySearch(binaryBox);
    d->binSearch->addBinary(d->jalbumBin);
    d->binSearch->addBinary(d->jalbumJava);

    vbox->setStretchFactor(desc,      2);
    vbox->setStretchFactor(d->hbox,   1);
    vbox->setStretchFactor(binaryBox, 3);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("text-html")));

#ifdef Q_OS_WIN
    d->binSearch->addDirectory(QLatin1String(qgetenv("ProgramFiles").constData()) + QLatin1String("\\jAlbum\\"));
#else
    d->binSearch->addDirectory(QLatin1String("/usr/share/"));
    d->binSearch->addDirectory(QLatin1String("/usr/share/jAlbum/"));
    d->binSearch->addDirectory(QLatin1String("/usr/share/jalbum/"));
    d->binSearch->addDirectory(QLatin1String("/usr/lib/jalbum/"));
#endif

    connect(d->binSearch, SIGNAL(signalBinariesFound(bool)),
            this, SLOT(slotBinariesFound()));
}

JAlbumIntroPage::~JAlbumIntroPage()
{
    delete d;
}

void JAlbumIntroPage::initializePage()
{
    bool albumSupport = (d->iface && d->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(JAlbumSettings::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->settings->m_getOption);
    }

    d->binSearch->allBinariesFound();
    slotBinariesFound();
}

bool JAlbumIntroPage::validatePage()
{
    d->settings->m_getOption = (JAlbumSettings::ImageGetOption)d->imageGetOption->currentIndex();

    return true;
}

void JAlbumIntroPage::slotBinariesFound()
{
    d->settings->m_jalbumPath = d->jalbumBin.path();
    d->settings->m_javaPath   = d->jalbumJava.path();

    emit completeChanged();
}

bool JAlbumIntroPage::isComplete() const
{
    QString val = d->wizard->settings()->m_javaPath + d->wizard->settings()->m_jalbumPath;
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << val;

    return (!val.isEmpty());
}

} // namespace DigikamGenericJAlbumPlugin
