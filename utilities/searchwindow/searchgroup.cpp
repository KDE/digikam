/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes.

#include "searchgroup.h"
#include "searchgroup.moc"

// Qt includes.

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QRadioButton>
#include <QVBoxLayout>

// KDE includes.

#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Digikam includes.

#include "searchview.h"
#include "searchfields.h"
#include "searchfieldgroup.h"
#include "searchutilities.h"

namespace Digikam
{


SearchGroup::SearchGroup(SearchView *parent)
    : AbstractSearchGroupContainer(parent), m_view(parent), m_layout(0), m_label(0), m_groupType(FirstGroup)
{
}

void SearchGroup::setup(Type type)
{
    m_groupType = type;

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_label = new SearchGroupLabel(m_view, m_groupType, this);
    m_layout->addWidget(m_label);

    connect(m_label, SIGNAL(removeClicked()),
            this, SIGNAL(removeRequested()));

    SearchFieldGroup *group;
    SearchFieldGroupLabel *label;

    // ----- //

    group = new SearchFieldGroup(this);
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
    group->addField(SearchField::createField("tagid", group));
    group->addField(SearchField::createField("tagname", group));
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
    group->addField(SearchField::createField("dimension", group));
    group->addField(SearchField::createField("orientation", group));
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

    /*
    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Geographic position");
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField("latitude", group));
    group->addField(SearchField::createField("longitude", group));
    group->addField(SearchField::createField("altitude", group));
    group->addField(SearchField::createField("nogps", group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);
    */

    // ----- //

    // prepare subgroup layout
    QHBoxLayout *indentLayout = new QHBoxLayout;
    indentLayout->setContentsMargins(0, 0, 0, 0);
    indentLayout->setSpacing(0);

    QStyleOption option;
    option.initFrom(this);
    int indent = 5 * style()->pixelMetric(QStyle::PM_LayoutLeftMargin, &option, this);
    indent = qMax(indent, 20);
    indentLayout->addSpacing(indent);

    m_subgroupLayout = new QVBoxLayout;
    m_subgroupLayout->setContentsMargins(0, 0, 0, 0);
    m_subgroupLayout->setSpacing(0);

    indentLayout->addLayout(m_subgroupLayout);

    m_layout->addLayout(indentLayout);

    // ----- //

    m_layout->addStretch(1);
    setLayout(m_layout);
}

void SearchGroup::read(SearchXmlCachingReader &reader)
{
    reset();

    m_label->setGroupOperator(reader.groupOperator());
    m_label->setDefaultFieldOperator(reader.defaultFieldOperator());

    startReadingGroups(reader);
    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isEndElement())
            break;

        // subgroup
        if (reader.isGroupElement())
        {
            readGroup(reader);
        }

        if (reader.isFieldElement())
        {
            QString name = reader.fieldName();

            SearchField *field = 0;
            SearchFieldGroup *fieldGroup = 0;
            foreach (fieldGroup, m_fieldGroups)
            {
                if ( (field = fieldGroup->fieldForName(name)) )
                    break;
            }

            if (field)
            {
                field->read(reader);
                fieldGroup->markField(field);
                fieldGroup->setFieldsVisible(true);
            }
            else
            {
                kWarning(50003) << "Unhandled search field in XML with field name" << name << endl;
                reader.readToEndOfElement();
            }
        }
    }
    finishReadingGroups();
}

SearchGroup *SearchGroup::createSearchGroup()
{
    // create a sub group - view is the same
    SearchGroup *group = new SearchGroup(m_view);
    group->setup(SearchGroup::ChainGroup);
    return group;
}

void SearchGroup::addGroupToLayout(SearchGroup *group)
{
    // insert in front of the stretch
    m_subgroupLayout->addWidget(group);
}

void SearchGroup::write(SearchXmlWriter &writer)
{
    writer.writeGroup();
    writer.setGroupOperator(m_label->groupOperator());
    writer.setDefaultFieldOperator(m_label->defaultFieldOperator());

    foreach (SearchFieldGroup *fieldGroup, m_fieldGroups)
    {
        fieldGroup->write(writer);
    }

    // take care for subgroups
    writeGroups(writer);

    writer.finishGroup();
}

void SearchGroup::reset()
{
    foreach (SearchFieldGroup *fieldGroup, m_fieldGroups)
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
    foreach (SearchFieldGroup *fieldGroup, m_fieldGroups)
        rects += fieldGroup->areaOfMarkedFields();
    // adjust position relative to parent
    for (QList<QRect>::iterator it = rects.begin(); it != rects.end(); ++it)
        (*it).translate(pos());
    return rects;
}

// ----------------------------------- //


SearchGroupLabel::SearchGroupLabel(SearchViewThemedPartsCache *cache, SearchGroup::Type type, QWidget *parent)
    : QWidget(parent), m_groupOpBox(0), m_themeCache(cache)
{
    QGridLayout *m_layout = new QGridLayout;

    // leave styling to style sheet (by object name)

    QLabel *mainLabel = new QLabel(i18n("Find Pictures"));
    mainLabel->setObjectName("SearchGroupLabel_MainLabel");

    m_allBox = new QRadioButton(i18n("Match All of the following conditions"));
    m_allBox->setObjectName("SearchGroupLabel_CheckBox");

    m_anyBox = new QRadioButton(i18n("Match Any of the following conditions"));
    m_anyBox->setObjectName("SearchGroupLabel_CheckBox");

    if (type == SearchGroup::FirstGroup)
    {
        QLabel *logo = new QLabel;
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        m_layout->addWidget(mainLabel,      0, 0, 1, 1);
        m_layout->addWidget(m_allBox,       1, 0, 1, 1);
        m_layout->addWidget(m_anyBox,       2, 0, 1, 1);
        m_layout->addWidget(logo,           0, 2, 3, 1);
        m_layout->setColumnStretch(1, 10);
    }
    else
    {
        m_groupOpBox = new KComboBox;
        m_groupOpBox->addItem("- OR -", SearchXml::Or);
        m_groupOpBox->addItem("- AND -", SearchXml::And);
        m_groupOpBox->addItem("- AND NOT -", SearchXml::AndNot);

        m_removeLabel = new SearchClickLabel(i18n("Remove Group"));
        m_removeLabel->setObjectName("SearchGroupLabel_RemoveLabel");
        connect(m_removeLabel, SIGNAL(leftClicked()),
                this, SIGNAL(removeClicked()));

        m_layout->addWidget(m_groupOpBox,   0, 0, 1, 1);
        m_layout->addWidget(m_allBox,       1, 0, 1, 1);
        m_layout->addWidget(m_anyBox,       2, 0, 1, 1);
        m_layout->addWidget(m_removeLabel,  0, 2, 1, 1); //, Qt::AlignRight | Qt::AlignTop);
        m_layout->setColumnStretch(1, 10);
    }

    setLayout(m_layout);
}

void SearchGroupLabel::setGroupOperator(SearchXml::Operator op)
{
    if (m_groupOpBox)
        m_groupOpBox->setCurrentIndex(m_groupOpBox->findData(op));
}

void SearchGroupLabel::setDefaultFieldOperator(SearchXml::Operator op)
{
    if (op == SearchXml::Or)
        m_anyBox->setChecked(true);
    else
        m_allBox->setChecked(true);
}

SearchXml::Operator SearchGroupLabel::groupOperator() const
{
    if (m_groupOpBox && m_groupOpBox->currentIndex() != -1)
        return (SearchXml::Operator)m_groupOpBox->itemData(m_groupOpBox->currentIndex()).toInt();
    return SearchXml::Or;
}

SearchXml::Operator SearchGroupLabel::defaultFieldOperator() const
{
    if (m_anyBox->isChecked())
        return SearchXml::Or;
    else
        return SearchXml::And;
}

void SearchGroupLabel::paintEvent(QPaintEvent *)
{
    // paint themed background
    QPainter p(this);
    p.drawPixmap(0, 0, m_themeCache->groupLabelPixmap(width(), height()));
}

}



