/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : a dialog to perform advanced search in albums
 * 
 * Copyright (C) 2005 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
 
/** @file searchadvanceddialog.cpp */

// Qt includes.

#include <qvbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kurl.h>
#include <kiconloader.h>
#include <klocale.h>
#include <klineedit.h>

// Local includes.

#include "ddebug.h"
#include "searchwidgets.h"
#include "searchresultsview.h"
#include "searchadvanceddialog.h"
#include "searchadvanceddialog.moc"

namespace Digikam
{

class SearchAdvancedDialogPriv
{
public:

    SearchAdvancedDialogPriv()
    {
        timer         = 0;
        title         = 0;
        optionsCombo  = 0;
        resultsView   = 0;
        ungroupButton = 0;
        groupButton   = 0;
        delButton     = 0;
        addButton     = 0;
        rulesBox      = 0;
    }

    QVGroupBox                      *rulesBox;

    QPushButton                     *addButton;
    QPushButton                     *delButton;
    QPushButton                     *groupButton;
    QPushButton                     *ungroupButton;

    QComboBox                       *optionsCombo;

    QValueList<SearchAdvancedBase*>  baseList;

    QTimer                          *timer;

    KLineEdit                       *title;

    SearchResultsView               *resultsView;
};

SearchAdvancedDialog::SearchAdvancedDialog(QWidget* parent, KURL& url)
                    : KDialogBase(parent, 0, true, i18n("Advanced Search"),
                                  Help|Ok|Cancel, Ok, true), m_url(url)
{
    d = new SearchAdvancedDialogPriv;
    d->timer = new QTimer(this);
    setHelp("advancedsearchtool.anchor", "digikam");

    QWidget *page = new QWidget( this );
    setMainWidget(page);

    resize(configDialogSize("AdvancedSearch Dialog"));

    // ----------------------------------------------------------------
    // two columns, one for the rules, one for the preview.

    QHBoxLayout* hbox     = new QHBoxLayout( page );
    QVBoxLayout* leftSide = new QVBoxLayout();
    d->resultsView        = new SearchResultsView(page);
    d->resultsView->setMinimumSize(QSize(200, 200));
    QWhatsThis::add(d->resultsView, i18n("<p>Here you can review the images found "
                                         "using the current search settings."));
    hbox->addLayout(leftSide, 10);
    hbox->setSpacing(spacingHint());
    hbox->addWidget(d->resultsView, 5);

    // ----------------------------------------------------------------
    // Box for all the rules

    d->rulesBox = new QVGroupBox(i18n("Search Rules"), page);
    QWhatsThis::add(d->rulesBox, i18n("<p>Here you can review the search rules used to filter image-"
                                      "searching in album library."));
    d->rulesBox->layout()->setSpacing( spacingHint() );
    d->rulesBox->layout()->setMargin( spacingHint() );
    d->rulesBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    d->rulesBox->layout()->setAlignment( Qt::AlignTop );

    // ----------------------------------------------------------------
    // Box for the add/delete

    QGroupBox *groupbox1 = new QGroupBox( i18n("Add/Delete Option"), page, "groupbox1" );
    QWhatsThis::add(groupbox1, i18n("<p>You can edit the search rules "
                                    "by adding/removing criteria."));
                                         
    groupbox1->setColumnLayout(0, Qt::Vertical );
    groupbox1->layout()->setSpacing( KDialog::spacingHint() );
    groupbox1->layout()->setMargin( KDialog::marginHint() );
    d->optionsCombo = new QComboBox(groupbox1);
    d->optionsCombo->insertItem(i18n("As well as"), 0);
    d->optionsCombo->insertItem(i18n("Or"), 1);
    d->optionsCombo->setEnabled(false);

    d->addButton = new QPushButton(i18n("&Add"), groupbox1);
    d->delButton = new QPushButton(i18n("&Del"), groupbox1);
    d->addButton->setIconSet(SmallIcon("add"));
    d->delButton->setIconSet(SmallIcon("remove"));

    QHBoxLayout* box1 = new QHBoxLayout(groupbox1->layout());
    box1->addWidget(d->optionsCombo);
    box1->addWidget(d->addButton);
    box1->addStretch(10);
    box1->addWidget(d->delButton);

    // ----------------------------------------------------------------
    // Box for the group/ungroup

    QGroupBox *groupbox2 = new QGroupBox( i18n("Group/Ungroup Options"), page, "groupbox2" );
    QWhatsThis::add(groupbox1, i18n("<p>You can group or ungroup any search criteria "
                                    "from the Search Rule set."));
    groupbox2->setColumnLayout(0, Qt::Vertical);
    groupbox2->layout()->setSpacing( KDialog::spacingHint() );
    groupbox2->layout()->setMargin( KDialog::marginHint() );
    d->groupButton   = new QPushButton(i18n("&Group"), groupbox2);
    d->ungroupButton = new QPushButton(i18n("&Ungroup"), groupbox2);

    QHBoxLayout* box2 = new QHBoxLayout(groupbox2->layout());
    box2->addWidget(d->groupButton);
    box2->addStretch(10);
    box2->addWidget(d->ungroupButton);

    // ----------------------------------------------------------------
    // box for saving the search.

    QGroupBox *groupbox3 = new QGroupBox( page, "groupbox3");
    groupbox3->setColumnLayout(0, Qt::Vertical );
    groupbox3->layout()->setSpacing( KDialog::spacingHint() );
    groupbox3->setFrameStyle( QFrame::NoFrame );
    QLabel* label = new QLabel(i18n("&Save search as: "), groupbox3);
    d->title      = new KLineEdit(groupbox3, "searchTitle");
    QWhatsThis::add(d->title, i18n("<p>Enter the name used to save the current search in "
                                   "\"My Searches\" view"));

    QHBoxLayout* box3 = new QHBoxLayout(groupbox3->layout());
    box3->addWidget(label);
    box3->addWidget(d->title);
    label->setBuddy(d->title);

    // ----------------------------------------------------------------

    leftSide->addWidget( d->rulesBox );
    leftSide->addStretch(10);           // Push the rulesbox to top and the buttons down.
    leftSide->addWidget(groupbox1);
    leftSide->addWidget(groupbox2);
    leftSide->addWidget(groupbox3);

    // ----------------------------------------------------------------

    if ( url.isEmpty() )
    {
        d->title->setText(i18n("Last Search"));
        slotAddRule();
    }
    else
    {
        d->title->setText(url.queryItem("name"));
        fillWidgets( url );
    }

    slotChangeButtonStates();
    d->timer->start(0, true);

    // ----------------------------------------------------------------

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddRule()));

    connect(d->delButton, SIGNAL(clicked()),
            this, SLOT(slotDelRules()));

    connect(d->groupButton, SIGNAL(clicked()),
            this, SLOT(slotGroupRules()));

    connect(d->ungroupButton, SIGNAL(clicked()),
            this, SLOT(slotUnGroupRules()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    connect(d->title, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotChangeButtonStates()));
}

SearchAdvancedDialog::~SearchAdvancedDialog()
{
    saveDialogSize("AdvancedSearch Dialog");
    delete d->timer;
    delete d;
}

void SearchAdvancedDialog::slotAddRule()
{
    SearchAdvancedBase::Option type = SearchAdvancedBase::NONE;
    if ( !d->baseList.isEmpty() )
    {
        if (d->optionsCombo->currentItem() == 0 )
            type = SearchAdvancedBase::AND;
        else
            type = SearchAdvancedBase::OR;
    }

    SearchAdvancedRule* rule = new SearchAdvancedRule( d->rulesBox, type );
    d->baseList.append(rule);

    connect( rule, SIGNAL( signalBaseItemToggled() ),
             this, SLOT( slotChangeButtonStates() ) );

    connect( rule, SIGNAL( signalPropertyChanged() ),
             this, SLOT(slotPropertyChanged()));

    slotChangeButtonStates();
    slotPropertyChanged();
}

void SearchAdvancedDialog::slotDelRules()
{
    if (d->baseList.isEmpty())
        return;

    typedef QValueList<SearchAdvancedBase*> BaseList;

    BaseList itemsToRemove;

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->isChecked())
        {
            itemsToRemove.append(base);
        }
    }

    for (BaseList::iterator it = itemsToRemove.begin();
         it != itemsToRemove.end(); ++it)
    {
        SearchAdvancedBase* base = (SearchAdvancedBase*) *it;
        d->baseList.remove(base);
        delete base;
    }

    BaseList::iterator it = d->baseList.begin();
    if (it != d->baseList.end())
        (*it)->removeOption();

    slotChangeButtonStates();
    slotPropertyChanged();
    if (d->baseList.isEmpty()) 
    {
        d->optionsCombo->setEnabled(false);
        d->addButton->setEnabled(true);
        enableButtonOK( false );
    }
}

void SearchAdvancedDialog::slotGroupRules()
{
    typedef QValueList<SearchAdvancedBase*> BaseList;

    BaseList itemsToGroup;
    BaseList groupsToUnGroupAndGroup;

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if ( base->isChecked() )
        {
            itemsToGroup.append( base );
            if ( base->type() == SearchAdvancedBase::GROUP)
                groupsToUnGroupAndGroup.append( base );
        }
    }

    // ungroup every found group so it can be regrouped later on.
    for (BaseList::iterator it = groupsToUnGroupAndGroup.begin();
         it != groupsToUnGroupAndGroup.end(); ++it)
    {
        SearchAdvancedGroup* group = (SearchAdvancedGroup*)*it;
        BaseList::iterator itemsToGroupPos = itemsToGroup.find(group);
        BaseList::iterator itPos = d->baseList.find(group);
        QValueList<SearchAdvancedRule*> childRules = group->childRules();
        for (QValueList<SearchAdvancedRule*>::iterator iter = childRules.begin();
                 iter != childRules.end(); ++iter)
        {
            d->baseList.insert(itPos, *iter);
            itemsToGroup.insert(itemsToGroupPos, *iter);
        }
        group->removeRules();
        d->baseList.remove(group);
        itemsToGroup.remove(group);
        delete group;
    }

    // if there is only one or no item return
    if (itemsToGroup.size() < 2)
        return;

    BaseList::iterator it = itemsToGroup.begin();
    SearchAdvancedRule* rule = (SearchAdvancedRule*)(*it);

    SearchAdvancedGroup* group = new SearchAdvancedGroup(d->rulesBox);
    BaseList::iterator itPos = d->baseList.find(rule);
    d->baseList.insert(itPos, group);

    for (BaseList::iterator it = itemsToGroup.begin();
         it != itemsToGroup.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->type() == SearchAdvancedBase::RULE)
        {
            SearchAdvancedRule* rule = (SearchAdvancedRule*)base;
            group->addRule(rule);
            d->baseList.remove(rule);
        }
    }

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        d->rulesBox->layout()->remove((*it)->widget());
        d->rulesBox->layout()->add((*it)->widget());
    }

    connect( group, SIGNAL( signalBaseItemToggled() ),
             this, SLOT( slotChangeButtonStates() ) );

    slotChangeButtonStates();
    slotPropertyChanged();
}

void SearchAdvancedDialog::slotUnGroupRules()
{
    typedef QValueList<SearchAdvancedBase*>  BaseList;
    typedef QValueList<SearchAdvancedGroup*> GroupList;

    GroupList itemsToUnGroup;

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->type() == SearchAdvancedBase::GROUP &&
            base->isChecked())
        {
            itemsToUnGroup.append((SearchAdvancedGroup*)base);
        }
    }

    if (itemsToUnGroup.isEmpty())
        return;

    for (GroupList::iterator it = itemsToUnGroup.begin();
         it != itemsToUnGroup.end(); ++it)
    {
        SearchAdvancedGroup *group = *it;
        QValueList<SearchAdvancedRule*> childRules = group->childRules();

        BaseList::iterator itPos = d->baseList.find(group);

        for (QValueList<SearchAdvancedRule*>::iterator iter = childRules.begin();
             iter != childRules.end(); ++iter)
        {
            d->baseList.insert(itPos, *iter);
        }

        group->removeRules();
        d->baseList.remove(group);
        delete group;
    }

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        d->rulesBox->layout()->remove((*it)->widget());
        d->rulesBox->layout()->add((*it)->widget());
    }

    slotChangeButtonStates();
    slotPropertyChanged();
}

void SearchAdvancedDialog::slotPropertyChanged()
{
    d->timer->start(500, true);
}

void SearchAdvancedDialog::slotOk()
{
    // calculate the latest url and name.
    slotTimeOut();

    // Since it's not possible to check the state of the ok button,
    // check the state of the add button.
    if ( d->addButton->isEnabled() )
        KDialogBase::slotOk();
}

void SearchAdvancedDialog::slotTimeOut()
{
    if (d->baseList.isEmpty())
        return;

    typedef QValueList<SearchAdvancedBase*>  BaseList;

    QString grouping;
    int     count    = 0;
    bool    emptyVal = false;

    KURL url;
    url.setProtocol("digikamsearch");

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->type() == SearchAdvancedBase::RULE)
        {
            SearchAdvancedRule* rule = (SearchAdvancedRule*)base;
            QString val = rule->urlValue();
            if ( !val.isEmpty() )
            {
                if (rule->option() != SearchAdvancedBase::NONE &&
                    !count == 0 )
                    grouping += (rule->option() == SearchAdvancedBase::AND) ?
                        " AND " : " OR ";
                grouping += QString::number(++count);
                url.addQueryItem( QString::number(count) + ".key", rule->urlKey());
                url.addQueryItem( QString::number(count) + ".op", rule->urlOperator());
                url.addQueryItem( QString::number(count) + ".val", val);
            }
            else
                emptyVal = true;
        }
        else
        {
            SearchAdvancedGroup* group = (SearchAdvancedGroup*)base;

            QString tempGrouping;
            int curCount = count;

            QValueList<SearchAdvancedRule*> childRules = group->childRules();
            for (QValueList<SearchAdvancedRule*>::iterator iter =
                    childRules.begin();
                 iter != childRules.end(); ++iter)
            {
                SearchAdvancedRule* rule = (SearchAdvancedRule*)(*iter);
                QString val = rule->urlValue();
                if ( !val.isEmpty() )
                {
                    if (rule->option() != SearchAdvancedBase::NONE &&
                    !count == 0 )
                        tempGrouping += (rule->option() == SearchAdvancedBase::AND) ?
                            " AND " : " OR ";
                    tempGrouping += QString::number(++count);
                    url.addQueryItem( QString::number(count) + ".key", rule->urlKey());
                    url.addQueryItem( QString::number(count) + ".op", rule->urlOperator());
                    url.addQueryItem( QString::number(count) + ".val", val);
                }
                else
                    emptyVal = true;
            }

            if (!tempGrouping.isEmpty())
            {
                if (group->option() != SearchAdvancedBase::NONE &&
                    !curCount == 0 )
                    grouping += (group->option() == SearchAdvancedBase::AND) ?
                            " AND " : " OR ";
                    grouping += " ( " + tempGrouping + " ) ";
            }
        }
    }

    url.setPath(grouping);
    url.addQueryItem("name", d->title->text());
    url.addQueryItem("count", QString::number(count));
    m_url = url;
    if (!count == 0)
        d->resultsView->openURL( url );
    DDebug() << url << endl;

    if (!d->baseList.isEmpty())
    {
        if (!d->title->text().isEmpty())
            enableButtonOK( !emptyVal );
        d->addButton->setEnabled( !emptyVal );
        d->optionsCombo->setEnabled( !emptyVal );
    }
}

void SearchAdvancedDialog::slotChangeButtonStates()
{
    bool group  = false;
    int counter = 0;

    typedef QValueList<SearchAdvancedBase*>  BaseList;
    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->isChecked())
        {
            ++counter;
            if (base->type() == SearchAdvancedBase::GROUP)
                group = true;
        }
    }

    d->ungroupButton->setEnabled( group );

    if ( counter == 0)
    {
        d->delButton->setEnabled(false);
        d->groupButton->setEnabled(false);
    } 
    else if ( counter == 1)
    {
        if (d->baseList.count() > 1)
            d->delButton->setEnabled(true);
        d->groupButton->setEnabled(false);
    }
    else if ( counter > 1 )
    {
        d->delButton->setEnabled(true);
        d->groupButton->setEnabled(true);
    }

    enableButtonOK( !d->title->text().isEmpty() );
}

void SearchAdvancedDialog::fillWidgets( const KURL& url )
{
    int  count = url.queryItem("count").toInt();
    if (count <= 0)
        return;

    QMap<int, KURL> rulesMap;

    for (int i=1; i<=count; i++)
    {
        KURL newRule;

        QString key = url.queryItem(QString::number(i) + ".key");
        QString op  = url.queryItem(QString::number(i) + ".op");
        QString val = url.queryItem(QString::number(i) + ".val");

        newRule.setPath("1");
        newRule.addQueryItem("1.key",key);
        newRule.addQueryItem("1.op",op);
        newRule.addQueryItem("1.val",val);

        rulesMap.insert(i, newRule);
    }

    QStringList strList = QStringList::split(" ", url.path());

    SearchAdvancedGroup* group      = 0;
    bool groupingActive             = false;
    SearchAdvancedBase::Option type = SearchAdvancedBase::NONE;

    for ( QStringList::Iterator it = strList.begin(); it != strList.end(); ++it )
    {
        bool ok;
        int  num = (*it).toInt(&ok);
        if (ok)
        {
            SearchAdvancedRule* rule = new SearchAdvancedRule( d->rulesBox, type );
            rule->setValues( rulesMap[num] );

            connect( rule, SIGNAL( signalBaseItemToggled() ),
                     this, SLOT( slotChangeButtonStates() ) );

            connect( rule, SIGNAL( signalPropertyChanged() ),
                     this, SLOT(slotPropertyChanged()));

            if (groupingActive)
                group->addRule(rule);
            else
                d->baseList.append(rule);
        }
        else if (*it == "OR")
        {
            type = SearchAdvancedRule::OR;
        }
        else if (*it == "AND")
        {
            type = SearchAdvancedRule::AND;
        }
        else if (*it == "(")
        {
            group = new SearchAdvancedGroup(d->rulesBox);
            d->baseList.append(group);

            connect( group, SIGNAL( signalBaseItemToggled() ),
                     this, SLOT( slotChangeButtonStates() ) );

            groupingActive = true;
        }
        else if (*it == ")")
        {
            groupingActive = false;
        }
        else
        {
            DDebug() << "IGNORED:" << *it << endl;
        }
    }

    enableButtonOK( true );
}

}  // namespace Digikam
