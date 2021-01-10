﻿#include "loadImages.h"
#include "features.h"
#include "cameraCalibration.h"
#include "cameraPosition.h"


std::string xmlFilePath = "../calib.xml";// "calib.xml"; //camera calbration file location
std::string chessboardImagesPath = ""; // chess board images for camera calibration
std::string objectImagesPath = "C:\\Users\\BennySobol\\Desktop\\dataset\\fountain-P5\\images"; // Images of object surface to extract points cloud from
double focalLength = 0;
bool isSorted = true;
bool optimization = true;
bool showMatchFeatures = true;

//TO DO
// main params => -b -s -h -s -o -cf=1234 -cxml=calib.xml -cchessboard=C:\\chessboard  -u=C:\\images 

int main(int argc, char* argv[])
{
    try
    {
        cameraCalibration cameraCalibrator;

		//load images
        auto images = loadImages::getInstance()->load(objectImagesPath, isSorted);
        std::cout << "Images path were loaded\n";
		//extract features from every image
        features features(images);

        //features features("features.xml"); //TO DO - FIX
        //features.load("features.xml");

        std::cout << "Features were extracted\n";
        if (fs::exists(xmlFilePath)) // load calibration - if exsits
        {
            cameraCalibrator.load(xmlFilePath);
        }
        else if (chessboardImagesPath != "") // calibrate camera
        {
            std::vector<std::string> ImagesPath = loadImages::getInstance()->load(chessboardImagesPath, true);
            cameraCalibrator.addChessboardPoints(ImagesPath, cv::Size(9, 6)); // size of inner chessboard corners
            //print if there are errors, and save calibration file
            std::cout << "Error: " << cameraCalibrator.calibrate() << "\n";
            cameraCalibrator.save("calib.xml");
        }
        else if (focalLength != 0) // cameraMatrix from focal length
        {
            cameraCalibrator.estimateCameraMatrix(focalLength, features.getFeatures()[0].image.size());
        }
        else
        {
            throw std::exception("There mast be calibration parms");
        }
        //print camera calibration params and camera metrix extracted from calibration
        std::cout << "Calibration parms:\n CameraMatrix:\n" << cameraCalibrator.getCameraMatrix() << "\nDistortionCoefficients\n" << cameraCalibrator.getDistortionCoefficients() << "\n";

		//find matching features between every 2 images - do it to all images
        features.matchFeatures(cameraCalibrator, optimization, showMatchFeatures);
        std::cout << "Images features were matched\n";

        //features.save("features.xml");

		//load the features
        auto imagesFeatures = features.getFeatures();
		//get the PCL and display it using camera position reconstract
        cameraPosition cameraPosition(cameraCalibrator, imagesFeatures, optimization);
        cameraPosition.savePointCloud(objectImagesPath);
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: " << e.what() << '\n';
    }

    return 0;
}