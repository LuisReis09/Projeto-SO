#include <iostream>
#include <opencv2/opencv.hpp>
#include "image.hpp"

using namespace std;
using namespace cv;

int main(void){
    // Image image = Image("teste.jpg", ImageColorType::GRAYSCALE, ImageType::JPG);
    // image.show();
    // image.negative_filter();
    // image.show();
    // image.save();

    int image[10][10] = { {  1,  2,  3,  4,  5,  6,  7,  8,  9, 10},
                          { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20},
                          { 21, 22, 23, 24, 25, 26, 27, 28, 29, 30},
                          { 31, 32, 33, 34, 35, 36, 37, 38, 39, 40},
                          { 41, 42, 43, 44, 45, 46, 47, 48, 49, 50},
                          { 51, 52, 53, 54, 55, 56, 57, 58, 59, 60},
                          { 61, 62, 63, 64, 65, 66, 67, 68, 69, 70},
                          { 71, 72, 73, 74, 75, 76, 77, 78, 79, 80},
                          { 81, 82, 83, 84, 85, 86, 87, 88, 89, 90},
                          { 91, 92, 93, 94, 95, 96, 97, 98, 99,100} };

    vector<Coordinates> coords = getRegionCoordinates(10, 10, 10);

    for (int coord = 0; coord < coords.size(); coord++){
        for (int i = coords[coord].x_begin; i < coords[coord].x_end; i++){
            for (int j = coords[coord].y_begin; j < coords[coord].y_end; j++){
                cout << image[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    return 0;
}