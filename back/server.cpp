#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <thread>
#include "httplib.h"
#include "image.hpp"

using namespace std;
using namespace cv;



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


    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        string html_content = read_frontend_file("../front/index.html");
        if (html_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(html_content, "text/html");
    });

    server.Get("/styles.css", [](const httplib::Request&, httplib::Response& res) {
        string css_content = read_frontend_file("../front/styles.css");
        if (css_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(css_content, "text/css");
    });

    server.Get("/scripts.js", [](const httplib::Request&, httplib::Response& res) {
        string js_content = read_frontend_file("../front/scripts.js");
        if (js_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(js_content, "application/javascript");
    });

    server.Get("/getFont", [](const httplib::Request&, httplib::Response& res) {
        string font_content = read_frontend_file("../front/assets/fonts/Oldenburg-Regular.ttf");
        if (font_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(font_content, "application/x-font-ttf");
    });

    server.Get("/default.png", [](const httplib::Request&, httplib::Response& res) {
        string image_content = read_frontend_file("../front/assets/img/default.png");
        if (image_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(image_content, "image/png");
    });

    server.Get("/def-member.png", [](const httplib::Request&, httplib::Response& res) {
        string image_content = read_frontend_file("../front/assets/img/def-member.png");
        if (image_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(image_content, "image/png");
    });

    server.Get("/def-mult.png", [](const httplib::Request&, httplib::Response& res) {
        string image_content = read_frontend_file("../front/assets/img/def-mult.png");
        if (image_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(image_content, "image/png");
    });

    server.Get("/def-sing.png", [](const httplib::Request&, httplib::Response& res) {
        string image_content = read_frontend_file("../front/assets/img/def-sing.png");
        if (image_content.empty()) {
            res.status = 500;
            return;
        }
        res.set_content(image_content, "image/png");
    });

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
            img->overwriteImage(buffer, stringToImageColorType(param_colorOption), stringToImageType(param_filetype));
            thread t_process(&Image::process, img,  param_filter, stoi(param_qtdThreads), stoi(param_intensity));
            t_process.detach(); // Desanexa a thread para que ela possa continuar executando em segundo plano

            // Retorna um json com o status_code e a mensagem de erro ou a imagem resultante
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

    server.Get("/getThreadsOptions", [](const httplib::Request& req, httplib::Response& res) {
        try{
            res.status = 200;
            // Consulta do sistema operacional quantas threads maximo o sistema suporta
            // e subtrai 2, uma para o processamento single-thread e outra pro backend do servidor
            // A fim de que o servidor não fique sobrecarregado com threads que ele não consegue suportar
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
            // Se faltou parametro, avisa
            cout << "Error: " << e.what() << endl;
            res.status = 400;
            string json_response = R"({"error": "bad request!"})";
            res.set_content(json_response, "application/json");
        }
    });

    server.Get("/getIntensityRange", [](const httplib::Request& req, httplib::Response& res) {
        try{
            const auto param_filter = req.get_param_value("filter");
            
            // Implementar a logica de pegar na classe Image as intensidades minimas e maximas de acordo com o filtro escolhido

            // Retorna um json com o status_code e a mensagem de erro ou as opcoes possiveis
            res.status = 200;
            string json_response = R"({"message": "intensity range!"})";
            res.set_content(json_response, "application/json");
        }catch(exception& e){
            // Se faltou parametro, avisa
            cout << "Error: " << e.what() << endl;
            res.status = 400;
            string json_response = R"({"error": "bad request!"})";
            res.set_content(json_response, "application/json");
        }
    });


    // Hosteia o server na porta 8080
    cout << "Server is running on http://localhost:4750" << endl;
    server.listen("localhost", 4750);
    
    return 0;
}