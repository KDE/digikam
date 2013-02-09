/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-12-19
 * Description : Workflow properties dialog.
 *
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "workflowdlg.moc"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRegExp>
#include <QValidator>

// KDE includes

#include <khbox.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kstandarddirs.h>
#include <kseparator.h>
#include <klocale.h>

namespace Digikam
{

class WorkflowDlg::Private
{

public:

    Private() :
        titleEdit(0),
        descEdit(0)
    {
    }

    KLineEdit* titleEdit;
    KLineEdit* descEdit;
};

WorkflowDlg::WorkflowDlg(const Workflow& wf, bool create)
    : KDialog(0), d(new Private)
{
    setCaption(create ? i18n("New Workflow") : i18n("Edit Workflow"));
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("workflowdlg.anchor", "digikam");

    QWidget* const page    = new QWidget(this);
    QLabel* const logo     = new QLabel(page);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                    .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* const topLabel = new QLabel(page);

    if (create)
    {
        topLabel->setText(i18n("<qt><b>Create new Workflow</b></qt>"));
    }
    else
    {
        topLabel->setText(i18n("<qt><b>Workflow Properties</b></qt>"));
    }

    topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    topLabel->setWordWrap(false);

    KSeparator* const topLine = new KSeparator(Qt::Horizontal);

    // --------------------------------------------------------

    QRegExp           reg("[^/]+");
    QValidator* const validator = new QRegExpValidator(reg, this);

    QLabel* const titleLabel = new QLabel(page);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new KLineEdit(page);
    d->titleEdit->setClearButtonShown(true);
    d->titleEdit->setValidator(validator);
    d->titleEdit->selectAll();
    d->titleEdit->setFocus();
    titleLabel->setBuddy(d->titleEdit);

    // --------------------------------------------------------

    QLabel* const descLabel = new QLabel(page);
    descLabel->setText(i18n("Description:"));

    d->descEdit = new KLineEdit(page);
    d->titleEdit->setClearButtonShown(true);
    d->descEdit->setValidator(validator);
    descLabel->setBuddy(d->descEdit);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout();
    grid->addWidget(logo,         0, 0, 1, 1);
    grid->addWidget(topLabel,     0, 1, 1, 1);
    grid->addWidget(topLine,      1, 0, 1, 2);
    grid->addWidget(titleLabel,   2, 0, 1, 1);
    grid->addWidget(d->titleEdit, 2, 1, 1, 1);
    grid->addWidget(descLabel,    3, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    grid->addWidget(d->descEdit,  3, 1, 1, 1);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());
    page->setLayout(grid);

    if (create)
    {
        d->titleEdit->setText(i18n("New Workflow"));
    }
    else
    {
        d->titleEdit->setText(wf.title);
        d->descEdit->setText(wf.desc);
    }

    // -- slots connections -------------------------------------------

    connect(d->titleEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotTitleChanged(QString)));

    // --------------------------------------------------------

    setMainWidget(page);
}

WorkflowDlg::~WorkflowDlg()
{
    delete d;
}

QString WorkflowDlg::title() const
{
    return d->titleEdit->text();
}

QString WorkflowDlg::description() const
{
    return d->descEdit->text();
}

bool WorkflowDlg::editProps(Workflow& wf)
{
    QPointer<WorkflowDlg> dlg = new WorkflowDlg(wf);
    bool ok                   = (dlg->exec() == QDialog::Accepted);

    if (ok)
    {
        wf.title = dlg->title();
        wf.desc  = dlg->description();
    }

    delete dlg;
    return ok;
}

bool WorkflowDlg::createNew(Workflow& wf)
{
    QPointer<WorkflowDlg> dlg = new WorkflowDlg(wf, true);
    bool ok                   = (dlg->exec() == QDialog::Accepted);

    if (ok)
    {
        wf.title = dlg->title();
        wf.desc  = dlg->description();
    }

    delete dlg;
    return ok;
}

void WorkflowDlg::slotTitleChanged(const QString& text)
{
    Workflow wf = WorkflowManager::instance()->findByTitle(text);
    bool enable = (wf.title.isEmpty() && !text.isEmpty());
    enableButtonOk(enable);
}

}  // namespace Digikam
