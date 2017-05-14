/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-16
 * Description : Face pre-processing CLI tool
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

// OpenCV includes

#include "libopencv.h"
#include "asmopencv.h"

// Qt includes

#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QImage>
#include <QTime>
#include <QDebug>

// Implementation not exported with digiKam core.

#include "../../libs/facesengine/preprocessing-tantriggs/tantriggspreprocessor.h"
#include "../../libs/facesengine/preprocessing-tantriggs/tantriggspreprocessor.cpp"

using namespace Digikam;

// --------------------------------------------------------------------------------------------------

QStringList toPaths(char** const argv, int startIndex, int argc)
{
    QStringList files;

    for (int i = startIndex; i < argc; i++)
    {
        files << QString::fromLocal8Bit(argv[i]);
    }

    return files;
}

QList<cv::Mat> toImages(const QStringList& paths)
{
    QList<cv::Mat> images;

    foreach (const QString& path, paths)
    {
        QByteArray s = path.toLocal8Bit();
        images << cv::imread(std::string(s.data()), CV_LOAD_IMAGE_GRAYSCALE);
    }

    return images;
}

// --------------------------------------------------------------------------------------------------

class OpenCVSideBySideDisplay
{
public:

    OpenCVSideBySideDisplay(int rows, int uiSize = 200)
        : uiSize(uiSize),
          currentRow(0)
    {
        bigImage = cv::Mat::zeros(uiSize*rows, 2*uiSize, CV_8UC3);
    }

    void add(const cv::Mat& left, const cv::Mat& right)
    {
        // Draw images side-by-side for later display
        QSize size(left.cols, left.rows);
        size.scale(uiSize, uiSize, Qt::KeepAspectRatio);
        cv::Size scaleSize(size.width(), size.height());

        const int top = currentRow*uiSize;
        cv::Mat scaledLeft, scaledRight;
        cv::resize(left,  scaledLeft,  scaleSize);
        cv::resize(right, scaledRight, scaleSize);

        if (scaledLeft.channels() == 1)
        {
            cv::cvtColor(scaledLeft, scaledLeft, CV_GRAY2BGR);
        }

        if (scaledRight.channels() == 1)
        {
            cv::cvtColor(scaledRight, scaledRight, CV_GRAY2BGR);
        }

        scaledLeft.copyTo(bigImage.colRange(0, scaledLeft.cols).rowRange(top, top+scaledLeft.rows));
        scaledRight.copyTo(bigImage.colRange(uiSize, uiSize+scaledRight.cols).rowRange(top, top+scaledRight.rows));

        currentRow++;
    }

    void show()
    {
        QLabel label;
        label.setPixmap(ASM::cvMatToQPixmap(bigImage));
        label.show();
    }

    cv::Mat   bigImage;
    const int uiSize;
    int       currentRow;
};

// --------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        qDebug() << "Bad Arguments!!!\nUsage: " << argv[0] << " preprocess <image1> <image2> ... ";
        return 0;
    }

    QApplication app(argc, argv);

    QStringList paths     = toPaths(argv, 1, argc);
    QList<cv::Mat> images = toImages(paths);

    QTime time;
    time.start();

    TanTriggsPreprocessor preprocessor;
    OpenCVSideBySideDisplay display(images.size());

    foreach (const cv::Mat& image, images)
    {
        qDebug() << "channels " << image.channels();
        cv::Mat processed = preprocessor.preprocess(image);
        display.add(image, processed);
    }

    int elapsed = time.elapsed();
    qDebug() << "Preprocessing took " << elapsed << " for " << images.size() << " , "
                          << ((float)elapsed/images.size()) << " per image";

    display.show();
    app.exec();

    return 0;
}
