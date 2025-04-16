#include <iostream>
#include <opencv2/opencv.hpp>
#include "image.hpp"

using namespace std;
using namespace cv;

int main(void){
    Image image = Image("teste.jpg", ImageColorType::GRAYSCALE, ImageType::JPG);
    image.show();
    image.negative_filter();
    image.show();
    image.save();

    return 0;
}