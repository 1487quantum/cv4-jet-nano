#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace cv;

int main(int argc, char** argv)
{
    std::cout << "Reading IMG..." << std::endl;

    Mat src = imread(argv[1], IMREAD_COLOR); //Load img from cmd line
    Mat grayImg = imread(argv[1], IMREAD_GRAYSCALE); //Grayscale

    if (src.empty()) {
        std::cout << "IMG is empty!" << std::endl;
        return -1;
    }

    medianBlur(grayImg, grayImg, 31);

    Mat dst, detected_edges;

    //Threshold
    Mat ithr;
    adaptiveThreshold(grayImg, ithr, 255, ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 191, 0);

    //Dilate
    int erosion_type = MORPH_ELLIPSE; //MORPH_RECT;
    int erosion_size{ 5 };
    Mat element = getStructuringElement(erosion_type,
        Size(2 * erosion_size + 1, 2 * erosion_size + 1),
        Point(erosion_size, erosion_size));
    dilate(ithr, ithr, element);

    std::vector<Vec3f> circles;
    HoughCircles(ithr, circles, HOUGH_GRADIENT, 1,
        grayImg.rows / 20, // change this value to detect circles with different distances to each other
        100, 30, 600, 850 // change the last two parameters
        // (min_radius & max_radius) to detect larger circles
        );
    Point minPt(10000, 10000);
    int minRadius{ 10000 };
    int minDiff{ 10000 };

    //Draw center lines
    int thicknessLine{ 1 };
    cv::line(src, Point(0, grayImg.rows / 2), Point(grayImg.cols, grayImg.rows / 2), Scalar(0, 0, 255), thicknessLine);
    cv::line(src, Point(grayImg.cols / 2, 0), Point(grayImg.cols / 2, grayImg.rows), Scalar(0, 0, 255), thicknessLine);

    for (size_t i = 0; i < circles.size(); i++) {

        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        // circle outline
        int radius = c[2];

        if (center.y > (grayImg.rows / 3) && center.y < (grayImg.rows * 2 / 3)) {
            if (radius + grayImg.cols / 2 < grayImg.cols) {
                // circle center
                circle(src, center, 1, Scalar(0, 100, 100), 3, LINE_AA);

                int centerDiff{ abs(center.y - grayImg.rows / 2) };
                if (centerDiff < minDiff) {
                    std::cout << abs(center.y - grayImg.rows / 2) << std::endl;
                    std::cout << minPt.y << std::endl;
                    minPt.x = center.x;
                    minPt.y = center.y;
                    minDiff = centerDiff;
                    minRadius = radius;
                }

                circle(src, center, radius, Scalar(255, 0, 255), 3, LINE_AA);
                putText(src, (std::string) "(" + std::to_string(center.x) + ", " + std::to_string(center.y) + ", r" + std::to_string(radius) + "), " + std::to_string(centerDiff), Point(center.x, center.y + 5),
                    FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(0, 200, 0), 1, CV_AA);
            }
        }
    }
    circle(src, minPt, 1, Scalar(250, 0, 0), 3, LINE_AA);
    circle(src, minPt, minRadius, Scalar(255, 0, 0), 3, LINE_AA);

    cv::Mat pRoi(Mat::zeros(src.size(), CV_8UC1));
    if (minRadius != 10000) {
        // Remove details outside circle
        Mat mask = Mat::zeros(src.size(), CV_8UC1);
        circle(mask, minPt, minRadius, Scalar(255), -1, 8);

        src.copyTo(pRoi, mask);
        //Crop img
        constexpr int border{ 10 };
        int cOffset{ minPt.x - pRoi.cols / 2 < 0 ? -border : border };
        pRoi = pRoi(Rect(pRoi.cols / 2 - (minRadius + cOffset), 0, 2 * (minRadius + cOffset + (minPt.x - pRoi.cols / 2)), pRoi.rows));
    }

    namedWindow("Color", WINDOW_NORMAL);
    namedWindow("Gray", WINDOW_NORMAL);
    namedWindow("Mask", WINDOW_NORMAL);
    namedWindow("Threshold", WINDOW_NORMAL);
    //namedWindow("Color", WINDOW_OPENGL);		//With openCV support
    //namedWindow("Color", WINDOW_AUTOSIZE);	//Cannot resize window
    imshow("Color", src);
    imshow("Threshold", ithr);
    imshow("Mask", pRoi);
    imshow("Gray", grayImg);
    waitKey(0);
    destroyWindow("Color");
    return 0;
}
