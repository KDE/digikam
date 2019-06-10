#!/usr/bin/env python3

from imutils import paths
import imutils
import cv2
import argparse
import os
import numpy
import sys

# Default values
fileDir = os.path.dirname(os.path.realpath(__file__))
modelDir = os.path.join(fileDir, '..', 'models')
dlibModelDir = os.path.join(modelDir, 'dlib')
openfaceModelDir = os.path.join(modelDir, 'openface')
imgDimDefault = 96

sys.path.append("/home/ttdinh/devel/personal/openface")

import openface

# Function to detect face
def detectFaceCascade(image, detector):
    image = detectFaceNeutral(image, detector)
    image_gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    image_gray = cv2.equalizeHist(image_gray)
    #-- Detect faces
    face = []
    detectedFaces = detector.detectMultiScale(image_gray)
    # print(detectedFaces)
    for (x,y,w,h) in detectedFaces:
        face = image[y:y+h,x:x+w,:]
    if(face == []):
        face = image
    return face

def detectFaceNeutral(image, detector):
    # image = imutils.resize(image, width=600)
    return image

def detectFaceDNN(image, detector):
    face = []
    (h, w) = image.shape[:2]
    # construct a blob from the image
    imageBlob = cv2.dnn.blobFromImage(
        cv2.resize(image, (300, 300)), 1.0, (300, 300),
        (104.0, 177.0, 123.0), swapRB=False, crop=False)
    # apply OpenCV's deep learning-based face detector to localize
    # faces in the input image
    detector.setInput(imageBlob)
    detections = detector.forward()
    # ensure at least one face was found
    if len(detections) > 0:
        # we're making the assumption that each image has only ONE
        # face, so find the bounding box with the largest probability
        i = numpy.argmax(detections[0, 0, :, 2])
        confidence = detections[0, 0, i, 2]
        # print(confidence)
        # ensure that the detection with the largest probability also
        # means our minimum probability test (thus helping filter out
        # weak detections)
        if confidence >= 0.85:
            # compute the (x, y)-coordinates of the bounding box for
            # the face
            box = detections[0, 0, i, 3:7] * numpy.array([w, h, w, h])
            (startX, startY, endX, endY) = box.astype("int")
            # extract the face ROI and grab the ROI dimensions
            face = image[startY:endY, startX:endX, :]
            (fH, fW) = face.shape[:2]
            # ensure the face width and height are sufficiently large
            if fW < 20 or fH < 20:
                face = []
    if face == []:
        face = image
    return face


def distanceToGroup(face, faceGroups):
    for faceRef in faceGroups:
        dist = cv2.norm(numpy.array(face)-numpy.array(faceRef),cv2.NORM_L2)
    return dist*1.0/len(faceGroups)


def cosineDistance(v1, v2):
    vec1 = numpy.array(v1)
    vec2 = numpy.array(v2)
    return numpy.dot(vec1, vec2) / (cv2.norm(vec1,cv2.NORM_L2)*cv2.norm(vec2,cv2.NORM_L2))


def alignFace(face, alignTool, imgDim):
    bb = alignTool.getLargestFaceBoundingBox(face)
    if bb is None:
        print("Unable to find a face")
        return face
    alignedFace = alignTool.align(imgDim, face, bb,
                                  landmarkIndices=openface.AlignDlib.OUTER_EYES_AND_NOSE)
    if alignedFace is None:
        print("Unable to align face")
        return face
    # alignedFace = face
    return alignedFace


def process(imagePath, detector, alignTool, imgDim, model):
    image = cv2.imread(imagePath)
    face = detectFaceDNN(image, detector) #[:,:,0]
    faceAligned = alignFace(face, alignTool, imgDim)
    faceBlob = cv2.dnn.blobFromImage(faceAligned, 1.0 / 255,
                (imgDim, imgDim), (0, 0, 0), swapRB=False, crop=True) # work with Pytorch openface model
    # faceBlob = cv2.dnn.blobFromImage(face, 1.0,
    #         (224, 224), (0, 0, 0), swapRB=True, crop=False) # work with resnet 50 model
    model.setInput(faceBlob)
    return model.forward()


# contruct the argument parser and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-d", "--dataset", required=True,
                help="path to dataset directory")
ap.add_argument("-m", "--model", required=False,
                help="path to deep learning model directory for OpenCV DNN",
                default=os.path.join(openfaceModelDir, 'nn4.small2.v1.t7'))
ap.add_argument("-p", "--proto", required=False,
                help="path to deep learning proto model directory for OpenCV DNN")
ap.add_argument("-dt", "--detector", required=False,
                help="path to detector model folder")
ap.add_argument("-o", "--objects", required=True,
                help="number of objects to identify")
ap.add_argument("-s", "--samples", required=True,
                help="number of samples per object")
ap.add_argument("-r", "--ratio", required=True,
                help="ratio to divide test set / whole set")
ap.add_argument("-fp", "--dlibFacePredictor", required=False, 
                help="Path to dlib's face predictor.",
                default=os.path.join(dlibModelDir, "shape_predictor_68_face_landmarks.dat"))
args = vars(ap.parse_args())


# Parse arguments
nbObjects = int(args["objects"])
nbSamples = int(args["samples"])
ratio = float(args["ratio"])
dataset = args["dataset"]


# Load detector
protoPath = os.path.sep.join([args["detector"], "deploy.prototxt"])
modelPath = os.path.sep.join([args["detector"],
    "VGG_VOC0712_SSD_300x300_iter_120000.caffemodel"])
detector = cv2.dnn.readNetFromCaffe(protoPath, modelPath)
# detector = cv2.CascadeClassifier(args["detector"])


# Load model
# protoPath = os.path.sep.join([args["model"], "deploy.prototxt"])
# modelPath = os.path.sep.join([args["model"], "deploy.caffemodel"])
model = cv2.dnn.readNetFromTorch(args["model"])
# model = cv2.dnn.readNetFromCaffe(args["proto"], args["model"])


# Face alignment
alignTool = openface.AlignDlib(args["dlibFacePredictor"])


# List training set and test set
trainingSet = []
testSet = []
for i in range(0, nbObjects):
    pathToDataSetObj = os.path.sep.join([dataset, "s%d" %(i+1)])
    trainingSetObj = []
    testSetObj = []
    for j in range(0, nbSamples):
        if(j < nbSamples*ratio):
            trainingSetObj.append(os.path.sep.join([pathToDataSetObj, "%d.pgm" %(j+1)]))
        else:
            testSetObj.append(os.path.sep.join([pathToDataSetObj, "%d.pgm" %(j+1)]))
    trainingSet.append(trainingSetObj.copy())
    testSet.append(testSetObj.copy())
    trainingSetObj.clear()
    testSetObj.clear()


# Compute embedding of faces for training set
trainingDict = {}
print("Training...")
for i in range(0, nbObjects):
    trainingVec = []
    for imagePath in trainingSet[i]:
        # print(imagePath)
        trainingRes = process(imagePath, detector, alignTool, imgDimDefault, model)
        trainingVec.append(trainingRes.flatten())
    trainingDict[i + 1] = trainingVec.copy()
    trainingVec.clear()
print("trainingDict len: %d" %(len(trainingDict)))


# Compute embedding of faces for test set
testDict = {}
print("Compute test...")
for i in range(0, nbObjects):
    testVec = []
    for imagePath in testSet[i]:
        testRes = process(imagePath, detector, alignTool, imgDimDefault, model)
        testVec.append(testRes.flatten())
    testDict[i + 1] = testVec.copy()
    testVec.clear()
print("testDict len %d" %(len(testVec)))


# Testing
testResult = {}
threshold = 1.6
print("Testing...")
for i in range(0, len(testDict)):
    testResultVec = []
    l = 0
    for test in testDict[i + 1]:
        min_dist = 1000000
        max_dist = -1
        label = 0
        closestImage = ""
        for j in range(0, len(trainingDict)):
            # dist = distanceToGroup(test, trainingDict[j + 1])
            # if(dist < min_dist):
            #         min_dist = dist
            #         label = j + 1
            # if(min_dist > threshold):
            #     label = 0
            for train in trainingDict[j + 1]:
                dist = cosineDistance(train, test)
                if(dist > max_dist):
                    max_dist = dist
                    label = j + 1
        # testResultVec.append([label, min_dist, testSet[i][l]])
        testResultVec.append([label, max_dist, testSet[i][l]])
    testResult[i + 1] = testResultVec.copy()
    testResultVec.clear()
    l += 1
# print(testResult)


# Evaluation
nbTests = 0
recognized = 0
falsePositive = 0
unknown = 0
for i in range(0, len(testResult)):
    nbTests += len(testResult[i + 1])
    for res in testResult[i + 1]:
        if(res[0] == 0):
            unknown += 1
        elif(res[0] != (i + 1)):
            falsePositive += 1
            print([i+1, res])
        else:
            recognized += 1
print("nbTests = %d" %(nbTests))
print("recognized = %d / %d (%.2f %%)" %(recognized, nbTests, recognized*100.0/nbTests))
print("falsePositive = %d / %d (%.2f %%)" %(falsePositive, nbTests, falsePositive*100.0/nbTests))
print("unknown = %d / %d (%.2f %%)" %(unknown, nbTests, unknown*100.0/nbTests))

