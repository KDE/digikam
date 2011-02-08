/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewpage.moc"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>

// KDE includes

#include <kdialog.h>
#include <kconfig.h>
#include <kvbox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

namespace Digikam
{

class PreviewPage::PreviewPagePriv
{
public:

    PreviewPagePriv() :
        loadReduced(0),
        loadimage(0),
        previewBehavior(0)
    {
    }

    QRadioButton* loadReduced;
    QRadioButton* loadimage;

    QButtonGroup* previewBehavior;
};

PreviewPage::PreviewPage(KAssistantDialog* dlg)
    : AssistantDlgPage(dlg, i18n("<b>Configure Preview Behavior</b>")),
      d(new PreviewPagePriv)
{
    KVBox* vbox    = new KVBox(this);
    QLabel* label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here how images are displayed in preview mode and on the light table:</p>"
                         "</qt>"));

    QWidget* btns      = new QWidget(vbox);
    QVBoxLayout* vlay  = new QVBoxLayout(btns);

    d->previewBehavior = new QButtonGroup(btns);
    d->loadReduced     = new QRadioButton(btns);
    d->loadReduced->setText(i18n("Load reduced version of image"));
    d->loadReduced->setChecked(true);
    d->previewBehavior->addButton(d->loadReduced);

    d->loadimage = new QRadioButton(btns);
    d->loadimage->setText(i18n("Load image"));
    d->previewBehavior->addButton(d->loadimage);

    vlay->addWidget(d->loadReduced);
    vlay->addWidget(d->loadimage);
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(KDialog::spacingHint());

    QLabel* label2 = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> loading a reduced version of an image is faster but can differ "
                         "from the original, especially with Raw. In this case, a JPEG version "
                         "of Raw pre-processed by camera is loaded, instead of the real image data. This JPEG "
                         "image is embedded in the file metadata and is used by the camera to display "
                         "a Raw image faster to a TV screen.</p>"
                         "</qt>"));

    setPageWidget(vbox);

    QPixmap leftPix = KStandardDirs::locate("data","digikam/data/assistant-preview.png");
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));
}

PreviewPage::~PreviewPage()
{
    delete d;
}

void PreviewPage::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group        = config->group("Album Settings");
    group.writeEntry("Preview Load Full Image Size", d->loadimage->isChecked());

    group                     = config->group(QString("LightTable Settings"));
    group.writeEntry("Load Full Image size", d->loadimage->isChecked());

    config->sync();
}

}   // namespace Digikam
