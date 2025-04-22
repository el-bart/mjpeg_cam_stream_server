#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

void write_to_disk(cv::Mat& frame, char const* path)
{
  // Ensure the frame is in the correct format for JPEG encoding
  if (frame.type() != CV_8UC3)
  {
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB); // Convert to RGB if needed
    frame.convertTo(frame, CV_8UC3); // Ensure 8-bit unsigned
  }

  // Encode the frame into JPEG format
  std::vector<uchar> buffer;
  std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90}; // Optional: Set JPEG quality
  cv::imencode(".jpg", frame, buffer, params);

  // Now 'buffer' contains the JPEG encoded frame data
  // You can use this buffer to send the frame over a network or save it to a file

  // Example: Save the buffer to a file
  FILE* file = fopen("output.jpg", "wb");
  if (!file)
  {
    std::cerr << "Could not open file for writing." << std::endl;
    return;
  }
  fwrite(buffer.data(), 1, buffer.size(), file);
  fclose(file);
}


int main()
{
    // Open the video capture device
    cv::VideoCapture cap("/dev/video0");

    if (!cap.isOpened())
    {
        std::cerr << "Cannot open camera" << std::endl;
        return -1;
    }

    // Check the default resolution
    int defaultWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int defaultHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    std::cout << "Default Resolution: " << defaultWidth << "x" << defaultHeight << std::endl;

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

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
    if (!cap.read(frame))
    {
      std::cerr << "failed to capture image\n";
      return 2;
    }
    write_to_disk(frame, "output.jpg");

    cap.release();
    cv::destroyAllWindows();
}
