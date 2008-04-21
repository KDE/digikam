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

// Qt includes

#include <QVBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QRadioButton>
#include <QLabel>
#include <QPainter>

// KDE includes

#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

// Local includes

#include "ddebug.h"
#include "searchview.h"
#include "searchfields.h"
#include "searchfieldgroup.h"
#include "searchgroup.h"
#include "searchgroup.moc"

namespace Digikam
{


SearchGroup::SearchGroup(SearchView *parent)
    : QWidget(parent), m_view(parent), m_layout(0), m_label(0), m_isFirstGroup(true)
{
}

void SearchGroup::setup()
{
    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_label = new SearchGroupLabel(m_view, this);
    m_layout->addWidget(m_label);

    SearchFieldGroup *group;
    SearchFieldGroupLabel *label;

    // ----- //

    group = new SearchFieldGroup(this);
    group->addField(SearchField::createField("keyword", group));
    m_fieldGroups << group;
    m_layout->addWidget(group);

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

    m_layout->addStretch(1);
    setLayout(m_layout);
}

void SearchGroup::setChainSearchGroup()
{
    m_isFirstGroup = false;
    m_label->addGroupOperatorOption();
}

void SearchGroup::read(SearchXmlCachingReader &reader)
{
    reset();

    m_label->setGroupOperator(reader.groupOperator());
    m_label->setDefaultFieldOperator(reader.defaultFieldOperator());

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isEndElement())
            break;

        // subgroup
        if (reader.isGroupElement())
        {
            // TODO
        }

        if (reader.isFieldElement())
        {
            QString name = reader.fieldName();

            SearchField *field = 0;
            foreach (SearchFieldGroup *fieldGroup, m_fieldGroups)
            {
                if ( (field = fieldGroup->fieldForName(name)) )
                    break;
            }

            if (field)
                field->read(reader);
            else
            {
                DWarning() << "Unhandled search field in XML with field name" << name;
                reader.readToEndOfElement();
            }
        }
    }
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

    writer.finishGroup();
}

void SearchGroup::reset()
{
    foreach (SearchFieldGroup *fieldGroup, m_fieldGroups)
    {
        fieldGroup->reset();
    }
}

// ----------------------------------- //


SearchGroupLabel::SearchGroupLabel(SearchViewThemedPartsCache *cache, QWidget *parent)
    : QWidget(parent), m_groupOpBox(0), m_themeCache(cache)
{
    QVBoxLayout *m_layout = new QVBoxLayout;

    // leave styling to style sheet (by object name)

    QWidget *header   = new QWidget(this);
    QGridLayout *grid = new QGridLayout(header);

    QLabel *mainLabel = new QLabel(i18n("Find Pictures"));
    mainLabel->setObjectName("SearchGroupLabel_MainLabel");

    m_allBox = new QRadioButton(i18n("Match All of the following conditions"));
    m_allBox->setObjectName("SearchGroupLabel_CheckBox");

    m_anyBox = new QRadioButton(i18n("Match Any of the following conditions"));
    m_anyBox->setObjectName("SearchGroupLabel_CheckBox");

    QLabel *logo = new QLabel(header);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                            .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    grid->addWidget(mainLabel, 0, 0, 1, 1);
    grid->addWidget(m_allBox,  1, 0, 1, 1);
    grid->addWidget(m_anyBox,  2, 0, 1, 1);
    grid->addWidget(logo,      0, 2, 3, 1);
    grid->setColumnStretch(1, 10);
    grid->setSpacing(0);
    grid->setMargin(0);

    m_layout->addWidget(header);

    setLayout(m_layout);
}

void SearchGroupLabel::addGroupOperatorOption()
{
    m_groupOpBox = new QComboBox;
    m_groupOpBox->addItem("- OR -", SearchXml::Or);
    m_groupOpBox->addItem("- AND -", SearchXml::And);
    m_groupOpBox->addItem("- AND NOT -", SearchXml::AndNot);
    m_layout->insertWidget(0, m_groupOpBox, 0, Qt::AlignHCenter);
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



