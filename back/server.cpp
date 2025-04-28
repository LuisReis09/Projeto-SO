#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <thread>
#include "httplib.h"
#include "image.hpp"

using namespace std;
using namespace cv;


/*
    * Função auxiliar para ler arquivos do frontend
    * @param filename Nome do arquivo a ser lido
    * @return Conteúdo do arquivo como string
*/
string read_frontend_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(){
    httplib::Server server;
    shared_ptr<Image> img = make_shared<Image>();

    /*
        * Configuração do servidor HTTP
        * O servidor escuta na porta 4750 e responde a requisições GET e POST
        * para processar imagens e servir arquivos estáticos do frontend.
    */
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        string html_content = read_frontend_file("../front/index.html");
        if (html_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(html_content, "text/html");
    });

    // Arquivos CSS do frontend
    server.Get("/styles.css", [](const httplib::Request&, httplib::Response& res) {
        string css_content = read_frontend_file("../front/styles.css");
        if (css_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(css_content, "text/css");
    });

    // Arquivo JavaScript do frontend
    server.Get("/scripts.js", [](const httplib::Request&, httplib::Response& res) {
        string js_content = read_frontend_file("../front/scripts.js");
        if (js_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(js_content, "application/javascript");
    });

    // Arquivo de fonte do frontend
    server.Get("/getFont", [](const httplib::Request&, httplib::Response& res) {
        string font_content = read_frontend_file("../front/assets/fonts/Oldenburg-Regular.ttf");
        if (font_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(font_content, "application/x-font-ttf");
    });

    // Arquivos de imagem do frontend
    server.Get("/default.png", [](const httplib::Request&, httplib::Response& res) {
        string image_content = read_frontend_file("../front/assets/img/default.png");
        if (image_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(image_content, "image/png");
    });

    // Arquivos de imagem do frontend
    server.Get("/def-member.png", [](const httplib::Request&, httplib::Response& res) {
        string image_content = read_frontend_file("../front/assets/img/def-member.png");
        if (image_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(image_content, "image/png");
    });

    // Arquivos de imagem do frontend
    server.Get("/def-mult.png", [](const httplib::Request&, httplib::Response& res) {
        string image_content = read_frontend_file("../front/assets/img/def-mult.png");
        if (image_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(image_content, "image/png");
    });

    // Arquivos de imagem do frontend
    server.Get("/def-sing.png", [](const httplib::Request&, httplib::Response& res) {
        string image_content = read_frontend_file("../front/assets/img/def-sing.png");
        if (image_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(image_content, "image/png");
    });

    /*
        * Endpoint para processar a imagem recebida do frontend
        
        @params:
            - image: imagem a ser processada (formato binário)
            - intensity: intensidade do filtro (inteiro)
            - qtdThreads: quantidade de threads a serem utilizadas (inteiro)
            - filter: tipo de filtro a ser aplicado (string)
            - colorOption: opção de cor da imagem (string)
            - filetype: tipo de arquivo da imagem (string)

        * O servidor espera receber uma imagem no formato form-data com os parâmetros acima.

        * O servidor chama a função `process` da classe `Image` para aplicar o filtro na imagem recebida.
        * O servidor retorna um JSON informando se começou a processar a imagem ou se houve erro.
    */
    server.Post("/process", [&img](const httplib::Request& req, httplib::Response& res) {
        try{
            auto it = req.files.find("image");
            if (it == req.files.end()) {
                res.status = 400;
                res.set_content(R"({"error": "no image uploaded"})", "application/json");
                return;
            }
            const auto& file = it->second;
            const std::string& image_data = file.content;
            
            // Função auxiliar para extrair campos do form-data
            auto get_form_field = [&](const std::string& name) -> std::string {
                auto it = req.files.find(name);
                if (it == req.files.end()) {
                    throw std::runtime_error("Campo '" + name + "' não encontrado");
                }
                return it->second.content;
            };

            // Extrai os parâmetros corretamente
            const auto param_intensity = get_form_field("intensity");
            const auto param_qtdThreads = get_form_field("qtdThreads");
            const auto param_filter = get_form_field("filter");
            const auto param_colorOption = get_form_field("colorOption");
            const auto param_filetype = get_form_field("filetype");
            
            vector<uchar> buffer(image_data.begin(), image_data.end());
            
            
            cout << endl << "Image received!";

            // Carrega os dados recebidos no objeto da classe Image
            img->overwriteImage(buffer, stringToImageColorType(param_colorOption), stringToImageType(param_filetype));

            // Inicia o processamento da imagem com os parâmetros recebidos
            img->process(param_filter, stoi(param_qtdThreads), stoi(param_intensity));

            // Retorna um json com o status_code e a mensagem de que comecou a processar a imagem
            res.status = 200;
            string json_response = R"({"message": "Image processed successfully!"})";
            res.set_content(json_response, "application/json");
        }catch (exception& e){
            // Se faltou parametro, avisa
            cout << "Error: " << e.what() << endl;
            res.status = 400;
            string json_response = R"({"error": "bad request!"})";
            res.set_content(json_response, "application/json");
        }
    });

    /*
        * Endpoint para obter a imagem processada em um único thread
        * Através dessa função, o front-end acompanha o progresso do processamento da imagem em uma thread única.
        * 
        * @returns:
            - done: booleano indicando se o processamento foi concluído (cabeçalho)
            - duration: duração do processamento em milissegundos (cabeçalho)
            - image: imagem processada (formato binário)
    */
    server.Get("/getSingleThreadImage", [&img](const httplib::Request& req, httplib::Response& res) {
        try{
            bool single_thread_done  = img->get_single_thread_done();
            double single_thread_duration = img->get_single_thread_duration(single_thread_done);
            vector<uchar> single_thread_image = img->get_single_thread_image();


            res.status = 200;
            // Set o cabeçalho para indicar que é uma imagem binária
            res.set_header("Content-Type", "image/" + img->get_image_type());  // Defina o tipo de imagem correto (pode ser PNG, JPEG, etc.)
            res.set_header("done", to_string(single_thread_done));  // Adiciona "done" como cabeçalho
            res.set_header("duration", to_string(single_thread_duration));  // Adiciona "duration" como cabeçalho

            // Envia os bytes da imagem diretamente no corpo da resposta
            res.set_content(reinterpret_cast<const char*>(single_thread_image.data()), single_thread_image.size(), "image/" + img->get_image_type());  // Defina o tipo de imagem correto (pode ser PNG, JPEG, etc.)
        }catch(exception& e){
            // Se faltou parametro, avisa
            cout << "Error: " << e.what() << endl;
            res.status = 400;
            string json_response = R"({"error": "bad request!"})";
            res.set_content(json_response, "application/json");
        }
    });

    /*
        * Endpoint para obter a imagem processada em múltiplas threads
        * Através dessa função, o front-end acompanha o progresso do processamento da imagem em múltiplas threads.
        * 
        * @returns:
            - done: booleano indicando se o processamento foi concluído (cabeçalho)
            - duration: duração do processamento em milissegundos (cabeçalho)
            - image: imagem processada (formato binário)
    */
    server.Get("/getMultiThreadImage", [&img](const httplib::Request& req, httplib::Response& res) {
        try{
            bool multi_thread_done = img->get_multi_thread_done();
            double multi_thread_duration = img->get_multi_thread_duration(multi_thread_done);
            vector<uchar> multi_thread_image = img->get_multi_thread_image();


            res.status = 200;
            // Set o cabeçalho para indicar que é uma imagem binária
            res.set_header("Content-Type", "image/" + img->get_image_type());  // Defina o tipo de imagem correto (pode ser PNG, JPEG, etc.)
            res.set_header("done", to_string(multi_thread_done));  // Adiciona "done" como cabeçalho
            res.set_header("duration", to_string(multi_thread_duration));  // Adiciona "duration" como cabeçalho
            res.set_content(reinterpret_cast<const char*>(multi_thread_image.data()), multi_thread_image.size(), "image/" + img->get_image_type());  // Defina o tipo de imagem correto (pode ser PNG, JPEG, etc.)
        }catch(exception& e){
            // Se faltou parametro, avisa
            cout << "Error: " << e.what() << endl;
            res.status = 400;
            string json_response = R"({"error": "bad request!"})";
            res.set_content(json_response, "application/json");
        }
    });

    /*
        * Endpoint para obter as opções de threads para o processamento de imagens via multi-threading.
        * 
        * @returns:
            - maxThreads: número máximo de threads - 2, que é o número de threads disponíveis no sistema menos 2 (cabeçalho)
            * Pois as outras duas threads são utilizadas para o processamento da imagem em single-threading e para a execução contínua do backend.
    */
    server.Get("/getThreadsOptions", [](const httplib::Request& req, httplib::Response& res) {
        try{
            res.status = 200;
            int thread_max = thread::hardware_concurrency() - 2;
            string json_response = R"({"maxThreads": )" + to_string(thread_max) + R"(})";
            res.set_content(json_response, "application/json");
        }catch(exception& e){
            // Se faltou parametro, avisa
            cout << "Error: " << e.what() << endl;
            res.status = 400;
            string json_response = R"({"error": "bad request!"})";
            res.set_content(json_response, "application/json");
        }
    });

    /*
        * Endpoint para obter as opções de filtros disponíveis para o processamento de imagens.
        * 
        * @returns:
            - options: lista de filtros disponíveis (JSON)
    */
    server.Get("/getFiltersOptions", [](const httplib::Request& req, httplib::Response& res) {
        try{
            const auto param_colorOption = req.get_param_value("colorOption");
            
            // Implementar a logica de pegar na classe Image os filtros disponiveis de acordo com ser colorido (rgb) ou em escala de cinza (gray_scale)

            // Retorna um json com o status_code e a mensagem de erro ou as opcoes possiveis
            res.status = 200;
            string json_response = R"({"options": [
                "negative", 
                "blur", 
                "sharpen", 
                "grayscale", 
                "median", 
                "gaussian", 
                "laplacian90 border",
                "laplacian45 border",
                "laplacian90 sharpen",
                "laplacian45 sharpen"
            ]})";
            res.set_content(json_response, "application/json");
        }catch(exception& e){
            cout << "Error: " << e.what() << endl;
            res.status = 400;
            string json_response = R"({"error": "bad request!"})";
            res.set_content(json_response, "application/json");
        }
    });


    // Hosteia o server na porta 4750
    cout << "Server is running on http://localhost:4750" << endl;
    server.listen("localhost", 4750);
    
    return 0;
}