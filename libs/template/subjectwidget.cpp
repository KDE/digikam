/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC subjects editor.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "subjectwidget.h"
#include "subjectwidget.moc"

// Qt includes

#include <QCheckBox>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QValidator>

// KDE includes

#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistwidget.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "debug.h"

namespace Digikam
{

class SubjectWidgetPriv
{
public:

    enum EditionMode
    {
        STANDARD = 0,
        CUSTOM
    };

    SubjectWidgetPriv()
    {
        addSubjectButton = 0;
        delSubjectButton = 0;
        repSubjectButton = 0;
        subjectsBox      = 0;
        iprLabel         = 0;
        refLabel         = 0;
        nameLabel        = 0;
        matterLabel      = 0;
        detailLabel      = 0;
        btnGroup         = 0;
        stdBtn           = 0;
        customBtn        = 0;
        refCB            = 0;
        optionsBox       = 0;
    }

    typedef QMap<QString, SubjectData>  SubjectCodesMap;

    SubjectCodesMap                     subMap;

    QStringList                         subjectsList;

    QWidget                            *optionsBox;

    QPushButton                        *addSubjectButton;
    QPushButton                        *delSubjectButton;
    QPushButton                        *repSubjectButton;

    QLabel                             *iprLabel;
    QLabel                             *refLabel;
    QLabel                             *nameLabel;
    QLabel                             *matterLabel;
    QLabel                             *detailLabel;

    QButtonGroup                       *btnGroup;

    QRadioButton                       *stdBtn;
    QRadioButton                       *customBtn;

    KComboBox                          *refCB;

    KListWidget                        *subjectsBox;
};

// --------------------------------------------------------------------------------

SubjectWidget::SubjectWidget(QWidget* parent)
             : QWidget(parent), d(new SubjectWidgetPriv)
{
    // Load subject codes provided by IPTC/NAA as xml file.
    // See http://iptc.cms.apa.at/std/topicset/topicset.iptc-subjectcode.xml for details.

    KGlobal::dirs()->addResourceDir("iptcschema", KStandardDirs::installPath("data") +
                                                  QString("digikam/data"));
    QString path = KGlobal::dirs()->findResource("iptcschema", "topicset.iptc-subjectcode.xml");

    if (!loadSubjectCodesFromXML(KUrl(path)))
        kDebug(digiKamAreaCode) << "Cannot load IPTC/NAA subject codes XML database";

    // --------------------------------------------------------

    // Subject Reference Number only accept digit.
    QRegExp refDigitRx("^[0-9]{8}$");
    QValidator *refValidator = new QRegExpValidator(refDigitRx, this);

    // --------------------------------------------------------

    d->optionsBox    = new QWidget;
    d->btnGroup      = new QButtonGroup(this);
    d->stdBtn        = new QRadioButton;
    d->customBtn     = new QRadioButton;
    d->refCB         = new KComboBox;
    QLabel *codeLink = new QLabel(i18n("Use standard "
                                       "<b><a href='http://www.iptc.org/NewsCodes'>"
                                       "reference code</a></b>"));
    codeLink->setOpenExternalLinks(true);
    codeLink->setWordWrap(false);

    QLabel *customLabel = new QLabel(i18n("Use custom definition"));

    d->btnGroup->addButton(d->stdBtn,    SubjectWidgetPriv::STANDARD);
    d->btnGroup->addButton(d->customBtn, SubjectWidgetPriv::CUSTOM);
    d->btnGroup->setExclusive(true);
    d->stdBtn->setChecked(true);

    for (SubjectWidgetPriv::SubjectCodesMap::Iterator it = d->subMap.begin();
         it != d->subMap.end(); ++it)
        d->refCB->addItem(it.key());

    // --------------------------------------------------------

    m_iprEdit = new KLineEdit;
    m_iprEdit->setClearButtonShown(true);
    m_iprEdit->setMaxLength(32);

    // --------------------------------------------------------

    m_refEdit = new KLineEdit;
    m_refEdit->setClearButtonShown(true);
    m_refEdit->setValidator(refValidator);
    m_refEdit->setMaxLength(8);

    // --------------------------------------------------------

    m_nameEdit = new KLineEdit;
    m_nameEdit->setClearButtonShown(true);
    m_nameEdit->setMaxLength(64);

    // --------------------------------------------------------

    m_matterEdit = new KLineEdit;
    m_matterEdit->setClearButtonShown(true);
    m_matterEdit->setMaxLength(64);

    // --------------------------------------------------------

    m_detailEdit = new KLineEdit;
    m_detailEdit->setClearButtonShown(true);
    m_detailEdit->setMaxLength(64);

    // --------------------------------------------------------

    d->iprLabel    = new QLabel(i18n("I.P.R:"));
    d->refLabel    = new QLabel(i18n("Reference:"));
    d->nameLabel   = new QLabel(i18n("Name:"));
    d->matterLabel = new QLabel(i18n("Matter:"));
    d->detailLabel = new QLabel(i18n("Detail:"));

    // --------------------------------------------------------

    int left, top, right, bottom;
    d->subjectsBox = new KListWidget;
    d->subjectsBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    d->subjectsBox->getContentsMargins(&left, &top, &right, &bottom);
    d->subjectsBox->setFixedHeight(top + bottom + d->subjectsBox->frameWidth() +
                                   d->subjectsBox->fontMetrics().lineSpacing()*8);

    d->addSubjectButton = new QPushButton(i18n("&Add"));
    d->delSubjectButton = new QPushButton(i18n("&Delete"));
    d->repSubjectButton = new QPushButton(i18n("&Replace"));
    d->addSubjectButton->setIcon(SmallIcon("list-add"));
    d->delSubjectButton->setIcon(SmallIcon("edit-delete"));
    d->repSubjectButton->setIcon(SmallIcon("view-refresh"));
    d->delSubjectButton->setEnabled(false);
    d->repSubjectButton->setEnabled(false);

    // --------------------------------------------------------

    m_note = new QLabel;
    m_note->setMaximumWidth(150);
    m_note->setOpenExternalLinks(true);
    m_note->setWordWrap(true);
    m_note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    QGridLayout *optionsBoxLayout = new QGridLayout;
    optionsBoxLayout->addWidget(d->stdBtn,      0, 0, 1, 1);
    optionsBoxLayout->addWidget(codeLink,       0, 1, 1, 2);
    optionsBoxLayout->addWidget(d->refCB,       0, 3, 1, 1);
    optionsBoxLayout->addWidget(d->customBtn,   1, 0, 1, 4);
    optionsBoxLayout->addWidget(customLabel,    1, 1, 1, 4);
    optionsBoxLayout->addWidget(d->iprLabel,    2, 0, 1, 1);
    optionsBoxLayout->addWidget(m_iprEdit,      2, 1, 1, 4);
    optionsBoxLayout->addWidget(d->refLabel,    3, 0, 1, 1);
    optionsBoxLayout->addWidget(m_refEdit,      3, 1, 1, 1);
    optionsBoxLayout->addWidget(d->nameLabel,   4, 0, 1, 1);
    optionsBoxLayout->addWidget(m_nameEdit,     4, 1, 1, 4);
    optionsBoxLayout->addWidget(d->matterLabel, 5, 0, 1, 1);
    optionsBoxLayout->addWidget(m_matterEdit,   5, 1, 1, 4);
    optionsBoxLayout->addWidget(d->detailLabel, 6, 0, 1, 1);
    optionsBoxLayout->addWidget(m_detailEdit,   6, 1, 1, 4);
    optionsBoxLayout->setColumnStretch(4, 10);
    optionsBoxLayout->setMargin(0);
    optionsBoxLayout->setSpacing(KDialog::spacingHint());
    d->optionsBox->setLayout(optionsBoxLayout);

    // --------------------------------------------------------

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setAlignment( Qt::AlignTop );
    mainLayout->addWidget(d->optionsBox,       0, 0, 1, 4);
    mainLayout->addWidget(d->subjectsBox,      1, 0, 5, 3);
    mainLayout->addWidget(d->addSubjectButton, 1, 3, 1, 1);
    mainLayout->addWidget(d->delSubjectButton, 2, 3, 1, 1);
    mainLayout->addWidget(d->repSubjectButton, 3, 3, 1, 1);
    mainLayout->addWidget(m_note,              4, 3, 1, 1);
    mainLayout->setRowStretch(5, 10);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->subjectsBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSubjectSelectionChanged()));

    connect(d->addSubjectButton, SIGNAL(clicked()),
            this, SLOT(slotAddSubject()));

    connect(d->delSubjectButton, SIGNAL(clicked()),
            this, SLOT(slotDelSubject()));

    connect(d->repSubjectButton, SIGNAL(clicked()),
            this, SLOT(slotRepSubject()));

    connect(d->btnGroup, SIGNAL(buttonReleased(int)),
            this, SLOT(slotEditOptionChanged(int)));

    connect(d->refCB, SIGNAL(activated(int)),
            this, SLOT(slotRefChanged()));

    // --------------------------------------------------------

    connect(d->addSubjectButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->delSubjectButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->repSubjectButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    slotEditOptionChanged(d->btnGroup->id(d->btnGroup->checkedButton()));
}

SubjectWidget::~SubjectWidget()
{
    delete d;
}

void SubjectWidget::slotEditOptionChanged(int b)
{
    if (b == SubjectWidgetPriv::CUSTOM)
    {
        d->refCB->setEnabled(false);
        m_iprEdit->setEnabled(true);
        m_refEdit->setEnabled(true);
        m_nameEdit->setEnabled(true);
        m_matterEdit->setEnabled(true);
        m_detailEdit->setEnabled(true);
    }
    else
    {
        d->refCB->setEnabled(true);
        m_iprEdit->setEnabled(false);
        m_refEdit->setEnabled(false);
        m_nameEdit->setEnabled(false);
        m_matterEdit->setEnabled(false);
        m_detailEdit->setEnabled(false);
        slotRefChanged();
    }
}

void SubjectWidget::slotRefChanged()
{
    QString key = d->refCB->currentText();
    QString name, matter, detail;

    for (SubjectWidgetPriv::SubjectCodesMap::Iterator it = d->subMap.begin();
         it != d->subMap.end(); ++it)
    {
        if (key == it.key())
        {
            name   = it.value().name;
            matter = it.value().matter;
            detail = it.value().detail;
        }
    }

    m_refEdit->setText(key);
    m_nameEdit->setText(name);
    m_matterEdit->setText(matter);
    m_detailEdit->setText(detail);
}

QString SubjectWidget::buildSubject() const
{
    QString subject = m_iprEdit->text();
    subject.append(":");
    subject.append(m_refEdit->text());
    subject.append(":");
    subject.append(m_nameEdit->text());
    subject.append(":");
    subject.append(m_matterEdit->text());
    subject.append(":");
    subject.append(m_detailEdit->text());
    return subject;
}

void SubjectWidget::slotDelSubject()
{
    QListWidgetItem *item = d->subjectsBox->currentItem();
    if (!item) return;
    d->subjectsBox->takeItem(d->subjectsBox->row(item));
    delete item;
}

void SubjectWidget::slotRepSubject()
{
    QString newSubject = buildSubject();
    if (newSubject.isEmpty()) return;

    if (!d->subjectsBox->selectedItems().isEmpty())
    {
        d->subjectsBox->selectedItems()[0]->setText(newSubject);
        m_iprEdit->clear();
        m_refEdit->clear();
        m_nameEdit->clear();
        m_matterEdit->clear();
        m_detailEdit->clear();
    }
}

void SubjectWidget::slotSubjectSelectionChanged()
{
    if (!d->subjectsBox->selectedItems().isEmpty())
    {
        QString subject = d->subjectsBox->selectedItems()[0]->text();
        m_iprEdit->setText(subject.section(':', 0, 0));
        m_refEdit->setText(subject.section(':', 1, 1));
        m_nameEdit->setText(subject.section(':', 2, 2));
        m_matterEdit->setText(subject.section(':', 3, 3));
        m_detailEdit->setText(subject.section(':', 4, 4));
        d->delSubjectButton->setEnabled(true);
        d->repSubjectButton->setEnabled(true);
    }
    else
    {
        d->delSubjectButton->setEnabled(false);
        d->repSubjectButton->setEnabled(false);
    }
}

void SubjectWidget::slotAddSubject()
{
    QString newSubject = buildSubject();
    if (newSubject.isEmpty()) return;

    bool found = false;
    for (int i = 0 ; i < d->subjectsBox->count(); i++)
    {
        QListWidgetItem *item = d->subjectsBox->item(i);
        if (newSubject == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->subjectsBox->insertItem(d->subjectsBox->count(), newSubject);
        m_iprEdit->clear();
        m_refEdit->clear();
        m_nameEdit->clear();
        m_matterEdit->clear();
        m_detailEdit->clear();
    }
}

bool SubjectWidget::loadSubjectCodesFromXML(const KUrl& url)
{
    QFile xmlfile(url.toLocalFile());

    if (!xmlfile.open(QIODevice::ReadOnly))
        return false;

    QDomDocument xmlDoc("NewsML");
    if (!xmlDoc.setContent(&xmlfile))
        return false;

    QDomElement xmlDocElem = xmlDoc.documentElement();
    if (xmlDocElem.tagName()!="NewsML")
        return false;

    for (QDomNode nbE1 = xmlDocElem.firstChild();
         !nbE1.isNull(); nbE1 = nbE1.nextSibling())
    {
        QDomElement newsItemElement = nbE1.toElement();
        if (newsItemElement.isNull()) continue;
        if (newsItemElement.tagName() != "NewsItem") continue;

        for (QDomNode nbE2 = newsItemElement.firstChild();
            !nbE2.isNull(); nbE2 = nbE2.nextSibling())
        {
            QDomElement topicSetElement = nbE2.toElement();
            if (topicSetElement.isNull()) continue;
            if (topicSetElement.tagName() != "TopicSet") continue;

            for (QDomNode nbE3 = topicSetElement.firstChild();
                !nbE3.isNull(); nbE3 = nbE3.nextSibling())
            {
                QDomElement topicElement = nbE3.toElement();
                if (topicElement.isNull()) continue;
                if (topicElement.tagName() != "Topic") continue;

                QString type, name, matter, detail, ref;
                for (QDomNode nbE4 = topicElement.firstChild();
                    !nbE4.isNull(); nbE4 = nbE4.nextSibling())
                {
                    QDomElement topicSubElement = nbE4.toElement();
                    if (topicSubElement.isNull()) continue;

                    if (topicSubElement.tagName() == "TopicType")
                        type = topicSubElement.attribute("FormalName");

                    if (topicSubElement.tagName() == "FormalName")
                        ref = topicSubElement.text();

                    if (topicSubElement.tagName() == "Description" &&
                        topicSubElement.attribute("Variant") == "Name")
                    {
                        if (type == "Subject")
                            name = topicSubElement.text();
                        else if (type == "SubjectMatter")
                            matter = topicSubElement.text();
                        else if (type == "SubjectDetail")
                            detail = topicSubElement.text();
                    }
                }

                d->subMap.insert(ref, SubjectData(name, matter, detail));
            }
        }
    }

    // Set the Subject Name everywhere on the map.

    for (SubjectWidgetPriv::SubjectCodesMap::Iterator it = d->subMap.begin();
         it != d->subMap.end(); ++it)
    {
        QString name, keyPrefix;
        if (it.key().endsWith("00000"))
        {
            keyPrefix = it.key().left(3);
            name      = it.value().name;

            for (SubjectWidgetPriv::SubjectCodesMap::Iterator it2 = d->subMap.begin();
                it2 != d->subMap.end(); ++it2)
            {
                if (it2.key().startsWith(keyPrefix) &&
                    !it2.key().endsWith("00000"))
                {
                    it2.value().name = name;
                }
            }
        }
    }

    // Set the Subject Matter Name everywhere on the map.

    for (SubjectWidgetPriv::SubjectCodesMap::Iterator it = d->subMap.begin();
         it != d->subMap.end(); ++it)
    {
        QString matter, keyPrefix;
        if (it.key().endsWith("000"))
        {
            keyPrefix = it.key().left(5);
            matter    = it.value().matter;

            for (SubjectWidgetPriv::SubjectCodesMap::Iterator it2 = d->subMap.begin();
                it2 != d->subMap.end(); ++it2)
            {
                if (it2.key().startsWith(keyPrefix) &&
                    !it2.key().endsWith("000"))
                {
                    it2.value().matter = matter;
                }
            }
        }
    }

    return true;
}

void SubjectWidget::setSubjectList(const QStringList& list)
{
    d->subjectsList = list;

    blockSignals(true);
    d->subjectsBox->clear();
    if (!d->subjectsList.isEmpty())
        d->subjectsBox->insertItems(0, d->subjectsList);

    blockSignals(false);
}

QStringList SubjectWidget::subjectsList() const
{
    QStringList newSubjects;

    for (int i = 0 ; i < d->subjectsBox->count(); i++)
    {
        QListWidgetItem *item = d->subjectsBox->item(i);
        newSubjects.append(item->text());
    }

    return newSubjects;
}

}  // namespace Digikam
