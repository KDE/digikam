/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-16
 * Description : Advanced Configuration tab for metadata.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail.com>
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

#include "advancedmetadatatab.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>

#include "klocale.h"
#include "dmetadatasettings.h"
#include "namespacelistview.h"
#include "namespaceeditdlg.h"

namespace Digikam
{

class AdvancedMetadataTab::Private
{
public:
    Private()
    {

    }
    QComboBox* metadataType;
    QComboBox* operationType;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* deleteButton;
    QPushButton* moveUpButton;
    QPushButton* moveDownButton;
    QPushButton* revertChanges;
    QPushButton* resetButton;
    QCheckBox*   unifyReadWrite;
    QList<QStandardItemModel*> models;
    NamespaceListView* namespaceView;
    DMetadataSettingsContainer container;

    bool                       changed;
};
AdvancedMetadataTab::AdvancedMetadataTab(QWidget* parent)
    :QWidget(parent), d(new Private())
{
    // ---------- Advanced Configuration Panel -----------------------------
    d->container = DMetadataSettings::instance()->settings();
    setUi();
    setModels();
    connectButtons();

    d->unifyReadWrite->setChecked(d->container.unifyReadWrite);
    connect(d->unifyReadWrite, SIGNAL(toggled(bool)), this, SLOT(slotUnifyChecked(bool)));
    connect(d->metadataType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIndexChanged()));
    connect(d->operationType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIndexChanged()));

    /**
     * Connect all actions to slotRevertAvailable, which will enable revert to original
     * if an add, edit, delete, or reorder was made
     */
    connect(d->namespaceView, SIGNAL(signalItemsChanged()), this, SLOT(slotRevertChangesAvailable()));


    d->changed = false;

    if(d->unifyReadWrite->isChecked())
    {
        d->operationType->setEnabled(false);
    }
}

AdvancedMetadataTab::~AdvancedMetadataTab()
{
    delete d;
}

void AdvancedMetadataTab::slotResetToDefault()
{
    d->container.defaultValues();
    d->models.at(getModelIndex())->clear();
    setModelData(d->models.at(getModelIndex()),getCurrentContainer());

    d->namespaceView->setModel(d->models.at(getModelIndex()));
}

void AdvancedMetadataTab::slotRevertChanges()
{
    d->models.at(getModelIndex())->clear();
    setModelData(d->models.at(getModelIndex()),getCurrentContainer());

    d->namespaceView->setModel(d->models.at(getModelIndex()));

    d->changed = false;
    d->revertChanges->setEnabled(false);
}

void AdvancedMetadataTab::slotAddNewNamespace()
{
    NamespaceEntry entry;

    /**
     * Setting some default parameters;
     */
    entry.nsType = (NamespaceEntry::NamespaceType)d->metadataType->currentIndex();
    entry.isDefault = false;
    entry.subspace = NamespaceEntry::XMP;

    if (!NamespaceEditDlg::create(qApp->activeWindow(), entry))
    {
        return;
    }

    QStandardItem* root = d->models.at(getModelIndex())->invisibleRootItem();
    QStandardItem* item = new QStandardItem(entry.namespaceName);
    setDataToItem(item, entry);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    root->appendRow(item);
    d->container.readTagNamespaces.append(entry);

    slotRevertChangesAvailable();
}

void AdvancedMetadataTab::slotEditNamespace()
{

    if(!d->namespaceView->currentIndex().isValid())
        return;

    NamespaceEntry entry = getCurrentContainer().at(d->namespaceView->currentIndex().row());

    qDebug() << "Name before save: " << entry.namespaceName;
    if (!NamespaceEditDlg::edit(qApp->activeWindow(), entry))
    {
        return;
    }
    qDebug() << "Name after save: " << entry.namespaceName;
    QStandardItem* root = d->models.at(getModelIndex())->invisibleRootItem();
    QStandardItem* item = root->child(d->namespaceView->currentIndex().row());

    getCurrentContainer().replace(d->namespaceView->currentIndex().row(), entry);
    setDataToItem(item, entry);
    slotRevertChangesAvailable();
}

void AdvancedMetadataTab::applySettings()
{
    d->container.readTagNamespaces.clear();
    saveModelData(d->models.at(READ_TAGS),d->container.readTagNamespaces);

    d->container.readRatingNamespaces.clear();
    saveModelData(d->models.at(READ_RATINGS),d->container.readRatingNamespaces);

    d->container.readCommentNamespaces.clear();
    saveModelData(d->models.at(READ_COMMENTS),d->container.readCommentNamespaces);

    d->container.writeTagNamespaces.clear();
    saveModelData(d->models.at(WRITE_TAGS),d->container.writeTagNamespaces);

    d->container.writeRatingNamespaces.clear();
    saveModelData(d->models.at(WRITE_RATINGS),d->container.writeRatingNamespaces);

    d->container.writeCommentNamespaces.clear();
    saveModelData(d->models.at(WRITE_COMMENTS),d->container.writeCommentNamespaces);

    d->container.unifyReadWrite = d->unifyReadWrite->isChecked();
    DMetadataSettings::instance()->setSettings(d->container);
}

void AdvancedMetadataTab::slotUnifyChecked(bool value)
{
    d->operationType->setDisabled(value);
    d->container.unifyReadWrite = value;
    if(true)
        d->operationType->setCurrentIndex(0);
    slotIndexChanged();
}

void AdvancedMetadataTab::slotIndexChanged()
{
    d->namespaceView->setModel(d->models.at(getModelIndex()));
}

void AdvancedMetadataTab::slotRevertChangesAvailable()
{
    if(!d->changed)
    {
        d->revertChanges->setEnabled(true);
        d->changed = true;
    }
}

void AdvancedMetadataTab::connectButtons()
{
    connect(d->addButton, SIGNAL(clicked()), this, SLOT(slotAddNewNamespace()));
    connect(d->editButton, SIGNAL(clicked()), this, SLOT(slotEditNamespace()));
    connect(d->deleteButton, SIGNAL(clicked()), d->namespaceView, SLOT(slotDeleteSelected()));
    connect(d->resetButton, SIGNAL(clicked()), this, SLOT(slotResetToDefault()));
    connect(d->revertChanges, SIGNAL(clicked()), this, SLOT(slotRevertChanges()));
    connect(d->moveUpButton, SIGNAL(clicked()), d->namespaceView, SLOT(slotMoveItemUp()));
    connect(d->moveDownButton, SIGNAL(clicked()), d->namespaceView, SLOT(slotMoveItemDown()));
}

void AdvancedMetadataTab::setModelData(QStandardItemModel* model, QList<NamespaceEntry> const &container)
{

    QStandardItem* root = model->invisibleRootItem();
    for(NamespaceEntry e : container)
    {
        QStandardItem* item = new QStandardItem(e.namespaceName);

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        setDataToItem(item, e);
        root->appendRow(item);
    }
    connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(slotRevertChangesAvailable()));
}

void AdvancedMetadataTab::setUi()
{
    QVBoxLayout* const advancedConfLayout = new QVBoxLayout(this);

    QHBoxLayout* const topLayout = new QHBoxLayout();
    QHBoxLayout* const bottomLayout = new QHBoxLayout();

    //--- Top layout ----------------
    d->metadataType = new QComboBox(this);

    d->operationType = new QComboBox(this);

    d->metadataType->insertItems(0, QStringList() << i18n("Tags") << i18n("Ratings") << i18n("Comments"));

    d->operationType->insertItems(0, QStringList() << i18n("Read Options") << i18n("Write Options"));

    d->unifyReadWrite = new QCheckBox(i18n("Unify read and write"));


    topLayout->addWidget(d->metadataType);
    topLayout->addWidget(d->operationType);
    topLayout->addWidget(d->unifyReadWrite);

    //------------ Bottom Layout-------------
    // View
    d->namespaceView = new NamespaceListView(this);


    // Buttons
    QVBoxLayout* buttonsLayout = new QVBoxLayout();
    buttonsLayout->setAlignment(Qt::AlignTop);
    d->addButton = new QPushButton(QIcon::fromTheme(QLatin1String("list-add")),
                                   i18n("Add"));
    d->editButton = new QPushButton(QIcon::fromTheme(QLatin1String("document-edit")),
                                                     i18n("Edit"));

    d->deleteButton = new QPushButton(QIcon::fromTheme(QLatin1String("window-close")),
                                      i18n("Delete"));

    d->moveUpButton = new QPushButton(QIcon::fromTheme(QLatin1String("arrow-up")),
                                      i18n("Move Up"));

    d->moveDownButton = new QPushButton(QIcon::fromTheme(QLatin1String("arrow-down")),
                                        i18n("Move Down"));

    d->revertChanges = new QPushButton(QIcon::fromTheme(QLatin1String("edit-undo")),
                                       i18n("Revert Changes"));

    // Revert changes is disabled, until a change is made
    d->revertChanges->setEnabled(false);
    d->resetButton = new QPushButton(QIcon::fromTheme(QLatin1String("view-refresh")),
                                     i18n("Reset to Default"));


    buttonsLayout->addWidget(d->addButton);
    buttonsLayout->addWidget(d->editButton);
    buttonsLayout->addWidget(d->deleteButton);
    buttonsLayout->addWidget(d->moveUpButton);
    buttonsLayout->addWidget(d->moveDownButton);
    buttonsLayout->addWidget(d->revertChanges);
    buttonsLayout->addWidget(d->resetButton);

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(d->namespaceView);

    bottomLayout->addLayout(vbox);
    bottomLayout->addLayout(buttonsLayout);

    advancedConfLayout->addLayout(topLayout);
    advancedConfLayout->addLayout(bottomLayout);

}

void AdvancedMetadataTab::setDataToItem(QStandardItem *item, NamespaceEntry &entry)
{
    item->setData(entry.namespaceName,Qt::DisplayRole);
    item->setData(entry.namespaceName,NAME_ROLE);
    item->setData((int)entry.tagPaths, ISTAG_ROLE);
    item->setData(entry.separator, SEPARATOR_ROLE);
    item->setData(entry.extraXml, EXTRAXML_ROLE);
    item->setData((int)entry.nsType, NSTYPE_ROLE);
    if(entry.nsType == NamespaceEntry::RATING)
    {
       item->setData(entry.convertRatio.at(0), ZEROSTAR_ROLE);
       item->setData(entry.convertRatio.at(1), ONESTAR_ROLE);
       item->setData(entry.convertRatio.at(2), TWOSTAR_ROLE);
       item->setData(entry.convertRatio.at(3), THREESTAR_ROLE);
       item->setData(entry.convertRatio.at(4), FOURSTAR_ROLE);
       item->setData(entry.convertRatio.at(5), FIVESTAR_ROLE);
    }
    item->setData((int)entry.specialOpts, SPECIALOPTS_ROLE);
    item->setData(entry.alternativeName, ALTNAME_ROLE);
    item->setData((int)entry.subspace, SUBSPACE_ROLE);
    item->setData((int)entry.secondNameOpts, ALTNAMEOPTS_ROLE);
    item->setData(entry.isDefault, ISDEFAULT_ROLE);

    item->setCheckable(true);
    if(!entry.isDisabled)
        item->setCheckState(Qt::Checked);
}

int AdvancedMetadataTab::getModelIndex()
{
    if(d->unifyReadWrite->isChecked())
    {
        return d->metadataType->currentIndex();
    }
    else
    {
        // read operation = 3*0 + (0, 1, 2)
        // write operation = 3*1 + (0, 1, 2) = (3, 4 ,5)
        return (3*d->operationType->currentIndex()) + d->metadataType->currentIndex();
    }
}

QList<NamespaceEntry>& AdvancedMetadataTab::getCurrentContainer()
{
    switch(getModelIndex())
    {
    case 0:
        return d->container.readTagNamespaces;
    case 1:
        return d->container.readRatingNamespaces;
    case 2:
        return d->container.readCommentNamespaces;
    case 3:
        return d->container.writeTagNamespaces;
    case 4:
        return d->container.writeRatingNamespaces;
    case 5:
        return d->container.writeCommentNamespaces;
    default:
        qDebug() << "warning, Unknown case";
        return d->container.readTagNamespaces;
    }
}

void AdvancedMetadataTab::setModels()
{
    // Append 6 empty models
    for(int i = 0 ; i < 6; i++)
        d->models.append(new QStandardItemModel(this));

    setModelData(d->models.at(READ_TAGS), d->container.readTagNamespaces);
    setModelData(d->models.at(READ_RATINGS), d->container.readRatingNamespaces);
    setModelData(d->models.at(READ_COMMENTS), d->container.readCommentNamespaces);

    setModelData(d->models.at(WRITE_TAGS), d->container.writeTagNamespaces);
    setModelData(d->models.at(WRITE_RATINGS), d->container.writeRatingNamespaces);
    setModelData(d->models.at(WRITE_COMMENTS), d->container.writeCommentNamespaces);

    slotIndexChanged();

}

void AdvancedMetadataTab::saveModelData(QStandardItemModel *model, QList<NamespaceEntry> &container)
{
    QStandardItem* root = model->invisibleRootItem();

    if(!root->hasChildren())
        return;

    for(int i = 0 ; i < root->rowCount(); i++)
    {
        NamespaceEntry ns;
        QStandardItem  *current = root->child(i);
        ns.namespaceName        = current->data(NAME_ROLE).toString();
        ns.tagPaths             = (NamespaceEntry::TagType)current->data(ISTAG_ROLE).toInt();
        ns.separator            = current->data(SEPARATOR_ROLE).toString();
        ns.extraXml             = current->data(EXTRAXML_ROLE).toString();
        ns.nsType               = (NamespaceEntry::NamespaceType)current->data(NSTYPE_ROLE).toInt();
        if(ns.nsType == NamespaceEntry::RATING)
        {
            ns.convertRatio.append(current->data(ZEROSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(ONESTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(TWOSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(THREESTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(FOURSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(FIVESTAR_ROLE).toInt());
        }
        ns.specialOpts          = (NamespaceEntry::SpecialOptions)current->data(SPECIALOPTS_ROLE).toInt();
        ns.alternativeName      = current->data(ALTNAME_ROLE).toString();
        ns.subspace             = (NamespaceEntry::NsSubspace)current->data(SUBSPACE_ROLE).toInt();
        ns.secondNameOpts       = (NamespaceEntry::SpecialOptions)current->data(ALTNAMEOPTS_ROLE).toInt();
        ns.index                = i;
        ns.isDefault            = current->data(ISDEFAULT_ROLE).toBool();

        if(current->checkState() == Qt::Checked)
            ns.isDisabled = false;
        else
            ns.isDisabled = true;

        qDebug() << "saving+++++" << ns.namespaceName << " " << ns.index << " " << ns.specialOpts;
        container.append(ns);
    }
}

}


