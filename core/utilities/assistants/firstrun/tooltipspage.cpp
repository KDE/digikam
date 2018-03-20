/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"

namespace Digikam
{

class TooltipsPage::Private
{
public:

    explicit Private()
      : showTooltips(0),
        hideTooltips(0),
        tooltipsBehavior(0)
    {
    }

    QRadioButton* showTooltips;
    QRadioButton* hideTooltips;
    QButtonGroup* tooltipsBehavior;
};

TooltipsPage::TooltipsPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Enabled Contextual Tooltips</b>")),
      d(new Private)
{
    const int spacing    = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    DVBox* const vbox    = new DVBox(this);
    QLabel* const label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here if you want to show contextual tooltips in icon-view and folder-view:</p>"
                         "</qt>"));

    QWidget* const btns     = new QWidget(vbox);
    QVBoxLayout* const vlay = new QVBoxLayout(btns);

    d->tooltipsBehavior = new QButtonGroup(btns);
    d->hideTooltips     = new QRadioButton(btns);
    d->hideTooltips->setText(i18n("Do not show tooltips"));
    d->hideTooltips->setChecked(true);
    d->tooltipsBehavior->addButton(d->hideTooltips);

    d->showTooltips     = new QRadioButton(btns);
    d->showTooltips->setText(i18n("Use Tooltips"));
    d->tooltipsBehavior->addButton(d->showTooltips);

    vlay->addWidget(d->hideTooltips);
    vlay->addWidget(d->showTooltips);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    QLabel* const label2 = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> tooltips show photograph and digiKam metadata on the fly, "
                         "as the mouse moves over items. This can be useful when selecting items. "
                         "Tooltips are displayed in the album folder view, "
                         "album icon view, camera icon view, batch queue list, and thumb bar. "
                         "From the digiKam configuration dialog, you can customize the contents of these "
                         "tooltips and the fonts used.</p>"
                         "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("dialog-information")));
}

TooltipsPage::~TooltipsPage()
{
    delete d;
}

void TooltipsPage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Album Settings"));

    group.writeEntry(QLatin1String("Show ToolTips"),       d->showTooltips->isChecked());
    group.writeEntry(QLatin1String("Show Album ToolTips"), d->showTooltips->isChecked());

    config->sync();
}

}   // namespace Digikam
