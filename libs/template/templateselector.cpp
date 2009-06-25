/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-23
 * Description : a widget to select metadata template.
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

#include "templateselector.h"
#include "templateselector.moc"

// Qt includes

#include <QAbstractItemView>
#include <QLabel>
#include <QToolButton>

// KDE includes

#include <kglobalsettings.h>
#include <klocale.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/squeezedcombobox.h>

// Local includes

#include "setup.h"
#include "template.h"
#include "templatemanager.h"

using namespace KDcrawIface;

namespace Digikam
{

class TemplateSelectorPriv
{
public:

    TemplateSelectorPriv()
    {
        label         = 0;
        setupButton   = 0;
        templateCombo = 0;
    }

    QLabel           *label;

    QToolButton      *setupButton;

    SqueezedComboBox *templateCombo;
};

TemplateSelector::TemplateSelector(QWidget* parent=0)
                : KHBox(parent), d(new TemplateSelectorPriv)
{
    d->label         = new QLabel(i18n("Template: "), this);
    d->templateCombo = new SqueezedComboBox(this);
    d->setupButton   = new QToolButton(this);
    d->setupButton->setIcon(SmallIcon("document-edit"));
    d->setupButton->setWhatsThis(i18n("Open metadata template editor"));
    d->templateCombo->setWhatsThis(i18n("<p>Select here the action to do with metadata template.</p>"
                                        "<p><b>To remove</b>: delete template already assigned.</p>"
                                        "<p><b>Don't change</b>: Do not touch template information.</p>"
                                        "<p>All other values are template titles managed by digiKam. "
                                        "Select one will assign information as well.</p>"));

    setSpacing(KDialog::spacingHint());
    setMargin(0);
    setStretchFactor(d->templateCombo, 10);

    connect(d->templateCombo, SIGNAL(activated(int)),
            this, SIGNAL(signalTemplateSelected()));

    connect(d->setupButton, SIGNAL(clicked()),
            this, SLOT(slotOpenSetup()));

    TemplateManager* tm = TemplateManager::defaultManager();
    if (tm)
    {
        connect(tm, SIGNAL(signalTemplateAdded(const Template &)),
                this, SLOT(slotTemplateListChanged()));

        connect(tm, SIGNAL(signalTemplateRemoved(const Template &)),
                this, SLOT(slotTemplateListChanged()));
    }

    populateTemplates();
}

TemplateSelector::~TemplateSelector()
{
    delete d;
}

void TemplateSelector::populateTemplates()
{
    d->templateCombo->clear();
    d->templateCombo->insertSqueezedItem(i18n("To remove"), 0);
    d->templateCombo->insertSqueezedItem(i18n("Don't change"), 1);
    d->templateCombo->insertSeparator(2);

    TemplateManager* tm = TemplateManager::defaultManager();
    if (tm)
    {
        int i                  = 3;
        QList<Template> list  = tm->templateList();

        foreach (const Template &t, list)
        {
            d->templateCombo->insertSqueezedItem(t.templateTitle(), i);
            ++i;
        }
    }
}

Template TemplateSelector::getTemplate() const
{
    TemplateManager* tm = TemplateManager::defaultManager();
    if (tm)
    {
        switch(d->templateCombo->currentIndex())
        {
            case 0:
                return tm->removeTemplate();
                break;
            case 1:
                return tm->unknowTemplate();
                break;
            default:
                return tm->fromIndex(d->templateCombo->currentIndex()-3);
                break;
        }
    }
    return Template();
}

void TemplateSelector::setTemplate(const Template &t)
{
    TemplateManager* tm = TemplateManager::defaultManager();
    if (tm)
    {
        if (!t.isNull())
        {
            QString title = t.templateTitle();
            if (title == tm->removeTemplate().templateTitle())
                return d->templateCombo->setCurrentIndex(0);
            else if (title == tm->unknowTemplate().templateTitle())
                return d->templateCombo->setCurrentIndex(1);
            else
                return d->templateCombo->setCurrent(title);
        }
    }

    return d->templateCombo->setCurrentIndex(1);
}

int TemplateSelector::getTemplateIndex() const
{
    return d->templateCombo->currentIndex();
}

void TemplateSelector::setTemplateIndex(int i)
{
    d->templateCombo->setCurrentIndex(i);
}

void TemplateSelector::slotOpenSetup()
{
    Setup::execSinglePage(this, Setup::TemplatePage);
}

void TemplateSelector::slotTemplateListChanged()
{
    populateTemplates();
}

}  // namespace Digikam
