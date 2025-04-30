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
#include <chrono>
#include "ThreadPool.hpp"

using namespace std;
using namespace std::chrono;
using namespace cv;

enum class ImageType {
    JPEG, JPG, PNG, BMP, TIFF
};

enum class ImageColorType {
    RGB, HSV, GRAYSCALE
};

typedef struct Timer{
    time_point<high_resolution_clock> start, end;
    duration<double, milli> timer_duration;
} Timer;

typedef struct Region{
    int x_begin;
    int x_end;
    int y_begin;
    int y_end;
} Region;

typedef struct Mask_t{
    vector<vector<float>> mask_;

    // Construtor padrão (3x3 de zeros)
    Mask_t() : mask_(3, vector<float>(3, 0)) {}

    // Construtor com parametros 
    Mask_t(const vector<vector<float>>& values) : mask_(values) {}
} Mask_t;
    
class Image {
private:
    // Guarda a matriz de imagem recebida como input
    Mat image;

    // Guarda o tipo de imagem (JPEG, PNG, etc.)
    ImageType type;

    // Guarda o tipo de cor da imagem (RGB, HSV, GRAYSCALE)
    ImageColorType color_type;

    // Guarda o caminho da imagem recebida como input
    string path;

    // Guarda o buffer da imagem recebida como input
    vector<uchar> buffer;

    // Guarda o tamanho da imagem recebida como input (largura e altura)
    int width;
    int height;

    // Guarda a matriz de imagem processada em um único thread
    Mat image_singleThread;

    // Guarda a matriz de imagem processada em múltiplos threads
    Mat image_multiThread;

    // Guarda a intensidade do filtro aplicado
    int intensity = 1;

    // Guarda a pool de threads utilizadas para o processamento da imagem
    unique_ptr<ThreadPool> thread_pool;

    // Guarda quantas threads do multithreading já terminaram sua parte do processamento
    int threads_done = 0;

    // Booleanos para indicar se o processamento em single e multi-threading já foi concluído
    bool single_thread_ended = false;
    bool multi_thread_ended = false;

    // Guarda o tempo de execução do processamento em single e multi-threading
    Timer timer_singleThread;
    Timer timer_multiThread;

public: 
    // Métodos utilizados para o construtor, que inicializa o objeto Image
    Image();
    Image(const string& path, ImageColorType color_type, ImageType type);
    Image(const vector<uchar>& buffer, ImageColorType color_type, ImageType type);
    // Métodos utilizados para sobrescrever a imagem recebida como input
    void overwriteImage(const string& path, ImageColorType color_type, ImageType type);
    void overwriteImage(const vector<uchar>& buffer, ImageColorType color_type, ImageType type);

    // Funções para mostrar e salvar a imagem em um arquivo
    void show();
    void save();
    void save_singlethread();
    void save_multithread();

    // Funções para aplicar filtros na imagem

    /*
        * Torna cada pixel da imagem em seu negativo, ou seja, inverte as cores.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @returns: void
    */
    void negative_filter(Region region, Mat& image_output);

    /*
        * Para cada pixel da imagem, aplica o filtro de limiarização usando a intensidade como limiar para preto e branco.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Limite para a transformação em preto ou branco
        * @returns: void
    */
    void thresholding_filter(Region region, Mat& image_output, int intensity);

    /*
        * Aplica um filtro de desfoque (blur) na imagem.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Intensidade do filtro
        * @returns: void
    */
    void blur_filter(Region region, Mat& image_output, int intensity);

    /*
        * Aplica um filtro de nitidez (sharpen) na imagem.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Intensidade do filtro
        * @returns: void
    */
    void sharpen_filter(Region region, Mat& image_output, int intensity);

    /*
        * Aplica um filtro de mediana (median) na imagem, utilizado para reduzir ruídos.
        * Esse filtro demanda mais processamento conforme `intensity` aumenta, pois precisará analisar mais pixels adjacentes e ordená-los.
        * 
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Intensidade do filtro
        * @returns: void
    */
    void median_filter(Region region, Mat& image_output, int intensity);

    /*
        * Aplica um filtro de escala de cinza (grayscale) na imagem, transformando a imagem colorida em uma imagem em tons de cinza.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @returns: void
    */
    void grayscale_filter(Region region, Mat& image_output);

    /*
        * Aplica um filtro de máscara (mask) na imagem, utilizando uma máscara personalizada.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param mask_Mat Máscara a ser aplicada na imagem
        * @param intensity Intensidade da máscara
        * @param weight Somatório da máscara
        * @returns: void
    */
    void apply_mask_filter(Region region, Mat& image_output, Mask_t mask_Mat, int intensity, int weight);

    /*
        * Aplica um filtro gaussiano (gaussian) na imagem, utilizado para suavizar a imagem.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Tamanho do kernel do filtro gaussiano (Dimensão M da matriz MxM gaussiana)
        * @returns: void
    */
    void gaussian_filter(Region region, Mat& image_output, int intensity);

    /*
        * Aplica um filtro de laplaciano, somando o resultado da mascara com a imagem original, utilizando a intensidade passada como peso.
        * Nesse caso, a máscara é a matriz de laplaciano 3x3. Não usa o valor das diagonais, por isso 90°.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Intensidade do filtro
        * @returns: void
    */
    void laplacian90_sharpen_filter(Region region, Mat& image_output, int intensity);

    /*
        * Aplica um filtro de laplaciano e detecta as bordas da imagem.
        * Nesse caso, a máscara é a matriz de laplaciano 3x3. Não usa o valor das diagonais, por isso 90°.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Intensidade do filtro
        * @returns: void
    */
    void laplacian90_border_detection_filter(Region region, Mat& image_output, int intensity);

    /*
        * Aplica um filtro de laplaciano, somando o resultado da mascara com a imagem original, utilizando a intensidade passada como peso.
        * Nesse caso, a máscara é a matriz de laplaciano 3x3. Usa o valor das diagonais, por isso 45°.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Intensidade do filtro
        * @returns: void
    */
    void laplacian45_sharpen_filter(Region region, Mat& image_output, int intensity);

    /*
        * Aplica um filtro de laplaciano e detecta as bordas da imagem.
        * Nesse caso, a máscara é a matriz de laplaciano 3x3. Usa o valor das diagonais, por isso 45°.
        * @param region Região da imagem a ser processada
        * @param image_output Matriz de imagem que recebe o resultado do filtro
        * @param intensity Intensidade do filtro
        * @returns: void
    */
    void laplacian45_border_detection_filter(Region region, Mat& image_output, int intensity);
    
    // ===========================================================
    // Funções para processar a imagem em single e multi-threading

    /*
        * Processamento da imagem em uma das threads do pool.
        * Essa função é chamada para cada thread do pool e aplica o filtro na região da imagem correspondente à thread.
        * Quando a thread termina, ela incrementa o contador de threads concluídas e verifica se todas as threads terminaram.
        * Se todas as threads terminaram, o booleano `multi_thread_ended` é definido como verdadeiro.
        * @param filter Filtro a ser aplicado na imagem
        * @param threads Número de threads disponíveis para o processamento
        * @param region Região da imagem a ser processada
        * @returns: void
    */
    void thread_process(const string& filter, int threads, Region region);

    /*
        * Processamento da imagem em uma única thread.
        * Essa função aplica o filtro na imagem inteira e armazena o resultado na matriz `image_singleThread`.
        * O tempo de execução é medido e armazenado na variável `timer_singleThread`.
        * Quando o processamento termina, o booleano `single_thread_ended` é definido como verdadeiro.
        * @param filter Filtro a ser aplicado na imagem
        * @param region Região da imagem a ser processada
        * @returns: void
    */
    void single_thread_process(const string& filter, Region region);

    /*
        * Esta função é chamada para processar a imagem em múltiplas threads e em uma única thread.
        * Ela divide a imagem em regiões e chama a função `thread_process` para cada região.
        * Em paralelo, ela chama a função `single_thread_process` para processar a imagem inteira em uma única thread.
        * @param filter Filtro a ser aplicado na imagem
        * @param intensity Intensidade do filtro (1-10)
        * @param threads número de threads escolhido para o processamento
        * @returns: void
    */
    void process (const string& filter, int intensity, int threads);

    // Funções para retornar informações sobre a imagem processada

    /*
        * Retorna o tipo de imagem (JPEG, PNG, etc.) como string.
        * @returns: string com o tipo de imagem
    */
    string get_image_type();

    /*
        * Retorna a matriz de imagem processada em uma única thread como um vetor de bytes.
        * @returns: vetor de bytes com a imagem processada
    */
    vector<uchar> get_single_thread_image();

    /*
        * Retorna a matriz de imagem processada em múltiplas threads como um vetor de bytes.
        * @returns: vetor de bytes com a imagem processada
    */
    vector<uchar> get_multi_thread_image();

    /*
        * Retorna se o processamento em uma única thread foi concluído.
        * @returns: booleano indicando se o processamento foi concluído
    */
    bool get_single_thread_done();

    /*
        * Retorna se o processamento em múltiplas threads foi concluído.
        * @returns: booleano indicando se o processamento foi concluído
    */
    bool get_multi_thread_done();

    /*
        * Retorna a duração do processamento em uma única thread em milissegundos.
        * Se o processamento ainda não foi concluído, retorna o tempo decorrido até o momento.
        * @param done booleano indicando se o processamento foi concluído
        * @returns: duração do processamento em milissegundos
    */
    double get_single_thread_duration(bool);

    /*
        * Retorna a duração do processamento em múltiplas threads em milissegundos.
        * Se o processamento ainda não foi concluído, retorna o tempo decorrido até o momento.
        * @param done booleano indicando se o processamento foi concluído
        * @returns: duração do processamento em milissegundos
    */
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

        // trata a interpretação de cor da imagem de acordo com o especificado
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

        // Verifica se a imagem foi carregada corretamente
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

        // se o buffer estiver vazio
        if (buffer.empty()) { 
            throw invalid_argument("Erro: buffer de imagem vazio!");
        }

        // trata a interpretação de cor da imagem de acordo com o especificado
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

        // Verifica se a imagem foi carregada corretamente
        if (this->image.empty()) {
            cerr << "Erro: falha ao decodificar imagem!" << endl;
            exit(EXIT_FAILURE);
        }

        this->width = image.cols;
        this->height = image.rows;
    }

void Image::
    show() {
        // exibe a imagem em uma janela
        namedWindow("Janela Fixa", WINDOW_NORMAL); 
        resizeWindow("Janela Fixa", 800, 600);
        imshow("Janela Fixa", this->image);
        waitKey(0);
    }

void Image::
    save(){
        string imName =  "output/image";

        // salva a *imagem principal* da classe com a mesma extensão do arquivo original
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

        // salva a imagem processada em um único thread
        string imgName =  "output/image_singleThread";

        // salva a imagem processada em um único thread com a mesma extensão do arquivo original
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

        // caso a imagem seja em tons de cinza ou RGB
        }else 
            imwrite(imgName, this->image_singleThread);

        return;
    }

void Image::
    save_multithread(){

        // salva a imagem processada em múltiplos threads
        string imgName = "output/image_multiThread";

        // salva a imagem processada em múltiplos threads com a mesma extensão do arquivo original
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

        // caso a imagem seja em tons de cinza ou RGB
        } else
            imwrite(imgName, this->image_multiThread);

        return;
    }
// ===========================================================

// Intervalo para normalização
typedef struct Interval{
    float begin;
    float end;
} Interval;

// Normaliza o valor de intensidade para o intervalo especificado
inline float normalizeInInterval(int value, Interval& interval){
    
    // intervalo de intensidade original, recebido do frontend
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
                    pixel[0] = (this->image.at<Vec3b>(j, i)[0] + 90) % 180 ;    // H (girar "180 graus")
                    pixel[1] = 255 - this->image.at<Vec3b>(j, i)[1];            // S
                    pixel[2] = 255 - this->image.at<Vec3b>(j, i)[2];            // V
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
    thresholding_filter(Region region, Mat& image_output, int intensity) {
        // Normaliza a intensidade para o intervalo [0, 255]
        Interval interval = {0, 255};
        float normalized_intensity = normalizeInInterval(intensity, interval);

        // Se a imagem for BGR, aplica o filtro em cada canal
        if (this->color_type == ImageColorType::RGB){
            for (int i = region.x_begin; i <= region.x_end; i++){
                for (int j = region.y_begin; j <= region.y_end; j++){

                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);

                    int thresholding_value = (this->image.at<Vec3b>(j, i)[0] + this->image.at<Vec3b>(j, i)[1] + this->image.at<Vec3b>(j, i)[2]) / 3;
                    
                    // Aplica o filtro de limiarização
                    if (thresholding_value < normalized_intensity) {
                        pixel[0] = 0; // B
                        pixel[1] = 0; // G
                        pixel[2] = 0; // R
                    } else {
                        pixel[0] = 255; // B
                        pixel[1] = 255; // G
                        pixel[2] = 255; // R
                    }
                }
            }
        } else // se a imagem for HSV, aplica o filtro no canal de Valor
        if(this->color_type == ImageColorType::HSV){
            for (int i = region.x_begin; i <= region.x_end; i++){
                for (int j = region.y_begin; j <= region.y_end; j++){

                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);

                    int thresholding_value = (this->image.at<Vec3b>(j, i)[0] + this->image.at<Vec3b>(j, i)[1] + this->image.at<Vec3b>(j, i)[2]) / 3;
                    // Aplica o filtro de limiarização

                    if (thresholding_value < normalized_intensity) {
                        pixel[2] = 0; // V
                    } else {
                        pixel[2] = 255; // V
                    }
                }
            }  
        }else { // imagem em tons de cinza
            for (int i = region.x_begin; i <= region.x_end; i++){
                for (int j = region.y_begin; j <= region.y_end; j++){

                    // Pega o pixel da imagem de saída
                    uchar thresholding_value = this->image.at<uchar>(j, i);

                    // Aplica o filtro de limiarização
                    if (thresholding_value < normalized_intensity) {
                        image_output.at<uchar>(j, i) = 0; // Preto
                    } else {
                        image_output.at<uchar>(j, i) = 255; // Branco
                    }
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

                            const Vec3b* input_row = this->image.ptr<Vec3b>(y); // Usando ptr<> para acessar a linha de pixels

                            // soma os valores dos pixels vizinhos no canal de Valor
                            meanValue += input_row[x][2]; // V
                        }
                    }

                    Vec3b* output_row = image_output.ptr<Vec3b>(j);

                    output_row[i][0] = this->image.ptr<Vec3b>(j)[i][0]; // H (mantém o mesmo valor)
                    output_row[i][1] = this->image.ptr<Vec3b>(j)[i][1]; // S (mantém o mesmo valor)

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

                            // soma os valores dos pixels vizinhos no canal único, usando ptr<> para acessar os pixels rapidamente
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
    
        int meanFilterIntensity = 5; // Intensidade do filtro de média (um valor fixo)
    
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
    
                    output_row[i][0] = this->image.ptr<Vec3b>(j)[i][0]; // H (mantém o mesmo valor)
                    output_row[i][1] = this->image.ptr<Vec3b>(j)[i][1]; // S (mantém o mesmo valor)

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
        if(this->color_type == ImageColorType::RGB){

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
                            pixels[0].push_back(input_row[x][0]); // B
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

        // Caso em que é pra imagem resultado ser HSV
        }else 
        if(this->color_type == ImageColorType::HSV){
            
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
    
                    output_row[i][0] = this->image.ptr<Vec3b>(j)[i][0]; // H (mantém o mesmo valor)
                    output_row[i][1] = this->image.ptr<Vec3b>(j)[i][1]; // S (mantém o mesmo valor)

                    // Define o pixel da imagem de saída como a mediana dos pixels vizinhos
                    output_row[i][2] = medianValue;
                }
            }

        // Caso em que é pra imagem resultado ser em tons de cinza
        } else {

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
    grayscale_filter(Region region, Mat& image_output) {

        // Se a imagem for BGR, aplica o filtro em cada canal
        if(this->color_type == ImageColorType::RGB){
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
    
// Função que gera a mascara gaussiana de acordo com o tamanho do kernel passado
vector<vector<float>> generateGaussianKernel(int kernelSize) {
    if(kernelSize % 2 == 0) {
        kernelSize++; // Garante que o tamanho do kernel seja ímpar
    }

    vector<vector<float>> kernel(kernelSize, vector<float>(kernelSize));

    float sigma = kernelSize / 2.0f; // uma aproximação pra sigma baseada no tamanho
    float sum = 0.0f;
    int center = kernelSize / 2;

    // Gera a máscara gaussiana
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            int x = i - center;
            int y = j - center;
            kernel[i][j] = std::exp(-(x*x + y*y) / (2.0f * sigma * sigma));
            sum += kernel[i][j];
        }
    }

    // Normaliza para que a soma total seja 1
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] /= sum;
        }
    }

    return kernel;
}

void Image::
    gaussian_filter(Region region, Mat& image_output, int intensity){

        Interval interval = {1, 40};
        intensity = (int) normalizeInInterval(intensity, interval); // Normaliza a intensidade para o intervalo [1, 40]

        Mask_t gaussian_mask = Mask_t(generateGaussianKernel(intensity)); // gera a mascara gaussiana de acordo com a intensidade passada
        int kernel_radius = gaussian_mask.mask_.size() / 2; // raio do kernel

        // Se a imagem for BGR, aplica a mascara em cada canal
        if(this->color_type == ImageColorType::RGB){

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){
                    float meanValue[] = {0.0, 0.0, 0.0};

                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -kernel_radius; k <= kernel_radius; k++){
                        for(int l = -kernel_radius; l <= kernel_radius; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Calcula a média ponderada dos pixels vizinhos em cada canal
                            meanValue[0] += pixel[0] * gaussian_mask.mask_[l + kernel_radius][k + kernel_radius]; // B
                            meanValue[1] += pixel[1] * gaussian_mask.mask_[l + kernel_radius][k + kernel_radius]; // G
                            meanValue[2] += pixel[2] * gaussian_mask.mask_[l + kernel_radius][k + kernel_radius]; // R
                        }

                    }

                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);

                    // Aplica a mascara em cada canal
                    pixel[0] = clamp((double) round(meanValue[0]), 0.0, 255.0); // B
                    pixel[1] = clamp((double) round(meanValue[1]), 0.0, 255.0); // G
                    pixel[2] = clamp((double) round(meanValue[2]), 0.0, 255.0); // R
                }
            }
                
        }else // se a imagem for HSV, aplica a mascara no canal de Valor
        if(this->color_type == ImageColorType::HSV){
            
            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){
                    float meanValue = 0.0;

                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -kernel_radius; k <= kernel_radius; k++){
                        for(int l = -kernel_radius; l <= kernel_radius; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Calcula a média ponderada dos pixels vizinhos no canal de Valor
                            meanValue += pixel[2] * gaussian_mask.mask_[l + kernel_radius][k + kernel_radius]; // V
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);

                    pixel[0] = this->image.ptr<Vec3b>(j)[i][0]; // H (mantém o mesmo valor)
                    pixel[1] = this->image.ptr<Vec3b>(j)[i][1]; // S (mantém o mesmo valor)

                    // Aplica a mascara no canal de Valor
                    pixel[2] = clamp((double) round(meanValue), 0.0, 255.0); // V
                }
            }

        }else{ // imagem em tons de conza

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){
                    float meanValue = 0.0;

                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -kernel_radius; k <= kernel_radius; k++){
                        for(int l = -kernel_radius; l <= kernel_radius; l++){
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
                            meanValue += pixel * gaussian_mask.mask_[l + kernel_radius][k + kernel_radius];
                        }

                    }
                    // Pega o pixel da imagem de saída
                    uchar& pixel = image_output.at<uchar>(j, i);

                    // Aplica a mascara no canal único
                    pixel = clamp((double) round(meanValue), 0.0, 255.0); // R
                }
            }
        }
    }

void Image::
    apply_mask_filter(Region region, Mat& image_output, Mask_t mask_Mat, int intensity, int weight) {

        Interval interval = {1.0, 2.0};
        float k = normalizeInInterval(intensity, interval); // Normaliza a intensidade para o intervalo [1.0, 2]

        int kernel_radius = mask_Mat.mask_.size() / 2; // raio do kernel

        // Se a imagem for BGR, aplica a mascara em cada canal
        if(this->color_type == ImageColorType::RGB){

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){
                    int meanValue[] = {0, 0, 0};

                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -kernel_radius; k <= kernel_radius; k++){
                        for(int l = -kernel_radius; l <= kernel_radius; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Calcula a média ponderada dos pixels vizinhos em cada canal
                            meanValue[0] += pixel[0] * mask_Mat.mask_[l+1][k+1]; // B
                            meanValue[1] += pixel[1] * mask_Mat.mask_[l+1][k+1]; // G
                            meanValue[2] += pixel[2] * mask_Mat.mask_[l+1][k+1]; // R
                        }

                    }
                    int g_mask[] = {0, 0, 0};
                    // Calcula a mascara no pixel atual para cada canal
                    // g_mask[0] = this->image.ptr<Vec3b>(j)[i][0] - meanValue[0]/weight; // B
                    // g_mask[1] = this->image.ptr<Vec3b>(j)[i][1] - meanValue[1]/weight; // G
                    // g_mask[2] = this->image.ptr<Vec3b>(j)[i][2] - meanValue[2]/weight; // R

                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);

                    // Aplica a mascara em cada canal
                    // pixel[0] = clamp((double) round(this->image.ptr<Vec3b>(j)[i][0] + k * g_mask[0]), 0.0, 255.0); // B
                    // pixel[1] = clamp((double) round(this->image.ptr<Vec3b>(j)[i][1] + k * g_mask[1]), 0.0, 255.0); // G
                    // pixel[2] = clamp((double) round(this->image.ptr<Vec3b>(j)[i][2] + k * g_mask[2]), 0.0, 255.0); // R
                    pixel[0] = clamp((double) round(meanValue[0]/weight), 0.0, 255.0); // B
                    pixel[1] = clamp((double) round(meanValue[1]/weight), 0.0, 255.0); // G
                    pixel[2] = clamp((double) round(meanValue[2]/weight), 0.0, 255.0); // R
                }
            }
                
        }else // se a imagem for HSV, aplica a mascara no canal de Valor
        if(this->color_type == ImageColorType::HSV){
            
            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){
                    int meanValue = 0;

                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -kernel_radius; k <= kernel_radius; k++){
                        for(int l = -kernel_radius; l <= kernel_radius; l++){
                            // Pega o pixel vizinho
                            int x = i + k;
                            int y = j + l;

                            // Ignora pixels fora da imagem (mesma coisa que somar com 0, ou seja, usar o zero-padding)
                            if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
                                continue;
                            }
                            
                            // Pega o pixel da imagem de saída
                            Vec3b& pixel = this->image.at<Vec3b>(y, x);

                            // Calcula a média ponderada dos pixels vizinhos no canal de Valor
                            meanValue += pixel[2] * mask_Mat.mask_[l+1][k+1]; // V
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);

                    // Calcula a mascara no pixel atual para o canal de Valor
                    // int g_mask = 0;
                    // g_mask = this->image.ptr<Vec3b>(j)[i][2] - meanValue/weight;

                    pixel[0] = this->image.ptr<Vec3b>(j)[i][0]; // H (mantém o mesmo valor)
                    pixel[1] = this->image.ptr<Vec3b>(j)[i][1]; // S (mantém o mesmo valor)

                    // Aplica a mascara no canal de Valor
                    // pixel[2] = clamp((double) round(this->image.ptr<Vec3b>(j)[i][2] + k * g_mask), 0.0, 255.0); // V
                    pixel[2] = clamp((double) round(meanValue/weight), 0.0, 255.0); // V
                }
            }

        // imagem em tons de conza 
        }else{ 

            // pra cada pixel da imagem, aplica o filtro
            for(int i = region.x_begin; i <= region.x_end; i++){
                for(int j = region.y_begin; j <= region.y_end; j++){
                    int meanValue = 0;

                    // na vizinhanca da imagem de entrada, aplica a mascara
                    for(int k = -kernel_radius; k <= kernel_radius; k++){
                        for(int l = -kernel_radius; l <= kernel_radius; l++){
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
                    // Calcula a mascara no pixel atual para o canal único
                    // int g_mask = 0;
                    // g_mask = this->image.ptr<uchar>(j)[i] - meanValue/weight;

                    // Pega o pixel da imagem de saída
                    uchar& pixel = image_output.at<uchar>(j, i);

                    // Aplica a mascara no canal de Valor
                    // pixel = clamp((double) round(this->image.ptr<uchar>(j)[i] + k * g_mask), 0.0, 255.0); // V
                    pixel = clamp((double) round(meanValue/weight), 0.0, 255.0); // V
                }
            }
        }
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

                            // Calcula a média ponderada dos pixels vizinhos em cada canal
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


                            // Calcula a média ponderada dos pixels vizinhos no canal de Valor
                            meanValue += pixel[2] * laplacian90_mask.mask_[l+1][k+1]; // V
                        }

                    }
                    // Pega o pixel da imagem de saída
                    Vec3b& pixel = image_output.at<Vec3b>(j, i);
                    Vec3b& input_pixel = this->image.at<Vec3b>(j, i);

                    pixel[0] = input_pixel[0]; // H (mantém o mesmo valor)
                    pixel[1] = input_pixel[1]; // S (mantém o mesmo valor)

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
                                            {1,  1, 1} }); // mascara de laplaciano com as diagonais

        Interval interval = {0.5, 3.5};
        float k = normalizeInInterval(intensity, interval); // Normaliza a intensidade para o intervalo [0.5, 3.5]

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

                            // Calcula a média ponderada dos pixels vizinhos em cada canal
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

                    pixel[0] = input_pixel[0]; // H (mantém o mesmo valor)
                    pixel[1] = input_pixel[1]; // S (mantém o mesmo valor)

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
                            
                            // Aplica a mascara no canal unico
                            meanValue += pixel * laplacian45_mask.mask_[l+1][k+1];
                        }
                        
                    }
                    // Pega o pixel da imagem de saída
                    uchar& pixel = image_output.at<uchar>(j, i);
                    uchar& input_pixel = this->image.at<uchar>(j, i);

                    // Aplica a mascara no canal único
                    pixel = clamp((double) round(input_pixel - k * meanValue), 0.0, 255.0); 
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

        cout << "Filter: " << filter << endl;

        if(filter == "negative")
            this->negative_filter(region, this->image_multiThread);
        else if(filter == "thresholding")
            this->thresholding_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "blur")
            this->blur_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "sharpen")
            this->sharpen_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "median")
            this->median_filter(region, this->image_multiThread, this->intensity);
        else if(filter == "grayscale")
            this->grayscale_filter(region, this->image_multiThread);
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
            this->timer_multiThread.end = high_resolution_clock::now();
            this->timer_multiThread.timer_duration = duration_cast<milliseconds>(this->timer_multiThread.end - this->timer_multiThread.start);
            this->multi_thread_ended = true;
            cout << "Multi-Threads terminaram o processamento! Em " << this->timer_multiThread.timer_duration.count() << " milissegundos"<< endl;
        }
    }

void Image::
    single_thread_process(const string& filter, Region region) {

        if(filter == "negative")
            this->negative_filter(region, this->image_singleThread);
        else if(filter == "thresholding")
            this->thresholding_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "blur")
            this->blur_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "sharpen")
            this->sharpen_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "median")
            this->median_filter(region, this->image_singleThread, this->intensity);
        else if(filter == "grayscale")
            this->grayscale_filter(region, this->image_singleThread);
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

        this->timer_singleThread.end = high_resolution_clock::now();
        this->timer_singleThread.timer_duration = duration_cast<milliseconds>(this->timer_singleThread.end - this->timer_singleThread.start);
        this->single_thread_ended = true;
        cout << "Single-Thread terminou o processamento!" << " Em " << this->timer_singleThread.timer_duration.count() << " milissegundos"<< endl;
    }


void Image::
    process(const string& filter, int threads, int intensity = 1) {
        // Define, inicialmente, que o processamento em multi-thread e em single-thread ainda não acabou, ou seja, que ainda ainda estão em processamento.
        this->single_thread_ended = false;
        this->multi_thread_ended = false;

        // Deve guardar no objeto o atributo intensity
        this->intensity = intensity;

        // Limpa as imagens de saída, colocando todos os pixels como 0 (preto)
        // Para que no front a visualização do processamento seja melhor, mudando os pixels a medida que eles são processados
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
        
        // 1. SINGLE-THREADING:
        /*
            * Inicia o timer para o processamento em single-thread
            * Cria uma região que representa a imagem inteira (x: 0 a width-1, y: 0 a height-1)
            * Envia a região para o processamento em single-thread
            * Quando o processamento acabar, o timer é parado e a variável single_thread_ended é setada como true
        */
        this->timer_singleThread.start = high_resolution_clock::now();
        this->thread_pool->enqueue([this, filter, threads] {
            Region region = {0, this->width-1, 0, this->height-1}; // A imagem inteira
            this->single_thread_process(filter, region);
        });
    
        // 2. MULTI-THREADING:
        // Primeiramente, reparte a imagem em partes quase iguais, de acordo com o número de threads
        cout << endl << "Iniciando processamento em " << threads << " threads..." << endl;
        vector<Region> regions = getRegions(this->width, this->height, threads);

        // Em seguida, reseta o contador de threads e o timer
        this->threads_done = 0;                                                            
        this->timer_multiThread.start = high_resolution_clock::now();
        
        // Atribui a cada thread uma parte da imagem para processar
        for (int i = 0; i < threads; i++){
            this->thread_pool->enqueue([this, filter, threads,  regions, i] {
                this->thread_process(filter, threads, regions[i]);
            });
        }

        // Quando todas as threads terminarem, o timer é parado e a variável multi_thread_ended é setada como true
    }


// AUXILIARES //////////////////////////////////
// Recebe uma string e retorna o tipo de imagem correspondente
inline ImageType stringToImageType(const string& str) {
    if (str == "jpeg") return ImageType::JPEG;
    if (str == "jpg") return ImageType::JPG;
    if (str == "png") return ImageType::PNG;
    if (str == "bmp") return ImageType::BMP;
    if (str == "tiff") return ImageType::TIFF;
    throw invalid_argument("Tipo de imagem inválido");
}
// Recebe uma string e retorna o tipo de cor correspondente
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
            return this->timer_multiThread.timer_duration.count();
        else
            return duration_cast<duration<double, milli>>(high_resolution_clock::now() - this->timer_multiThread.start).count();
    }


double Image::
    get_single_thread_duration(bool done){
        if (done)
            return this->timer_singleThread.timer_duration.count();
        else
            return duration_cast<duration<double, milli>>(high_resolution_clock::now() - this->timer_singleThread.start).count();
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