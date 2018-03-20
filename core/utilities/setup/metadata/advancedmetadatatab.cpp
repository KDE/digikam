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

// Qt includes

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
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadatasettings.h"
#include "namespacelistview.h"
#include "namespaceeditdlg.h"
#include "dmessagebox.h"
#include "digikam_debug.h"

namespace Digikam
{

class AdvancedMetadataTab::Private
{
public:

    Private()
    {
        metadataType     = 0;
        operationType    = 0;
        addButton        = 0;
        editButton       = 0;
        deleteButton     = 0;
        moveUpButton     = 0;
        moveDownButton   = 0;
        revertChanges    = 0;
        resetButton      = 0;
        unifyReadWrite   = 0;
        namespaceView    = 0;
        metadataTypeSize = 0;
        changed          = false;
    }

    QComboBox*                  metadataType;
    QComboBox*                  operationType;
    QPushButton*                addButton;
    QPushButton*                editButton;
    QPushButton*                deleteButton;
    QPushButton*                moveUpButton;
    QPushButton*                moveDownButton;
    QPushButton*                revertChanges;
    QPushButton*                resetButton;
    QCheckBox*                  unifyReadWrite;
    QList<QStandardItemModel*>  models;
    NamespaceListView*          namespaceView;
    DMetadataSettingsContainer  container;
    int                         metadataTypeSize;

    bool                        changed;
};

AdvancedMetadataTab::AdvancedMetadataTab(QWidget* const parent)
    : QWidget(parent),
      d(new Private())
{
    // ---------- Advanced Configuration Panel -----------------------------

    d->container = DMetadataSettings::instance()->settings();
    setUi();
    setModels();
    connectButtons();

    d->unifyReadWrite->setChecked(d->container.unifyReadWrite());

    connect(d->unifyReadWrite, SIGNAL(toggled(bool)),
            this, SLOT(slotUnifyChecked(bool)));

    connect(d->metadataType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged()));

    connect(d->operationType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged()));

    /**
     * Connect all actions to slotRevertAvailable, which will enable revert to original
     * if an add, edit, delete, or reorder was made
     */
    connect(d->namespaceView, SIGNAL(signalItemsChanged()),
            this, SLOT(slotRevertChangesAvailable()));

    if (d->unifyReadWrite->isChecked())
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
    const int result = DMessageBox::showContinueCancel(QMessageBox::Warning,
                                                       this,
                                                       i18n("Warning"),
                                                       i18n("This option will reset configuration to default\n"
                                                            "All your changes will be lost.\n "
                                                            "Do you want to continue?"));

    if (result != QMessageBox::Yes)
    {
        return;
    }

    d->container.defaultValues();
    d->models.at(getModelIndex())->clear();
    setModelData(d->models.at(getModelIndex()), getCurrentContainer());

    d->namespaceView->setModel(d->models.at(getModelIndex()));
}

void AdvancedMetadataTab::slotRevertChanges()
{
    d->models.at(getModelIndex())->clear();
    setModelData(d->models.at(getModelIndex()), getCurrentContainer());

    d->namespaceView->setModel(d->models.at(getModelIndex()));

    d->changed = false;
    d->revertChanges->setEnabled(false);
}

void AdvancedMetadataTab::slotAddNewNamespace()
{
    NamespaceEntry entry;

    // Setting some default parameters;

    if (d->metadataType->currentData().toString() == QLatin1String(DM_TAG_CONTAINER))
    {
        entry.nsType = NamespaceEntry::TAGS;
    }
    else if (d->metadataType->currentData().toString() == QLatin1String(DM_RATING_CONTAINER))
    {
        entry.nsType = NamespaceEntry::RATING;
    }
    else if (d->metadataType->currentData().toString() == QLatin1String(DM_COMMENT_CONTAINER))
    {
        entry.nsType = NamespaceEntry::COMMENT;
    }

    entry.isDefault  = false;
    entry.subspace   = NamespaceEntry::XMP;

    if (!NamespaceEditDlg::create(qApp->activeWindow(), entry))
    {
        return;
    }

    QStandardItem* const root = d->models.at(getModelIndex())->invisibleRootItem();
    QStandardItem* const item = new QStandardItem(entry.namespaceName);
    setDataToItem(item, entry);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    root->appendRow(item);
    getCurrentContainer().append(entry);

    slotRevertChangesAvailable();
}

void AdvancedMetadataTab::slotEditNamespace()
{
    if (!d->namespaceView->currentIndex().isValid())
    {
        return;
    }

    NamespaceEntry entry = getCurrentContainer().at(d->namespaceView->currentIndex().row());


    if (!NamespaceEditDlg::edit(qApp->activeWindow(), entry))
    {
        return;
    }

    QStandardItem* const root = d->models.at(getModelIndex())->invisibleRootItem();
    QStandardItem* const item = root->child(d->namespaceView->currentIndex().row());

    getCurrentContainer().replace(d->namespaceView->currentIndex().row(), entry);
    setDataToItem(item, entry);
    slotRevertChangesAvailable();
}

void AdvancedMetadataTab::applySettings()
{
    QList<QLatin1String> keys = d->container.mappingKeys();
    int index                 = 0;

    foreach(const QLatin1String& str, keys)
    {
        d->container.getReadMapping(str).clear();
        saveModelData(d->models.at(index++), d->container.getReadMapping(str));
    }

    foreach(const QLatin1String& str, keys)
    {
        d->container.getWriteMapping(str).clear();
        saveModelData(d->models.at(index++), d->container.getWriteMapping(str));
    }

    DMetadataSettings::instance()->setSettings(d->container);
}

void AdvancedMetadataTab::slotUnifyChecked(bool value)
{
    d->operationType->setDisabled(value);
    d->container.setUnifyReadWrite(value);

    d->operationType->setCurrentIndex(0);

    slotIndexChanged();
}

void AdvancedMetadataTab::slotIndexChanged()
{
    d->namespaceView->setModel(d->models.at(getModelIndex()));
}

void AdvancedMetadataTab::slotRevertChangesAvailable()
{
    if (!d->changed)
    {
        d->revertChanges->setEnabled(true);
        d->changed = true;
    }
}

void AdvancedMetadataTab::connectButtons()
{
    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddNewNamespace()));

    connect(d->editButton, SIGNAL(clicked()),
            this, SLOT(slotEditNamespace()));

    connect(d->deleteButton, SIGNAL(clicked()),
            d->namespaceView, SLOT(slotDeleteSelected()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetToDefault()));

    connect(d->revertChanges, SIGNAL(clicked()),
            this, SLOT(slotRevertChanges()));

    connect(d->moveUpButton, SIGNAL(clicked()),
            d->namespaceView, SLOT(slotMoveItemUp()));

    connect(d->moveDownButton, SIGNAL(clicked()),
            d->namespaceView, SLOT(slotMoveItemDown()));
}

void AdvancedMetadataTab::setModelData(QStandardItemModel* model, const QList<NamespaceEntry>& container)
{
    QStandardItem* const root = model->invisibleRootItem();

    for (NamespaceEntry e : container)
    {
        QStandardItem* const item = new QStandardItem(e.namespaceName);

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        setDataToItem(item, e);
        root->appendRow(item);
    }

    connect(model, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(slotRevertChangesAvailable()));
}

void AdvancedMetadataTab::setUi()
{
    QVBoxLayout* const advancedConfLayout = new QVBoxLayout(this);
    QHBoxLayout* const topLayout          = new QHBoxLayout();
    QHBoxLayout* const bottomLayout       = new QHBoxLayout();

    QLabel* const tipLabel = new QLabel(this);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
    tipLabel->setText(i18n("Advanced configuration menu allow you to manage metadata namespaces"
                           " used by digiKam to store and retrieve tags, rating and comments.<br>"
                           "<b>Note: </b>Order is important when reading metadata"
                          ));

    //--- Top layout ----------------

    d->metadataType  = new QComboBox(this);
    d->operationType = new QComboBox(this);

    d->operationType->insertItems(0, QStringList() << i18n("Read Options") << i18n("Write Options"));

    d->unifyReadWrite = new QCheckBox(i18n("Unify read and write"));

    topLayout->addWidget(d->metadataType);
    topLayout->addWidget(d->operationType);
    topLayout->addWidget(d->unifyReadWrite);

    //------------ Bottom Layout-------------

    // View
    d->namespaceView = new NamespaceListView(this);

    // Buttons
    QVBoxLayout* const buttonsLayout = new QVBoxLayout();
    buttonsLayout->setAlignment(Qt::AlignTop);

    d->addButton      = new QPushButton(QIcon::fromTheme(QLatin1String("list-add")),
                                        i18n("Add"));

    d->editButton     = new QPushButton(QIcon::fromTheme(QLatin1String("document-edit")),
                                        i18n("Edit"));

    d->deleteButton   = new QPushButton(QIcon::fromTheme(QLatin1String("window-close")),
                                        i18n("Delete"));

    d->moveUpButton   = new QPushButton(QIcon::fromTheme(QLatin1String("go-up")),
                                        i18n("Move Up"));

    d->moveDownButton = new QPushButton(QIcon::fromTheme(QLatin1String("go-down")),
                                        i18n("Move Down"));

    d->revertChanges  = new QPushButton(QIcon::fromTheme(QLatin1String("edit-undo")),
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

    QVBoxLayout* const vbox = new QVBoxLayout();
    vbox->addWidget(d->namespaceView);

    bottomLayout->addLayout(vbox);
    bottomLayout->addLayout(buttonsLayout);

    advancedConfLayout->addWidget(tipLabel);
    advancedConfLayout->addLayout(topLayout);
    advancedConfLayout->addLayout(bottomLayout);
}

void AdvancedMetadataTab::setDataToItem(QStandardItem* item, NamespaceEntry& entry)
{
    item->setData(entry.namespaceName,  Qt::DisplayRole);
    item->setData(entry.namespaceName,  NAME_ROLE);
    item->setData((int)entry.tagPaths,  ISTAG_ROLE);
    item->setData(entry.separator,      SEPARATOR_ROLE);
    item->setData((int)entry.nsType,    NSTYPE_ROLE);

    if (entry.nsType == NamespaceEntry::RATING)
    {
       item->setData(entry.convertRatio.at(0), ZEROSTAR_ROLE);
       item->setData(entry.convertRatio.at(1), ONESTAR_ROLE);
       item->setData(entry.convertRatio.at(2), TWOSTAR_ROLE);
       item->setData(entry.convertRatio.at(3), THREESTAR_ROLE);
       item->setData(entry.convertRatio.at(4), FOURSTAR_ROLE);
       item->setData(entry.convertRatio.at(5), FIVESTAR_ROLE);
    }

    item->setData((int)entry.specialOpts,    SPECIALOPTS_ROLE);
    item->setData(entry.alternativeName,     ALTNAME_ROLE);
    item->setData((int)entry.subspace,       SUBSPACE_ROLE);
    item->setData((int)entry.secondNameOpts, ALTNAMEOPTS_ROLE);
    item->setData(entry.isDefault,           ISDEFAULT_ROLE);

    item->setCheckable(true);

    if (!entry.isDisabled)
    {
        item->setCheckState(Qt::Checked);
    }
}

int AdvancedMetadataTab::getModelIndex()
{
    if (d->unifyReadWrite->isChecked())
    {
        return d->metadataType->currentIndex();
    }
    else
    {
        // for 3 metadata types:
        // read operation  = 3*0 + (0, 1, 2)
        // write operation = 3*1 + (0, 1, 2) = (3, 4 ,5)
        return (d->metadataTypeSize * d->operationType->currentIndex())
                + d->metadataType->currentIndex();
    }
}

QList<NamespaceEntry>& AdvancedMetadataTab::getCurrentContainer()
{
    int currentIndex = getModelIndex();

    if (currentIndex >= d->metadataTypeSize)
    {
        return d->container.getWriteMapping(QLatin1String(d->metadataType->currentData().toByteArray()));
    }
    else
    {
        return d->container.getReadMapping(QLatin1String(d->metadataType->currentData().toByteArray()));
    }
}

void AdvancedMetadataTab::setModels()
{
    QList<QLatin1String> keys = d->container.mappingKeys();

    foreach(const QLatin1String& str, keys)
    {
        d->metadataType->addItem(i18n(str.data()), str);
    }

    d->metadataTypeSize = keys.size();

    for (int i = 0 ; i < keys.size() * 2; i++)
    {
        d->models.append(new QStandardItemModel(this));
    }

    int index = 0;

    foreach(const QLatin1String& str, keys)
    {
        setModelData(d->models.at(index++), d->container.getReadMapping(str));
    }

    foreach(const QLatin1String& str, keys)
    {
        setModelData(d->models.at(index++), d->container.getWriteMapping(str));
    }

    slotIndexChanged();
}

void AdvancedMetadataTab::saveModelData(QStandardItemModel* model, QList<NamespaceEntry>& container)
{
    QStandardItem* const root = model->invisibleRootItem();

    if (!root->hasChildren())
    {
        return;
    }

    for (int i = 0 ; i < root->rowCount(); i++)
    {
        NamespaceEntry ns;
        QStandardItem* const current = root->child(i);
        ns.namespaceName             = current->data(NAME_ROLE).toString();
        ns.tagPaths                  = (NamespaceEntry::TagType)current->data(ISTAG_ROLE).toInt();
        ns.separator                 = current->data(SEPARATOR_ROLE).toString();
        ns.nsType                    = (NamespaceEntry::NamespaceType)current->data(NSTYPE_ROLE).toInt();

        if (ns.nsType == NamespaceEntry::RATING)
        {
            ns.convertRatio.append(current->data(ZEROSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(ONESTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(TWOSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(THREESTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(FOURSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(FIVESTAR_ROLE).toInt());
        }

        ns.specialOpts     = (NamespaceEntry::SpecialOptions)current->data(SPECIALOPTS_ROLE).toInt();
        ns.alternativeName = current->data(ALTNAME_ROLE).toString();
        ns.subspace        = (NamespaceEntry::NsSubspace)current->data(SUBSPACE_ROLE).toInt();
        ns.secondNameOpts  = (NamespaceEntry::SpecialOptions)current->data(ALTNAMEOPTS_ROLE).toInt();
        ns.index           = i;
        ns.isDefault       = current->data(ISDEFAULT_ROLE).toBool();

        if (current->checkState() == Qt::Checked)
        {
            ns.isDisabled = false;
        }
        else
        {
            ns.isDisabled = true;
        }

        container.append(ns);
    }
}

} // namespace Digikam
