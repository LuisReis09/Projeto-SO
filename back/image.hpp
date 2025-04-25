// g++ teste.cpp -o teste `pkg-config --cflags --libs opencv4`

#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;

enum class ImageType {
    JPEG, JPG, PNG, BMP, TIFF
};

enum class ImageColorType {
    RGB, HSV, GRAYSCALE
};

typedef struct Timer{
    double start;
    double end;
} Timer;

class Image {
private:
    Mat image;
    ImageType type;
    ImageColorType color_type;
    string path;
    vector<uchar> buffer;
    int width;
    int height;

public: 
    // Constructor
    Image(const string& path, ImageColorType color_type, ImageType type);
    Image(const vector<uchar>& buffer, ImageColorType color_type, ImageType type);

    // Functions
    void show();
    void save();

    // Filters
    void negative_filter();

    // Threads
    void process (const string& filter, int intensity, int threads);
};

inline Image::
    Image(const string& path, ImageColorType color_type, ImageType type) : 
        path(path), color_type(color_type), type(type) {

        switch(this->color_type){
            case ImageColorType::RGB:
                this->image = imread(path, IMREAD_COLOR);
                break;
            case ImageColorType::HSV:
                this->image = imread(path, IMREAD_COLOR);
                cvtColor(this->image, this->image, COLOR_BGR2HSV);
                break;
            case ImageColorType::GRAYSCALE:
                this->image = imread(path, IMREAD_GRAYSCALE);
                break;
            default:
                throw invalid_argument("Erro: tipo de cor inválido!");
        }
        if (this->image.empty()) {
            cerr << "Erro: falha ao decodificar imagem!" << endl;
            exit(EXIT_FAILURE);
        }

        this->width = image.cols;
        this->height = image.rows;
    }

inline Image::
    Image(const vector<uchar>& buffer, ImageColorType color_type, ImageType type) : 
        buffer(buffer), color_type(color_type), type(type) {

        if (buffer.empty()) { // se o buffer estiver vazio
            throw invalid_argument("Erro: buffer de imagem vazio!");
        }

        this->image = imdecode(buffer, color_type == ImageColorType::RGB ? IMREAD_COLOR : IMREAD_GRAYSCALE);
        if (this->image.empty()) {
            cerr << "Erro: falha ao decodificar imagem!" << endl;
            exit(EXIT_FAILURE);
        }

        this->width = image.cols;
        this->height = image.rows;
    }

inline void Image::
    show() {
        namedWindow("Janela Fixa", WINDOW_NORMAL); 
        resizeWindow("Janela Fixa", 800, 600);
        imshow("Janela Fixa", this->image);
        waitKey(0);
    }

inline void Image::
    save(){
        string imName =  "output/image";
        switch (this->type){
            case ImageType::JPEG: 
                imName += ".jpeg";
                break;
            case ImageType::JPG: 
                imName += ".jpg";
                break;
            case ImageType::PNG: 
                imName += ".png";
                break;
            case ImageType::BMP: 
                imName += ".bpm";
                break;
            case ImageType::TIFF: 
                imName += ".tiff";
            default:
                break;
        }

        imwrite(imName, this->image);
        return;
    }

inline void Image::
    negative_filter() {
        if(this->color_type == ImageColorType::RGB) {
            for(int i = 0; i < this->width; i++){
                for(int j = 0; j < this->height; j++){
                    Vec3b& pixel = this->image.at<Vec3b>(j, i);
                    pixel[0] = 255 - pixel[0]; // canal azul
                    pixel[1] = 255 - pixel[1]; // canal verde
                    pixel[2] = 255 - pixel[2]; // canal vermelho
                }
            }
        }
        else { // imagem em escala de cinza
            for(int i = 0; i < this->width; i++){
                for(int j = 0; j < this->height; j++){
                    uchar& pixel = this->image.at<uchar>(j, i);
                    pixel = 255 - pixel;
                }
            }
        }
    }

void Image::
    process(const string& filter, int intensity, int threads) {
        sleep(10); // Simula processamento
    }


// funções auxiliares de conversão de string pra Enum
inline ImageType stringToImageType(const string& str) {
    if (str == "jpeg") return ImageType::JPEG;
    if (str == "jpg") return ImageType::JPG;
    if (str == "png") return ImageType::PNG;
    if (str == "bmp") return ImageType::BMP;
    if (str == "tiff") return ImageType::TIFF;
    throw invalid_argument("Tipo de imagem inválido");
}
inline ImageColorType stringToImageColorType(const string& str) {
    if (str == "rgb") return ImageColorType::RGB;
    if (str == "hsv") return ImageColorType::HSV;
    if (str == "gray_scale") return ImageColorType::GRAYSCALE;
    throw invalid_argument("Tipo de cor inválido");
}
#endif