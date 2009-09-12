/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the ManualRenameParser
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "manualrenamewidget.h"
#include "manualrenamewidget.moc"

// Qt includes

#include <QAction>
#include <QDateTime>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QList>
#include <QMenu>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes

#include "dcursortracker.h"
#include "parser.h"
#include "manualrenameparser.h"
#include "manualrenameinput.h"

using namespace Digikam::ManualRename;
namespace Digikam
{
namespace ManualRename
{

class ManualRenameWidgetPriv
{
public:

    ManualRenameWidgetPriv()
    {
        parserLineEdit        = 0;
        tooltipTracker        = 0;
        tooltipToggleButton   = 0;
        insertTokenToolButton = 0;
        parser                = 0;
        parserRegistered      = false;
        maxColumnsLayout      = 2;
        inputStyles           = ManualRenameWidget::BigButtons;
    }

    bool                            parserRegistered;
    int                             maxColumnsLayout;
    ManualRenameWidget::InputStyles inputStyles;

    QToolButton*                    tooltipToggleButton;
    QToolButton*                    insertTokenToolButton;
    QGroupBox*                      btnContainer;

    DTipTracker*                    tooltipTracker;
    ManualRenameInput*              parserLineEdit;
    MainParser*                     parser;
};

ManualRenameWidget::ManualRenameWidget(QWidget* parent)
                 : QWidget(parent), d(new ManualRenameWidgetPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // --------------------------------------------------------

    d->parserLineEdit = new ManualRenameInput;

    d->tooltipToggleButton = new QToolButton;
    d->tooltipToggleButton->setCheckable(true);
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));

    d->insertTokenToolButton = new QToolButton;
    d->insertTokenToolButton->setPopupMode(QToolButton::InstantPopup);
    d->insertTokenToolButton->setIcon(SmallIcon("list-add"));

    // --------------------------------------------------------

    d->btnContainer = new QGroupBox(i18n("Renaming Options"), this);

    // --------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->parserLineEdit,        0, 0, 1, 1);
    mainLayout->addWidget(d->tooltipToggleButton,   0, 1, 1, 1);
    mainLayout->addWidget(d->insertTokenToolButton, 0, 2, 1, 1);
    mainLayout->addWidget(d->btnContainer,          1, 0, 1,-1);
    mainLayout->setColumnStretch(0, 10);
    setLayout(mainLayout);

    // --------------------------------------------------------

    d->tooltipTracker = new DTipTracker(QString(), d->parserLineEdit, Qt::AlignLeft);
    d->tooltipTracker->setEnable(false);
    d->tooltipTracker->setKeepOpen(true);
    d->tooltipTracker->setOpenExternalLinks(true);
    setTrackerAlignment(Qt::AlignLeft);

    // --------------------------------------------------------

    setParser(new ManualRenameParser());
    setInputStyle(d->inputStyles);

    // --------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->parserLineEdit, SIGNAL(signalTextChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));
}

ManualRenameWidget::~ManualRenameWidget()
{
    // we need to delete it manually, because it has no parent
    delete d->tooltipTracker;

    delete d->parser;
    delete d;
}

QString ManualRenameWidget::text() const
{
    return d->parserLineEdit->input()->text();
}

void ManualRenameWidget::setText(const QString& text)
{
    d->parserLineEdit->input()->setText(text);
}

void ManualRenameWidget::setTrackerAlignment(Qt::Alignment alignment)
{
    d->tooltipTracker->setTrackerAlignment(alignment);
}

void ManualRenameWidget::clear()
{
    d->parserLineEdit->input()->clear();
}

void ManualRenameWidget::slotHideToolTipTracker()
{
    d->tooltipToggleButton->setChecked(false);
    slotToolTipButtonToggled(false);
}

QString ManualRenameWidget::parse(ParseInformation& info) const
{
    if (!d->parser)
        return QString();

    QString parseString = d->parserLineEdit->input()->text();

    QString parsed;
    parsed = d->parser->parse(parseString, info);

    return parsed;
}

void ManualRenameWidget::createToolTip()
{
    if (!d->parser)
        d->tooltipTracker->setText(QString());

    QString tooltip;
    tooltip += QString("<p><table>");

    foreach (Parser* parser, d->parser->parsers())
    {
        foreach (Token* token, parser->tokens())
        {
            tooltip += QString("<tr><td><b>%1</b></td><td>:</td><td>%2</td></tr>")
                    .arg(token->id())
                    .arg(token->description());
        }
    }

    tooltip += QString("</table></p>");
    tooltip += d->parserLineEdit->input()->toolTip();

    d->tooltipTracker->setText(tooltip);
}

void ManualRenameWidget::slotToolTipButtonToggled(bool checked)
{
    d->tooltipTracker->setVisible(checked);
    slotUpdateTrackerPos();
}

void ManualRenameWidget::slotUpdateTrackerPos()
{
    d->tooltipTracker->refresh();
}

void ManualRenameWidget::setInputStyle(InputStyles inputMask)
{
    bool enable = d->parser;

    d->btnContainer->setVisible(enable && (inputMask & BigButtons));
    d->insertTokenToolButton->setVisible(enable && (inputMask & ToolButton));
    d->parserLineEdit->setEnabled(enable);
    d->tooltipToggleButton->setVisible(enable);

    d->inputStyles = inputMask;
}

void ManualRenameWidget::registerParsers(int maxLayoutColumns)
{
   if (d->parser && !d->parserRegistered)
   {
       QMenu* tokenToolBtnMenu = new QMenu(this);
       int column              = 0;
       int row                 = 0;
       QPushButton* btn        = 0;
       QAction* action         = 0;

       if (d->btnContainer->layout())
           delete d->btnContainer->layout();

       QGridLayout* gridLayout = new QGridLayout;

       int maxParsers = d->parser->parsers().count();
       foreach (Parser* parser, d->parser->parsers())
       {
           btn    = parser->registerButton(this);
           action = parser->registerMenu(tokenToolBtnMenu);

           if (!btn || !action)
               continue;

           gridLayout->addWidget(btn, row, column, 1, 1);

           connect(parser, SIGNAL(signalTokenTriggered(const QString&)),
                   d->parserLineEdit, SLOT(slotAddToken(const QString&)));

           ++column;

           if (column % maxLayoutColumns == 0)
           {
               ++row;
               column = 0;
           }
       }

       // --------------------------------------------------------

       // If the buttons don't fill up all columns, expand the last button to fit the layout
       if ((row >= (maxParsers / maxLayoutColumns)) && (column == 0))
       {
           gridLayout->removeWidget(btn);
           gridLayout->addWidget(btn, (row - 1), (maxLayoutColumns - 1), 1, 1);
       }
       else if (column != maxLayoutColumns)
       {
           gridLayout->removeWidget(btn);
           gridLayout->addWidget(btn, row, (column - 1), 1, -1);
       }

       // --------------------------------------------------------

       d->btnContainer->setLayout(gridLayout);
       d->insertTokenToolButton->setMenu(tokenToolBtnMenu);

       d->parserLineEdit->input()->setParser(d->parser);
       d->parserRegistered = true;
   }
}

void ManualRenameWidget::setParser(MainParser* parser)
{
    if (!parser)
        return;

    if (d->parserRegistered)
    {
        delete d->parser;
        d->parserRegistered = false;
    }

    d->parser = parser;
    registerParsers(d->maxColumnsLayout);
    createToolTip();
}

}  // namespace ManualRename
}  // namespace Digikam
