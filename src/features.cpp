#include "features.h"
#include "wtypes.h"
#include "loadImages.h"

#define RATIO_THRESH 0.7

std::vector<imageFeatures> features::_features;

// detect and compute all of the images features
features::features(std::vector<std::string> images, int downScale)
{
    // find features using SIFT feature detector
    cv::Ptr<cv::FeatureDetector> detector = cv::SIFT::create();
	// find keypoints and descriptors of image using SIFT for every image in the vector
	// the detector with detect and compute are locating FEATURES and descriptors
    for (std::string path : images)
    {
        cv::Mat src = cv::imread(path);
        cv::resize(src, src, src.size() / downScale);
        if (!src.empty())
        {
            cv::Mat descriptors;
            std::vector<cv::KeyPoint> keypoints;
            detector->detectAndCompute(src, cv::noArray(), keypoints, descriptors);
            _features.push_back(imageFeatures{ path, src, keypoints, descriptors });
            std::cout << getFileNameWithExtension(path) << " " << keypoints.size() << " features were extracted\n";
        }
    }
}

// match the features between the images
void features::matchFeatures(cameraCalibration calib, bool showMatchingFeature)
{
    int screenWidth = getScreenWidth();
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE);

	//check for all features found
    for (unsigned int i = 0; i < _features.size() - 1; i++)
    {
        std::vector<std::vector<cv::DMatch>> matches;
        std::vector<cv::Point2f> points1, points2;
        std::vector<int> points1Idx, points2Idx;
        cv::Mat mask, image;

        // 2 nearest neighbour match
        matcher->knnMatch(_features[i].descriptors, _features[i + 1].descriptors, matches, 2); // k=2 therefore there will be two DMatch in matches[i]

        for (std::vector<cv::DMatch>& matche : matches) // the DMatches in matche vector are arranged in descending order of quality
        {
            //good matche is one with small distance measurement
            if (matche[0].distance < RATIO_THRESH * matche[1].distance)
            {
                // queryIdx refers to keypoints1 and trainIdx refers to keypoints2
                points1.push_back(_features[i].keyPoints[matche[0].queryIdx].pt);
                points2.push_back(_features[i + 1].keyPoints[matche[0].trainIdx].pt);
                points1Idx.push_back(matche[0].queryIdx);
                points2Idx.push_back(matche[0].trainIdx);
            }
        }
        if (showMatchingFeature)
        {
            hconcat(_features[i].image, _features[i + 1].image, image); // create image with image i and image  j
        }

        // erase bad matches using fundamental matrix constraints
        cv::Mat E = findEssentialMat(points1, points2, calib.getCameraMatrix(), cv::RANSAC, 0.9999999999999999, 1.0, mask);

        for (int j = mask.rows - 1; j >= 0; j--)
        {
            if (mask.at<uchar>(j, 0)) // if the point was used to solve the essential matrix - the value is 1 otherwise 0 
            {
                if (showMatchingFeature)
                {
                    // draw line on the image 
                    line(image, points1[j], points2[j] + cv::Point2f(_features[i].image.cols, 0), cv::Scalar(rand() % 255, rand() % 255, rand() % 255), 2);
                }
            }
            else
            {
                points1Idx.erase(points1Idx.begin() + j);
                points2Idx.erase(points2Idx.begin() + j);
            }
        }

        _features[i].matchingKeyPoints = { points1Idx, points2Idx };

        std::cout << "Feature matching " << i << " / " << i + 1 << ", good matches " << points1.size() << std::endl;
        if (showMatchingFeature)
        {
            resizeWithAspectRatio(image, screenWidth);
            cv::imshow("Feature matching", image);
            cv::waitKey(100);
        }
    }
    cv::destroyAllWindows();
}

void features::getCurrentKeyPoints(std::vector<cv::Point2f>& currentKeyPoints, int featureIndex)
{
    currentKeyPoints.clear();
    for (int index : _features[featureIndex].matchingKeyPoints.currentKeyPointsIdx)
    {
        currentKeyPoints.push_back(_features[featureIndex].keyPoints[index].pt);
    }
}

void features::getOtherKeyPoints(std::vector<cv::Point2f>& otherKeyPoints, int featureIndex)
{
    otherKeyPoints.clear();
    for (int index : _features[featureIndex].matchingKeyPoints.otherKeyPointsIdx)
    {
        otherKeyPoints.push_back(_features[featureIndex + 1].keyPoints[index].pt);
    }
}


// return the features vector
std::vector<imageFeatures>& features::getFeatures()
{
    return _features;
}

// resize image height to be with aspect ratio with a given width
void resizeWithAspectRatio(cv::Mat &image, int width)
{
    float ratio = width / float(image.size().width); 
    cv::Size dim = cv::Size(width, int(image.size().height * ratio));

    resize(image, image, dim);
}

// get the screen width
int getScreenWidth()
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);

    return desktop.right; //screen width
}