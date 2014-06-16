/** -*- mode: c++ ; c-basic-offset: 3 -*-
 * @file   MainWindow.h
 * @author Sebastien Fourey
 * @date   July 2010
 * @brief  Declaration of the class ImageFilter
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
#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QtXml>
#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QLineEdit>
#include "ui_MainWindow.h"
#include "WebcamGrabber.h"
#include "FilterThread.h"
class QScrollArea;
class ImageView;
class QSpinBox;
class QComboBox;
class QNetworkReply;
class QNetworkAccessManager;
class QMenu;

class MainWindow : public QMainWindow, public Ui::MainWindow {
   Q_OBJECT
public:

   MainWindow( QWidget * parent = 0 );
   ~MainWindow();
   QString getPreset( const QString & name );

public slots:

   void play();
   void stop();
   void onImageAvailable();
   void commandModified();
   void presetClicked( QTreeWidgetItem * item, int column );
   void presetDoubleClicked( QTreeWidgetItem * item, int column );

   void imageViewMouseEvent( QMouseEvent * );
   void snapshot();
   void about();
   void licence();
   void visitGMIC();
   void setFrameSkip(int );
   void setPresetsFile( const QString & = QString() );
   void savePresetsFile();

   void onWebcamSelected( QAction * );
   void onUseOnlinePresets( bool );
   void onUseBuiltinPresets( bool );
   void networkReplyFinished(QNetworkReply*);
   void onPreviewModeChanged( int index );
   void onRightPanel( bool );
   void onPlay();

private:

   void addPresets( const QDomElement &, QTreeWidgetItem * );

   int _firstWebcamIndex;
   int _secondWebcamIndex;
   WebcamGrabber _webcam;
   FilterThread * _filterThread;
   QDomDocument _presets;
   QString _currentDir;
   QString _imageFilters;
   QNetworkAccessManager * _networkManager;
   QButtonGroup * _bgZoom;
   QString _filtersPath;

   QAction * _onlinePresetsAction;
   QAction * _builtInPresetsAction;
};

#endif
