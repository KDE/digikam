/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "searchgroup.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QRadioButton>
#include <QStackedLayout>
#include <QVBoxLayout>

// KDE includes

#include <kcombobox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes

#include "searchfieldgroup.h"
#include "searchfields.h"
#include "searchutilities.h"
#include "searchview.h"

namespace Digikam
{

SearchGroup::SearchGroup(SearchView* const parent)
    : AbstractSearchGroupContainer(parent),
      m_view(parent),
      m_layout(0),
      m_label(0),
      m_subgroupLayout(0),
      m_groupType(FirstGroup)
{
}

void SearchGroup::setup(Type type)
{
    m_groupType = type;

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_label  = new SearchGroupLabel(m_view, m_groupType, this);
    m_layout->addWidget(m_label);

    connect(m_label, SIGNAL(removeClicked()),
            this, SIGNAL(removeRequested()));

    SearchFieldGroup*      group = 0;
    SearchFieldGroupLabel* label = 0;

    // ----- //

    group    = new SearchFieldGroup(this);
    group->addField(SearchField::createField("keyword", group));
    m_fieldGroups << group;
    m_layout->addWidget(group);

    // this group has no label. Need to show, else it is hidden forever
    group->setFieldsVisible(true);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("File, Album, Tags"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField("albumid", group));
    group->addField(SearchField::createField("albumname", group));
    group->addField(SearchField::createField("albumcollection", group));
    group->addField(SearchField::createField("tagid", group));
    group->addField(SearchField::createField("tagname", group));
    group->addField(SearchField::createField("notag", group));
    group->addField(SearchField::createField("filename", group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Picture Properties"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField("creationdate", group));
    group->addField(SearchField::createField("rating", group));
    group->addField(SearchField::createField("labels", group));
    group->addField(SearchField::createField("dimension", group));
    group->addField(SearchField::createField("pageorientation", group));
    group->addField(SearchField::createField("width", group));
    group->addField(SearchField::createField("height", group));
    group->addField(SearchField::createField("aspectratioimg", group));
    group->addField(SearchField::createField("pixelsize", group));
    group->addField(SearchField::createField("format", group));
    group->addField(SearchField::createField("colordepth", group));
    group->addField(SearchField::createField("colormodel", group));
    group->addField(SearchField::createField("modificationdate", group));
    group->addField(SearchField::createField("digitizationdate", group));
    group->addField(SearchField::createField("filesize", group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Video Properties"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField("videoaspectratio", group));
    group->addField(SearchField::createField("videoduration", group));
    group->addField(SearchField::createField("videoframerate", group));
    group->addField(SearchField::createField("videocodec", group));
    group->addField(SearchField::createField("videoaudiobitrate", group));
    group->addField(SearchField::createField("videoaudiochanneltype", group));
    group->addField(SearchField::createField("videoaudiocompressor", group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Caption, Comment, Title"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField("comment", group));
    group->addField(SearchField::createField("commentauthor", group));
    group->addField(SearchField::createField("headline", group));
    group->addField(SearchField::createField("title", group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Photograph Information"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField("make", group));
    group->addField(SearchField::createField("model", group));
    group->addField(SearchField::createField("aperture", group));
    group->addField(SearchField::createField("focallength", group));
    group->addField(SearchField::createField("focallength35", group));
    group->addField(SearchField::createField("exposuretime", group));
    group->addField(SearchField::createField("exposureprogram", group));
    group->addField(SearchField::createField("exposuremode", group));
    group->addField(SearchField::createField("sensitivity", group));
    group->addField(SearchField::createField("orientation", group));
    group->addField(SearchField::createField("flashmode", group));
    group->addField(SearchField::createField("whitebalance", group));
    group->addField(SearchField::createField("whitebalancecolortemperature", group));
    group->addField(SearchField::createField("meteringmode", group));
    group->addField(SearchField::createField("subjectdistance", group));
    group->addField(SearchField::createField("subjectdistancecategory", group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //


    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Geographic position"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    //group->addField(SearchField::createField("latitude", group));
    //group->addField(SearchField::createField("longitude", group));
    group->addField(SearchField::createField("altitude", group));
    group->addField(SearchField::createField("nogps", group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);


    // ----- //

    // prepare subgroup layout
    QHBoxLayout* indentLayout = new QHBoxLayout;
    indentLayout->setContentsMargins(0, 0, 0, 0);
    indentLayout->setSpacing(0);

    QStyleOption option;
    option.initFrom(this);
    int indent = 5 * style()->pixelMetric(QStyle::PM_LayoutLeftMargin, &option, this);
    indent     = qMax(indent, 20);
    indentLayout->addSpacing(indent);

    m_subgroupLayout = new QVBoxLayout;
    m_subgroupLayout->setContentsMargins(0, 0, 0, 0);
    m_subgroupLayout->setSpacing(0);

    indentLayout->addLayout(m_subgroupLayout);

    m_layout->addLayout(indentLayout);

    // ----- //

    m_layout->addStretch(1);
    setLayout(m_layout);

    // ----- //

    // initialization as empty group
    reset();
}

void SearchGroup::read(SearchXmlCachingReader& reader)
{
    reset();

    m_label->setGroupOperator(reader.groupOperator());
    m_label->setDefaultFieldOperator(reader.defaultFieldOperator());

    startReadingGroups(reader);

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isEndElement())
        {
            break;
        }

        // subgroup
        if (reader.isGroupElement())
        {
            readGroup(reader);
        }

        if (reader.isFieldElement())
        {
            QString name = reader.fieldName();

            SearchField* field = 0;
            SearchFieldGroup* fieldGroup = 0;
            foreach(fieldGroup, m_fieldGroups)
            {
                if ((field = fieldGroup->fieldForName(name)))
                {
                    break;
                }
            }

            if (field)
            {
                field->read(reader);
                fieldGroup->markField(field);
                fieldGroup->setFieldsVisible(true);
            }
            else
            {
                kWarning() << "Unhandled search field in XML with field name" << name;
                reader.readToEndOfElement();
            }
        }
    }

    finishReadingGroups();
}

SearchGroup* SearchGroup::createSearchGroup()
{
    // create a sub group - view is the same
    SearchGroup* group = new SearchGroup(m_view);
    group->setup(SearchGroup::ChainGroup);
    return group;
}

void SearchGroup::addGroupToLayout(SearchGroup* group)
{
    // insert in front of the stretch
    m_subgroupLayout->addWidget(group);
}

void SearchGroup::write(SearchXmlWriter& writer)
{
    writer.writeGroup();
    writer.setGroupOperator(m_label->groupOperator());
    writer.setDefaultFieldOperator(m_label->defaultFieldOperator());

    foreach(SearchFieldGroup* fieldGroup, m_fieldGroups)
    {
        fieldGroup->write(writer);
    }

    // take care for subgroups
    writeGroups(writer);

    writer.finishGroup();
}

void SearchGroup::reset()
{
    foreach(SearchFieldGroup* fieldGroup, m_fieldGroups)
    {
        fieldGroup->reset();
    }

    m_label->setGroupOperator(SearchXml::standardGroupOperator());
    m_label->setDefaultFieldOperator(SearchXml::standardFieldOperator());
}

SearchGroup::Type SearchGroup::groupType() const
{
    return m_groupType;
}

QList<QRect> SearchGroup::startupAnimationArea() const
{
    QList<QRect> rects;
    // from subgroups;
    rects += startupAnimationAreaOfGroups();
    // field groups
    foreach(SearchFieldGroup* fieldGroup, m_fieldGroups)
    {
        rects += fieldGroup->areaOfMarkedFields();
    }

    // adjust position relative to parent
    for (QList<QRect>::iterator it = rects.begin(); it != rects.end(); ++it)
    {
        (*it).translate(pos());
    }

    return rects;
}

// -------------------------------------------------------------------------

class RadioButtonHBox : public QHBoxLayout
{
public:

    RadioButtonHBox(QWidget* const left, QWidget* const right, Qt::LayoutDirection dir)
        : QHBoxLayout()
    {
        if (dir == Qt::RightToLeft)
        {
            addWidget(right, Qt::AlignRight);
            addWidget(left);
        }
        else
        {
            addWidget(left);
            addWidget(right, Qt::AlignLeft);
        }

        setSpacing(0);
    }
};

class SearchGroupLabel::Private
{
public:

    Private() :
      extended(false),
      groupOp(SearchXml::And),
      fieldOp(SearchXml::And),
      layout(0),
      groupOpLabel(0),
      allBox(0),
      anyBox(0),
      noneBox(0),
      oneNotBox(0),
      optionsLabel(0),
      removeLabel(0),
      stackedLayout(0),
      themeCache(0)
    {
    }

    bool                        extended;
    SearchXml::Operator         groupOp;
    SearchXml::Operator         fieldOp;
    QGridLayout*                layout;
    //QComboBox*                  groupOpBox;
    RClickLabel*                groupOpLabel;
    QRadioButton*               allBox;
    QRadioButton*               anyBox;
    QRadioButton*               noneBox;
    QRadioButton*               oneNotBox;
    RClickLabel*                optionsLabel;
    RClickLabel*                removeLabel;
    QStackedLayout*             stackedLayout;
    SearchViewThemedPartsCache* themeCache;
};

SearchGroupLabel::SearchGroupLabel(SearchViewThemedPartsCache* const cache, SearchGroup::Type type, QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    d->themeCache = cache;
    d->layout     = new QGridLayout;

    // leave styling to style sheet (by object name)

    QLabel* mainLabel     = new QLabel(i18n("Find Pictures"));
    mainLabel->setObjectName("SearchGroupLabel_MainLabel");

    // Use radio button with a separate label to fix styling problem, see bug 195809
    d->allBox             = new QRadioButton;
    QLabel* allBoxLabel   = new QLabel(i18n("Meet All of the following conditions"));
    allBoxLabel->setObjectName("SearchGroupLabel_CheckBox");

    d->anyBox = new QRadioButton;
    QLabel* anyBoxLabel   = new QLabel(i18n("Meet Any of the following conditions"));
    anyBoxLabel->setObjectName("SearchGroupLabel_CheckBox");

    d->noneBox = new QRadioButton;
    QLabel* noneBoxLabel  = new QLabel(i18n("None of these conditions are met"));
    noneBoxLabel->setObjectName("SearchGroupLabel_CheckBox");

    d->oneNotBox            = new QRadioButton;
    QLabel* oneNotBoxLabel = new QLabel(i18n("At least one of these conditions is not met"));
    oneNotBoxLabel->setObjectName("SearchGroupLabel_CheckBox");

    connect(d->allBox, SIGNAL(toggled(bool)),
            this, SLOT(boxesToggled()));

    connect(d->anyBox, SIGNAL(toggled(bool)),
            this, SLOT(boxesToggled()));

    connect(d->noneBox, SIGNAL(toggled(bool)),
            this, SLOT(boxesToggled()));

    connect(d->oneNotBox, SIGNAL(toggled(bool)),
            this, SLOT(boxesToggled()));

    if (type == SearchGroup::FirstGroup)
    {
        QLabel* logo = new QLabel;
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                        .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        d->optionsLabel = new RClickLabel;
        d->optionsLabel->setObjectName("SearchGroupLabel_OptionsLabel");

        connect(d->optionsLabel, SIGNAL(activated()),
                this, SLOT(toggleShowOptions()));

        QWidget* simpleHeader     = new QWidget;
        QVBoxLayout* headerLayout = new QVBoxLayout;
        QLabel* simpleLabel1      = new QLabel;
        //simpleLabel->setText(i18n("Find Pictures meeting all of these conditions"));
        //simpleLabel->setPixmap(SmallIcon("edit-find", 128));
        simpleLabel1->setText(i18n("<qt><p>Search your collection<br/>for pictures meeting the following conditions</p></qt>"));
        simpleLabel1->setObjectName("SearchGroupLabel_SimpleLabel");
        headerLayout->addStretch(3);
        headerLayout->addWidget(simpleLabel1);
        headerLayout->addStretch(1);
        headerLayout->setMargin(0);
        simpleHeader->setLayout(headerLayout);

        QWidget* optionsBox        = new QWidget;
        QGridLayout* optionsLayout = new QGridLayout;
        optionsLayout->addLayout(new RadioButtonHBox(d->allBox, allBoxLabel, layoutDirection()),       0, 0);
        optionsLayout->addLayout(new RadioButtonHBox(d->anyBox, anyBoxLabel, layoutDirection()),       1, 0);
        optionsLayout->addLayout(new RadioButtonHBox(d->noneBox, noneBoxLabel, layoutDirection()),     0, 1);
        optionsLayout->addLayout(new RadioButtonHBox(d->oneNotBox, oneNotBoxLabel, layoutDirection()), 1, 1);
        optionsLayout->setMargin(0);
        optionsBox->setLayout(optionsLayout);

        d->stackedLayout = new QStackedLayout;
        d->stackedLayout->addWidget(simpleHeader);
        d->stackedLayout->addWidget(optionsBox);
        d->stackedLayout->setMargin(0);

        d->layout->addWidget(mainLabel,       0, 0, 1, 1);
        d->layout->addLayout(d->stackedLayout, 1, 0, 1, 1);
        d->layout->addWidget(d->optionsLabel,  1, 1, 1, 1, Qt::AlignRight | Qt::AlignBottom);
        d->layout->addWidget(logo,            0, 2, 2, 1, Qt::AlignTop);
        d->layout->setColumnStretch(1, 10);

        setExtended(false);
    }
    else
    {
        d->groupOpLabel = new RClickLabel;
        d->groupOpLabel->setObjectName("SearchGroupLabel_GroupOpLabel");

        connect(d->groupOpLabel, SIGNAL(activated()),
                this, SLOT(toggleGroupOperator()));

        d->removeLabel = new RClickLabel(i18n("Remove Group"));
        d->removeLabel->setObjectName("SearchGroupLabel_RemoveLabel");

        connect(d->removeLabel, SIGNAL(activated()),
                this, SIGNAL(removeClicked()));

        d->layout->addWidget(d->groupOpLabel, 0, 0, 1, 1);
        d->layout->addLayout(new RadioButtonHBox(d->allBox, allBoxLabel, layoutDirection()),       1, 0, 1, 1);
        d->layout->addLayout(new RadioButtonHBox(d->anyBox, anyBoxLabel, layoutDirection()),       2, 0, 1, 1);
        d->layout->addLayout(new RadioButtonHBox(d->noneBox, noneBoxLabel, layoutDirection()),     3, 0, 1, 1);
        d->layout->addLayout(new RadioButtonHBox(d->oneNotBox, oneNotBoxLabel, layoutDirection()), 4, 0, 1, 1);
        d->layout->addWidget(d->removeLabel,  0, 2, 1, 1);
        d->layout->setColumnStretch(1, 10);
    }

    setLayout(d->layout);

    // Default values
    setGroupOperator(SearchXml::standardGroupOperator());
    setDefaultFieldOperator(SearchXml::standardFieldOperator());
}

SearchGroupLabel::~SearchGroupLabel()
{
    delete d;
}

void SearchGroupLabel::setExtended(bool extended)
{
    d->extended = extended;

    if (!d->stackedLayout)
    {
        return;
    }

    if (d->extended)
    {
        d->stackedLayout->setCurrentIndex(1);
        d->allBox->setVisible(true);
        d->anyBox->setVisible(true);
        d->noneBox->setVisible(true);
        d->oneNotBox->setVisible(true);
        d->optionsLabel->setText(i18n("Hide Options <<"));
    }
    else
    {
        d->stackedLayout->setCurrentIndex(0);
        // hide to reduce reserved space in stacked layout
        d->allBox->setVisible(false);
        d->anyBox->setVisible(false);
        d->noneBox->setVisible(false);
        d->oneNotBox->setVisible(false);
        d->optionsLabel->setText(i18n("Options >>"));
    }
}

void SearchGroupLabel::toggleShowOptions()
{
    setExtended(!d->extended);
}

void SearchGroupLabel::toggleGroupOperator()
{
    if (d->groupOp == SearchXml::And)
    {
        d->groupOp = SearchXml::Or;
    }
    else if (d->groupOp == SearchXml::Or)
    {
        d->groupOp = SearchXml::And;
    }
    else if (d->groupOp == SearchXml::AndNot)
    {
        d->groupOp = SearchXml::OrNot;
    }
    else if (d->groupOp == SearchXml::OrNot)
    {
        d->groupOp = SearchXml::AndNot;
    }

    updateGroupLabel();
}

void SearchGroupLabel::boxesToggled()
{
    // set field op
    if (d->allBox->isChecked() || d->oneNotBox->isChecked())
    {
        d->fieldOp = SearchXml::And;
    }
    else
    {
        d->fieldOp = SearchXml::Or;
    }

    // negate group op
    if (d->allBox->isChecked() || d->anyBox->isChecked())
    {
        if (d->groupOp == SearchXml::AndNot)
        {
            d->groupOp = SearchXml::And;
        }
        else if (d->groupOp == SearchXml::OrNot)
        {
            d->groupOp = SearchXml::Or;
        }
    }
    else
    {
        if (d->groupOp == SearchXml::And)
        {
            d->groupOp = SearchXml::AndNot;
        }
        else if (d->groupOp == SearchXml::Or)
        {
            d->groupOp = SearchXml::OrNot;
        }
    }
}

void SearchGroupLabel::setGroupOperator(SearchXml::Operator op)
{
    d->groupOp = op;
    adjustOperatorOptions();
    updateGroupLabel();
}

void SearchGroupLabel::updateGroupLabel()
{
    if (d->groupOpLabel)
    {
        if (d->groupOp == SearchXml::And || d->groupOp == SearchXml::AndNot)
        {
            d->groupOpLabel->setText(i18n("AND"));
        }
        else
        {
            d->groupOpLabel->setText(i18n("OR"));
        }
    }
}

void SearchGroupLabel::setDefaultFieldOperator(SearchXml::Operator op)
{
    d->fieldOp = op;
    adjustOperatorOptions();
}

void SearchGroupLabel::adjustOperatorOptions()
{
    // In the UI, the NOT is done at the level of the field operator,
    // but in fact we put a NOT in front of the whole group, so it is the group operator!

    // 1. allBox, All of these conditions are met: (A && B && C), Group And/Or, Field And
    // 2. anyBox, Any of these conditions are met: (A || B || C), Group And/Or, Field Or
    // 3. oneNotBox, At least one of these conditions is not met: !(A && B && C) = (!A || !B || !C),
    //    Group AndNot/OrNot, Field And
    // 4. noneBox, None of these conditions are met: !(A || B || C) = (!A && !B && !C),
    //    Group AndNot/OrNot, Field Or

    switch (d->groupOp)
    {
        case SearchXml::And:
        case SearchXml::Or:

            if (d->fieldOp == SearchXml::And)
            {
                d->allBox->setChecked(true);
            }
            else
            {
                d->anyBox->setChecked(true);
            }

            break;

        case SearchXml::AndNot:
        case SearchXml::OrNot:

            if (d->fieldOp == SearchXml::And)
            {
                d->oneNotBox->setChecked(true);
            }
            else
            {
                d->noneBox->setChecked(true);
            }

            break;
    }

    if (!d->allBox->isChecked())
    {
        setExtended(true);
    }
}

SearchXml::Operator SearchGroupLabel::groupOperator() const
{
    return d->groupOp;
}

SearchXml::Operator SearchGroupLabel::defaultFieldOperator() const
{
    if (d->anyBox->isChecked() || d->noneBox->isChecked())
    {
        return SearchXml::Or;
    }
    else
    {
        return SearchXml::And;
    }
}

void SearchGroupLabel::paintEvent(QPaintEvent*)
{
    // paint themed background
    QPainter p(this);
    p.drawPixmap(0, 0, d->themeCache->groupLabelPixmap(width(), height()));
}

} // namespace Digikam
