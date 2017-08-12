/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "photolayoutswindow.h"
#include "photolayoutswindow_p.h"

// Qt includes

#include <QStyle>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <QDockWidget>
#include <QHeaderView>
#include <QLabel>
#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include <QPluginLoader>
#include <QFile>
#include <QPrintPreviewDialog>
#include <QImageWriter>
#include <QPrinter>
#include <QPrintDialog>
#include <QDesktopWidget>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QMenuBar>
#include <QPrinter>

// KDE includes

#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kconfigdialog.h>
#include <kservice.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "imagedialog.h"
#include "dmessagebox.h"
#include "CanvasSizeDialog.h"
#include "Canvas.h"
#include "Scene.h"
#include "LayersSelectionModel.h"
#include "UndoCommandEventFilter.h"
#include "PhotoEffectsLoader.h"
#include "AbstractPhotoEffectFactory.h"
#include "GridSetupDialog.h"
#include "PLEConfigDialog.h"
#include "PLEConfigSkeleton.h"
#include "StandardEffectsFactory.h"
#include "StandardBordersFactory.h"
#include "global.h"
#include "ProgressEvent.h"
#include "BorderDrawerInterface.h"
#include "BorderDrawersLoader.h"
#include "NewCanvasDialog.h"

using namespace Digikam;

// Q_*_RESOURCE cannot be used in a namespace
inline void initIconsResource() { Q_INIT_RESOURCE(icons); }
inline void cleanupIconsResource() { Q_CLEANUP_RESOURCE(icons); }

namespace PhotoLayoutsEditor
{

class CanvasSizeChangeCommand
    : public QUndoCommand
{
    CanvasSize m_size;
    Canvas*    m_canvas;

public:

    CanvasSizeChangeCommand(const CanvasSize & size, Canvas * canvas, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Canvas size change"), parent),
        m_size(size),
        m_canvas(canvas)
    {}

    virtual void redo()
    {
        this->run();
    }

    virtual void undo()
    {
        this->run();
    }

    void run()
    {
        CanvasSize temp = m_canvas->canvasSize();
        m_canvas->setCanvasSize(m_size);
        m_size = temp;
    }
};

PhotoLayoutsWindow* PhotoLayoutsWindow::m_instance = 0;

PhotoLayoutsWindow::PhotoLayoutsWindow(QWidget * parent) :
    KXmlGuiWindow(parent),
    m_canvas(0),
    m_interface(0),
    d(new Private)
{
    m_instance = this;

    initIconsResource();
    setXMLFile(QLatin1String("photolayoutseditorui.rc"));
    setWindowTitle(i18n("Photo Layouts Editor"));

    loadEffects();
    loadBorders();
    setupActions();
    createWidgets();
    refreshActions();

    setAcceptDrops(true);
    int height = QApplication::desktop()->height()*0.8;
    resize(qRound(height*16.0/9.0),height);
    QDesktopWidget* d = qApp->desktop();
    move(d->rect().center() - this->frameGeometry().center());
}

PhotoLayoutsWindow::~PhotoLayoutsWindow()
{
    PLEConfigSkeleton::self()->save();

    if (m_canvas)
        m_canvas->deleteLater();
    if (d)
        delete d;

    m_instance = 0;

    cleanupIconsResource();
}

PhotoLayoutsWindow * PhotoLayoutsWindow::instance(QWidget * parent)
{
    if (m_instance)
        return m_instance;
    else
    {
        qApp->installEventFilter(new UndoCommandEventFilter(qApp));
        return (m_instance = new PhotoLayoutsWindow(parent));
    }
}

void PhotoLayoutsWindow::addUndoCommand(QUndoCommand * command)
{
    if (command)
    {
#ifdef QT_DEBUG
        qCDebug(DIGIKAM_GENERAL_LOG) << command->text();
#endif
        if (m_canvas)
            m_canvas->undoStack()->push(command);
        else
        {
            command->redo();
            delete command;
        }
    }
}

void PhotoLayoutsWindow::beginUndoCommandGroup(const QString & name)
{
    if (m_canvas)
        m_canvas->undoStack()->beginMacro(name);
}

void PhotoLayoutsWindow::endUndoCommandGroup()
{
    if (m_canvas)
        m_canvas->undoStack()->endMacro();
}

void PhotoLayoutsWindow::setInterface(DInfoInterface* const interface)
{
    if (interface)
        m_interface = interface;
}

bool PhotoLayoutsWindow::hasInterface() const
{
    return (bool) m_interface;
}

DInfoInterface* PhotoLayoutsWindow::interface() const
{
    return this->m_interface;
}

void PhotoLayoutsWindow::setItemsList(const QList<QUrl> & images)
{
    if (!m_canvas)
        return;

    m_canvas->addImages(images);
}

void PhotoLayoutsWindow::setupActions()
{
    d->openNewFileAction = KStandardAction::openNew(this, SLOT(open()), actionCollection());
    actionCollection()->addAction(QLatin1String("open_new"), d->openNewFileAction);
    //------------------------------------------------------------------------
    d->openFileAction = new QAction(i18n("Open Template File..."), actionCollection());
    connect(d->openFileAction, SIGNAL(triggered()), this, SLOT(openDialog()));
    actionCollection()->addAction(QLatin1String("open"), d->openFileAction);
    //------------------------------------------------------------------------
    d->openRecentFilesMenu = KStandardAction::openRecent(this, SLOT(open(QUrl)), actionCollection());
    QList<QUrl> urls = PLEConfigSkeleton::recentFiles();

    foreach(QUrl url, urls)
    {
        d->openRecentFilesMenu->addUrl(url);
    }

    connect(d->openRecentFilesMenu, SIGNAL(recentListCleared()), this, SLOT(clearRecentList()));
    actionCollection()->addAction(QLatin1String("open_recent"), d->openRecentFilesMenu);
    //------------------------------------------------------------------------
    d->saveAction = KStandardAction::save(this, SLOT(save()), actionCollection());
    actionCollection()->addAction(QLatin1String("save"), d->saveAction);
    //------------------------------------------------------------------------
    d->saveAsAction = KStandardAction::saveAs(this, SLOT(saveAs()), actionCollection());
    actionCollection()->setDefaultShortcut(d->saveAsAction, Qt::SHIFT + Qt::CTRL + Qt::Key_S);
    actionCollection()->addAction(QLatin1String("save_as"), d->saveAsAction);
    //------------------------------------------------------------------------
    d->saveAsTemplateAction = new QAction(i18nc("Saves canvas as a template file...", "Save As Template..."), actionCollection());
    connect(d->saveAsTemplateAction, SIGNAL(triggered()), this, SLOT(saveAsTemplate()));
    actionCollection()->addAction(QLatin1String("save_as_template"), d->saveAsTemplateAction);
    //------------------------------------------------------------------------
    d->exportFileAction = new QAction(i18nc("Export current frame layout to image file...", "Export..."), actionCollection());
    actionCollection()->setDefaultShortcut(d->exportFileAction, Qt::SHIFT + Qt::CTRL + Qt::Key_E);
    connect(d->exportFileAction, SIGNAL(triggered()), this, SLOT(exportFile()));
    actionCollection()->addAction(QLatin1String("export"), d->exportFileAction);
    //------------------------------------------------------------------------
    d->printPreviewAction = KStandardAction::printPreview(this, SLOT(printPreview()), actionCollection());
    actionCollection()->setDefaultShortcut(d->printPreviewAction, Qt::SHIFT + Qt::CTRL + Qt::Key_P);
    actionCollection()->addAction(QLatin1String("print_preview"), d->printPreviewAction);
    //------------------------------------------------------------------------
    d->printAction = KStandardAction::print(this, SLOT(print()), actionCollection());
    actionCollection()->addAction(QLatin1String("print"), d->printAction);
    //------------------------------------------------------------------------
    d->closeAction = KStandardAction::close(this, SLOT(closeDocument()), actionCollection());
    actionCollection()->addAction(QLatin1String("close"), d->closeAction);
    //------------------------------------------------------------------------
    d->quitAction = KStandardAction::quit(this, SLOT(close()), actionCollection());
    actionCollection()->addAction(QLatin1String("quit"), d->quitAction);
    //------------------------------------------------------------------------
    d->undoAction = KStandardAction::undo(0, 0, actionCollection());
    actionCollection()->addAction(QLatin1String("undo"), d->undoAction);
    //------------------------------------------------------------------------
    d->redoAction = KStandardAction::redo(0, 0, actionCollection());
    actionCollection()->addAction(QLatin1String("redo"), d->redoAction);
    //------------------------------------------------------------------------
    d->settingsAction = KStandardAction::preferences(this, SLOT(settings()), actionCollection());
    actionCollection()->addAction(QLatin1String("settings"), d->settingsAction);
    //------------------------------------------------------------------------
    d->addImageAction = new QAction(i18nc("Adds new image(s) from file...", "Add image(s)..."), actionCollection());
    connect(d->addImageAction, SIGNAL(triggered()), this, SLOT(loadNewImage()));
    actionCollection()->addAction(QLatin1String("new_image"), d->addImageAction);
    //------------------------------------------------------------------------
    d->showGridToggleAction = new KToggleAction(i18nc("View grid lines...", "Show..."), actionCollection());
    actionCollection()->setDefaultShortcut(d->showGridToggleAction, Qt::SHIFT + Qt::CTRL + Qt::Key_G);
    d->showGridToggleAction->setChecked( PLEConfigSkeleton::self()->showGrid() );
    connect(d->showGridToggleAction, SIGNAL(triggered(bool)), this, SLOT(setGridVisible(bool)));
    actionCollection()->addAction(QLatin1String("grid_toggle"), d->showGridToggleAction);
    //------------------------------------------------------------------------
    d->gridConfigAction = new QAction(i18nc("Configure grid lines visibility...", "Setup grid..."), actionCollection());
    connect(d->gridConfigAction, SIGNAL(triggered()), this, SLOT(setupGrid()));
    actionCollection()->addAction(QLatin1String("grid_config"), d->gridConfigAction);
    //------------------------------------------------------------------------
    d->changeCanvasSizeAction = new QAction(i18nc("Configure canvas size...", "Change canvas size..."), actionCollection());
    connect(d->changeCanvasSizeAction, SIGNAL(triggered()), this, SLOT(changeCanvasSize()));
    actionCollection()->addAction(QLatin1String("canvas_size"), d->changeCanvasSizeAction);

    createGUI(xmlFile());
}

void PhotoLayoutsWindow::refreshActions()
{
    bool isEnabledForCanvas = false;
    if (m_canvas)
    {
        isEnabledForCanvas = true;
        d->undoAction->setEnabled(m_canvas->undoStack()->canUndo());
        d->redoAction->setEnabled(m_canvas->undoStack()->canRedo());
        d->saveAction->setEnabled(isEnabledForCanvas && !m_canvas->isSaved());
    }
    d->saveAsAction->setEnabled(isEnabledForCanvas);
    d->saveAsTemplateAction->setEnabled(isEnabledForCanvas);
    d->exportFileAction->setEnabled(isEnabledForCanvas);
    d->printPreviewAction->setEnabled(isEnabledForCanvas);
    d->printAction->setEnabled(isEnabledForCanvas);
    d->closeAction->setEnabled(isEnabledForCanvas);
    d->addImageAction->setEnabled(isEnabledForCanvas);
    d->showGridToggleAction->setEnabled(isEnabledForCanvas);
    d->gridConfigAction->setEnabled(isEnabledForCanvas);
    d->changeCanvasSizeAction->setEnabled(isEnabledForCanvas);
    d->treeWidget->setEnabled(isEnabledForCanvas);
    d->toolsWidget->setEnabled(isEnabledForCanvas);
}

void PhotoLayoutsWindow::addRecentFile(const QUrl & url)
{
    if (url.isValid())
    {
        QList<QUrl> tempList = PLEConfigSkeleton::recentFiles();
        tempList.removeAll(url);
        tempList.push_back(url);
        unsigned maxCount = PLEConfigSkeleton::recentFilesCount();
        while ( ((unsigned)tempList.count()) > maxCount)
            tempList.removeAt(0);
        PLEConfigSkeleton::setRecentFiles(tempList);
        if ( !d->openRecentFilesMenu->urls().contains( url ) )
            d->openRecentFilesMenu->addUrl( url );
        PLEConfigSkeleton::self()->save();
    }
}

void PhotoLayoutsWindow::clearRecentList()
{
    PLEConfigSkeleton::setRecentFiles(QList<QUrl>());
}

void PhotoLayoutsWindow::createWidgets()
{
    // Tools
    d->toolsWidget = ToolsDockWidget::instance(this);
    this->addDockWidget(Qt::RightDockWidgetArea, d->toolsWidget);

    // Layers dockwidget
    d->treeWidget = new QDockWidget(i18n("Layers"), this);
    d->treeWidget->setFeatures(QDockWidget::DockWidgetMovable);
    d->treeWidget->setFloating(false);
    d->treeWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    d->tree = new LayersTree(d->treeWidget);
    d->tree->setAnimated(true);
    d->treeWidget->setWidget(d->tree);
    d->treeTitle = new LayersTreeTitleWidget(d->treeTitle);
    d->treeWidget->setTitleBarWidget(d->treeTitle);
    this->addDockWidget(Qt::LeftDockWidgetArea, d->treeWidget);
    d->treeWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(d->toolsWidget,SIGNAL(requireMultiSelection()),d->tree,SLOT(setMultiSelection()));
    connect(d->toolsWidget,SIGNAL(requireSingleSelection()),d->tree,SLOT(setSingleSelection()));

    // Central widget (widget with canvas)
    d->centralWidget = new QWidget(this);
    d->centralWidget->setLayout(new QHBoxLayout(d->centralWidget));
    d->centralWidget->layout()->setContentsMargins(QMargins());
    d->centralWidget->layout()->setSpacing(0);
    setCentralWidget(d->centralWidget);

    d->statusBar = new PLEStatusBar(this);
    setStatusBar(d->statusBar);
}

void PhotoLayoutsWindow::createCanvas(const CanvasSize & size)
{
    if (m_canvas)
    {
        d->centralWidget->layout()->removeWidget(m_canvas);
        m_canvas->deleteLater();
    }
    m_canvas = new Canvas(size, d->centralWidget);
    this->prepareSignalsConnections();
}

void PhotoLayoutsWindow::createCanvas(const QUrl & fileUrl)
{
    if (m_canvas)
    {
        d->centralWidget->layout()->removeWidget(m_canvas);
        m_canvas->deleteLater();
    }

    QFile file(fileUrl.toLocalFile());
    QDomDocument document;
    document.setContent(&file, true);
    m_canvas = Canvas::fromSvg(document);
    if (m_canvas)
    {
        if (!m_canvas->isTemplate())
        {
            m_canvas->setFile(fileUrl);
            // Adds recent open file
            this->addRecentFile(m_canvas->file());
        }
        m_canvas->setParent(d->centralWidget);
        this->prepareSignalsConnections();
    }
    else
    {
        QMessageBox::critical(this,
                              i18n("Error"),
                              i18n("Cannot read image file."));
    }
    file.close();
}

void PhotoLayoutsWindow::prepareSignalsConnections()
{
    d->centralWidget->layout()->addWidget(m_canvas);
    d->tree->setModel(m_canvas->model());
    d->tree->setSelectionModel(m_canvas->selectionModel());
    d->toolsWidget->setScene(m_canvas->scene());

    // undo stack signals
    connect(m_canvas,               SIGNAL(savedStateChanged()),    this,                   SLOT(refreshActions()));
    connect(m_canvas->undoStack(),  SIGNAL(canRedoChanged(bool)),   d->redoAction,          SLOT(setEnabled(bool)));
    connect(m_canvas->undoStack(),  SIGNAL(canUndoChanged(bool)),   d->undoAction,          SLOT(setEnabled(bool)));
    connect(d->undoAction,          SIGNAL(triggered()),            m_canvas->undoStack(),  SLOT(undo()));
    connect(d->redoAction,          SIGNAL(triggered()),            m_canvas->undoStack(),  SLOT(redo()));

    // model/tree/canvas synchronization signals
    connect(d->tree,    SIGNAL(selectedRowsAboutToBeRemoved()),     m_canvas,   SLOT(removeSelectedRows()));
    connect(d->tree,    SIGNAL(selectedRowsAboutToBeMovedUp()),     m_canvas,   SLOT(moveSelectedRowsUp()));
    connect(d->tree,    SIGNAL(selectedRowsAboutToBeMovedDown()),   m_canvas,   SLOT(moveSelectedRowsDown()));
    connect(d->treeTitle->moveUpButton(),   SIGNAL(clicked()),      m_canvas,   SLOT(moveSelectedRowsUp()));
    connect(d->treeTitle->moveDownButton(), SIGNAL(clicked()),      m_canvas,   SLOT(moveSelectedRowsDown()));
    // interaction modes (tools)
    connect(m_canvas,       SIGNAL(selectedItem(AbstractPhoto*)),       d->toolsWidget,SLOT(itemSelected(AbstractPhoto*)));
    connect(d->toolsWidget, SIGNAL(undoCommandCreated(QUndoCommand*)),  m_canvas,   SLOT(newUndoCommand(QUndoCommand*)));
    connect(d->toolsWidget, SIGNAL(pointerToolSelected()),              m_canvas,   SLOT(enableDefaultSelectionMode()));
    connect(d->toolsWidget, SIGNAL(handToolSelected()),                 m_canvas,   SLOT(enableViewingMode()));
    connect(d->toolsWidget, SIGNAL(zoomToolSelected()),                 m_canvas,   SLOT(enableZoomingMode()));
    connect(d->toolsWidget, SIGNAL(canvasToolSelected()),               m_canvas,   SLOT(enableCanvasEditingMode()));
    connect(d->toolsWidget, SIGNAL(effectsToolSelected()),              m_canvas,   SLOT(enableEffectsEditingMode()));
    connect(d->toolsWidget, SIGNAL(textToolSelected()),                 m_canvas,   SLOT(enableTextEditingMode()));
    connect(d->toolsWidget, SIGNAL(rotateToolSelected()),               m_canvas,   SLOT(enableRotateEditingMode()));
    connect(d->toolsWidget, SIGNAL(scaleToolSelected()),                m_canvas,   SLOT(enableScaleEditingMode()));
    connect(d->toolsWidget, SIGNAL(cropToolSelected()),                 m_canvas,   SLOT(enableCropEditingMode()));
    connect(d->toolsWidget, SIGNAL(borderToolSelected()),               m_canvas,   SLOT(enableBordersEditingMode()));
    connect(d->toolsWidget, SIGNAL(newItemCreated(AbstractPhoto*)),     m_canvas,   SLOT(addNewItem(AbstractPhoto*)));
    connect(m_canvas->scene()->toGraphicsScene(), SIGNAL(mousePressedPoint(QPointF)), d->toolsWidget, SLOT(mousePositionChoosen(QPointF)));

    d->toolsWidget->setDefaultTool();
}

void PhotoLayoutsWindow::open()
{
    NewCanvasDialog * dialog = new NewCanvasDialog(this);
    dialog->setModal(true);

    int result = dialog->exec();
    if (result != QDialog::Accepted)
        return;

    QString tmp;
    if (dialog->hasTemplateSelected() && !(tmp = dialog->templateSelected()).isEmpty())
    {
        open(QUrl(dialog->templateSelected()));
    }
    else
    {
        CanvasSize size = dialog->canvasSize();
        if (size.isValid())
        {
            closeDocument();
            createCanvas(size);
            refreshActions();
        }
    }
    delete dialog;
}

void PhotoLayoutsWindow::openDialog()
{
    if (!d->fileDialog)
    {
        d->fileDialog = new QFileDialog(this,
                                        QString(),
                                        QString(),
                                        i18n("*.ple|Photo Layouts Editor files"));
    }

    d->fileDialog->setFileMode(QFileDialog::AnyFile);

    int result = d->fileDialog->exec();

    if (result == QFileDialog::Accepted &&
        !d->fileDialog->selectedUrls().isEmpty())
    {
        QUrl url = d->fileDialog->selectedUrls().first();
        qCDebug(DIGIKAM_GENERAL_LOG) << url;
        open(url);
    }
}

void PhotoLayoutsWindow::open(const QUrl & fileUrl)
{
    if (m_canvas && m_canvas->file() == fileUrl)
        return;

    if (fileUrl.isValid())
    {
        closeDocument();
        createCanvas(fileUrl);
        refreshActions();
    }
}

void PhotoLayoutsWindow::save()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << !m_canvas->file().isValid() <<  m_canvas->file().fileName().isEmpty() << m_canvas->isTemplate();
    if (!m_canvas)
        return;
    if (!m_canvas->file().isValid() || m_canvas->file().fileName().isEmpty() || m_canvas->isTemplate())
        saveAs();
    else
        saveFile();
}

void PhotoLayoutsWindow::saveAs()
{
    if (!d->fileDialog)
    {
        d->fileDialog = new QFileDialog(this,
                                        QString(),
                                        QString(),
                                        i18n("*.ple|Photo Layouts Editor files"));
    }

    d->fileDialog->setFileMode(QFileDialog::AnyFile);

    int result = d->fileDialog->exec();

    if (result == QFileDialog::Accepted &&
        !d->fileDialog->selectedUrls().isEmpty())
    {
        saveFile(d->fileDialog->selectedUrls().first());
    }
}

void PhotoLayoutsWindow::saveAsTemplate()
{
    if (!d->fileDialog)
    {
        d->fileDialog = new QFileDialog(this,
                                        QString(),
                                        QString(),
                                        i18n("*.ple|Photo Layouts Editor files"));
    }

    d->fileDialog->setFileMode(QFileDialog::AnyFile);

    int result = d->fileDialog->exec();

    if (result == QFileDialog::Accepted &&
        !d->fileDialog->selectedUrls().isEmpty())
    {
        QUrl url = d->fileDialog->selectedUrls().first();

        if (m_canvas)
            m_canvas->saveTemplate(url);
        else
            QMessageBox::critical(this,
                                  i18n("Error"),
                                  i18n("There is nothing to save."));
    }
}

void PhotoLayoutsWindow::saveFile(const QUrl & fileUrl, bool setFileAsDefault)
{
    if (m_canvas)
        m_canvas->save(fileUrl, setFileAsDefault);
    else
            QMessageBox::critical(this,
                                  i18n("Error"),
                                  i18n("There is nothing to save."));
}

void PhotoLayoutsWindow::exportFile()
{
    if (!m_canvas)
        return;

    QString all;
    QStringList list                       = supportedImageMimeTypes(QIODevice::WriteOnly, all);
    QFileDialog* const imageFileSaveDialog = new QFileDialog(this);
    imageFileSaveDialog->setWindowTitle(i18n("New Image File Name"));
    imageFileSaveDialog->setAcceptMode(QFileDialog::AcceptSave);
    imageFileSaveDialog->setFileMode(QFileDialog::AnyFile);
    imageFileSaveDialog->setNameFilters(list);

    int result       = imageFileSaveDialog->exec();
    QList<QUrl> urls = imageFileSaveDialog->selectedUrls();
    QString ext      = imageFileSaveDialog->selectedNameFilter().section(QLatin1String("*."), 1, 1);
    ext              = ext.left(ext.length() - 1);

    if (result == QFileDialog::Accepted && !urls.isEmpty() && !ext.isEmpty())
    {
        QUrl url = urls.first();

        QPixmap image(m_canvas->sceneRect().size().toSize());
        image.fill(Qt::transparent);
        m_canvas->renderCanvas(&image);
        QImageWriter writer(url.toLocalFile());
        writer.setFormat(ext.toLatin1());

        if (!writer.canWrite())
        {
            QMessageBox::critical(this, i18n("Error"),
                                    i18n("Image can't be saved in selected file."));
        }

        if (!writer.write(image.toImage()))
        {
            DMessageBox::showInformationList(
                QMessageBox::Critical,
                qApp->activeWindow(),
                qApp->applicationName(),
                i18n("Unexpected error while saving an image."),
                QStringList() << writer.errorString());
        }
    }

    delete imageFileSaveDialog;
}

void PhotoLayoutsWindow::printPreview()
{
    if (m_canvas && m_canvas->scene())
    {
        QPrinter * printer = new QPrinter();
        m_canvas->preparePrinter(printer);
        QPrintPreviewDialog * dialog = new QPrintPreviewDialog(printer, this);
        connect(dialog, SIGNAL(paintRequested(QPrinter*)), m_canvas, SLOT(renderCanvas(QPrinter*)));
        dialog->exec();
        delete dialog;
        delete printer;
    }
}

void PhotoLayoutsWindow::print()
{
    QPrinter * printer = new QPrinter();
    m_canvas->preparePrinter(printer);
    QPrintDialog * dialog = new QPrintDialog(printer, this);
    connect(dialog, SIGNAL(accepted(QPrinter*)), m_canvas, SLOT(renderCanvas(QPrinter*)));
    dialog->exec();
    delete dialog;
    delete printer;
}

bool PhotoLayoutsWindow::closeDocument()
{
    if (m_canvas)
    {
        // Adds recent open file
        this->addRecentFile(m_canvas->file());

        // Try to save unsaved changes
        int saving = QMessageBox::No;

        if (!m_canvas->isSaved())
            saving = QMessageBox::question(this,
                                           i18n("Save"),
                                           i18n("Save changes to current frame?"),
                                           QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));

        switch (saving)
        {
            case QMessageBox::Yes:
                save();
            case QMessageBox::No:
                d->tree->setModel(0);
                m_canvas->deleteLater();
                m_canvas = 0;
                refreshActions();
                return true;
            default:
                return false;
        }
    }

    refreshActions();
    return true;
}

void PhotoLayoutsWindow::progressEvent(ProgressEvent * event)
{
    if (m_canvas)
        m_canvas->progressEvent(event);
}

bool PhotoLayoutsWindow::queryClose()
{
    if (closeDocument())
        return true;
    else
        return false;
}

void PhotoLayoutsWindow::settings()
{
    if ( KConfigDialog::showDialog( QLatin1String("settings") ) )
        return;

    PLEConfigDialog * dialog = new PLEConfigDialog(this);
    dialog->show();
}

void PhotoLayoutsWindow::loadNewImage()
{
    if (!m_canvas)
        return;

    QList<QUrl> urls = ImageDialog::getImageURLs(this, QUrl());
    if (!urls.isEmpty())
        m_canvas->addImages(urls);
}

void PhotoLayoutsWindow::setGridVisible(bool isVisible)
{
    d->showGridToggleAction->setChecked(isVisible);
    PLEConfigSkeleton::setShowGrid(isVisible);
    PLEConfigSkeleton::self()->save();
    if (m_canvas && m_canvas->scene())
        m_canvas->scene()->setGridVisible(isVisible);
}

void PhotoLayoutsWindow::setupGrid()
{
    if (m_canvas && m_canvas->scene())
    {
        GridSetupDialog * dialog = new GridSetupDialog(this);
        dialog->setHorizontalDistance( m_canvas->scene()->gridHorizontalDistance() );
        dialog->setVerticalDistance( m_canvas->scene()->gridVerticalDistance() );
        dialog->exec();
        m_canvas->scene()->setGrid(dialog->horizontalDistance(),
                                   dialog->verticalDistance());
        delete dialog;
    }
}

void PhotoLayoutsWindow::changeCanvasSize()
{
    if (!m_canvas)
        return;

    CanvasSizeDialog* ccd = new CanvasSizeDialog(m_canvas->canvasSize(), this);
    int result            = ccd->exec();
    CanvasSize size       = ccd->canvasSize();

    if (result == QDialog::Accepted)
    {
        if (size.isValid())
        {
            if (m_canvas->canvasSize() != size)
            {
                CanvasSizeChangeCommand * command = new CanvasSizeChangeCommand(size, m_canvas);
                PLE_PostUndoCommand(command);
            }
        }
        else
        {
            QMessageBox::critical(this, i18n("Error"), i18n("Invalid image size."));
        }
    }

    delete ccd;
}

void PhotoLayoutsWindow::setTemplateEditMode(bool isEnabled)
{
    Q_UNUSED(isEnabled);

    //d->toolsWidget->setTemplateEditMode(isEnabled);
    //d->treeWidget->setTemplateEditMode(isEnabled);
    //m_canvas->setTemplateEditMode(isEnabled);
}

void PhotoLayoutsWindow::loadEffects()
{
    StandardEffectsFactory * stdEffects = new StandardEffectsFactory( PhotoEffectsLoader::instance() );
    PhotoEffectsLoader::registerEffect( stdEffects );

    const KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("PhotoLayoutsEditor/EffectPlugin"));
    foreach(const KService::Ptr& service, offers)
    {
        if (service)
            d->effectsServiceMap[service->name()] = service;
    }

    foreach(const QString& name, d->effectsServiceMap.keys())
    {
        KService::Ptr service = d->effectsServiceMap.value(name);
        AbstractPhotoEffectFactory * plugin;

        if ( d->effectsMap.contains(name) )
            continue;
        else
        {
            QString error;
            plugin = service->createInstance<AbstractPhotoEffectFactory>( PhotoEffectsLoader::instance(), QVariantList(), &error);
            if (plugin)
            {
                d->effectsMap[name] = plugin;
                PhotoEffectsLoader::registerEffect(plugin);
                qCDebug(DIGIKAM_GENERAL_LOG) << "PhotoLayoutsWindow: Loaded effect " << service->name();
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "PhotoLayoutsWindow: createInstance returned 0 for "
                           << service->name()
                           << " (" << service->library() << ")"
                           << " with error: "
                           << error;
            }
        }
    }
}

void PhotoLayoutsWindow::loadBorders()
{
    StandardBordersFactory * stdBorders = new StandardBordersFactory( BorderDrawersLoader::instance() );
    BorderDrawersLoader::registerDrawer( stdBorders );

    const KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("PhotoLayoutsEditor/BorderPlugin"));
    foreach(const KService::Ptr& service, offers)
    {
        if (service)
            d->bordersServiceMap[service->name()] = service;
    }

    foreach(const QString& name, d->bordersServiceMap.keys())
    {
        KService::Ptr service = d->bordersServiceMap.value(name);
        BorderDrawerFactoryInterface * plugin;

        if ( d->bordersMap.contains(name) )
            continue;
        else
        {
            QString error;
            plugin = service->createInstance<BorderDrawerFactoryInterface>(this, QVariantList(), &error);
            if (plugin)
            {
                d->bordersMap[name] = plugin;
                BorderDrawersLoader::registerDrawer(plugin);
                qCDebug(DIGIKAM_GENERAL_LOG) << "PhotoLayoutsWindow: Loaded border:" << service->name();
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "PhotoLayoutsWindow: createInstance returned 0 for "
                           << service->name()
                           << " (" << service->library() << ")"
                           << " with error: "
                           << error;
            }
        }
    }
}

} // namespace PhotoLayoutsEditor
