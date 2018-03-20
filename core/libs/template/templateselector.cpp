/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-23
 * Description : a widget to select metadata template.
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

#include "templateselector.h"

// Qt includes

#include <QAbstractItemView>
#include <QLabel>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "setup.h"
#include "template.h"
#include "templatemanager.h"
#include "squeezedcombobox.h"

namespace Digikam
{

class TemplateSelector::Private
{
public:

    Private()
    {
        label         = 0;
        setupButton   = 0;
        templateCombo = 0;
    }

    QLabel*           label;

    QToolButton*      setupButton;

    SqueezedComboBox* templateCombo;

    Template          metadataTemplate;
};

TemplateSelector::TemplateSelector(QWidget* const parent)
    : DHBox(parent), d(new Private)
{
    d->label         = new QLabel(i18n("Template: "), this);
    d->templateCombo = new SqueezedComboBox(this);
    d->setupButton   = new QToolButton(this);
    d->setupButton->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));
    d->setupButton->setWhatsThis(i18n("Open metadata template editor"));
    d->templateCombo->setWhatsThis(i18n("<p>Select here the action to perform using the metadata template.</p>"
                                        "<p><b>To remove</b>: delete already-assigned template.</p>"
                                        "<p><b>Do not change</b>: Do not touch template information.</p>"
                                        "<p>All other values are template titles managed by digiKam. "
                                        "Selecting one will assign information as well.</p>"));

    setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    setContentsMargins(QMargins());
    setStretchFactor(d->templateCombo, 10);

    connect(d->templateCombo, SIGNAL(activated(int)),
            this, SIGNAL(signalTemplateSelected()));

    connect(d->setupButton, SIGNAL(clicked()),
            this, SLOT(slotOpenSetup()));

    TemplateManager* tm = TemplateManager::defaultManager();

    if (tm)
    {
        connect(tm, SIGNAL(signalTemplateAdded(Template)),
                this, SLOT(slotTemplateListChanged()));

        connect(tm, SIGNAL(signalTemplateRemoved(Template)),
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
    d->templateCombo->insertSqueezedItem(i18n("To remove"),     REMOVETEMPLATE);
    d->templateCombo->insertSqueezedItem(i18n("Do not change"), DONTCHANGE);
    d->templateCombo->insertSeparator(DONTCHANGE + 1);

    TemplateManager* tm = TemplateManager::defaultManager();

    if (tm)
    {
        int i                 = DONTCHANGE + 2;
        QList<Template> list  = tm->templateList();

        foreach(const Template& t, list)
        {
            d->templateCombo->insertSqueezedItem(t.templateTitle(), i);
            ++i;
        }
    }
}

Template TemplateSelector::getTemplate() const
{
    switch (d->templateCombo->currentIndex())
    {
        case REMOVETEMPLATE:
        {
            Template t;
            t.setTemplateTitle(Template::removeTemplateTitle());
            return t;
        }

        case DONTCHANGE:
        {
            return Template();
        }

        default:
        {
            TemplateManager* tm = TemplateManager::defaultManager();

            if (tm)
            {
                return tm->fromIndex(d->templateCombo->currentIndex() - 3);
            }

            break;
        }
    }

    return Template();
}

void TemplateSelector::setTemplate(const Template& t)
{
    d->metadataTemplate = t;
    QString title       = d->metadataTemplate.templateTitle();

    if (title == Template::removeTemplateTitle())
    {
        d->templateCombo->setCurrentIndex(REMOVETEMPLATE);
    }
    else if (title.isEmpty())
    {
        d->templateCombo->setCurrentIndex(DONTCHANGE);
    }

    d->templateCombo->setCurrent(title);
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
    Setup::execTemplateEditor(this, d->metadataTemplate);
}

void TemplateSelector::slotTemplateListChanged()
{
    populateTemplates();
}

}  // namespace Digikam
