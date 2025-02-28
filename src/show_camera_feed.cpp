#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    // Open the video capture device
    cv::VideoCapture cap(0);

    if (!cap.isOpened())
    {
        std::cerr << "Cannot open camera" << std::endl;
        return -1;
    }

    // Check the default resolution
    int defaultWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int defaultHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    std::cout << "Default Resolution: " << defaultWidth << "x" << defaultHeight << std::endl;

    // Try setting the resolution to a higher value if supported
    // Replace 1920 and 1080 with the maximum resolution your camera supports
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    // Check the actual resolution after setting
    int actualWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int actualHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    std::cout << "Actual Resolution: " << actualWidth << "x" << actualHeight << std::endl;

    // Read frames
    cv::Mat frame;
    while (true)
    {
        if (!cap.read(frame))
            break;
        std::cerr << ".";

        // Display the frame
        cv::imshow("Frame", frame);

        // Exit on key press
        if (cv::waitKey(1) == 'q')
            break;
    }

    std::cerr << "\nexiting\n";
    // Release resources
    cap.release();
    cv::destroyAllWindows();
}
