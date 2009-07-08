/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-04
 * Description : metadata template setup page.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setuptemplate.h"
#include "setuptemplate.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QFrame>

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kmessagebox.h>

// Local includes

#include "altlangstredit.h"
#include "templatelist.h"
#include "templatepanel.h"

namespace Digikam
{

class SetupTemplatePriv
{
public:

    SetupTemplatePriv()
    {
        addButton = 0;
        delButton = 0;
        repButton = 0;
        listView  = 0;
        tview     = 0;
        titleEdit = 0;
    }

    QPushButton   *addButton;
    QPushButton   *delButton;
    QPushButton   *repButton;

    KLineEdit     *titleEdit;

    TemplateList  *listView;

    TemplatePanel *tview;
};

SetupTemplate::SetupTemplate(QWidget* parent)
             : QScrollArea(parent), d(new SetupTemplatePriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QGridLayout* grid = new QGridLayout(panel);
    d->listView       = new TemplateList(panel);
    d->listView->setFixedHeight(100);

    // --------------------------------------------------------

    QLabel *label0 = new QLabel(i18n("Template Title:"), panel);
    d->titleEdit   = new KLineEdit(panel);
    d->titleEdit->setClearButtonShown(true);
    d->titleEdit->setClickMessage(i18n("Enter here the metadata template title."));
    d->titleEdit->setWhatsThis(i18n("<p>Enter here the metadata template title. This title will be "
                                    "used to identify a template in your collection.</p>"));
    label0->setBuddy(d->titleEdit);

    // --------------------------------------------------------

    d->tview = new TemplatePanel(panel);

    // --------------------------------------------------------

    QLabel *note = new QLabel(i18n("<b>Note: These information are used to set "
                   "<b><a href='http://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a></b> "
                   "and <b><a href='http://en.wikipedia.org/wiki/IPTC'>IPTC</a></b> tag contents. "
                   "There is no limitation with XMP, but note that IPTC text tags "
                   "only support the printable <b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                   "character set, and tag sizes are limited. "
                   "Use contextual help for details.</b>"), panel);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // -------------------------------------------------------------

    d->addButton = new QPushButton(panel);
    d->delButton = new QPushButton(panel);
    d->repButton = new QPushButton(panel);

    d->addButton->setText(i18n("&Add..."));
    d->addButton->setIcon(SmallIcon("list-add"));
    d->delButton->setText(i18n( "&Remove"));
    d->delButton->setIcon(SmallIcon("list-remove"));
    d->repButton->setText(i18n("&Replace..."));
    d->repButton->setIcon(SmallIcon("view-refresh"));
    d->delButton->setEnabled(false);
    d->repButton->setEnabled(false);

    // -------------------------------------------------------------

    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    grid->setAlignment(Qt::AlignTop);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(4, 10);
    grid->addWidget(d->listView,  0, 0, 4, 2);
    grid->addWidget(d->addButton, 0, 2, 1, 1);
    grid->addWidget(d->delButton, 1, 2, 1, 1);
    grid->addWidget(d->repButton, 2, 2, 1, 1);
    grid->addWidget(label0,       4, 0, 1, 1);
    grid->addWidget(d->titleEdit, 4, 1, 1, 1);
    grid->addWidget(d->tview,     5, 0, 1, 3);
    grid->addWidget(note,         6, 0, 1, 3);

    // --------------------------------------------------------

    connect(d->listView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddTemplate()));

    connect(d->delButton, SIGNAL(clicked()),
            this, SLOT(slotDelTemplate()));

    connect(d->repButton, SIGNAL(clicked()),
            this, SLOT(slotRepTemplate()));

    // --------------------------------------------------------

    readSettings();
    d->titleEdit->setFocus();

}

SetupTemplate::~SetupTemplate()
{
    delete d;
}

void SetupTemplate::applySettings()
{
    d->listView->applySettings();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Setup Dialog"));
    group.writeEntry("Template Tab", (int)(d->tview->currentIndex()));
    config->sync();
}

void SetupTemplate::readSettings()
{
    d->listView->readSettings();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Setup Dialog"));
    d->tview->setCurrentIndex((TemplatePanel::TemplateTab)group.readEntry("Template Tab", (int)TemplatePanel::RIGHTS));
}

void SetupTemplate::slotSelectionChanged()
{
    TemplateListItem *item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());
    if (!item)
    {
        d->delButton->setEnabled(false);
        d->repButton->setEnabled(false);
        return;
    }
    d->delButton->setEnabled(true);
    d->repButton->setEnabled(true);
    populateTemplate(item->getTemplate());
}

void SetupTemplate::populateTemplate(const Template& t)
{
    d->tview->setTemplate(t);
    d->titleEdit->setText(t.templateTitle());
    d->titleEdit->setFocus();
}

void SetupTemplate::slotAddTemplate()
{
    QString title = d->titleEdit->text();

    if (title.isEmpty())
    {
        KMessageBox::error(this, i18n("Cannot register new metadata template without title."));
        return;
    }

    if (d->listView->contains(title))
    {
        KMessageBox::error(this, i18n("A metadata template named '%1' already exist.", title));
        return;
    }

    d->tview->apply();

    Template t = d->tview->getTemplate();
    t.setTemplateTitle(d->titleEdit->text());
    new TemplateListItem(d->listView, t);
}

void SetupTemplate::slotDelTemplate()
{
    TemplateListItem *item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());
    if (item) delete item;
}

void SetupTemplate::slotRepTemplate()
{
    QString title = d->titleEdit->text();

    if (title.isEmpty())
    {
        KMessageBox::error(this, i18n("Cannot register new metadata template without title."));
        return;
    }

    TemplateListItem *item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());
    if (!item) return;

    d->tview->apply();

    Template t = d->tview->getTemplate();
    t.setTemplateTitle(title);
    item->setTemplate(t);
}

}  // namespace Digikam
