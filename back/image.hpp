#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <iostream>
#include <opencv2/opencv.hpp> // Ensure OpenCV is installed and the include path is set
#include <opencv2/core.hpp>
#include <string>

using namespace std;
using namespace cv;

// typedef enum{
//     IMAGE_TYPE_JPEG = "jpeg",
//     IMAGE_TYPE_JPG = "jpg",
//     IMAGE_TYPE_PNG = "png",
//     IMAGE_TYPE_BMP = "bmp",
//     IMAGE_TYPE_TIFF = "tiff"
// } IMAGE_TYPE;

// typedef enum{
//     IMAGE_COLOR_RGB = "rgb",
//     IMAGE_GRAYSCALE = "gray_scale"
// } IMAGE_COLOR_TYPE;

// typedef enum{
//     IMAGE_BLUR_FILTER = "blur",
//     IMAGE_NEGATIVE_FILTER = "negative",
//     IMAGE_SHARPEN_FILTER = "sharpen",
//     IMAGE_GAUSSIAN_FILTER = "gaussian",
// } IMAGE_FILTER_TYPE;

class Image {
    private:
        Mat image;
        // IMAGE_TYPE type;
        // IMAGE_COLOR_TYPE color_type;
        string path;
        int width;
        int height;
        string type;
        string color_type;


    public: 
        Image(const vector<uchar>& buffer, const string color_type, const string type);

};


Image::Image(const vector<uchar>& buffer, const string color_type, const string type) {
    this->color_type = color_type;
    this->type = type;

    // Decodificando a imagem com base no tipo de cor
    this->image = imdecode(buffer, color_type == "rgb" ? IMREAD_COLOR : IMREAD_GRAYSCALE);
    if (this->image.empty()) {
        cerr << "Erro: falha ao decodificar imagem!" << endl;
        exit(EXIT_FAILURE);
    }

    this->width = image.cols;
    this->height = image.rows;

    // Exibe a imagem recebida:
    imshow("Imagem Recebida", this->image);
    waitKey(0); // Aguarda até que uma tecla seja pressionada
}

// void Image::saveImage(){
//     if (this->image.empty()) {
//         cerr << "Error: No image to save!" << endl;
//         return;
//     }
//     string output_path = "output/output." + string(this->type);

//     return;
// }


#endif