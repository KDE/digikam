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

#include "openfilepage.moc"

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

// Local settings.

#include "albumsettings.h"

namespace Digikam
{

class OpenFilePage::OpenFilePagePriv
{
public:

    OpenFilePagePriv() :
        openAsPreview(0),
        openInEditor(0),
        openFileBehavior(0)
    {
    }

    QRadioButton* openAsPreview;
    QRadioButton* openInEditor;

    QButtonGroup* openFileBehavior;
};

OpenFilePage::OpenFilePage(KAssistantDialog* dlg)
    : AssistantDlgPage(dlg, i18n("<b>Configure Open File Behavior</b>")),
      d(new OpenFilePagePriv)
{
    KVBox* vbox    = new KVBox(this);
    QLabel* label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Specify how images should be opened when right-clicked on in the icon view:</p>"
                         "</qt>"));

    QWidget* btns       = new QWidget(vbox);
    QVBoxLayout* vlay   = new QVBoxLayout(btns);

    d->openFileBehavior = new QButtonGroup(btns);
    d->openAsPreview    = new QRadioButton(btns);
    d->openAsPreview->setText(i18n("Open a preview"));
    d->openAsPreview->setChecked(true);
    d->openFileBehavior->addButton(d->openAsPreview);

    d->openInEditor = new QRadioButton(btns);
    d->openInEditor->setText(i18n("Open in the editor"));
    d->openFileBehavior->addButton(d->openInEditor);

    vlay->addWidget(d->openAsPreview);
    vlay->addWidget(d->openInEditor);
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(KDialog::spacingHint());

    QLabel* label2 = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> using a preview is always faster than using the editor, "
                         "especially when checking a series of shots. However, you cannot change or fix the image "
                         "in preview mode. "
                         "Note that if you want to compare images quickly, it is often better to use the light table: "
                         "images can be displayed side-by-side, and synchronized zooming and panning can be performed.</p>"
                         "</qt>"));

    setPageWidget(vbox);

    QPixmap leftPix = KStandardDirs::locate("data","digikam/data/assistant-openfile.png");
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));
}

OpenFilePage::~OpenFilePage()
{
    delete d;
}

void OpenFilePage::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Album Settings");
    group.writeEntry("Item Left Click Action", (int)(d->openInEditor->isChecked() ?
                                                     AlbumSettings::StartEditor : AlbumSettings::ShowPreview));

    config->sync();
}

}   // namespace Digikam
