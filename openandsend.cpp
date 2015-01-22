#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <string>
#include <cstdlib>


void openAndSend(std::string path, std::string ip)
{
    cv::Mat img = cv::imread(path, CV_LOAD_IMAGE_COLOR);
    if(img.data != NULL)
    {
        std::string cmd = "cat "+path+" | nc "+ip+" 5001";
        system(cmd.c_str());
    }
}

int main(int argc, char* argv[])
{
    if(argc > 1)
    {
        openAndSend(argv[1], "192.168.1.2");
    }
    return 0;
}
