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

#include "tooltipspage.h"
#include "tooltipspage.moc"

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

namespace Digikam
{

class TooltipsPagePriv
{
public:

    TooltipsPagePriv()
    {
        showTooltips     = 0;
        hideTooltips     = 0;
        tooltipsBehavour = 0;
    }

    QRadioButton *showTooltips;
    QRadioButton *hideTooltips;

    QButtonGroup *tooltipsBehavour;
};

TooltipsPage::TooltipsPage(KAssistantDialog* dlg)
            : AssistantDlgPage(dlg, i18n("<b>Enabled Contextual Tooltips</b>")), 
              d(new TooltipsPagePriv)
{
    KVBox *vbox    = new KVBox(this);
    QLabel *label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Enabled Contextual Tooltips.</p>"
                         "<p>Set here if you want to show contextual tooltips in icon-view and folder-view:</p>"
                         "</qt>"));

    QWidget *btns       = new QWidget(vbox);
    QVBoxLayout *vlay   = new QVBoxLayout(btns);

    d->tooltipsBehavour = new QButtonGroup(btns);
    d->hideTooltips     = new QRadioButton(btns);
    d->hideTooltips->setText(i18n("Do not show tooltips"));
    d->hideTooltips->setChecked(true);
    d->tooltipsBehavour->addButton(d->hideTooltips);

    d->showTooltips     = new QRadioButton(btns);
    d->showTooltips->setText(i18n("Use Tooltips"));
    d->tooltipsBehavour->addButton(d->showTooltips);

    vlay->addWidget(d->hideTooltips);
    vlay->addWidget(d->showTooltips);
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(KDialog::spacingHint());

    QLabel *label2 = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> tooltips show photograph and digiKam meta-data on the fly, "
                         "accordingly with mouse move over items. This can be usefull to make items "
                         "selection. Tooltips are displayed over album folder-view, "
                         "album icon-view, camera icon-view, batch queue list, and thumb-bar. "
                         "From digiKam config dialog, you can customize tooltips contents, "
                         "and fonts.</p>"
                         "</qt>"));

    setPageWidget(vbox);

    QPixmap leftPix = KStandardDirs::locate("data","digikam/data/assistant-tooltips.png");
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation)); 
}

TooltipsPage::~TooltipsPage()
{
    delete d;
}

void TooltipsPage::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Album Settings");

    group.writeEntry("Show ToolTips",       d->showTooltips->isChecked());
    group.writeEntry("Show Album ToolTips", d->showTooltips->isChecked());

    config->sync();
}

}   // namespace Digikam
