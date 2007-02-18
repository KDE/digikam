/* ============================================================
 * Authors: Tom Albers <tomalbers@kde.nl>
 *          Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier 
 * Date   : 2005-01-01
 * Description : a dialog to perform advanced search in albums
 * 
 * Copyright 2005 by Tom Albers and Renchi Raju
 *           2006 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <klocale.h>
#include <klineedit.h>

// Local includes.

#include "ddebug.h"
#include "searchwidgets.h"
#include "searchadvanceddialog.h"
#include "searchresultsview.h"

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

    // two columns, one for the rules, one for the preview.
    QHBoxLayout* hbox = new QHBoxLayout( page );
    hbox->setSpacing( spacingHint() );
    QVBoxLayout* leftSide = new QVBoxLayout( hbox );
    d->resultsView   = new SearchResultsView( page );
    d->resultsView->setMinimumSize( QSize(200, 200) );
    d->resultsView->setMaximumWidth(  130*2 + d->resultsView->spacing()*3 + 20 );
    QWhatsThis::add( d->resultsView, i18n("<p>Here you can see the items found in album "
                                         "library using the current search settings."));
    hbox->addWidget( d->resultsView );

    // Box for all the rules
    d->rulesBox = new QVGroupBox( i18n("Search Rules"), page);
    QWhatsThis::add( d->rulesBox, i18n("<p>Here you can see the search rules list used to process items "
                                      "searching in album library."));
    d->rulesBox->layout()->setSpacing( spacingHint() );
    d->rulesBox->layout()->setMargin( 5 );
    d->rulesBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    d->rulesBox->layout()->setAlignment( Qt::AlignTop );
    leftSide->addWidget( d->rulesBox );

    // Push the rulesbox to top and the buttons down.
    leftSide->addStretch(10);

    // Box for the add/delete
    QGroupBox* groupbox = 0;
    groupbox            = new QGroupBox( i18n("Add/Delete Option"),
                                         page, "groupbox" );
    QWhatsThis::add( groupbox, i18n("<p>You can control here the search rules list contents, "
                                    "to adding/removing an option."));
                                         
    groupbox->setColumnLayout(0, Qt::Vertical );
    groupbox->layout()->setSpacing( KDialog::spacingHint() );
    groupbox->layout()->setMargin( KDialog::marginHint() );
    d->optionsCombo = new QComboBox( groupbox );
    d->optionsCombo->insertItem(i18n("As Well As"), 0);
    d->optionsCombo->insertItem(i18n("Or"), 1);
    d->optionsCombo->setEnabled(false);
    d->addButton    = new QPushButton(i18n("&Add"), groupbox);
    d->delButton    = new QPushButton(i18n("&Del"), groupbox);

    QHBoxLayout* box = 0;
    box = new QHBoxLayout( groupbox->layout() );
    box->addWidget( d->optionsCombo );
    box->addWidget( d->addButton );
    box->addStretch( 10 );
    box->addWidget( d->delButton );
    leftSide->addWidget( groupbox );

    // Box for the group/ungroup
    groupbox            = new QGroupBox( i18n("Group/Ungroup Options"),
                                         page, "groupbox" );
    QWhatsThis::add( groupbox, i18n("<p>You can group together or ungroup any search options "
                                    "from the Search Rules list."));   
    groupbox->setColumnLayout(0, Qt::Vertical );
    groupbox->layout()->setSpacing( KDialog::spacingHint() );
    groupbox->layout()->setMargin( KDialog::marginHint() );
    d->groupButton       = new QPushButton(i18n("&Group"), groupbox);
    d->ungroupButton     = new QPushButton(i18n("&Ungroup"), groupbox);

    box = new QHBoxLayout( groupbox->layout() );
    box->addWidget( d->groupButton );
    box->addStretch( 10 );
    box->addWidget( d->ungroupButton );
    leftSide->addWidget( groupbox );

    // box for saving the search.
    groupbox = new QGroupBox( page, "groupbox");
    groupbox->setColumnLayout(0, Qt::Vertical );
    groupbox->layout()->setSpacing( KDialog::spacingHint() );
    QLabel* label = new QLabel(i18n("&Save search as:"), groupbox);
    d->title = new KLineEdit(groupbox, "searchTitle");
    QWhatsThis::add( d->title, i18n("<p>Enter here the name used to save the current search in "
                                   "\"My Searches\" view"));
    groupbox->setFrameStyle( QFrame::NoFrame );

    box = new QHBoxLayout( groupbox->layout() );
    box->addWidget( label );
    box->addWidget( d->title );
    label->setBuddy(d->title);
    leftSide->addWidget( groupbox );

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

    connect(d->title, SIGNAL ( textChanged(const QString&) ),
            this, SLOT(slotChangeButtonStates() ));
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
        if (d->optionsCombo->currentItem() == 0 )
            type = SearchAdvancedBase::AND;
        else
            type = SearchAdvancedBase::OR;

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
    if (d->baseList.isEmpty()) {
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
    int     count  = 0;
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
    bool group = false;
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
        QString val  = url.queryItem(QString::number(i) + ".val");

        newRule.setPath("1");
        newRule.addQueryItem("1.key",key);
        newRule.addQueryItem("1.op",op);
        newRule.addQueryItem("1.val",val);

        rulesMap.insert(i, newRule);
    }

    QStringList strList = QStringList::split(" ", url.path());

    SearchAdvancedGroup* group = 0;
    bool groupingActive=false;
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

#include "searchadvanceddialog.moc"
