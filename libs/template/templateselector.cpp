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

#include <QLabel>
#include <QToolButton>

// KDE includes

#include <kglobalsettings.h>
#include <klocale.h>
#include <kdialog.h>
#include <kiconloader.h>

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
    d->templateCombo->setWhatsThis(i18n("Select here your metadata template to use."));

    setSpacing(KDialog::spacingHint());
    setMargin(0);
    setStretchFactor(d->templateCombo, 10);

    connect(d->templateCombo, SIGNAL(activated(int)),
            this, SLOT(slotChangeTemplate(int)));

    connect(d->setupButton, SIGNAL(clicked()),
            this, SLOT(slotOpenSetup()));

    populateTemplates();
}

TemplateSelector::~TemplateSelector()
{
    delete d;
}

void TemplateSelector::populateTemplates()
{
    d->templateCombo->clear();
    d->templateCombo->insertSqueezedItem(i18n("None"), 0);

    TemplateManager* tm = TemplateManager::defaultManager();
    if (tm)
    {
        int i                  = 1;
        QList<Template*>* list = tm->templateList();

        foreach (Template *t, *list)
        {
            d->templateCombo->insertSqueezedItem(t->templateTitle(), i);
            ++i;
        }
    }
}

Template* TemplateSelector::getTemplate() const
{
    TemplateManager* tm = TemplateManager::defaultManager();
    if (tm)
    {
        if (d->templateCombo->currentIndex() > 0)
            return tm->fromIndex(d->templateCombo->currentIndex()-1);
    }
    return 0;
}

void TemplateSelector::setTemplate(Template *t)
{
    if (t)
        d->templateCombo->setCurrent(t->templateTitle());
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
    if (Setup::execSinglePage(this, Setup::TemplatePage))
    {
        populateTemplates();
        emit signalTemplateChanged();
    }
}

void TemplateSelector::slotChangeTemplate(int)
{
    emit signalTemplateChanged();
}

}  // namespace Digikam
