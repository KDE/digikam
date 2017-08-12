/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 *
 * Copyright (C) 2011-2012 by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "NewCanvasDialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTableView>
#include <QLabel>
#include <QPrinter>
#include <QStackedLayout>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QButtonGroup>
#include <QPushButton>
#include <QIcon>
#include <QStandardPaths>
#include <QDialogButtonBox>

#include <klocalizedstring.h>

#include "CanvasSizeWidget.h"
#include "TemplatesView.h"
#include "TemplatesModel.h"
#include "CanvasSize.h"

#define PAPER_SIZE_ROLE 128

namespace PhotoLayoutsEditor
{

class NewCanvasDialog::Private
{
    Private() :
        stack(0),
        paperSize(0),
        templatesList(0),
        canvasSize(0),
        horizontalButton(0),
        verticalButton(0),
        orientationGroup(0)
    {
        QListWidgetItem* temp = new QListWidgetItem(QLatin1String("Custom"));
        temp->setData(PAPER_SIZE_ROLE, -1);
        paperSizes.append(temp);

        names.insert( QPrinter::A0, QPair<QString,QString>(QLatin1String("A0"), QLatin1String("a0")));
        names.insert( QPrinter::A1, QPair<QString,QString>(QLatin1String("A1"), QLatin1String("a1")));
        names.insert( QPrinter::A2, QPair<QString,QString>(QLatin1String("A2"), QLatin1String("a2")));
        names.insert( QPrinter::A3, QPair<QString,QString>(QLatin1String("A3"), QLatin1String("a3")));
        names.insert( QPrinter::A4, QPair<QString,QString>(QLatin1String("A4"), QLatin1String("a4")));
        names.insert( QPrinter::A5, QPair<QString,QString>(QLatin1String("A5"), QLatin1String("a5")));
        names.insert( QPrinter::A6, QPair<QString,QString>(QLatin1String("A6"), QLatin1String("a6")));
        names.insert( QPrinter::A7, QPair<QString,QString>(QLatin1String("A7"), QLatin1String("a7")));
        names.insert( QPrinter::A8, QPair<QString,QString>(QLatin1String("A8"), QLatin1String("a8")));
        names.insert( QPrinter::A9, QPair<QString,QString>(QLatin1String("A9"), QLatin1String("a9")));
        names.insert( QPrinter::B0, QPair<QString,QString>(QLatin1String("B0"), QLatin1String("b0")));
        names.insert( QPrinter::B1, QPair<QString,QString>(QLatin1String("B1"), QLatin1String("b1")));
        names.insert( QPrinter::B2, QPair<QString,QString>(QLatin1String("B2"), QLatin1String("b2")));
        names.insert( QPrinter::B3, QPair<QString,QString>(QLatin1String("B3"), QLatin1String("b3")));
        names.insert( QPrinter::B4, QPair<QString,QString>(QLatin1String("B4"), QLatin1String("b4")));
        names.insert( QPrinter::B5, QPair<QString,QString>(QLatin1String("B5"), QLatin1String("b5")));
        names.insert( QPrinter::B6, QPair<QString,QString>(QLatin1String("B6"), QLatin1String("b6")));
        names.insert( QPrinter::B7, QPair<QString,QString>(QLatin1String("B7"), QLatin1String("b7")));
        names.insert( QPrinter::B8, QPair<QString,QString>(QLatin1String("B8"), QLatin1String("b8")));
        names.insert( QPrinter::B9, QPair<QString,QString>(QLatin1String("B9"), QLatin1String("b9")));
        names.insert( QPrinter::B10, QPair<QString,QString>(QLatin1String("B10"), QLatin1String("b10")));
        names.insert( QPrinter::C5E, QPair<QString,QString>(QLatin1String("C5E"), QLatin1String("c5e")));
        names.insert( QPrinter::DLE, QPair<QString,QString>(QLatin1String("DLE"), QLatin1String("dle")));
        names.insert( QPrinter::Executive, QPair<QString,QString>(QLatin1String("Executive"), QLatin1String("executive")));
        names.insert( QPrinter::Folio, QPair<QString,QString>(QLatin1String("Folio"), QLatin1String("folio")));
        names.insert( QPrinter::Ledger, QPair<QString,QString>(QLatin1String("Ledger"), QLatin1String("ledger")));
        names.insert( QPrinter::Legal, QPair<QString,QString>(QLatin1String("Legal"), QLatin1String("legal")));
        names.insert( QPrinter::Letter, QPair<QString,QString>(QLatin1String("Letter"), QLatin1String("letter")));
        names.insert( QPrinter::Tabloid, QPair<QString,QString>(QLatin1String("Tabloid"), QLatin1String("tabloid")));

        QStringList sl = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/templates/"));

        if (sl.count() == 0)
            return;

        QString dir = sl.first();
        for (QMap<int, QPair<QString,QString> >::iterator pair = names.begin(); pair != names.end(); ++pair)
        {
            QString tmp = dir + pair->second;
            QDir dv(tmp + QLatin1String("/v"));
            if (dv.exists() && dv.entryList(QStringList() << QLatin1String("*.ple"), QDir::Files).count())
            {
                QListWidgetItem * temp = new QListWidgetItem(pair->first);
                temp->setData(PAPER_SIZE_ROLE, pair.key());
                paperSizes.append(temp);
                continue;
            }
            QDir dh(tmp + QLatin1String("/h"));
            if (dh.exists() && dh.entryList(QStringList() << QLatin1String("*.ple"), QDir::Files).count())
            {
                QListWidgetItem * temp = new QListWidgetItem(pair->first);
                temp->setData(PAPER_SIZE_ROLE, pair.key());
                paperSizes.append(temp);
                continue;
            }
        }
    }

    QStackedLayout * stack;
    QListWidget * paperSize;
    TemplatesView * templatesList;
    CanvasSizeWidget * canvasSize;

    QPushButton * horizontalButton;
    QPushButton * verticalButton;
    QButtonGroup * orientationGroup;

    QList<QListWidgetItem *> paperSizes;

    QMap<int, QPair<QString,QString> > names;

    friend class NewCanvasDialog;
};

NewCanvasDialog::NewCanvasDialog(QWidget *parent) :
    QDialog(parent),
    d(new Private)
{
    setupUI();
}

NewCanvasDialog::~NewCanvasDialog()
{
    delete d;
}

bool NewCanvasDialog::hasTemplateSelected() const
{
    return (d->stack->currentWidget() == d->templatesList);
}

QString NewCanvasDialog::templateSelected() const
{
    return d->templatesList->selectedPath();
}

CanvasSize NewCanvasDialog::canvasSize() const
{
    if (d->stack->currentWidget() == d->canvasSize)
        return d->canvasSize->canvasSize();
    else
    {
        int w = 0;
        int h = 0;
        switch (d->paperSize->currentItem()->data(PAPER_SIZE_ROLE).toInt())
        {
            case QPrinter::A0:
                w = 841; h = 1189;
                break;
            case QPrinter::A1:
                w = 594; h = 841;
                break;
            case QPrinter::A2:
                w = 420; h = 594;
                break;
            case QPrinter::A3:
                w = 297; h = 420;
                break;
            case QPrinter::A4:
                w = 210; h = 297;
                break;
            case QPrinter::A5:
                w = 148; h = 210;
                break;
            case QPrinter::A6:
                w = 105; h = 148;
                break;
            case QPrinter::A7:
                w = 74; h = 105;
                break;
            case QPrinter::A8:
                w = 52; h = 74;
                break;
            case QPrinter::A9:
                w = 37; h = 52;
                break;
            case QPrinter::B0:
                w = 1030; h = 1456 ;
                break;
            case QPrinter::B1:
                w = 728; h = 1030;
                break;
            case QPrinter::B2:
                w = 515; h = 728;
                break;
            case QPrinter::B3:
                w = 364; h = 515;
                break;
            case QPrinter::B4:
                w = 257; h = 364;
                break;
            case QPrinter::B5:
                w = 182; h = 257;
                break;
            case QPrinter::B6:
                w = 128; h = 182;
                break;
            case QPrinter::B7:
                w = 91; h = 128;
                break;
            case QPrinter::B8:
                w = 64; h = 91;
                break;
            case QPrinter::B9:
                w = 45; h = 64;
                break;
            case QPrinter::B10:
                w = 32; h = 45;
                break;
            case QPrinter::C5E:
                w = 163; h = 229;
                break;
            case QPrinter::Comm10E:
                w = 105; h = 241;
                break;
            case QPrinter::DLE:
                w = 110; h = 220;
                break;
            case QPrinter::Executive:
                w = 191; h = 254;
                break;
            case QPrinter::Folio:
                w = 210; h = 330;
                break;
            case QPrinter::Ledger:
                w = 432; h = 279;
                break;
            case QPrinter::Legal:
                w = 216; h = 356;
                break;
            case QPrinter::Letter:
                w = 216; h = 279;
                break;
            case QPrinter::Tabloid:
                w = 279; h = 432;
                break;
            case -1:
                return d->canvasSize->canvasSize();
        }

        if (d->horizontalButton->isChecked() && w < h)
        {
            int t = w;
            w = h;
            h = t;
        }

        return CanvasSize(QSizeF(w, h), CanvasSize::Milimeters, QSizeF(72, 72), CanvasSize::PixelsPerInch);
    }
}

void NewCanvasDialog::paperSizeSelected(QListWidgetItem * current, QListWidgetItem * /*previous*/)
{
    int size = current->data(PAPER_SIZE_ROLE).toInt();
    // Custom size
    if (size == -1)
    {
        d->stack->setCurrentWidget(d->canvasSize);
    }
    // Template
    else
    {
        d->stack->setCurrentWidget(d->templatesList);

        TemplatesModel * model = new TemplatesModel();
        d->templatesList->setModel(model);

        QPair<QString,QString> paper = d->names[size];
        model->addTemplate(QString(), i18n("Empty"));
        if (!d->horizontalButton->isChecked())
            this->loadTemplatesList(QLatin1String("digikam/data/templates/") + paper.second + QLatin1String("/v"), model);
        if (!d->verticalButton->isChecked())
            this->loadTemplatesList(QLatin1String("digikam/data/templates/") + paper.second + QLatin1String("/h"), model);
    }
}

void NewCanvasDialog::orientationChanged()
{
    if (d->stack->currentWidget() == d->templatesList)
        this->paperSizeSelected(d->paperSize->currentItem(), 0);
    else
    {
        if (d->canvasSize->orientation() == CanvasSizeWidget::Vertical)
            d->verticalButton->setChecked(true);
        else
            d->horizontalButton->setChecked(true);
    }
}

void NewCanvasDialog::setHorizontal(bool isset)
{
    if (!isset || d->horizontalButton->isChecked())
        return;
    if (d->stack->currentWidget() == d->templatesList)
        this->paperSizeSelected(d->paperSize->currentItem(), 0);
}

void NewCanvasDialog::setVertical(bool isset)
{
    if (!isset || d->verticalButton->isChecked())
        return;
    if (d->stack->currentWidget() == d->templatesList)
        this->paperSizeSelected(d->paperSize->currentItem(), 0);
}

void NewCanvasDialog::setupUI()
{
    setWindowTitle(i18n("Create New Canvas..."));

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    QHBoxLayout * mainLayout = new QHBoxLayout();
    layout->addLayout(mainLayout);

    QVBoxLayout * leftLayout = new QVBoxLayout();
    mainLayout->addLayout(leftLayout);

    leftLayout->addWidget(new QLabel(i18n("Paper sizes"), this));

    d->paperSize = new QListWidget(this);
    d->paperSize->setMaximumWidth(150);
    connect(d->paperSize, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(paperSizeSelected(QListWidgetItem*,QListWidgetItem*)));
    foreach (QListWidgetItem * i, d->paperSizes)
        d->paperSize->addItem(i);
    leftLayout->addWidget(d->paperSize);

    // Orientation buttons
    d->horizontalButton = new QPushButton(QIcon(QLatin1String(":/horizontal_orientation.png")), QString(), this);
    d->horizontalButton->setCheckable(true);
    d->horizontalButton->setIconSize(QSize(24,24));
    d->verticalButton = new QPushButton(QIcon(QLatin1String(":/vertical_orientation.png")), QString(), this);
    d->verticalButton->setCheckable(true);
    d->verticalButton->setIconSize(QSize(24,24));
    QHBoxLayout * hLayout = new QHBoxLayout();
    hLayout->addWidget(d->horizontalButton);
    hLayout->addWidget(d->verticalButton);
    leftLayout->addLayout(hLayout);
    d->orientationGroup = new QButtonGroup(this);
    d->orientationGroup->addButton(d->horizontalButton);
    d->orientationGroup->addButton(d->verticalButton);
    connect(d->horizontalButton, SIGNAL(toggled(bool)), this, SLOT(setHorizontal(bool)));
    connect(d->verticalButton, SIGNAL(toggled(bool)), this, SLOT(setVertical(bool)));

    QVBoxLayout * rightLayout = new QVBoxLayout();
    mainLayout->addLayout(rightLayout);

    rightLayout->addWidget(new QLabel(i18n("Select a template"), this));

    d->stack = new QStackedLayout();
    rightLayout->addLayout(d->stack, 1);

    d->canvasSize = new CanvasSizeWidget(this);
    d->stack->addWidget(d->canvasSize);
    connect(d->canvasSize, SIGNAL(orientationChanged()), this, SLOT(orientationChanged()));
    connect(d->horizontalButton, SIGNAL(toggled(bool)), d->canvasSize, SLOT(setHorizontal(bool)));
    connect(d->verticalButton, SIGNAL(toggled(bool)), d->canvasSize, SLOT(setVertical(bool)));

    d->templatesList = new TemplatesView(this);
    d->stack->addWidget(d->templatesList);

    d->paperSize->setCurrentRow(0);

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    layout->addWidget(buttons);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));
}

void NewCanvasDialog::loadTemplatesList(const QString & path, TemplatesModel * model)
{
    QStringList sl = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/templates/"));

    if (sl.count() == 0)
        return;

    QDir diro(sl.first());
    QStringList files = diro.entryList(QStringList() << QLatin1String("*.ple"), QDir::Files);
    foreach (QString s, files)
        model->addTemplate(diro.path() + QLatin1String("/") + s, s);
}

} // namespace PhotoLayoutsEditor
