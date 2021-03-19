#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <X11/Xlib.h>


using namespace cv;

Rect2d getSelection(Mat img, char* title, bool fromCenter, bool showCrosshair){
    Rect2d r = selectROI(title, img, fromCenter, showCrosshair);
    if(r.empty()){
        std::cout << "Cancelled selection, exiting.\n";
        exit(0);
    }
    return r;
}

Mat getImgFromFile(char* impath){
    Mat img = imread(impath);
    if(img.empty())
    {
        std::cerr<< "Could not read image from: " << impath << std::endl;
        exit(1);
    }
    return img;
}

Mat getImgFromClipboard(){
    // TODO implement a C++ implementation instead
    system("xclip -o -selection clipboard > /tmp/from.png");
    Mat img = imread("/tmp/from.png");
    if(img.empty())
    {
        std::cerr<< "Could not read image from clipboard. "<< std::endl;
        exit(1);
    }
    return img;
}

void printRect(Rect r){
    std::cout << "Rect(" << r.x << "," << r.y << "," << r.height << "," << r.width << ")\n"; 
}

void saveImgToClipboard(Mat img){
    // TODO implement a C++ implementation instead
    imwrite("/tmp/to.png", img);
    system("xclip -selection clipboard -target image/png -i /tmp/to.png");
}

int searchChar(char* str, char key){
    for(int i = 0; i < strlen(str); i++){
        if(str[i] == key)
            return i;
    }
    return -1;
}

Rect calcOriginalRect(Rect resized, Size newSize, Size origSize){
    int x, y, h, w;
    double Rx, Ry;

    x = resized.x;
    y = resized.y;
    h = resized.height;
    w = resized.width;

    Rx = origSize.width / newSize.width;
    Ry = origSize.height / newSize.height;

    Rect newRect = Rect(x*Rx, y*Ry, w*Rx, h*Ry);

    return newRect;
}

Size parseGeometry(char* geometryString){
    int dloc = searchChar(geometryString, 'x');
    int x,y;
    if(dloc < 1){
        std::cout << "Incorrect geometry string!\n";
        exit(1);
    }
    char xstr[dloc];
    char ystr[strlen(geometryString) - dloc - 1];
    for(int i = 0; i < strlen(geometryString); i++){
        if(i < dloc)
            xstr[i] = geometryString[i];
        else if(i > dloc)
            ystr[i - dloc - 1] = geometryString[i];
    }

    x = atoi(xstr);
    y = atoi(ystr);
    return Size(x,y);
}

void exitHelp(){
    const char* HELP_TEXT = "Usage:\n"
                            "  simcrop [OPTION?] -f(c) [PATH?] -s(c) [PATH?]\n\n"
                            "Help Options:\n"
                            "  -h, --help       Show help options\n\n"
                            "Application Options:\n"
                            "  -c, --center     Select from center\n"
                            "  -x, --crosshair  Show crosshair\n"
                            "  -g, --geometry   Image display size\n\n"
                            "From & To:\n"
                            "  -f(c) [PATH]     Image to crop, adding c fetches the image from clipboard\n"
                            "  -s(c) [PATH]     Path to save cropped, adding c saves the image to clipboard\n\n"
                            "Keybinds:\n"
                            "  * In selection, press enter or space to confirm.\n"
                            "  * To cancel, press c in selection window.\n"
                            "  * In confirmation, press q to exit and s to save.\n";
    std::cout << HELP_TEXT << "\n";
    exit(0);
}

int main(int argc, char** argv)
{
    bool showCrosshair = false;
    bool selectFromCenter = false;
    Mat fromImg = Mat();
    Mat fromImgResized = Mat();
    bool saveClipboard = false;
    char* savePath = "";
    char* title = "SimCrop";
    Size geometry = Size();

    if(argc < 3){
        std::cout << "Not enough parameters!\n";
        exitHelp();
    }

    for (int i = 0; i < argc; ++i){
        if(argv[i][0] != '-'){
            continue;
        }
        if(!strcmp(argv[i], "-h")){
            exitHelp();
        }else if(!(strcmp(argv[i], "-c") && strcmp(argv[i], "--center"))){
            selectFromCenter = true;
        }else if(!(strcmp(argv[i], "-g") && strcmp(argv[i], "--geometry"))){
            geometry = parseGeometry(argv[i+1]);
            resize(fromImg, fromImgResized, geometry, 0, 0, INTER_LINEAR);
        }else if(!(strcmp(argv[i], "-x") && strcmp(argv[i], "--crosshair"))){
        }else if(!(strcmp(argv[i], "-t") && strcmp(argv[i], "--title"))){
            if(argc <= i+1){
                std::cerr << "You must provide a string with -t!\n";
                exitHelp();
            }
            title = argv[i+1];
        }else if(!strcmp(argv[i], "-f")){
            if(!fromImg.empty()){
                std::cerr << "You can only provide one from argument!\n";
                exitHelp();
            }
            if(argc <= i+1){
                std::cerr << "You must provide a path with -f!\n";
                exitHelp();
            }
            fromImg = getImgFromFile(argv[i+1]);
        }else if(!strcmp(argv[i], "-s")){
            if(strcmp(savePath, "") || saveClipboard){
                std::cerr << "You can only provide one save argument!\n";
                exitHelp();
            }
            if(argc <= i+1){
                std::cerr << "You must provide a path with -s!\n";
                exitHelp();
            }
            savePath = argv[i+1];
        }else if(!strcmp(argv[i], "-fc")){
            if(!fromImg.empty()){
                std::cerr << "You can only provide one from arguement!\n";
                exitHelp();
            }
            fromImg = getImgFromClipboard();
        }else if(!strcmp(argv[i], "-sc")){
            if(strcmp(savePath, "") || saveClipboard){
                std::cerr << "You can only provide one save argument!\n";
                exitHelp();
            }
            saveClipboard = true;
        }else{
            std::cout << "Unknown parameter " << argv[i] << "\n";
            exitHelp();
        }
    }

    printf("%d", fromImg.empty());
    if(!(saveClipboard || strcmp(savePath, "")) || fromImg.empty()){
        std::cerr << "From and To arguements are mandatory!\n";
        exitHelp();
    }

    uint8_t key = 98;
    Mat cropped = Mat();
    Rect2d selection = Rect2d();
    while(key == 98){
        if(geometry.empty()){
            selection = getSelection(fromImg, title, selectFromCenter, showCrosshair);
            cropped = fromImg(selection);
        }else{
            selection = getSelection(fromImgResized, title, selectFromCenter, showCrosshair);
            cropped = fromImgResized(selection);
        }
        imshow(title, cropped);
        do{
            key = waitKey(1);
        }while(key != 113 && key != 115 && key != 98);
    }

    if(key == 113){
        return 0;
    }

    if(geometry.empty()){
        if(saveClipboard){
            saveImgToClipboard(cropped);
        }else{
            imwrite(savePath, cropped);
        }
    }else{
        if(saveClipboard){
            saveImgToClipboard(cropped);
        }else{
            imwrite(savePath, cropped);
        }
    }

    return 0;
}
