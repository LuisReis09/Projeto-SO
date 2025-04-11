#include <bits/stdc++.h>
#include "httplib.h"

using namespace std;

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

    // Hosteia o server na porta 8080
    printf("Server is running on http://localhost:4750");
    server.listen("localhost", 4750);
    
    return 0;
}