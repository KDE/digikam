/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-11
 * Description : script interface for digiKam
 *
 * Copyright (C) 2010 by Kunal Ghosh <kunal dot t2 at gmail dot com>
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "scriptiface.moc"

// KDE includes

#include <ktextedit.h>
#include <klocale.h>

// Qt includes

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QtScript>
#include <QtCore/QCoreApplication>

namespace Digikam
{

class ScriptIface::ScriptIfacePriv
{

public:

    ScriptIfacePriv()
    {
        edit   = 0;
        label  = 0;
        engine = 0;
    }


    KTextEdit* edit;
    QLabel*    label;
    //create a new QScript Engine object
    QScriptEngine* engine;
};

ScriptIface::ScriptIface(QWidget* parent)
    : KDialog(parent),
      d(new ScriptIfacePriv)
{
    setCaption(i18n("Script Console"));
    setButtons(Help|User1|Close);
    setDefaultButton(User1);
    setButtonText(User1, i18n("Evaluate"));
    setButtonIcon(User1, KIcon("run-build"));
    setHelp("scriptconsole.anchor", "digikam");
    setModal(false);

    QWidget* w        = new QWidget(this);
    QGridLayout* grid = new QGridLayout(w);

    d->engine         = new QScriptEngine(this);
    d->edit           = new KTextEdit(w);
    d->label          = new QLabel(w);
    grid->addWidget(d->edit,  0, 0, 2, 1);
    grid->addWidget(d->label, 3, 0, 2, 1);
    grid->setMargin(0);
    grid->setSpacing(spacingHint());
    grid->setRowStretch(0, 10);

    setMainWidget(w);
    adjustSize();

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotEvaluate()));

    //the following function calls
    //importExtension() for each binding
    ImportQtBindings();

}

ScriptIface::~ScriptIface()
{
    delete d;
}

void ScriptIface::slotEvaluate()
{
    QString script      = d->edit->toPlainText();
    QScriptValue result = d->engine->evaluate(script);
    d->label->setText(result.toString());
    /*if(d->label->text() == QString())
    {
        d->label->setText("hello");
    }
    else d->label->clear();*/
}

void ScriptIface::ImportQtBindings()
{
    //import the plugins
    d->engine->importExtension("qt.gui");
    d->engine->importExtension("qt.core");
    d->engine->importExtension("qt.network");
    d->engine->importExtension("qt.opengl");
    d->engine->importExtension("qt.phonon");
    d->engine->importExtension("qt.sql");
    d->engine->importExtension("qt.svg");
    d->engine->importExtension("qt.uitools");
    d->engine->importExtension("qt.webkit");
    d->engine->importExtension("qt.xmlpatterns");
    d->engine->importExtension("qt.xml");
}

} // namespace DigiKam
