/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the AdvancedRename utility
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

#include "advancedrenamewidget.h"
#include "advancedrenamewidget.moc"

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
#include "defaultparser.h"
#include "advancedrenameinput.h"
#include "themeengine.h"

namespace Digikam
{

class AdvancedRenameWidgetPriv
{
public:

    AdvancedRenameWidgetPriv()
    {
        parserLineEdit          = 0;
        tooltipTracker          = 0;
        tooltipToggleButton     = 0;
        tokenToolButton         = 0;
        btnContainer            = 0;
        parser                  = 0;
        inputColumns            = 2;
        controlWidgetsMask      = AdvancedRenameWidget::TokenButtons | AdvancedRenameWidget::ToolTipButton;
        tooltipTrackerAlignment = Qt::AlignLeft;
    }

    int                                  inputColumns;
    AdvancedRenameWidget::ControlWidgets controlWidgetsMask;

    QToolButton*                         tooltipToggleButton;
    QToolButton*                         tokenToolButton;
    QGroupBox*                           btnContainer;

    Qt::Alignment                        tooltipTrackerAlignment;

    DTipTracker*                         tooltipTracker;
    AdvancedRenameInput*                 parserLineEdit;
    Parser*                              parser;
};

AdvancedRenameWidget::AdvancedRenameWidget(QWidget* parent)
                 : QWidget(parent), d(new AdvancedRenameWidgetPriv)
{
    setupWidgets();
    setParser(new DefaultParser());
}

AdvancedRenameWidget::~AdvancedRenameWidget()
{
    // we need to delete it manually, because it has no parent
    delete d->tooltipTracker;

    delete d->parser;
    delete d;
}

QString AdvancedRenameWidget::text() const
{
    return d->parserLineEdit->input()->text();
}

void AdvancedRenameWidget::setText(const QString& text)
{
    d->parserLineEdit->input()->setText(text);
}

void AdvancedRenameWidget::setTrackerAlignment(Qt::Alignment alignment)
{
    d->tooltipTrackerAlignment = alignment;
    d->tooltipTracker->setTrackerAlignment(alignment);
}

void AdvancedRenameWidget::clear()
{
    d->parserLineEdit->input()->clear();
}

void AdvancedRenameWidget::slotHideToolTipTracker()
{
    d->tooltipToggleButton->setChecked(false);
    slotToolTipButtonToggled(false);
}

QString AdvancedRenameWidget::parse(ParseInformation& info) const
{
    if (!d->parser)
        return QString();

    QString parseString = d->parserLineEdit->input()->text();

    QString parsed;
    parsed = d->parser->parse(parseString, info);

    return parsed;
}

void AdvancedRenameWidget::createToolTip()
{
#define TOOLTIP_HEADER(str)                                                          \
    do                                                                               \
    {                                                                                \
        tooltip += QString("<tr bgcolor=\"%1\"><td colspan=\"2\">"                   \
                            "<nobr><font color=\"%2\"><center><b>")                  \
                            .arg(ThemeEngine::instance()->baseColor().name())        \
                            .arg(ThemeEngine::instance()->textRegColor().name());    \
        tooltip += QString(str);                                                     \
        tooltip += QString("</b></center></font></nobr></td></tr>");                 \
    } while (0)                                                                      \


#define TOOLTIP_ENTRIES(type, data)                                                  \
    do                                                                               \
    {                                                                                \
        foreach (type* t, data)                                                      \
        {                                                                            \
            tooltip += QString("<tr><td><b>%1</b></td><td>: %2</td></tr>")           \
                                        .arg(t->id())                                \
                                        .arg(t->description());                      \
        }                                                                            \
    } while (0)

    // --------------------------------------------------------

    if (!d->parser)
        d->tooltipTracker->setText(QString());

    QString tooltip;
    tooltip += QString("<qt><table cellspacing=\"0\" cellpadding=\"0\" border=\"0\">");

    // --------------------------------------------------------

    if (!d->parser->subParsers().isEmpty())
    {
        TOOLTIP_HEADER(i18n("Renaming Options"));
        foreach (SubParser* subparser, d->parser->subParsers())
        {
            TOOLTIP_ENTRIES(Token, subparser->tokens());
        }
    }

    // --------------------------------------------------------

    if (!d->parser->modifiers().isEmpty())
    {
        TOOLTIP_HEADER(i18n("Modifiers"));
        TOOLTIP_ENTRIES(Modifier, d->parser->modifiers());
    }
    // --------------------------------------------------------

    tooltip += QString("</table></qt>");
    tooltip += QString("<i>%1</i>").arg(d->parserLineEdit->input()->toolTip());

    d->tooltipTracker->setText(tooltip);

    // --------------------------------------------------------

#undef TOOLTIP_HEADER
#undef TOOLTIP_ENTRIES
}

void AdvancedRenameWidget::slotToolTipButtonToggled(bool checked)
{
    d->tooltipTracker->setVisible(checked);
    slotUpdateTrackerPos();
}

void AdvancedRenameWidget::slotUpdateTrackerPos()
{
    d->tooltipTracker->refresh();
}

void AdvancedRenameWidget::setControlWidgets(ControlWidgets mask)
{
    bool enable = d->parser && d->parser->subParsers().count() > 0;

    d->btnContainer->setVisible(enable && (mask & TokenButtons));
    d->tokenToolButton->setVisible(enable && (mask & TokenToolButton));
    d->parserLineEdit->setEnabled(enable);
    d->tooltipToggleButton->setVisible(enable && (mask & ToolTipButton));

    d->controlWidgetsMask = mask;
}

void AdvancedRenameWidget::registerParserControls()
{
   if (d->parser)
   {
       setupWidgets();

       QMenu* tokenToolBtnMenu = new QMenu(d->tokenToolButton);
       int column              = 0;
       int row                 = 0;
       QPushButton* btn        = 0;
       QAction* action         = 0;
       QGridLayout* gridLayout = new QGridLayout;
       gridLayout->setSpacing(KDialog::marginHint());
       gridLayout->setMargin(KDialog::marginHint());

       int maxParsers = d->parser->subParsers().count();
       foreach (SubParser* subparser, d->parser->subParsers())
       {
           btn    = subparser->registerButton(this);
           action = subparser->registerMenu(tokenToolBtnMenu);

           if (!btn || !action)
               continue;

           gridLayout->addWidget(btn, row, column, 1, 1);

           connect(subparser, SIGNAL(signalTokenTriggered(const QString&)),
                   d->parserLineEdit, SLOT(slotAddToken(const QString&)));

           ++column;

           if (column % d->inputColumns == 0)
           {
               ++row;
               column = 0;
           }
       }

       // --------------------------------------------------------

       // If the buttons don't fill up all columns, expand the last button to fit the layout
       if ((row >= (maxParsers / d->inputColumns)) && (column == 0))
       {
           gridLayout->removeWidget(btn);
           gridLayout->addWidget(btn, (row - 1), (d->inputColumns - 1), 1, 1);
       }
       else if (column != d->inputColumns)
       {
           gridLayout->removeWidget(btn);
           gridLayout->addWidget(btn, row, (column - 1), 1, -1);
       }

       // --------------------------------------------------------

       d->btnContainer->setLayout(gridLayout);
       d->tokenToolButton->setMenu(tokenToolBtnMenu);

       d->parserLineEdit->input()->setParser(d->parser);
       createToolTip();
   }
}

void AdvancedRenameWidget::setParser(Parser* parser)
{
    if (!parser)
        return;

    if (d->parser)
        delete d->parser;
    d->parser = parser;

    setInputColumns(d->inputColumns);
    registerParserControls();
    setControlWidgets(d->controlWidgetsMask);
}

void AdvancedRenameWidget::setInputColumns(int col)
{
    if (col == d->inputColumns)
        return;

    d->inputColumns = col;
    registerParserControls();
    setControlWidgets(d->controlWidgetsMask);
}

void AdvancedRenameWidget::setupWidgets()
{
    delete d->parserLineEdit;
    d->parserLineEdit = new AdvancedRenameInput;

    delete d->tooltipToggleButton;
    d->tooltipToggleButton = new QToolButton;
    d->tooltipToggleButton->setCheckable(true);
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));

    // --------------------------------------------------------

    delete d->btnContainer;
    d->btnContainer = new QGroupBox(i18n("Renaming Options"), this);

    delete d->tokenToolButton;
    d->tokenToolButton = new QToolButton;
    d->tokenToolButton->setPopupMode(QToolButton::InstantPopup);
    d->tokenToolButton->setIcon(SmallIcon("list-add"));

    // --------------------------------------------------------

    delete d->tooltipTracker;
    d->tooltipTracker = new DTipTracker(QString(), d->parserLineEdit, Qt::AlignLeft);
    d->tooltipTracker->setTextFormat(Qt::RichText);
    d->tooltipTracker->setEnable(false);
    d->tooltipTracker->setKeepOpen(true);
    d->tooltipTracker->setOpenExternalLinks(true);
    setTrackerAlignment(d->tooltipTrackerAlignment);

    // --------------------------------------------------------

    delete layout();
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->parserLineEdit,        0, 0, 1, 1);
    mainLayout->addWidget(d->tooltipToggleButton,   0, 1, 1, 1);
    mainLayout->addWidget(d->tokenToolButton,       0, 2, 1, 1);
    mainLayout->addWidget(d->btnContainer,          1, 0, 1,-1);
    mainLayout->setColumnStretch(0, 10);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::marginHint());
    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->parserLineEdit, SIGNAL(signalTextChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));
}

}  // namespace Digikam
