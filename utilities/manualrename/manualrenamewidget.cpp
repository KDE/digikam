/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : an input widget for the ManualRenameParser
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
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes

#include "dcursortracker.h"
#include "parser.h"
#include "manualrenameparser.h"

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
        parseStringLineEdit   = 0;
        tooltipTracker        = 0;
        tooltipToggleButton   = 0;
        insertTokenToolButton = 0;
        parser                = 0;
    }

    QToolButton*        tooltipToggleButton;
    QToolButton*        insertTokenToolButton;

    KLineEdit*          parseStringLineEdit;
    DTipTracker*        tooltipTracker;

    QGroupBox*          btnContainer;

    ManualRenameParser* parser;

    QTimer*             parseTimer;
};

ManualRenameWidget::ManualRenameWidget(QWidget* parent)
                 : QWidget(parent), d(new ManualRenameWidgetPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // --------------------------------------------------------

    d->parseStringLineEdit  = new KLineEdit;

    d->tooltipToggleButton  = new QToolButton;
    d->tooltipToggleButton->setCheckable(true);
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));

    d->insertTokenToolButton = new QToolButton;
    d->insertTokenToolButton->setPopupMode(QToolButton::InstantPopup);
    d->insertTokenToolButton->setIcon(SmallIcon("list-add"));

    // --------------------------------------------------------

    d->btnContainer = new QGroupBox(i18n("Quick access"), this);

    // --------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->parseStringLineEdit,   1, 1, 1, 1);
    mainLayout->addWidget(d->tooltipToggleButton,   1, 2, 1, 1);
    mainLayout->addWidget(d->insertTokenToolButton, 1, 3, 1, 1);
    mainLayout->addWidget(d->btnContainer,          2, 0, 1,-1);
    mainLayout->setColumnStretch(1, 10);
    setLayout(mainLayout);

    // --------------------------------------------------------

    registerParsers();
    setParserInputStyle(ToolButton);

    // --------------------------------------------------------

    QString tooltip   = createToolTip();
    d->tooltipTracker = new DTipTracker(tooltip, d->parseStringLineEdit, Qt::AlignLeft);
    d->tooltipTracker->setEnable(false);
    d->tooltipTracker->setKeepOpen(true);
    d->tooltipTracker->setOpenExternalLinks(true);

    d->parseStringLineEdit->setWhatsThis(tooltip);
    d->parseStringLineEdit->setClearButtonShown(true);
    d->parseStringLineEdit->setCompletionMode(KGlobalSettings::CompletionAuto);
    d->parseStringLineEdit->setClickMessage(i18n("Enter custom rename string"));

    // --------------------------------------------------------

    d->parseTimer = new QTimer(this);
    d->parseTimer->setInterval(500);
    d->parseTimer->setSingleShot(true);

    // --------------------------------------------------------

    connect(d->parseTimer, SIGNAL(timeout()),
            this, SLOT(slotParseTimer()));

    connect(d->tooltipToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->parseStringLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextChanged()));
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
    return d->parseStringLineEdit->text();
}

void ManualRenameWidget::setText(const QString& text)
{
    d->parseStringLineEdit->setText(text);
}

void ManualRenameWidget::setTrackerAlignment(Qt::Alignment alignment)
{
    d->tooltipTracker->setTrackerAlignment(alignment);
}

KLineEdit* ManualRenameWidget::input() const
{
    return d->parseStringLineEdit;
}

void ManualRenameWidget::slotHideToolTipTracker()
{
    d->tooltipToggleButton->setChecked(false);
    slotToolTipButtonToggled(false);
}

QString ManualRenameWidget::parse(ParseInformation& info) const
{
    QString parseString = d->parseStringLineEdit->text();

    QString parsed;
    parsed = d->parser->parse(parseString, info);

    return parsed;
}

QString ManualRenameWidget::createToolTip()
{
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
    return tooltip;
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

void ManualRenameWidget::addToken2ParserInput(const QString& token)
{
    if (!token.isEmpty())
    {
        int cursorPos = d->parseStringLineEdit->cursorPosition();
        QString tmp   = d->parseStringLineEdit->text();
        tmp.insert(cursorPos, token);
        d->parseStringLineEdit->setText(tmp);
        d->parseStringLineEdit->setCursorPosition(cursorPos + token.count());
    }
    d->parseStringLineEdit->setFocus();
}

void ManualRenameWidget::setParserInputStyle(ParserInputStyles inputMask)
{
    d->btnContainer->setVisible(inputMask & BigButtons);
    d->insertTokenToolButton->setVisible(inputMask & ToolButton);
}

void ManualRenameWidget::registerParsers(int maxColumns)
{
   if (!d->parser)
   {
       d->parser = new ManualRenameParser;

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
                   this, SLOT(addToken2ParserInput(const QString&)));

           ++column;

           if (column % maxColumns == 0)
           {
               ++row;
               column = 0;
           }
       }

       // --------------------------------------------------------

       // If the buttons don't fill up all columns, expand the last button to fit the layout
       if ((row >= (maxParsers / maxColumns)) && (column == 0))
       {
           gridLayout->removeWidget(btn);
           gridLayout->addWidget(btn, (row - 1), (maxColumns - 1), 1, 1);
       }
       else if (column != maxColumns)
       {
           gridLayout->removeWidget(btn);
           gridLayout->addWidget(btn, row, (column - 1), 1, -1);
       }

       // --------------------------------------------------------

       d->btnContainer->setLayout(gridLayout);
       d->insertTokenToolButton->setMenu(tokenToolBtnMenu);
   }
}

void ManualRenameWidget::slotTextChanged()
{
    d->parseTimer->start();
}

void ManualRenameWidget::slotParseTimer()
{
    emit signalTextChanged(d->parseStringLineEdit->text());
}

}  // namespace ManualRename
}  // namespace Digikam
