// g++ teste.cpp -o teste `pkg-config --cflags --libs opencv4`

#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
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
    double start;
    double end;
} Timer;

typedef struct Region{
    int x_begin;
    int x_end;
    int y_begin;
    int y_end;
} Region;

class Image {
private:
    Mat image;
    ImageType type;
    ImageColorType color_type;
    string path;
    vector<uchar> buffer;
    int width;
    int height;
    vector<vector<uchar>> image_singlethread;
    vector<vector<uchar>> image_multithread;

    string filter = "none";
    int intensity = 1;
    ThreadPool thread_pool; // 12 threads maximas por padrão

    int threads_done = 0;
    bool single_thread_ended = false;
    bool multi_thread_ended = false;

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
    void negative_filter(Region region, vector<vector<uchar>>& image_output);

    // Threads
    void thread_process(Region region);
    void process (const string& filter, int intensity, int threads);
};

// DA CLASSE //////////////////////////////////
Image::
    Image() : thread_pool(12) { 
        this->path = "none";
        this->color_type = ImageColorType::RGB;
        this->type = ImageType::JPEG;
        this->width = 0;
        this->height = 0;

    }


Image::
    Image(const string& path, ImageColorType color_type, ImageType type): thread_pool(12) {
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
    Image(const vector<uchar>& buffer, ImageColorType color_type, ImageType type): thread_pool(12) { 
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

void Image::
    save_singlethread(){
        // Coleta o vector de pixels da imagem e transforma em imagem
        Mat image_singlethread(this->height, this->width, CV_8UC1, this->image_singlethread.data());
        string imName =  "output/image_singlethread";
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
        imwrite(imName, image_singlethread);
        return;
    }

void Image::
    save_multithread(){
        // Coleta o vector de pixels da imagem e transforma em imagem
        Mat output_mat;
        if (this->color_type == ImageColorType::RGB) {
            output_mat = Mat(this->height, this->width, CV_8UC3, this->image_multithread.data());
        } else {
            output_mat = Mat(this->height, this->width, CV_8UC1, this->image_multithread.data());
        }

        string imName = "output/image_multithread";
        switch (this->type) {
            case ImageType::JPEG: imName += ".jpeg"; break;
            case ImageType::JPG: imName += ".jpg"; break;
            case ImageType::PNG: imName += ".png"; break;
            case ImageType::BMP: imName += ".bmp"; break; // cuidado: .bmp, não .bpm
            case ImageType::TIFF: imName += ".tiff"; break;
            default: break;
        }

        imwrite(imName, output_mat);
        return;
    }

// FILTROS //////////////////////////////////
void Image::
    negative_filter(Region region, vector<vector<uchar>>& image_output) {
        // Lê de image e coloca o resultado em image_out

        for (int i = region.x_begin; i <= region.x_end; i++){
            for (int j = region.y_begin; j <= region.y_end; j++){
                // Se a imagem for colorida, aplica o filtro em cada canal
                if (this->color_type == ImageColorType::RGB){
                    image_output[j][i * 3 + 0] = 255 - this->image.at<Vec3b>(j, i)[0]; // B
                    image_output[j][i * 3 + 1] = 255 - this->image.at<Vec3b>(j, i)[1]; // G
                    image_output[j][i * 3 + 2] = 255 - this->image.at<Vec3b>(j, i)[2]; // R
                } else {
                    image_output[j][i] = 255 - this->image.at<uchar>(j, i);
                }
            }
        }

    }

// FILTROS COM THREADS ////////////////////////

// ---
    
// THREADS ////////////////////////

// Reparte A em B pedacos de tamanhos aproximadamente iguais
vector<int> getSteps(int A, int B){
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
    thread_process(Region region) {
        this->negative_filter(region, this->image_multithread); // Chama o filtro negativo, mas pode ser qualquer outro filtro
        this->threads_done++;
    }


void Image::
    process(const string& filter, int threads, int intensity = 1) {
        // Deve guardar no objeto os atributos intensity e filter

        this->intensity = intensity;
        this->filter = filter;

        // Deve atribuir 0 a cada pixel das imagens de saída, ou seja, limpar elas;
        this->image_singlethread.assign(this->height, vector<uchar>(this->width, 0));
        this->image_multithread.assign(this->height, vector<uchar>(this->width, 0));

        
        // Cada thread deve processar a parte da imagem que lhe foi atribuida ou se for o caso de uma thread, processar a imagem inteira
        if(threads == 1){
            // Processa a imagem inteira
            this->single_thread_ended = false;
            this->thread_process({0, this->width-1, 0, this->height-1});
            this->single_thread_ended = true;
        } else {
            this->multi_thread_ended = false;
            cout << "Iniciando processamento em " << threads << " threads..." << endl;
            vector<Region> regions = getRegions(this->width, this->height, threads);
            for (int i = 0; i < threads; i++){
                this->thread_pool.enqueue([this, regions, i] {
                    this->thread_process(regions[i]);
                });
            }

            cout << "Threads Processando imagem..." << endl;
            while(this->threads_done < threads){
                this_thread::sleep_for(chrono::milliseconds(100)); // Espera 100ms
            }
            this->multi_thread_ended = true;
            this->save_multithread(); // Salva a imagem processada
            cout << "Processamento de imagem com threads finalizado!" << endl;
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
#endif