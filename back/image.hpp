// g++ teste.cpp -o teste `pkg-config --cflags --libs opencv4`

#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

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

typedef struct Coordinates{
    int x_begin;
    int x_end;
    int y_begin;
    int y_end;
} Coordinates;

class Image {
private:
    Mat image;
    Mat image_singlethread;
    Mat image_multithread;
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
    void overwriteImage(const string& path, ImageColorType color_type, ImageType type);
    void overwriteImage(const vector<uchar>& buffer, ImageColorType color_type, ImageType type);

    // Functions
    void show();
    void save();

    // Filters
    void negative_filter();

    // Threads
    void process (const string& filter, int intensity, int threads);
};

// DA CLASSE //////////////////////////////////
Image::
    Image(const string& path, ImageColorType color_type, ImageType type){
        overwriteImage(path, color_type, type);
    }
void Image::
    overwrite(const string& path, ImageColorType color_type, ImageType type): 
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

Image::
    Image(const vector<uchar>& buffer, ImageColorType color_type, ImageType type){ 
        overwriteImage(buffer, color_type, type);
    }
void Image::
    overwriteImage(const vector<uchar>& buffer, ImageColorType color_type, ImageType type) : 
        buffer(buffer), color_type(color_type), type(type) {

        if (buffer.empty()) { // se o buffer estiver vazio
            throw invalid_argument("Erro: buffer de imagem vazio!");
        }

        switch(this->color_type){
            case ImageColorType::RGB:
                this->image = imdecode(buffer, IMREAD_COLOR);
                break;
            case ImageColorType::HSV:
                this->image = imdecode(buffer, IMREAD_COLOR);
                cvtColor(this->image, this->image, COLOR_BGR2HSV);
                break;
            case ImageColorType::GRAYSCALE:
                this->image = imdecode(buffer, IMREAD_GRAYSCALE);
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

void Image::
    show() {
        namedWindow("Janela Fixa", WINDOW_NORMAL); 
        resizeWindow("Janela Fixa", 800, 600);
        imshow("Janela Fixa", this->image);
        waitKey(0);
    }

void Image::
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

// FILTROS //////////////////////////////////
void Image::
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

// FILTROS COM THREADS ////////////////////////

// ---
    
// THREADS ////////////////////////
void Image::
    process(const string& filter, int intensity, int threads) {
        // 1. Deve guardar no objeto os atributos intensity e filter

        // 2. Deve atribuir 0 a cada pixel das imagens de saída, ou seja, limpar elas;
        this->image_singlethread.assign(this->height, vector<uchar>(this->width, 0));
        this->image_multithread.assign(this->height, vector<uchar>(this->width, 0));

        // 3. Chamar a funcao que distribuira areas da imagem para cada thread

        // 4. Cada thread deve processar a parte da imagem que lhe foi atribuida

        for (int i = 0; i < threads; i++) {
            // Considerando que intensity e filter são atributos da classe Image, globalmente acessíveis, entao nao é necessário passar como argumento para a thread
            // Cada thread recebera sua area e processara a imagem
            thread t(this->thread_process, coordenada_inicial, coordenada_final);
        }
    }
    
vector<Coordinates> 
    getRegionCoordinates(int width, int height, int threads) {
        vector<Coordinates> coords;
        int step_x;
        int step_y;
        
        if(threads > height) {
            if( threads > width){
                int to_add = (threads % height == 0) ? 0 : 1;
                threads = width;
                step_y = (height / threads) + to_add;
                step_x = width;
            }
            else{
                int to_add = (threads % width == 0) ? 0 : 1;
                step_x = (width / threads) + to_add;
                step_y = height;
            }
        }else{
            step_x = width;
            int to_add = (threads % height == 0) ? 0 : 1;
            step_y = (height / threads) + to_add;  
        }
        
        // int linhas_split = floor(sqrt(threads));
        // int colunas_split = ceil((float)threads / linhas_split);

        // step_x = width / linhas_split;
        // step_y = height / colunas_split;

        for (int i = 0; i < threads; i++) {
            for (int j = 0; j < threads; j++) {
                Coordinates coord;
                coord.x_begin = i * step_x;
                coord.y_begin = j * step_y;
                coord.x_end = (i + 1) * step_x;
                coord.y_end = (j + 1) * step_y;

                if (coord.x_end >= width) coord.x_end = width;
                if (coord.y_end >= height) coord.y_end = height;
                coords.push_back(coord);
            }
        }

        // for (int i = 0, r = 0; i < linhas_split; ++i) {
        //     int linha_ini = i * step_x;
        //     int linha_fim = (i == linhas_split - 1) ? width : linha_ini + step_x;
    
        //     for (int j = 0; j < colunas_split; ++j) {
        //         if (r++ >= threads) break; // para no t-ésimo bloco
    
        //         int coluna_ini = j * step_y;
        //         int coluna_fim = (j == colunas_split - 1) ? height : coluna_ini + step_y;
    
        //         coords.push_back({linha_ini, linha_fim, coluna_ini, coluna_fim});
        //     }
        // }

        return coords;
    }


// AUXILIARES //////////////////////////////////
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