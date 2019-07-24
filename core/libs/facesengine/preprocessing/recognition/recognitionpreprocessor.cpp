/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-07-09
 * Description : Preprocessor for face recognition
 *
 * Copyright (C) 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#include "recognitionpreprocessor.h"

// Local includes

#include "openfacepreprocessor.h"
#include "digikam_debug.h"

using namespace Digikam;


class RecognitionPreprocessor::Private
{

public:

	explicit Private();
	~Private();

	void init(PreprocessorSelection mode);
	cv::Mat preprocess(const cv::Mat& image);

private:

	int						preprocessingMode;

	OpenfacePreprocessor*	ofpreprocessor;

};


RecognitionPreprocessor::Private::Private()
  : preprocessingMode(-1),
  	ofpreprocessor(0)
{
}

RecognitionPreprocessor::Private::~Private()
{
	delete ofpreprocessor;
}

void RecognitionPreprocessor::Private::init(PreprocessorSelection mode)
{
	preprocessingMode = mode;

	switch(mode)
	{
		case OPENFACE:
		{
			ofpreprocessor = new OpenfacePreprocessor;
			ofpreprocessor->init();
			break;
		}
		default:
		{
			qCDebug(DIGIKAM_FACEDB_LOG) << "Error unknown preprocessingMode " << preprocessingMode;
			preprocessingMode = -1;
		}
	}
}

cv::Mat RecognitionPreprocessor::Private::preprocess(const cv::Mat& image)
{
	switch(preprocessingMode)
	{
		case OPENFACE:
		{
			qCDebug(DIGIKAM_FACEDB_LOG) << "Align face for OpenFace neural network model";
			return ofpreprocessor->process(image);
		}
		default:
		{
			qCDebug(DIGIKAM_FACEDB_LOG) << "Error unknown preprocessingMode " << preprocessingMode;
			return image;
		}
	}
}

// ------------------------------------------------------------------------------------------------------

RecognitionPreprocessor::RecognitionPreprocessor()
  : Preprocessor(),
  	d(new Private)
{
}

RecognitionPreprocessor::~RecognitionPreprocessor()
{
	delete d;
}

void RecognitionPreprocessor::init(PreprocessorSelection mode)
{
	d->init(mode);
}

cv::Mat RecognitionPreprocessor::preprocess(const cv::Mat& image)
{
	return d->preprocess(image);
}