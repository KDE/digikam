/** -*- mode: c++ ; c-basic-offset: 3 -*-
 * @file   ZArt.cpp
 * @author Sebastien Fourey
 * @date   July 2010
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
using namespace std;
#include <QApplication>
#include <QMessageBox>
#include "WebcamGrabber.h"
#include "Common.h"
#include "MainWindow.h"

int main( int argc, char *argv[] )
{
  QApplication app( argc, argv );
  app.setWindowIcon( QIcon(":images/gmic_hat.png") );
  QCoreApplication::setOrganizationName("GREYC");
  QCoreApplication::setOrganizationDomain("greyc.fr");
  QCoreApplication::setApplicationName("ZArt");
  
  QStringList args = app.arguments();
  if ( args.size() == 2 && ( args[1] == "-h" || args[1] == "--help") ) {
     cout << "Usage:" << endl
          << "       " << QFileInfo(argv[0]).baseName().toAscii().constData()
	  << " [options]" << endl
          << "  " << "Options: " << endl
          << "      --help | -h : print this help." << endl
          << "      --cam N     : disable camera detection and force selection"
	  << "                    camera with index N." << endl
          << endl;
     exit(EXIT_FAILURE);
  }

  if ( ! WebcamGrabber::getWebcamList().count() ) {
     const char * message =
	"No webcam found.<br/><br/>"
	"(ZArt is useless without a webcam!)";
     QMessageBox::critical(0,
			   QString("ZArt %1: Error").arg(ZART_VERSION),
			   message);
     exit(EXIT_FAILURE);
  }

  MainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
