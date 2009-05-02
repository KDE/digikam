/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "openfilepage.h"
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
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// Local settings.

#include "albumsettings.h"

namespace Digikam
{

class OpenFilePagePriv
{
public:

    OpenFilePagePriv()
    {
        openAsPreview    = 0;
        openInEditor     = 0;
        openFileBehavour = 0;
    }

    QRadioButton *openAsPreview;
    QRadioButton *openInEditor;

    QButtonGroup *openFileBehavour;
};

OpenFilePage::OpenFilePage(KAssistantDialog* dlg)
            : AssistantDlgPage(dlg, i18n("<b>Configure Open File Behavior</b>")), 
              d(new OpenFilePagePriv)
{
    KVBox *vbox    = new KVBox(this);
    QLabel *label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Configure Open File Behavior.</p>"
                         "<p>Set here how images are open when user right-click on items from icon-view:</p>"
                         "</qt>"));

    QWidget *btns      = new QWidget(vbox);
    QVBoxLayout *vlay  = new QVBoxLayout(btns);

    d->openFileBehavour = new QButtonGroup(btns);
    d->openAsPreview    = new QRadioButton(btns);
    d->openAsPreview->setText(i18n("Open as preview"));
    d->openAsPreview->setChecked(true);
    d->openFileBehavour->addButton(d->openAsPreview);

    d->openInEditor = new QRadioButton(btns);
    d->openInEditor->setText(i18n("Load on editor"));
    d->openFileBehavour->addButton(d->openInEditor);

    vlay->addWidget(d->openAsPreview);
    vlay->addWidget(d->openInEditor);
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(KDialog::spacingHint());

    QLabel *label2 = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> using preview is always faster than to use editor, "
                         "especially to check a shots serie. But, you cannot change or fix image in "
                         "preview mode. "
                         "Anyway, if you to compare images quicly, the better way is to use light table. "
                         "where you can display images side by side and perform synchronized zoom and pan.</p>"
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

    KConfigGroup group = config->group("Album Settings");
    group.writeEntry("Item Right Click Action", d->openInEditor->isChecked() ? 
                                                AlbumSettings::StartEditor : AlbumSettings::ShowPreview);

    config->sync();
}

}   // namespace Digikam
