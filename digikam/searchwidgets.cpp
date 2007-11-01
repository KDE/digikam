/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : search widgets collection.
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005 by Tom Albers <tomalbers@kde.nl>
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

/** @file searchwidgets.cpp */

// Qt includes.

#include <Q3ValueList>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QLayout>
#include <QDateTime>
#include <QHBoxLayout>
#include <QFrame>
#include <QMouseEvent>

// KDE includes.

#include <klocale.h>
#include <kurl.h>
#include <kdialog.h>
#include <kvbox.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albuminfo.h"
#include "albummanager.h"
#include "ratingwidget.h"
#include "squeezedcombobox.h"
#include "kdateedit.h"
#include "searchwidgets.h"
#include "searchwidgets.moc"

namespace Digikam
{

static const int RuleKeyTableCount = 11;
static const int RuleOpTableCount  = 16;

static struct
{
    const char *keyText;
    QString key;
    SearchAdvancedRule::valueWidgetTypes cat;
}
RuleKeyTable[] =
{
    { I18N_NOOP("Album"),            "album",           SearchAdvancedRule::ALBUMS   },
    { I18N_NOOP("Album Name"),       "albumname",       SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Album Caption"),    "albumcaption",    SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Album Collection"), "albumcollection", SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Tag"),              "tag",             SearchAdvancedRule::TAGS     },
    { I18N_NOOP("Tag Name"),         "tagname",         SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Image Name"),       "imagename",       SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Image Date"),       "imagedate",       SearchAdvancedRule::DATE     },
    { I18N_NOOP("Image Caption"),    "imagecaption",    SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Keyword"),          "keyword",         SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Rating"),           "rating",          SearchAdvancedRule::RATING   },
};

static struct
{
    const char *keyText;
    QString     key;
    SearchAdvancedRule::valueWidgetTypes cat;
}
RuleOpTable[] =
{
    { I18N_NOOP("Contains"),           "LIKE",         SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Does Not Contain"),   "NLIKE",        SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Equals"),             "EQ",           SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Does Not Equal"),     "NE",           SearchAdvancedRule::LINEEDIT },
    { I18N_NOOP("Equals"),             "EQ",           SearchAdvancedRule::ALBUMS   },
    { I18N_NOOP("Does Not Equal"),     "NE",           SearchAdvancedRule::ALBUMS   },
    { I18N_NOOP("Equals"),             "EQ",           SearchAdvancedRule::TAGS     },
    { I18N_NOOP("Does Not Equal"),     "NE",           SearchAdvancedRule::TAGS     },
    { I18N_NOOP("Contains"),           "LIKE",         SearchAdvancedRule::TAGS     },
    { I18N_NOOP("Does Not Contain"),   "NLIKE",        SearchAdvancedRule::TAGS     },
    { I18N_NOOP("After"),              "GT",           SearchAdvancedRule::DATE     },
    { I18N_NOOP("Before"),             "LT",           SearchAdvancedRule::DATE     },
    { I18N_NOOP("Equals"),             "EQ",           SearchAdvancedRule::DATE     },
    { I18N_NOOP("At least"),           "GTE",          SearchAdvancedRule::RATING   },
    { I18N_NOOP("At most"),            "LTE",          SearchAdvancedRule::RATING   },
    { I18N_NOOP("Equals"),             "EQ",           SearchAdvancedRule::RATING   },
};

//-------------------------------------------------------------------------------

SearchRuleLabel::SearchRuleLabel(const QString& text, QWidget *parent,
                                 const char *name, Qt::WFlags f )
               : QLabel(text, parent)
{
    setObjectName(name);    
    setWindowFlags(f);    
}

void SearchRuleLabel::mouseDoubleClickEvent(QMouseEvent * e)
{
   emit signalDoubleClick( e );
}

//-------------------------------------------------------------------------------

SearchAdvancedRule::SearchAdvancedRule(QWidget* parent, SearchAdvancedRule::Option option)
                  : SearchAdvancedBase(SearchAdvancedBase::RULE)
{
    m_box = new KVBox(parent);
    m_box->layout()->setSpacing(KDialog::spacingHint());
    m_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_optionsBox = 0;
    m_option     = option;
    if (option != NONE)
    {
        m_optionsBox  = new KHBox(m_box);
        m_label       = new SearchRuleLabel(option == AND ? i18n("As well as") : i18n("Or"), m_optionsBox);
        QFrame* hline = new QFrame(m_optionsBox);
        hline->setFrameStyle(QFrame::HLine|QFrame::Sunken);
        m_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        hline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        connect(m_label, SIGNAL(signalDoubleClick(QMouseEvent*)),
                this, SLOT(slotLabelDoubleClick()));
    }

    m_hbox = new QWidget(m_box);
    m_hbox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_key = new QComboBox(m_hbox);
    m_key->setObjectName("key");
    m_key->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    for (int i = 0; i < RuleKeyTableCount; i++)
        m_key->insertItem(i, i18n(RuleKeyTable[i].keyText));

    m_operator = new QComboBox(m_hbox);
    m_operator->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    m_valueBox   = new KHBox(m_hbox);
    m_widgetType = NOWIDGET;

    slotKeyChanged(0);
    m_check = new QCheckBox(m_hbox);

    m_hboxLayout = new QHBoxLayout(m_hbox);
    m_hboxLayout->setMargin(KDialog::spacingHint());
    m_hboxLayout->setSpacing(KDialog::spacingHint());
    m_hboxLayout->addWidget(m_key);
    m_hboxLayout->addWidget(m_operator);
    m_hboxLayout->addWidget(m_valueBox);
    m_hboxLayout->addWidget(m_check, 0, Qt::AlignRight);

    m_box->show();

    connect(m_key, SIGNAL(activated(int)),
            this, SLOT(slotKeyChanged(int)));

    connect(m_key, SIGNAL(activated(int)),
            this, SIGNAL(signalPropertyChanged()));

    connect(m_operator, SIGNAL(activated(int)),
            this, SIGNAL(signalPropertyChanged()));

    connect(m_check, SIGNAL(toggled(bool)),
            this, SIGNAL(signalBaseItemToggled()));
}

void SearchAdvancedRule::setValues(const KUrl& url)
{
    if (url.isEmpty())
        return;

    // set the key widget
    for (int i=0; i< RuleKeyTableCount; i++)
    {
        if (RuleKeyTable[i].key == url.queryItem("1.key"))
        {
            m_key->setCurrentIndex(i);
        }
    }

    // set the operator and the last widget
    slotKeyChanged( m_key->currentIndex() );
    for (int i=0; i< RuleOpTableCount; i++)
    {
        if ( RuleOpTable[i].key == url.queryItem("1.op") &&
             RuleOpTable[i].cat == m_widgetType )
        {
            m_operator->setCurrentIndex(i);
        }
    }

    // Set the value for the last widget.
    QString value = url.queryItem("1.val");
    if (m_widgetType == LINEEDIT)
        m_lineEdit->setText( value );

    if (m_widgetType == DATE)
        m_dateEdit->setDate( QDate::fromString( value, Qt::ISODate) );

    if (m_widgetType == RATING)
    {
        bool ok;
        int  num = value.toInt(&ok);
        if (ok)
            m_ratingWidget->setRating( num );
    }

    if (m_widgetType == TAGS || m_widgetType == ALBUMS)
    {
        bool ok;
        int  num = value.toInt(&ok);
        if (ok)
        {
            QMap<int, int>::iterator it;
            for (it = m_itemsIndexIDMap.begin() ; it != m_itemsIndexIDMap.end(); ++it)
            {
                if (it.value() == num)
                    m_valueCombo->setCurrentIndex( it.key() );
            }
        }
    }
}

SearchAdvancedRule::~SearchAdvancedRule()
{
    delete m_box;
}

void SearchAdvancedRule::slotLabelDoubleClick()
{
    if (m_option == AND)
    {
        m_option=OR;
        m_label->setText( i18n("Or") );
    }
    else
    {
        m_option=AND;
        m_label->setText( i18n("As well as") );
    }
    emit signalPropertyChanged();
}

void SearchAdvancedRule::slotKeyChanged(int id)
{
    QString currentOperator      = m_operator->currentText();
    valueWidgetTypes currentType = m_widgetType;

    m_operator->clear();
    m_widgetType = RuleKeyTable[id].cat;

    for (int i = 0; i < RuleOpTableCount; i++)
    {
        if ( RuleOpTable[i].cat == m_widgetType )
        {
            m_operator->addItem( i18n(RuleOpTable[i].keyText) );
    
            if ( currentOperator == RuleOpTable[i].key )
                m_operator->setCurrentIndex(i);
        }
    }
    m_operator->adjustSize();
    setValueWidget( currentType, m_widgetType );
}

void SearchAdvancedRule::setValueWidget(valueWidgetTypes oldType, valueWidgetTypes newType)
{
    if (oldType == newType)
        return;

    if (m_lineEdit && oldType == LINEEDIT)
        delete m_lineEdit;

    if (m_dateEdit && oldType == DATE)
        delete m_dateEdit;

    if (m_ratingWidget && oldType == RATING)
	   delete m_ratingWidget;

    if (m_valueCombo && (oldType == ALBUMS || oldType == TAGS))
        delete m_valueCombo;

    if (newType == DATE)
    {
        m_dateEdit = new KDateEdit( m_valueBox);
        m_dateEdit->setObjectName("datepicker");
        m_dateEdit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        m_dateEdit->show();

        connect(m_dateEdit, SIGNAL( dateChanged(const QDate& ) ),
                this, SIGNAL(signalPropertyChanged()));
    }
    else if (newType == LINEEDIT)
    {
        m_lineEdit = new QLineEdit( m_valueBox);
        m_lineEdit->setObjectName("lineedit");
        m_lineEdit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        m_lineEdit->show();

        connect(m_lineEdit, SIGNAL(textChanged(const QString&)),
                this, SIGNAL(signalPropertyChanged()));
    }
    else if (newType == ALBUMS)
    {
        m_valueCombo = new SqueezedComboBox(m_valueBox);
        m_valueCombo->setObjectName("albumscombo");
        m_valueCombo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

        AlbumManager* aManager = AlbumManager::instance();
        AlbumList aList        = aManager->allPAlbums();

        m_itemsIndexIDMap.clear();
        int index = 0;
        for( AlbumList::Iterator it = aList.begin();
             it != aList.end(); ++it )
        {
            PAlbum *album = (PAlbum*)(*it);
            if ( !album->isRoot() )
            {
                m_valueCombo->insertSqueezedItem( album->albumPath().remove(0,1), index );
                m_itemsIndexIDMap.insert(index, album->id());
                index++;
            }
        }

        m_valueCombo->show();

        connect(m_valueCombo, SIGNAL(activated(int)),
                this, SIGNAL(signalPropertyChanged()));
    }
    else if (newType == TAGS)
    {
        m_valueCombo = new SqueezedComboBox(m_valueBox);
        m_valueCombo->setObjectName("tagscombo");
        m_valueCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        AlbumManager* aManager = AlbumManager::instance();
        AlbumList tList        = aManager->allTAlbums();

        m_itemsIndexIDMap.clear();
        int index = 0;
        for( AlbumList::Iterator it = tList.begin();
             it != tList.end(); ++it )
        {
            TAlbum *album = (TAlbum*)(*it);
            if ( !album->isRoot() )
            {
                m_valueCombo->insertSqueezedItem( album->tagPath(false), index );
                m_itemsIndexIDMap.insert( index, album->id() );
                ++index;
            }
        }

        m_valueCombo->show();

        connect(m_valueCombo, SIGNAL(activated(int)),
                this, SIGNAL(signalPropertyChanged()));
    }
    else if (newType == RATING)
    {
        m_ratingWidget = new RatingWidget( m_valueBox );
        m_ratingWidget->show();

        connect(m_ratingWidget, SIGNAL(signalRatingChanged(int)),
                this, SIGNAL(signalPropertyChanged()));
    }
}

QString SearchAdvancedRule::urlKey() const
{
    return RuleKeyTable[m_key->currentIndex()].key;
}

QString SearchAdvancedRule::urlOperator() const
{
    QString string;

    int countItems = 0;
    for (int i=0; i< RuleOpTableCount; i++)
    {
        if ( RuleOpTable[i].cat == m_widgetType )
        {
            if( countItems == m_operator->currentIndex() )
                string = RuleOpTable[i].key;
            ++countItems;
        }
    }

    return string;
}

QString SearchAdvancedRule::urlValue() const
{
    QString string;

    if (m_widgetType == LINEEDIT)
        string = m_lineEdit->text() ;

    else if (m_widgetType == DATE)
        string = m_dateEdit->date().toString(Qt::ISODate) ;

    else if (m_widgetType == TAGS || m_widgetType == ALBUMS)
        string = QString::number(m_itemsIndexIDMap[ m_valueCombo->currentIndex() ]);

    else if (m_widgetType == RATING)
        string = QString::number(m_ratingWidget->rating()) ;

    return string;
}

QWidget* SearchAdvancedRule::widget() const
{
    return m_box;
}

bool SearchAdvancedRule::isChecked() const
{
    return (m_check && m_check->isChecked());
}

void SearchAdvancedRule::addOption(Option option)
{
    if (option == NONE)
    {
        removeOption();
        return;
    }

    m_box->layout()->removeWidget(m_hbox);

    m_optionsBox = new KHBox(m_box);
    new QLabel(option == AND ? i18n("As well as") : i18n("Or"), m_optionsBox);
    QFrame* hline = new QFrame(m_optionsBox);
    hline->setFrameStyle(QFrame::HLine|QFrame::Sunken);
    hline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_optionsBox->show();

    m_box->layout()->addWidget(m_hbox);
    m_option = option;
}

void SearchAdvancedRule::removeOption()
{
    m_option = NONE;
    delete m_optionsBox;
    m_optionsBox = 0;
}

void SearchAdvancedRule::addCheck()
{
    m_check = new QCheckBox(m_hbox);
    m_check->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_hboxLayout->addWidget( m_check, 0, Qt::AlignRight );
    m_check->show();

    connect(m_check, SIGNAL(toggled(bool)),
            this, SIGNAL(signalBaseItemToggled()));
}

void SearchAdvancedRule::removeCheck()
{
    delete m_check;
    m_check = 0;
}

//-------------------------------------------------------------------------------

SearchAdvancedGroup::SearchAdvancedGroup(QWidget* parent)
                   : SearchAdvancedBase(SearchAdvancedBase::GROUP)
{
    m_box          = new KHBox(parent);
    m_box->layout()->setSpacing(KDialog::spacingHint());
    m_groupbox     = new QGroupBox(m_box);
    m_groupboxVLay = new QVBoxLayout(m_groupbox);
    m_groupboxVLay->setMargin(KDialog::spacingHint());
    m_groupboxVLay->setSpacing(KDialog::spacingHint());
    m_check        = new QCheckBox(m_box);
    m_option       = SearchAdvancedRule::NONE;
    m_box->show();

    connect(m_check, SIGNAL(toggled(bool)),
            this, SIGNAL(signalBaseItemToggled()));
}

SearchAdvancedGroup::~SearchAdvancedGroup()
{
    delete m_box;
}

QWidget* SearchAdvancedGroup::widget() const
{
    return m_box;
}

bool SearchAdvancedGroup::isChecked() const
{
    return m_check->isChecked();
}

void SearchAdvancedGroup::addRule(SearchAdvancedRule* rule)
{
    if (m_childRules.isEmpty() && rule->option() != SearchAdvancedRule::NONE)
    {
        // this is the first rule being inserted in this group.
        // get its option and remove its option
        addOption(rule->option());
        rule->removeOption();
    }

    rule->removeCheck();

    m_childRules.append(rule);
    rule->widget()->setParent(m_groupbox);
    m_groupboxVLay->addWidget(rule->widget());
    rule->widget()->show();
}

void SearchAdvancedGroup::removeRules()
{
    typedef Q3ValueList<SearchAdvancedRule*> RuleList;

    for (RuleList::iterator it = m_childRules.begin();
         it != m_childRules.end(); ++it)
    {
        SearchAdvancedRule* rule = (SearchAdvancedRule*)(*it);
        if (it == m_childRules.begin())
        {
            rule->addOption(m_option);
        }

        rule->addCheck();
        rule->widget()->setParent((QWidget*)m_box->parent());
        rule->widget()->show();
    }

    m_childRules.clear();
    removeOption();
}

Q3ValueList<SearchAdvancedRule*> SearchAdvancedGroup::childRules() const
{
    return m_childRules;
}

void SearchAdvancedGroup::addOption(Option option)
{
    m_option = option;
    m_groupbox->setTitle(m_option == SearchAdvancedRule::AND ? i18n("As well as") : i18n("Or"));
}

void SearchAdvancedGroup::removeOption()
{
    m_option = NONE;
    m_groupbox->setTitle("");
}

}  // namespace Digikam
