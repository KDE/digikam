/*
 * Copyright (c) 2011,2012. Philipp Wagner <bytefish[at]gmx[dot]de>.
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 */

// Pragma directives to reduce warnings from OpenCv header files.
#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"
#endif

#include "precomp.hpp"
#include "face.hpp"

// Restore warnings
#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace cv
{
namespace face
{

std::vector<int> FaceRecognizer::getLabelsByString(const String &str) const
{
  std::vector<int> labels;
  for (std::map<int, String>::const_iterator it = _labelsInfo.begin(); it != _labelsInfo.end(); it++)
  {
      size_t found = (it->second).find(str);
      if (found != String::npos)
          labels.push_back(it->first);
  }
  return labels;
}

String FaceRecognizer::getLabelInfo(int label) const
{
    std::map<int, String>::const_iterator iter(_labelsInfo.find(label));
    return iter != _labelsInfo.end() ? iter->second : "";
}

void FaceRecognizer::setLabelInfo(int label, const String &strInfo)
{
    _labelsInfo[label] = strInfo;
}

void FaceRecognizer::update(InputArrayOfArrays src, InputArray labels)
{
    (void)src;
    (void)labels;
    String error_msg = format("This FaceRecognizer does not support updating, you have to use FaceRecognizer::train to update it.");
    CV_Error(Error::StsNotImplemented, error_msg);
}

#if OPENCV_TEST_VERSION(3,4,0)
void FaceRecognizer::load(const String &filename)
#else
void FaceRecognizer::read(const String &filename)
#endif
{
    FileStorage fs(filename, FileStorage::READ);
    if (!fs.isOpened())
        CV_Error(Error::StsError, "File can't be opened for writing!");
#if OPENCV_TEST_VERSION(3,4,0)
    this->load(fs);
#else
    this->read(fs);
#endif
    fs.release();
}

#if OPENCV_TEST_VERSION(3,4,0)
void FaceRecognizer::save(const String &filename) const
#else
void FaceRecognizer::write(const String &filename) const
#endif
{
    FileStorage fs(filename, FileStorage::WRITE);
    if (!fs.isOpened())
        CV_Error(Error::StsError, "File can't be opened for writing!");
#if OPENCV_TEST_VERSION(3,4,0)
    this->save(fs);
#else
    this->write(fs);
#endif
    fs.release();
}

int FaceRecognizer::predict(InputArray src) const {
    int _label;
    double _dist;
    predict(src, _label, _dist);
    return _label;
}

void FaceRecognizer::predict(InputArray src, CV_OUT int &label, CV_OUT double &confidence) const {
    Ptr<StandardCollector> collector = StandardCollector::create(getThreshold());
    predict(src, collector);
    label = collector->getMinLabel();
    confidence = collector->getMinDist();
}

}
}

