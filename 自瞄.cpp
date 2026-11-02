#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

using namespace cv;
using namespace std;

int main() {
    Mat img = imread("C:/Users/HUAWEI/Desktop/light_bar.jpg");
    if (img.empty()) return -1;

    imwrite("D:/01_original.jpg", img);

    Mat hsv, mask;
    cvtColor(img, hsv, COLOR_BGR2HSV);

    Scalar lower_blue(80, 50, 100);
    Scalar upper_blue(130, 255, 255);
    inRange(hsv, lower_blue, upper_blue, mask);

    imwrite("D:/02_binary.jpg", mask);

    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<RotatedRect> light_bars;
    Mat result_bars = img.clone();

    for (size_t i = 0; i < contours.size(); i++) {
        RotatedRect rect = minAreaRect(contours[i]);
        float w = rect.size.width;
        float h = rect.size.height;
        float longSide = max(w, h);
        float shortSide = min(w, h);

        if (shortSide == 0) continue;

        float area = w * h;

        if (area > 15) {
            light_bars.push_back(rect);
            Point2f pts[4];
            rect.points(pts);
            for (int j = 0; j < 4; j++) {
                line(result_bars, pts[j], pts[(j + 1) % 4], Scalar(0, 255, 0), 2);
            }
        }
    }

    cout << "Found bars: " << light_bars.size() << endl;
    imwrite("D:/03_candidates.jpg", result_bars);

    int pair_count = 0;
    Mat result_armor = img.clone();

    for (size_t i = 0; i < light_bars.size(); i++) {
        for (size_t j = i + 1; j < light_bars.size(); j++) {
            Point2f center_i = light_bars[i].center;
            Point2f center_j = light_bars[j].center;

            float dx = abs(center_i.x - center_j.x);
            float dy = abs(center_i.y - center_j.y);

            float h_i = max(light_bars[i].size.width, light_bars[i].size.height);
            float h_j = max(light_bars[j].size.width, light_bars[j].size.height);

            if (dy < h_i * 0.8 && dx > h_i * 1.0 && dx < h_i * 5.0) {

                float min_x = min(center_i.x, center_j.x) - 10;
                float min_y = min(center_i.y, center_j.y) - 20;
                float max_x = max(center_i.x, center_j.x) + 10;
                float max_y = max(center_i.y, center_j.y) + 20;

                Rect armor_rect(min_x, min_y, max_x - min_x, max_y - min_y);

                if (armor_rect.x >= 0 && armor_rect.y >= 0 &&
                    armor_rect.x + armor_rect.width <= img.cols &&
                    armor_rect.y + armor_rect.height <= img.rows) {

                    rectangle(result_armor, armor_rect, Scalar(0, 0, 255), 2);

                    Mat roi = img(armor_rect);
                    if (!roi.empty()) {
                        string roi_name = "D:/05_roi_" + to_string(pair_count) + ".jpg";
                        imwrite(roi_name, roi);
                        pair_count++;
                    }
                }
            }
        }
    }
    cout << "Paired armors: " << pair_count << endl;
    imwrite("D:/04_paired.jpg", result_armor);

    namedWindow("Result", WINDOW_NORMAL);
    imshow("Result", result_armor);

    waitKey(0);
    return 0;
}