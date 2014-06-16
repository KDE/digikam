/** -*- mode: c++ ; c-basic-offset: 3 -*-
 * @file   MainWindow.cpp
 * @author Sebastien Fourey
 * @date   July 2010
 * @brief  Declaration of the class MainWindow
 *
 * This file is part of the ZArt software's source code.
 *
 * Copyright Sebastien Fourey / GREYC Ensicaen (2010-...)
 *
 *                    https://foureys.users.greyc.fr/
 *
 * This software is a computer program whose purpose is to demonstrate
 * the possibilities of the GMIC image processing language by offering the
 * choice of several manipulations on a video stream aquired from a webcam. In
 * other words, ZArt is a GUI for G'MIC real-time manipulations on the output
 * of a webcam.
 *
 * This software is governed by the CeCILL  license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". See also the directory "Licence" which comes
 * with this source code for the full text of the CeCILL licence.
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */
#include <iostream>

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageWriter>
#include <QKeySequence>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QShortcut>
#include <QSlider>
#include <QUrl>
#include <QtXml>

#include "Common.h"
#include "DialogAbout.h"
#include "DialogLicence.h"
#include "ImageConverter.h"
#include "ImageView.h"
#include "MainWindow.h"
#include "WebcamGrabber.h"
#include "FilterThread.h"

MainWindow::MainWindow( QWidget * parent )
   : QMainWindow( parent ),
     _currentDir( "." )
{
   setupUi(this);
   setWindowTitle( QString("ZArt %1").arg((ZART_VERSION)) );

   QSettings settings;

#if QT_VERSION >= 0x040600
   _tbPlay->setIcon( QIcon::fromTheme("media-playback-start",
				      QIcon(":/images/media-playback-start.png")) );
   _tbZoomOriginal->setIcon( QIcon::fromTheme("zoom-original",
					      QIcon(":/images/zoom-original.png") ));
   _tbZoomFit->setIcon( QIcon::fromTheme("zoom-fit-best",
					 QIcon(":/images/zoom-fit-best.png") ));
#else
   _tbPlay->setIcon( QIcon(":/images/media-playback-start.png") );
   _tbZoomOriginal->setIcon( QIcon(":/images/zoom-original.png") );
   _tbZoomFit->setIcon( QIcon(":/images/zoom-fit-best.png") );
#endif

   // Menu and actions
   QMenu * menu;
   menu = menuBar()->addMenu( "&File" );

   QAction * action = new QAction( "&Save presets...", this );
   action->setShortcut( QKeySequence::SaveAs );
#if QT_VERSION >= 0x040600
   action->setIcon( QIcon::fromTheme( "document-save-as" ) );
#endif
   menu->addAction( action );
   connect( action, SIGNAL( triggered() ),
  	   this, SLOT( savePresetsFile() ) );

   action = new QAction( "&Quit", this );
   action->setShortcut( QKeySequence::Quit );
#if QT_VERSION >= 0x040600
   action->setIcon( QIcon::fromTheme( "application-exit",
				      QIcon(":/images/application-exit.png") ) );
#else
   action->setIcon( QIcon(":/images/application-exit.png") );
#endif
   connect( action, SIGNAL( triggered() ),
  	   qApp, SLOT( closeAllWindows() ) );
   menu->addAction( action );

   QMenu * webcamMenu;
   QStringList args = qApp->arguments();
   if ( args.size() == 3 && args[1] == "--cam" ) {
      int index = args[2].toInt();
      _webcam.setCameraIndex( index );
   } else {
      // Find available cameras, and setup the default one
      QList<int> cameras = WebcamGrabber::getWebcamList();
      if ( cameras.size() > 1 ) {
         webcamMenu = menuBar()->addMenu( "&Webcam" );
         QActionGroup * actionGroup = new QActionGroup(this);
         actionGroup->setExclusive(true);
         QAction * action;
         QList<int>::iterator it = cameras.begin();
         while (it != cameras.end()) {
            action = new QAction( QString("Webcam %1").arg(*it), this );
            action->setData( QVariant( *it ) );
            action->setCheckable( true );
            webcamMenu->addAction( action );
            actionGroup->addAction( action );
            ++it;
         }
         actionGroup->actions()[0]->setChecked(true);
         _webcam.setCameraIndex( actionGroup->actions()[0]->data().toInt() );
         connect( actionGroup, SIGNAL( triggered( QAction * ) ),
                 this, SLOT( onWebcamSelected( QAction * ) ) );
      } else if ( cameras.size() == 1 ) {
         _webcam.setCameraIndex( cameras[0] );
      }
   }

   QSize cameraSize( _webcam.width(), _webcam.height() );
   _imageView->resize( cameraSize );

   // Network manager
   _networkManager = new QNetworkAccessManager(this);
   connect( _networkManager, SIGNAL(finished(QNetworkReply*)),
	   this, SLOT(networkReplyFinished(QNetworkReply*)) );

   // Options menu
   menu = menuBar()->addMenu( "&Options" );

   action = new QAction("Show right panel",menu);
   action->setCheckable(true);
   action->setChecked(settings.value("showRightPanel",true).toBool());
   action->setShortcut(QKeySequence("Ctrl+F"));
   connect( action, SIGNAL(triggered(bool)),
	    this, SLOT(onRightPanel(bool)));
   menu->addAction(action);
   menu->addSeparator();


   // Presets
   QString presetsConfig = settings.value("Presets",QString("Built-in")).toString();

   QActionGroup * group = new QActionGroup(menu);
   group->setExclusive(true);

   // Built-in
   _builtInPresetsAction = new QAction( "&Built-in presets", menu );
   _builtInPresetsAction->setCheckable( true );
   connect( _builtInPresetsAction, SIGNAL( toggled(bool) ),
	    this, SLOT( onUseBuiltinPresets(bool) ) );
   group->addAction( _builtInPresetsAction );
   menu->addAction( _builtInPresetsAction );
   _builtInPresetsAction->setChecked(true); // Default to Built-in presets

   // Online
   _onlinePresetsAction = new QAction( "&Online presets", menu );
   _onlinePresetsAction->setCheckable( true );
   connect( _onlinePresetsAction, SIGNAL( toggled(bool ) ),
	    this, SLOT( onUseOnlinePresets(bool ) ) );
   menu->addAction( _onlinePresetsAction );
   group->addAction( _onlinePresetsAction );
   if ( presetsConfig == "Online" ) {
      _onlinePresetsAction->setChecked(true);
   }

   // File
   action = new QAction( "&Presets file...", menu );
   action->setCheckable( true );
   group->addAction( action );
   menu->addAction( action );
   QString filename = settings.value("PresetsFile",QString()).toString();
   if ( presetsConfig == "File" &&
	!filename.isEmpty() ) {
      setPresetsFile(filename);
      action->setChecked(true);
   }
   connect( action, SIGNAL( triggered() ),
	   this, SLOT( setPresetsFile() ) );

   // Help menu
   menu = menuBar()->addMenu( "&Help" );

   action = new QAction( "&Visit G'MIC website", this );
   action->setIcon( QIcon(":/images/gmic_hat.png") );
   connect( action, SIGNAL( triggered() ),
           this, SLOT( visitGMIC() ) );
   menu->addAction( action );

   action = new QAction( "&Licence...", this );
   connect( action, SIGNAL( triggered() ),
           this, SLOT( licence() ) );
   menu->addAction( action );

   action = new QAction( "&About...", this );
   connect( action, SIGNAL( triggered() ),
           this, SLOT( about() ) );
   menu->addAction( action );

   _imageView->QWidget::resize( _webcam.width(), _webcam.height() );

   _bgZoom = new QButtonGroup(this);
   _bgZoom->setExclusive(true);
   _tbZoomFit->setCheckable(true);
   _tbZoomFit->setChecked(true);
   _tbZoomOriginal->setCheckable(true);
   _bgZoom->addButton(_tbZoomOriginal);
   _bgZoom->addButton(_tbZoomFit);

   _cbPreviewMode->addItem("Full",FilterThread::Full);
   _cbPreviewMode->addItem("Top",FilterThread::TopHalf);
   _cbPreviewMode->addItem("Left",FilterThread::LeftHalf);
   _cbPreviewMode->addItem("Bottom",FilterThread::BottomHalf);
   _cbPreviewMode->addItem("Right",FilterThread::RightHalf);
   _cbPreviewMode->addItem("Camera",FilterThread::Camera);

   connect( _cbPreviewMode, SIGNAL(activated(int)),
            this, SLOT(onPreviewModeChanged(int)));

   QShortcut * sc = new QShortcut(QKeySequence("Ctrl+P"),this);
   _tbPlay->setToolTip("Launch processing (Ctrl+P)");
   connect( sc, SIGNAL(activated()),
           this, SLOT(onPlay()) );
   connect( _tbPlay, SIGNAL(clicked()),
           this, SLOT(onPlay()));

   connect( _tbZoomOriginal, SIGNAL( clicked() ),
           _imageView, SLOT( zoomOriginal() ) );

   connect( _tbZoomFit, SIGNAL( clicked() ),
           _imageView, SLOT( zoomFitBest() ) );

   connect( _pbApply, SIGNAL( clicked() ),
	   this, SLOT( commandModified() ) );

   connect( _tbCamera, SIGNAL( clicked() ),
	   this, SLOT( snapshot() ) );

   _imageView->setMouseTracking( true );

   connect( _imageView, SIGNAL( mousePress( QMouseEvent * ) ),
           this, SLOT( imageViewMouseEvent( QMouseEvent * ) ) );

   connect( _imageView, SIGNAL( mouseMove( QMouseEvent * ) ),
           this, SLOT( imageViewMouseEvent( QMouseEvent * ) ) );

   connect( _treeGPresets, SIGNAL( itemClicked( QTreeWidgetItem *, int )),
	   this, SLOT( presetClicked( QTreeWidgetItem *, int ) ) );

   connect( _treeGPresets, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int )),
	   this, SLOT( presetDoubleClicked( QTreeWidgetItem *, int ) ) );

   connect( _commandEditor, SIGNAL( commandModified() ),
	   this, SLOT( commandModified() ) );

   _sliderSkipFrames->setRange(0,10);
   connect( _sliderSkipFrames, SIGNAL( valueChanged(int) ),
	   this, SLOT( setFrameSkip(int ) ) );

   // Image filter for the "Save as..." dialog
   QList<QByteArray> formats = QImageWriter::supportedImageFormats();
   QList<QByteArray>::iterator it = formats.begin();
   QList<QByteArray>::iterator end = formats.end();
   _imageFilters = "Image file (";
   while ( it != end ) {
      _imageFilters += QString("*.") + QString(*it).toLower() + " ";
      ++it;
   }
   _imageFilters.chop(1);
   _imageFilters += ")";

   _filterThread = 0;

   if ( ! settings.value("showRightPanel",true).toBool() )
      _rightPanel->hide();
}

MainWindow::~MainWindow()
{
   if ( _filterThread ) {
      _filterThread->stop();
      _filterThread->wait();
      delete _filterThread;
   }
}

void
MainWindow::addPresets( const QDomElement & domE, QTreeWidgetItem * parent )
{
   for( QDomNode n = domE.firstChild(); !n.isNull(); n = n.nextSibling() ) {
      QString name = n.attributes().namedItem( "name" ).nodeValue();
      if ( n.nodeName() == QString("preset") ) {
         QString text = n.firstChild().toText().data();
         QStringList strList;
         strList << name;
         if ( ! parent )
            _treeGPresets->addTopLevelItem( new QTreeWidgetItem( strList ) );
         else
            new QTreeWidgetItem( parent, strList );
      } else if ( n.nodeName() == QString("preset_group") ) {
         QStringList strList;
         strList << name;
         QTreeWidgetItem * parent;
         _treeGPresets->addTopLevelItem( parent = new QTreeWidgetItem( strList ) );
         addPresets( n.toElement(), parent );
      }
   }
}

void
MainWindow::about()
{
   DialogAbout * d = new DialogAbout( this );
   d->exec();
   delete d;
}

void
MainWindow::visitGMIC()
{
   QDesktopServices::openUrl(QUrl("http://gmic.sourceforge.net/"));
}

void
MainWindow::licence()
{
   DialogLicence * d = new DialogLicence( this );
   d->exec();
   delete d;
}

QString
MainWindow::getPreset( const QString & name )
{
   QDomNodeList list =  _presets.elementsByTagName("preset");
   for ( int i = 0; i < list.count(); ++i ) {
      QDomNode n = list.at(i);
      if ( n.attributes().namedItem( "name" ).nodeValue() == name ) {
         return n.firstChild().toText().data().trimmed();
      }
   }
   return QString();
}

void
MainWindow::onImageAvailable()
{
   _imageView->checkSize();
   _imageView->repaint();
}

void
MainWindow::play()
{
   int pm = _cbPreviewMode->itemData(_cbPreviewMode->currentIndex()).toInt();
   FilterThread::PreviewMode  previewMode = static_cast<FilterThread::PreviewMode>(pm);

   _filterThread = new FilterThread( _webcam,
				     _commandEditor->toPlainText(),
				     &_imageView->image(),
				     &_imageView->imageMutex(),
				     previewMode,
				     _sliderSkipFrames->value() );

   connect( _filterThread, SIGNAL( imageAvailable() ),
           this, SLOT( onImageAvailable() ) );

   _filterThread->start();
}

void
MainWindow::stop()
{
   if ( _filterThread ) {
      _filterThread->stop();
      _filterThread->wait();
      delete _filterThread;
      _filterThread = 0;
   }
}

void
MainWindow::onPlay()
{
  if ( _filterThread ) {
     stop();
#if QT_VERSION >= 0x040600
   _tbPlay->setIcon( QIcon::fromTheme("media-playback-start",
				      QIcon(":/images/media-playback-start.png")) );
#else
   _tbPlay->setIcon( QIcon(":/images/media-playback-start.png") );
#endif
   _tbPlay->setToolTip("Launch processing (Ctrl+P)");
  } else {
     play();
#if QT_VERSION >= 0x040600
   _tbPlay->setIcon( QIcon::fromTheme("media-playback-stop",
				      QIcon(":/images/media-playback-stop.png")) );
#else
   _tbPlay->setIcon( QIcon(":/images/media-playback-stop.png") );
#endif
   _tbPlay->setToolTip("Stop processing (Ctrl+P)");
  }
}

void
MainWindow::imageViewMouseEvent( QMouseEvent * e )
{
   int buttons = 0;
   if ( e->buttons() & Qt::LeftButton ) buttons |= 1;
   if ( e->buttons() & Qt::RightButton ) buttons |= 2;
   if ( e->buttons() & Qt::MidButton ) buttons |= 4;
   if (_filterThread)
      _filterThread->setMousePosition( e->x(), e->y(), buttons );
}

void
MainWindow::commandModified()
{
   if ( _filterThread && _filterThread->isRunning() ) {
      stop();
      play();
   }
}

void
MainWindow::presetClicked( QTreeWidgetItem * item, int  )
{
   if ( item->childCount() ) return;
   _commandEditor->setPlainText( getPreset( item->text(0) ) );
}

void
MainWindow::presetDoubleClicked( QTreeWidgetItem * item, int  )
{
   if ( item->childCount() ) return;
   _commandEditor->setPlainText( getPreset( item->text(0) ) );
   if ( _cbPreviewMode->currentText().startsWith("Camera") ) {
      _cbPreviewMode->setCurrentIndex(0);
      onPreviewModeChanged(0);
   }
   commandModified();
}

void
MainWindow::snapshot()
{
   if ( _filterThread )
      _tbPlay->click();
   QString filename = QFileDialog::getSaveFileName( this,
						   "Save image as...",
						   _currentDir,
						   _imageFilters,
						   0,
						   0 );
   if ( ! filename.isEmpty() ) {
      QFileInfo info( filename );
      _currentDir = info.filePath();
      QImageWriter writer( filename );
      _imageView->imageMutex().lock();
      writer.write( _imageView->image() );
      _imageView->imageMutex().unlock();
   }
}

void
MainWindow::setFrameSkip(int i)
{
   _labelSkipFrames->setText( QString("Frame skip (%1)").arg(i) );
   if ( _filterThread )
      _filterThread->setFrameSkip( i );
}

void
MainWindow::onWebcamSelected( QAction * action )
{
   int index = action->data().toInt();
   if ( action->isChecked() ) {
      if ( _filterThread && _filterThread->isRunning() ) {
         stop();
         _webcam.setCameraIndex( index );
         play();
      } else {
         _webcam.setCameraIndex( index );
      }
   }
}

void
MainWindow::onUseOnlinePresets( bool on )
{
   if ( on ) {
      QNetworkRequest request( QUrl("https://foureys.users.greyc.fr/ZArt/zart_presets.xml" ) );
      _networkManager->get( request );
   }
}

void
MainWindow::networkReplyFinished( QNetworkReply* reply )
{
   if ( reply->error() != QNetworkReply::NoError ) {
      QMessageBox::critical( this,
                            "Network Error",
                            "Could not retrieve the preset file from"
                            " the Web. Maybe a problem with your network"
   			     " connection." );
      _builtInPresetsAction->setChecked( true );
      QSettings().setValue( "Presets", "Built-in" );
      return;
   }
   QSettings().setValue( "Presets", "Online" );
   QString error;
   _presets.setContent( reply, false, &error );
   _treeGPresets->clear();
   addPresets( _presets.elementsByTagName("document").at(0).toElement(), 0 );
}

void
MainWindow::setPresetsFile( const QString & file )
{
   QString filename = file;
   if ( filename.isEmpty() )  {
      QSettings settings;
      QString s = settings.value("PresetsFile").toString();
      QString dir = ".";
      if ( QFileInfo(s).exists() )
	 dir = QFileInfo(s).absolutePath();
      filename = QFileDialog::getOpenFileName( this,
					       "Open a presets file",
					       dir,
					       "Preset files (*.xml)" );
   }
   if ( ! filename.isEmpty() ) {
      QSettings settings;
      settings.setValue( "PresetsFile", filename );
      settings.setValue( "Presets", "File" );

      QFile presetsTreeFile( filename );
      QString error;
      presetsTreeFile.open( QIODevice::ReadOnly );
      _presets.setContent( &presetsTreeFile, false, &error );
      presetsTreeFile.close();
      _treeGPresets->clear();
      addPresets( _presets.elementsByTagName("document").at(0).toElement(),
		  0 );
   } else {
      _builtInPresetsAction->setChecked( true );
   }
}

void
MainWindow::onUseBuiltinPresets(bool on)
{
   if ( on ) {
      QFile presetsTreeFile( ":/presets.xml" );
      QString error;
      presetsTreeFile.open( QIODevice::ReadOnly );
      _presets.setContent( &presetsTreeFile, false, &error );
      presetsTreeFile.close();
      _treeGPresets->clear();
      addPresets( _presets.elementsByTagName("document").at(0).toElement(),
		  0 );
      QSettings().setValue( "Presets", "Built-in" );
   }
}


void
MainWindow::savePresetsFile()
{
   QString filename = QFileDialog::getSaveFileName( this,
						   "Save presets file",
						   ".",
						   "Preset files (*.xml)" );
   if ( ! filename.isEmpty() ) {
      QFile presetsFile( filename );
      presetsFile.open( QIODevice::WriteOnly );
      presetsFile.write( _presets.toByteArray() );
      presetsFile.close();
   }
}

void
MainWindow::onPreviewModeChanged( int index )
{
   int mode = _cbPreviewMode->itemData(index).toInt();
   if ( _filterThread )
      _filterThread->setPreviewMode(static_cast<FilterThread::PreviewMode>(mode));
}

void
MainWindow::onRightPanel( bool on )
{
   if ( on && !_rightPanel->isVisible()) {
      _rightPanel->show();
      QSettings().setValue("showRightPanel",true);
      return;
   }
   if ( !on && _rightPanel->isVisible()) {
      _rightPanel->hide();
      QSettings().setValue("showRightPanel",false);
      return;
   }
}
