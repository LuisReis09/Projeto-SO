// g++ teste.cpp -o teste `pkg-config --cflags --libs opencv4`

#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <iostream>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>
#include <time.h>
#include "ThreadPool.hpp"

using namespace std;
using namespace cv;

enum class ImageType {
    JPEG, JPG, PNG, BMP, TIFF
};

enum class ImageColorType {
    RGB, HSV, GRAYSCALE
};

typedef struct Timer{
    clock_t start;
    clock_t end;
    double duration;
} Timer;

typedef struct Region{
    int x_begin;
    int x_end;
    int y_begin;
    int y_end;
} Region;

typedef struct Mask_t{
    vector<vector<char>> mask_;

    // Construtor padrão (3x3 de zeros)
    Mask_t() : mask_(3, vector<char>(3, 0)) {}

    // Construtor com parametros 
    Mask_t(const vector<vector<char>>& values) : mask_(values) {}
} Mask_t;
    
class Image {
private:
    Mat image;
    ImageType type;
    ImageColorType color_type;
    string path;
    vector<uchar> buffer;
    int width;
    int height;
    Mat image_singleThread;
    Mat image_multiThread;
    int intensity = 1;
    unique_ptr<ThreadPool> thread_pool; // 11 threads maximas por padrão

    int threads_done = 0;
    bool single_thread_ended = false;
    bool multi_thread_ended = false;
    Timer timer_singleThread;
    Timer timer_multiThread;

public: 
    // Constructor
    Image();
    Image(const string& path, ImageColorType color_type, ImageType type);
    Image(const vector<uchar>& buffer, ImageColorType color_type, ImageType type);
    void overwriteImage(const string& path, ImageColorType color_type, ImageType type);
    void overwriteImage(const vector<uchar>& buffer, ImageColorType color_type, ImageType type);

    // Functions
    void show();
    void save();
    void save_singlethread();
    void save_multithread();

    // Filters
    void negative_filter(Region region, Mat& image_output);
    void blur_filter(Region region, Mat& image_output, int intensity);
    void sharpen_filter(Region region, Mat& image_output, int intensity);
    void median_filter(Region region, Mat& image_output, int intensity);
    void grayscale_filter(Region region, Mat& image_output, int intensity);
    void apply_mask_filter(Region region, Mat& image_output, Mask_t mask_Mat, int intensity, int weight);
    void gaussian_filter(Region region, Mat& image_output, int intensity);
    void laplacian90_sharpen_filter(Region region, Mat& image_output, int intensity);
    void laplacian90_border_detection_filter(Region region, Mat& image_output, int intensity);
    void laplacian45_sharpen_filter(Region region, Mat& image_output, int intensity);
    void laplacian45_border_detection_filter(Region region, Mat& image_output, int intensity);
    
    // Threads
    void thread_process(const string& filter, int threads, Region region);
    void single_thread_process(const string& filter, Region region);
    void process (const string& filter, int intensity, int threads);

    // Getters
    string get_image_type();
    vector<uchar> get_single_thread_image();
    vector<uchar> get_multi_thread_image();
    bool get_single_thread_done();
    bool get_multi_thread_done();
    double get_single_thread_duration(bool);
    double get_multi_thread_duration(bool);
};

// DA CLASSE //////////////////////////////////
Image::
    Image() : thread_pool(make_unique<ThreadPool>(11)) { 
        this->path = "none";
        this->color_type = ImageColorType::RGB;
        this->type = ImageType::JPEG;
        this->width = 0;
        this->height = 0;

    }


Image::
    Image(const string& path, ImageColorType color_type, ImageType type): thread_pool(make_unique<ThreadPool>(11)) {
        overwriteImage(path, color_type, type);
    }
void Image::
    overwriteImage(const string& path, ImageColorType color_type, ImageType type) {
        
        this->path = path;
        this->color_type = color_type;
        this->type = type;
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
    Image(const vector<uchar>& buffer, ImageColorType color_type, ImageType type): thread_pool(make_unique<ThreadPool>(11)) { 
        overwriteImage(buffer, color_type, type);
    }
void Image::
    overwriteImage(const vector<uchar>& buffer, ImageColorType color_type, ImageType type) {

        this->buffer = buffer;
        this->color_type = color_type;
        this->type = type;

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
                imName += ".bmp";
                break;
            case ImageType::TIFF: 
                imName += ".tiff";
            default:
                break;
        }

        imwrite(imName, this->image);
        return;
    }

void Image::
    save_singlethread(){
        // Coleta o vector de pixels da imagem e transforma em imagem
        // Mat image_singleThread(this->height, this->width, CV_8UC1, this->image_singleThread.data());

        string imgName =  "output/image_singleThread";
        switch (this->type){
            case ImageType::JPEG: 
                imgName += ".jpeg";
                break;
            case ImageType::JPG: 
                imgName += ".jpg";
                break;
            case ImageType::PNG: 
                imgName += ".png";
                break;
            case ImageType::BMP: 
                imgName += ".bmp";
                break;
            case ImageType::TIFF: 
                imgName += ".tiff";
                break;
            default:
                break;
        }

        // se a imagem estiver em HSV, converte para BGR antes de salvar
        if (this->color_type == ImageColorType::HSV) {
            Mat temp;
            cvtColor(this->image_singleThread, temp, COLOR_HSV2BGR);
            imwrite(imgName, temp);
        }else // caso a imagem seja em tons de cinza ou RGB
            imwrite(imgName, this->image_singleThread);

        return;
    }

void Image::
    save_multithread(){

        string imgName = "output/image_multiThread";
        switch (this->type) {
            case ImageType::JPEG: imgName += ".jpeg"; break;
            case ImageType::JPG: imgName += ".jpg"; break;
            case ImageType::PNG: imgName += ".png"; break;
            case ImageType::BMP: imgName += ".bmp"; break;
            case ImageType::TIFF: imgName += ".tiff"; break;
            default: break;
        }

        // se a imagem estiver em HSV, converte para BGR antes de salvar
        if (this->color_type == ImageColorType::HSV) {
            Mat temp;
            cvtColor(this->image_multiThread, temp, COLOR_HSV2BGR);
            imwrite(imgName, temp);

        } else // caso a imagem seja em tons de cinza ou RGB
            imwrite(imgName, this->image_multiThread);

        return;
    }

typedef struct Interval{
    float begin;
    float end;
} Interval;

inline float normalizeInInterval(int value, Interval& interval){
    
    Interval intensity_interval = {1, 20};
    
    // Primeiro, normaliza o valor para o intervalo [0, 1]
    float value_in_0_1_interval = (float) (value - intensity_interval.begin) / (intensity_interval.end - intensity_interval.begin);

    // Depois escala para o novo intervalo interval
    // E, por fim, desloca para começar a partir de interval.begin
    float new_value = value_in_0_1_interval * (interval.end - interval.begin) + interval.begin;
    
    // Verifica se o novo valor está fora do intervalo
    if (new_value < interval.begin) return interval.begin;
    if (new_value > interval.end) return interval.end;

    // Se o valor estiver dentro do intervalo, retorna o novo valor
    return new_value;
}

// FILTROS //////////////////////////////////
void Image::
    negative_filter(Region region, Mat& image_output) {
        // Lê de image e coloca o resultado em image_out

        // Se a imagem for BGR, aplica o filtro em cada canal
        if (this->color_type == ImageColorType::RGB){
            for (int i = region.x_begin; i <= region.x_end; i++){
                for (int j = region.y_begin; j <= region.y_end; j++){
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    // Aplica o filtro negativo em cada canal
                    pixel[0] = 255 - this->image.at<Vec3b>(j, i)[0]; // B
                    pixel[1] = 255 - this->image.at<Vec3b>(j, i)[1]; // G
                    pixel[2] = 255 - this->image.at<Vec3b>(j, i)[2]; // R
                }
            }
        } else // se a imagem for HSV, aplica o filtro no canal de Valor
        if(this->color_type == ImageColorType::HSV){
            for (int i = region.x_begin; i <= region.x_end; i++){
                for (int j = region.y_begin; j <= region.y_end; j++){
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    // Aplica o filtro negativo em cada canal
                    pixel[0] = (this->image.at<Vec3b>(j, i)[0] + 90) % 180 ; // H (girar "180 graus")
                    pixel[1] = 255 - this->image.at<Vec3b>(j, i)[1]; // S
                    pixel[2] = 255 - this->image.at<Vec3b>(j, i)[2]; // V
                }
            }  
        }else { // imagem em tons de cinza
            for (int i = region.x_begin; i <= region.x_end; i++){
                for (int j = region.y_begin; j <= region.y_end; j++){
                    // aplica o filtro negativo no canal único
                    image_output.at<uchar>(j, i) = 255 - this->image.at<uchar>(j, i);
                }
            }
        }

    }

void Image::
    blur_filter(Region region, Mat& image_output, int intensity){

        // se a imagem for BGR, aplica o filtro em cada canal
        if (this->color_type == ImageColorType::RGB) {

            // Para cada pixel da imagem, aplica o filtro
            for (int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    int meanValue[] = {0, 0, 0};

                    // Para cada pixel vizinho, soma os valores dos pixels vizinhos em cada canal
                    for (int k = -intensity; k <= intensity; k++) {
                        for (int l = -intensity; l <= intensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }

                            // soma os valores dos pixels vizinhos em cada canal, usando ptr<> para acessar os pixels rapidamente
                            const Vec3b* input_row = this->image.ptr<Vec3b>(y);
                            meanValue[0] += input_row[x][0]; // B
                            meanValue[1] += input_row[x][1]; // G
                            meanValue[2] += input_row[x][2]; // R
                        }
                    }

                    Vec3b* output_row = image_output.ptr<Vec3b>(j);

                    // Define o pixel da imagem de saída como a média dos pixels vizinhos
                    output_row[i][0] = round(meanValue[0] / ((intensity * 2 + 1) * (intensity * 2 + 1))); // B
                    output_row[i][1] = round(meanValue[1] / ((intensity * 2 + 1) * (intensity * 2 + 1))); // G
                    output_row[i][2] = round(meanValue[2] / ((intensity * 2 + 1) * (intensity * 2 + 1))); // R
                }
            }

        } else  // se a imagem for HSV, aplica o filtro no canal de Valor
        if (this->color_type == ImageColorType::HSV) {

            // Para cada pixel da imagem, aplica o filtro
            for (int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    int meanValue = 0;

                    // Para cada pixel vizinho, soma os valores dos pixels vizinhos no canal de Valor
                    for (int k = -intensity; k <= intensity; k++) {
                        for (int l = -intensity; l <= intensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }

                            const Vec3b* input_row = this->image.ptr<Vec3b>(y);
                            meanValue += input_row[x][2]; // V
                        }
                    }

                    Vec3b* output_row = image_output.ptr<Vec3b>(j);

                    // define o pixel da imagem de saída como a média dos pixels vizinhos
                    output_row[i][2] = round(meanValue / ((intensity * 2 + 1) * (intensity * 2 + 1))); // V
                }
            }

        } else { // se a imagem for em tons de cinza, aplica no canal único

            // Para cada pixel da imagem, aplica o filtro
            for (int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    int meanValue = 0;

                    // Para cada pixel vizinho, soma os valores dos pixels vizinhos no canal único
                    for (int k = -intensity; k <= intensity; k++) {
                        for (int l = -intensity; l <= intensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }

                            const uchar* input_row = this->image.ptr<uchar>(y);
                            meanValue += input_row[x];
                        }
                    }

                    uchar* output_row = image_output.ptr<uchar>(j);

                    // define o pixel da imagem de saída como a média dos pixels vizinhos
                    output_row[i] = round(meanValue / ((intensity * 2 + 1) * (intensity * 2 + 1)));
                }
            }
        }
    }


void Image::
    sharpen_filter(Region region, Mat& image_output, int intensity){
        Interval interval = {1, 3};
        float k = normalizeInInterval(intensity, interval); // Normaliza a intensidade para o intervalo [1, 3]
    
        int meanFilterIntensity = 5; // Intensidade do filtro de média
    
        // Se a imagem for BGR, aplica o filtro em cada canal
        if (this->color_type == ImageColorType::RGB) {
    
            // Para cada pixel da imagem, aplica o filtro
            for (int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    int meanValue[] = {0, 0, 0};
    
                    // Para cada pixel vizinho, soma os valores dos pixels vizinhos em cada canal
                    for (int k = -meanFilterIntensity; k <= meanFilterIntensity; k++) {
                        for (int l = -meanFilterIntensity; l <= meanFilterIntensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;
    
                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
    
                            const Vec3b* input_row = this->image.ptr<Vec3b>(y); // Usando ptr<> para acessar a linha de pixels
                            meanValue[0] += input_row[x][0]; // B
                            meanValue[1] += input_row[x][1]; // G
                            meanValue[2] += input_row[x][2]; // R
                        }
                    }
    
                    // Calcula a mascara de nitidez no pixel atual para cada canal
                    float gMask[] = {0, 0, 0};
                    gMask[0] = (this->image.ptr<Vec3b>(j)[i][0]) - (meanValue[0] / ((meanFilterIntensity * 2 + 1) * (meanFilterIntensity * 2 + 1))); // B
                    gMask[1] = (this->image.ptr<Vec3b>(j)[i][1]) - (meanValue[1] / ((meanFilterIntensity * 2 + 1) * (meanFilterIntensity * 2 + 1))); // G
                    gMask[2] = (this->image.ptr<Vec3b>(j)[i][2]) - (meanValue[2] / ((meanFilterIntensity * 2 + 1) * (meanFilterIntensity * 2 + 1))); // R
    
                    Vec3b* output_row = image_output.ptr<Vec3b>(j);
    
                    // Aplica o filtro de nitidez com intensidade k em cada canal, evitando overflow com a função clamp
                    output_row[i][0] = clamp( (double) round(this->image.ptr<Vec3b>(j)[i][0] + k * gMask[0]), 0.0, 255.0); // B
                    output_row[i][1] = clamp( (double) round(this->image.ptr<Vec3b>(j)[i][1] + k * gMask[1]), 0.0, 255.0); // G
                    output_row[i][2] = clamp( (double) round(this->image.ptr<Vec3b>(j)[i][2] + k * gMask[2]), 0.0, 255.0); // R
                }
            }
    
        } else // Se a imagem for HSV, aplica o filtro no canal de Valor
        if (this->color_type == ImageColorType::HSV) {
    
            // Para cada pixel da imagem, aplica o filtro
            for (int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    int meanValue = 0;
    
                    // Para cada pixel vizinho, soma os valores dos pixels vizinhos no canal de Valor
                    for (int k = -meanFilterIntensity; k <= meanFilterIntensity; k++) {
                        for (int l = -meanFilterIntensity; l <= meanFilterIntensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;
    
                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
    
                            const Vec3b* input_row = this->image.ptr<Vec3b>(y); // Usando ptr<> para acessar a linha de pixels
                            meanValue += input_row[x][2]; // V
                        }
                    }
    
                    // Calcula a mascara de nitidez no pixel atual para o canal de Valor
                    float gMask = 0;
                    gMask = (this->image.ptr<Vec3b>(j)[i][2]) - (meanValue / ((meanFilterIntensity * 2 + 1) * (meanFilterIntensity * 2 + 1))); // V
    
                    Vec3b* output_row = image_output.ptr<Vec3b>(j);
    
                    // Aplica o filtro de nitidez com intensidade k no canal de Valor, evitando overflow com a função clamp
                    output_row[i][2] = clamp((double) round(this->image.ptr<Vec3b>(j)[i][2] + k * gMask), 0.0, 255.0); // V
                }
            }
            // ------------------- fim do else if HSV
    
        } else { // Se a imagem for em tons de cinza, aplica no canal único
    
            // Para cada pixel da imagem, aplica o filtro
            for (int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    int meanValue = 0;
    
                    // Para cada pixel vizinho, soma os valores dos pixels vizinhos em cada canal
                    for (int k = -meanFilterIntensity; k <= meanFilterIntensity; k++) {
                        for (int l = -meanFilterIntensity; l <= meanFilterIntensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;
    
                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
    
                            const uchar* input_row = this->image.ptr<uchar>(y); // Usando ptr<> para acessar a linha de pixels
                            meanValue += input_row[x]; // Canal único
                        }
                    }
    
                    // Calcula a mascara de nitidez no pixel atual para o canal único
                    float gMask = 0;
                    gMask = (this->image.ptr<uchar>(j)[i]) - (meanValue / ((meanFilterIntensity * 2 + 1) * (meanFilterIntensity * 2 + 1))); 
    
                    uchar* output_row = image_output.ptr<uchar>(j);
    
                    // Aplica o filtro de nitidez com intensidade k no canal único, evitando overflow com a função clamp
                    output_row[i] = clamp((double) round(this->image.ptr<uchar>(j)[i] + k * gMask), 0.0, 255.0);
                }
            }
            // ------------------- fim do else GRAYSCALE
        }
    }    


void Image::
    median_filter(Region region, Mat& image_output, int intensity) {
        // Se a imagem for BGR, aplica o filtro em cada canal
        // Se a imagem for HSV, aplica o filtro no canal de Valor

        if(this->color_type == ImageColorType::RGB){
            // Caso em que é pra imagem resultado tambem ser RGB
            // Aplica o filtro de mediana em cada canal

            for (int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    vector<uchar> pixels[3]; // Vetor para armazenar os pixels vizinhos de cada canal
    
                    // Para cada pixel vizinho, pega os valores dos pixels vizinhos em cada canal
                    for (int k = -intensity; k <= intensity; k++) {
                        for (int l = -intensity; l <= intensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;
    
                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
    
                            const Vec3b* input_row = this->image.ptr<Vec3b>(y); // Usando ptr<> para acessar a linha de pixels
                            pixels[0].push_back(input_row[x][0]); 
                            pixels[1].push_back(input_row[x][1]); // G
                            pixels[2].push_back(input_row[x][2]); // R
                        }
                    }
    
                    Vec3b* output_row = image_output.ptr<Vec3b>(j);
    
                    // Ordena os valores dos pixels vizinhos e pega a mediana para cada canal
                    for (int c = 0; c < 3; c++) {
                        sort(pixels[c].begin(), pixels[c].end());
                        output_row[i][c] = pixels[c][pixels[c].size() / 2];
                    }
                }
            }

        }else if(this->color_type == ImageColorType::HSV){
            // Caso em que é pra imagem resultado ser HSV
            // Aplica o filtro de mediana no canal de Valor

            for(int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    vector<uchar> pixels;
    
                    // Para cada pixel vizinho, pega os valores dos pixels vizinhos no canal de Valor
                    for (int k = -intensity; k <= intensity; k++) {
                        for (int l = -intensity; l <= intensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;
    
                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
    
                            const Vec3b* input_row = this->image.ptr<Vec3b>(y); // Usando ptr<> para acessar a linha de pixels
                            pixels.push_back(input_row[x][2]); // V
                        }
                    }
    
                    // Ordena os valores dos pixels vizinhos e pega a mediana
                    sort(pixels.begin(), pixels.end());
                    uchar medianValue = pixels[pixels.size() / 2];
    
                    Vec3b* output_row = image_output.ptr<Vec3b>(j);
    
                    // Define o pixel da imagem de saída como a mediana dos pixels vizinhos
                    output_row[i][2] = medianValue;
                }
            }

        } else {
            // Caso em que é pra imagem resultado ser em tons de cinza
            // Para isso, obtem a mediana a partir das medias dos pixels vizinhos
            // Para cada pixel da imagem, aplica o filtro

            for (int i = region.x_begin; i <= region.x_end; i++) {
                for (int j = region.y_begin; j <= region.y_end; j++) {
                    vector<uchar> pixels;
    
                    // Para cada pixel vizinho, pega os valores dos pixels vizinhos no canal único
                    for (int k = -intensity; k <= intensity; k++) {
                        for (int l = -intensity; l <= intensity; l++) {
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;
    
                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
    
                            const uchar* input_row = this->image.ptr<uchar>(y); // Usando ptr<> para acessar a linha de pixels
                            pixels.push_back(input_row[x]); // Canal único
                        }
                    }
    
                    // Ordena os valores dos pixels vizinhos e pega a mediana
                    sort(pixels.begin(), pixels.end());
                    uchar medianValue = pixels[pixels.size() / 2];
    
                    uchar* output_row = image_output.ptr<uchar>(j);
    
                    // Define o pixel da imagem de saída como a mediana dos pixels vizinhos
                    output_row[i] = medianValue;
                }
            }

        }
    }

void Image::
    grayscale_filter(Region region, Mat& image_output, int intensity) {
        // Se a imagem for BGR, aplica o filtro em cada canal
        // Se a imagem for HSV, aplica o filtro no canal de Valor

        if(this->color_type == ImageColorType::RGB){
            // Se a imagem for RGB, a intensidade é dada pelos três canais
            // O canal azul tem peso 0.114, o verde 0.587 e o vermelho 0.299

            for(int i = region.x_begin; i <= region.x_end; i++){
                for (int j = region.y_begin; j <= region.y_end; j++){
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    // Aplica o filtro de grayscale em cada canal
                    pixel[0] = round(0.114 * this->image.at<Vec3b>(j, i)[0] + 0.587 * this->image.at<Vec3b>(j, i)[1] + 0.299 * this->image.at<Vec3b>(j, i)[2]); // B
                    pixel[1] = pixel[0]; // G
                    pixel[2] = pixel[0]; // R
                }
            }



        }else if(this->color_type == ImageColorType::HSV){
            // Caso em que é pra imagem resultado ser HSV
            // Define a saturacao como 0, pois remove qualquer perceptibilidade de cor
            // O canal H nao precisa ser alterado, pois o valor de H eh irrelevante para a imagem em tons de cinza
            // O canal V mantem-se o mesmo, pois a intensidade eh dada pelo canal de Valor

            for(int i = region.x_begin; i <= region.x_end; i++){
                for (int j = region.y_begin; j <= region.y_end; j++){
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    // Aplica o filtro de grayscale em cada canal
                    pixel[0] = this->image.at<Vec3b>(j, i)[0]; // H
                    pixel[1] = 0; // S
                    pixel[2] = this->image.at<Vec3b>(j, i)[2]; // V
                }
            }
        }

    }


void Image::
    apply_mask_filter(Region region, Mat& image_output, Mask_t mask_Mat, int intensity, int weight) {
        Interval interval = {1.0, 2.0};
        float k = normalizeInInterval(intensity, interval); // Normaliza a intensidade para o intervalo [0.5, 2]

        // Se a imagem for BGR, aplica a mascara em cada canal
        if(this->color_type == ImageColorType::RGB){

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue[] = {0, 0, 0};
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Aplica a mascara em cada canal
                            meanValue[0] += pixel[0] * mask_Mat.mask_[l+1][k+1]; // B
                            meanValue[1] += pixel[1] * mask_Mat.mask_[l+1][k+1]; // G
                            meanValue[2] += pixel[2] * mask_Mat.mask_[l+1][k+1]; // R
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    // Aplica a mascara em cada canal
                    pixel[0] = clamp((double) round(k * meanValue[0]/weight), 0.0, 255.0); // B
                    pixel[1] = clamp((double) round(k * meanValue[1]/weight), 0.0, 255.0); // G
                    pixel[2] = clamp((double) round(k * meanValue[2]/weight), 0.0, 255.0); // R
                }
            }
                
        }else // se a imagem for HSV, aplica a mascara no canal de Valor
        if(this->color_type == ImageColorType::HSV){
            
            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue = 0;
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Aplica a mascara no canal de Valor
                            meanValue += pixel[2] * mask_Mat.mask_[l+1][k+1]; // V
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    // Aplica a mascara no canal de Valor
                    pixel[2] = clamp((double) round(k * meanValue/weight), 0.0, 255.0); // V
                }
            }

        }else{ // imagem em tons de conza

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue = 0;
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            uchar& pixel = this->image.at<uchar>(y, x);

                            // Aplica a mascara no canal de Valor
                            meanValue += pixel * mask_Mat.mask_[l+1][k+1];
                        }

                    }
                    // Pega o pixel da imagem de saída
                    uchar& pixel = image_output.at<uchar>(j, i);
                    // Aplica a mascara no canal de Valor
                    pixel = clamp((double) round(k * meanValue/weight), 0.0, 255.0); // R
                }
            }
        }
    }   

void Image::
    gaussian_filter(Region region, Mat& image_output, int intensity){
        Mask_t gaussian_mask = Mask_t({ {1, 2, 1}, 
                                        {2, 4, 2}, 
                                        {1, 2, 1} }); // mascara gaussiana
        apply_mask_filter(region, image_output, gaussian_mask, intensity, 16); // peso dos valores da mascara = 16
    }

void Image::
    laplacian90_border_detection_filter(Region region, Mat& image_output, int intensity){
        Mask_t laplacian90_mask = Mask_t({  {0,  1, 0}, 
                                            {1, -4, 1}, 
                                            {0,  1, 0} }); // mascara de laplaciano sem as diagonais

        apply_mask_filter(region, image_output, laplacian90_mask, intensity, 1); // peso dos valores da mascara = 1
    }
void Image::
    laplacian45_border_detection_filter(Region region, Mat& image_output, int intensity){
        Mask_t laplacian45_mask = Mask_t({  {1,  1, 1}, 
                                            {1, -8, 1}, 
                                            {1,  1, 1} }); // mascara de laplaciano com as diagonais

        apply_mask_filter(region, image_output, laplacian45_mask, intensity, 1); // peso dos valores da mascara = 1
    }


void Image::
    laplacian90_sharpen_filter(Region region, Mat& image_output, int intensity){
        Mask_t laplacian90_mask = Mask_t({  {0,  1, 0}, 
                                            {1, -4, 1}, 
                                            {0,  1, 0} }); // mascara de laplaciano sem as diagonais

        Interval interval = {0.5, 3.5};
        float k = normalizeInInterval(intensity, interval); // Normaliza a intensidade para o intervalo [0.5, 2]

        // Se a imagem for BGR, aplica a mascara em cada canal
        if(this->color_type == ImageColorType::RGB){

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue[] = {0, 0, 0};
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Aplica a mascara em cada canal
                            meanValue[0] += pixel[0] * laplacian90_mask.mask_[l+1][k+1]; // B
                            meanValue[1] += pixel[1] * laplacian90_mask.mask_[l+1][k+1]; // G
                            meanValue[2] += pixel[2] * laplacian90_mask.mask_[l+1][k+1]; // R
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    Vec3b& input_pixel = this->image.at<Vec3b>(j, i); 
                    // Aplica a mascara em cada canal
                    pixel[0] = clamp((double) round(input_pixel[0] - k * meanValue[0]), 0.0, 255.0); // B
                    pixel[1] = clamp((double) round(input_pixel[1] - k * meanValue[1]), 0.0, 255.0); // G
                    pixel[2] = clamp((double) round(input_pixel[2] - k * meanValue[2]), 0.0, 255.0); // R
                }
            }
                
        }else // se a imagem for HSV, aplica a mascara no canal de Valor
        if(this->color_type == ImageColorType::HSV){
            
            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue = 0;
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Aplica a mascara no canal de Valor
                            meanValue += pixel[2] * laplacian90_mask.mask_[l+1][k+1]; // V
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    Vec3b& input_pixel = this->image.at<Vec3b>(j, i);
                    // Aplica a mascara no canal de Valor
                    pixel[2] = clamp((double) round(input_pixel[2] - k * meanValue), 0.0, 255.0); // V
                }
            }

        }else{ // imagem em tons de conza

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue = 0;
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            uchar& pixel = this->image.at<uchar>(y, x);
                            
                            // Aplica a mascara no canal de Valor
                            meanValue += pixel * laplacian90_mask.mask_[l+1][k+1];
                        }
                        
                    }
                    // Pega o pixel da imagem de saída
                    uchar& pixel = image_output.at<uchar>(j, i);
                    uchar& input_pixel = this->image.at<uchar>(j, i);
                    // Aplica a mascara no canal de Valor
                    pixel = clamp((double) round(input_pixel - k * meanValue), 0.0, 255.0); // R
                }
            }
        }
    }   
void Image::
    laplacian45_sharpen_filter(Region region, Mat& image_output, int intensity){
        Mask_t laplacian45_mask = Mask_t({  {1,  1, 1}, 
                                            {1, -8, 1}, 
                                            {1,  1, 1} }); // mascara de laplaciano sem as diagonais

        Interval interval = {0.5, 3.5};
        float k = normalizeInInterval(intensity, interval); // Normaliza a intensidade para o intervalo [0.5, 2]

        // Se a imagem for BGR, aplica a mascara em cada canal
        if(this->color_type == ImageColorType::RGB){

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue[] = {0, 0, 0};
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Aplica a mascara em cada canal
                            meanValue[0] += pixel[0] * laplacian45_mask.mask_[l+1][k+1]; // B
                            meanValue[1] += pixel[1] * laplacian45_mask.mask_[l+1][k+1]; // G
                            meanValue[2] += pixel[2] * laplacian45_mask.mask_[l+1][k+1]; // R
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    Vec3b& input_pixel = this->image.at<Vec3b>(j, i); 
                    // Aplica a mascara em cada canal
                    pixel[0] = clamp((double) round(input_pixel[0] - k * meanValue[0]), 0.0, 255.0); // B
                    pixel[1] = clamp((double) round(input_pixel[1] - k * meanValue[1]), 0.0, 255.0); // G
                    pixel[2] = clamp((double) round(input_pixel[2] - k * meanValue[2]), 0.0, 255.0); // R
                }
            }
                
        }else // se a imagem for HSV, aplica a mascara no canal de Valor
        if(this->color_type == ImageColorType::HSV){
            
            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue = 0;
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Aplica a mascara no canal de Valor
                            meanValue += pixel[2] * laplacian45_mask.mask_[l+1][k+1]; // V
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    Vec3b& input_pixel = this->image.at<Vec3b>(j, i);
                    // Aplica a mascara no canal de Valor
                    pixel[2] = clamp((double) round(input_pixel[2] - k * meanValue), 0.0, 255.0); // V
                }
            }

        }else{ // imagem em tons de conza

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){

                    int meanValue = 0;
                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -1; k <= 1; k++){
                        for(int l = -1; l <= 1; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            uchar& pixel = this->image.at<uchar>(y, x);
                            
                            // Aplica a mascara no canal de Valor
                            meanValue += pixel * laplacian45_mask.mask_[l+1][k+1];
                        }
                        
                    }
                    // Pega o pixel da imagem de saída
                    uchar& pixel = image_output.at<uchar>(j, i);
                    uchar& input_pixel = this->image.at<uchar>(j, i);
                    // Aplica a mascara no canal de Valor
                    pixel = clamp((double) round(input_pixel - k * meanValue), 0.0, 255.0); // R
                }
            }
        }
    }   
    
// ---
    
// THREADS ////////////////////////

// Reparte A em B pedacos de tamanhos aproximadamente iguais
vector<int> 
    getSteps(int A, int B){
        vector<int> result(B, A/B); // inicializa o vetor com o valor base

        int excess = A % B; // resto da divisao

        for (int i = 0; i < excess; i++){
            result[i] += 1; // distribui o resto entre as partes
        }

        return result;
    }
    
vector<Region>
    getRegions(int width, int height, int threads) {
        vector<Region> regions;
        
        if (width > height) {
            vector<int> steps = getSteps(width, threads);
            int last = -1;

            for(int i = 0; i < threads; i++) {
                Region region;
                
                // Se for o último, pega o resto da imagem
                if (i == threads - 1) {
                    region.x_begin = last+1;
                    region.x_end = width-1;
                } else {
                    region.x_begin = last+1;
                    last = region.x_end = last + steps[i];
                }

                region.y_begin = 0;
                region.y_end = height-1;
                regions.push_back(region);
            }

        } else {
            vector<int> steps = getSteps(height, threads);
            int last = -1;

            if (threads > height){
                threads = height;  // garante que o número de threads nao exceda a altura da imagem
            }

            for(int i = 0; i < threads; i++) {
                Region region;
                
                // Se for o último, pega o resto da imagem
                if (i == threads - 1) {
                    region.y_begin = last+1;
                    region.y_end = height-1;
                } else {
                    region.y_begin = last+1;
                    last = region.y_end = last + steps[i];
                }

                region.x_begin = 0;
                region.x_end = width-1;
                regions.push_back(region);
            }
        }

        return regions;
    }

// Essa funcao devera delegar a regiao recebida a uma funcao de filtro, utilizando o atributo filter e intensity da classe Image
void Image::
    thread_process(const string& filter, int threads, Region region) {

        if(filter == "negative")
            this->negative_filter(region, this->image_multiThread);
        else if(filter == "blur")
            this->blur_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "sharpen")
            this->sharpen_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "median")
            this->median_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "grayscale")
            this->grayscale_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "gaussian")
            this->gaussian_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "laplacian90 border")
            this->laplacian90_border_detection_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "laplacian45 border")
            this->laplacian45_border_detection_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "laplacian90 sharpen")
            this->laplacian90_sharpen_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "laplacian45 sharpen")
            this->laplacian45_sharpen_filter(region, this->image_multiThread, this->intensity);
        else
            throw invalid_argument("Filtro inválido!");
        
        this->threads_done++;
        if (this->threads_done == threads) {
            this->timer_multiThread.end = clock();
            this->timer_multiThread.duration = double(this->timer_multiThread.end - this->timer_multiThread.start) / CLOCKS_PER_SEC;
            this->multi_thread_ended = true;
            cout << "Multi-Threads terminaram o processamento! Em " << this->timer_multiThread.duration << " milissegundos"<< endl;
        }
    }

void Image::
    single_thread_process(const string& filter, Region region) {

        if(filter == "negative")
            this->negative_filter(region, this->image_singleThread);
        else if(filter == "blur")
            this->blur_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "sharpen")
            this->sharpen_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "median")
            this->median_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "grayscale")
            this->grayscale_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "gaussian")
            this->gaussian_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "laplacian90 border")
            this->laplacian90_border_detection_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "laplacian45 border")
            this->laplacian45_border_detection_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "laplacian90 sharpen")
            this->laplacian90_sharpen_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "laplacian45 sharpen")
            this->laplacian45_sharpen_filter(region, this->image_singleThread, this->intensity);
        else
            throw invalid_argument("Filtro inválido!");

         // Salva a imagem processada
         // Coleta o vector de pixels da imagem e transforma em imagem

        this->timer_singleThread.end = clock();
        this->timer_singleThread.duration = double(this->timer_singleThread.end - this->timer_singleThread.start) / CLOCKS_PER_SEC;
        this->single_thread_ended = true;
        cout << "Single-Thread terminou o processamento!" << " Em " << this->timer_singleThread.duration << " milissegundos"<< endl;
    }


void Image::
    process(const string& filter, int threads, int intensity = 1) {
        // Deve guardar no objeto os atributos intensity e filter

        this->intensity = intensity;

        // Criação de das imagens de saida
        // opc1
            // copiando a imagem original para a imagem de saida e ir modificando depois
            // this->image_singleThread = this->image.clone();
            // this->image_multiThread =  this->image.clone();
        // opc2
            // declarar a imagem de saída como uma imagem de valor 0
            switch(this->color_type){
                // Se a imagem for colorida, cria uma imagem de 3 canais
                case ImageColorType::RGB:
                case ImageColorType::HSV:
                    this->image_singleThread = Mat(this->height, this->width, CV_8UC3, Scalar(0, 0, 0));
                    this->image_multiThread = Mat(this->height, this->width, CV_8UC3, Scalar(0, 0, 0));
                    break;
                case ImageColorType::GRAYSCALE:
                    this->image_singleThread = Mat(this->height, this->width, CV_8UC1, Scalar(0));
                    this->image_multiThread = Mat(this->height, this->width, CV_8UC1, Scalar(0));
                    break;
                default:
                    break;
            }

        
        // Cada thread deve processar a parte da imagem que lhe foi atribuida ou se for o caso de uma thread, processar a imagem inteira
        
        // 1. SINGLE-THREADING:
        this->single_thread_ended = false;
        this->timer_singleThread.start = clock();
        this->thread_pool->enqueue([this, filter, threads] {
            Region region = {0, this->width-1, 0, this->height-1}; // A imagem inteira
            this->single_thread_process(filter, region);
        });
    
        // 2. MULTI-THREADING:
        // Primeiramente, reparte a imagem em partes quase iguais, de acordo com o número de threads
        this->multi_thread_ended = false;
        cout << endl << "Iniciando processamento em " << threads << " threads..." << endl;
        vector<Region> regions = getRegions(this->width, this->height, threads);

        // Em seguida, reseta o contador de threads e o timer
        this->threads_done = 0;                                                            
        this->timer_multiThread.start = clock();
        
        // Atribui a cada thread uma parte da imagem para processar
        for (int i = 0; i < threads; i++){
            this->thread_pool->enqueue([this, filter, threads,  regions, i] {
                this->thread_process(filter, threads, regions[i]);
            });
        }
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

string Image::
    get_image_type(){
        switch (this->type) {
            case ImageType::JPEG: return "jpeg";
            case ImageType::JPG: return "jpg";
            case ImageType::PNG: return "png";
            case ImageType::BMP: return "bmp";
            case ImageType::TIFF: return "tiff";
            default: throw invalid_argument("Tipo de imagem inválido!");
        }
    }

bool Image::
    get_multi_thread_done(){
        return this->multi_thread_ended;
    }

bool Image::
    get_single_thread_done(){
        return this->single_thread_ended;
    }
    
double Image::
    get_multi_thread_duration(bool done){
        if (done)
            return this->timer_multiThread.duration;
        else
            return (double (clock() - this->timer_multiThread.start) / CLOCKS_PER_SEC);
    }


double Image::
    get_single_thread_duration(bool done){
        if (done)
            return this->timer_singleThread.duration;
        else
            return (double (clock() - this->timer_singleThread.start) / CLOCKS_PER_SEC);
    }


vector<uchar> Image::
    get_single_thread_image(){
        // Cria a imagem Mat a partir do vetor de pixels
        Mat image_output = this->image_singleThread.clone(); // Clona a imagem de saída para evitar modificações na original

        // Se estiver no formato HSV, converte para BGR
        if (this->color_type == ImageColorType::HSV) {
            cvtColor(image_output, image_output, COLOR_HSV2BGR);
        }

        // Cria um vetor de bytes para armazenar a imagem em formato JPEG
        vector<uchar> buf;
        imencode("." + this->get_image_type(), image_output, buf); // Codifica a imagem em JPEG

        return buf;
    }

vector<uchar> Image::
    get_multi_thread_image(){
        Mat image_output = this->image_multiThread.clone(); // Cria a imagem Mat a partir do vetor de pixels

        // Se estiver no formato HSV, converte para BGR
        if (this->color_type == ImageColorType::HSV) {
            cvtColor(image_output, image_output, COLOR_HSV2BGR);
        }

        // Cria um vetor de bytes para armazenar a imagem em formato JPEG
        vector<uchar> buf;
        imencode("." + this->get_image_type(), image_output, buf); // Codifica a imagem em JPEG
        return buf;
    }

#endif