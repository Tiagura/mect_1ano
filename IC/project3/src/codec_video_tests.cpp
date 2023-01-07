#include  "codec_video.h"

using namespace std;

int main(int argc, char *argv[]){
    //  Check if the number of arguments is correct
    if (argc < 4){
        cout << "Usage: " << argv[0] << " encode <input_video> <encoded_out> [-mode [1,8] (def: 8)]" << endl;
        cout << "                          (update m every x samples) (X>0) [-x [4,  ] (def: 2500)]" << endl;
        cout << "              (calculate medium of y samples) (x >= y > 0) [-y [4, x] (def: 2500)]" << endl;
        cout << "Usage: " << argv[0] << " decode <encoded_in>  <output_video>" << endl;
        return 1;
    }

    //  Check if the user wants to encode or decode
    if (string(argv[1]) == "encode"){
        //  Check if the user wants to use a different order
        int mode = 8;
        for (int n = 1; n < argc; n++)
            if (string(argv[n]) == "-order"){
                mode = atoi(argv[n+1]);
                if (mode < 1 || mode > 8){
                    cerr << "Error: invalid order" << endl;
                    return 1;
                }
                break;
            }

        //  Check if the user wants to use a different x
        int x = 2500;
        for (int n = 1; n < argc; n++)
            if (string(argv[n]) == "-x"){
                x = atoi(argv[n+1]);
                if (x < 3){
                    cerr << "Error: invalid x" << endl;
                    return 1;
                }
                break;
            }

        //  Check if the user wants to use a different y
        int y = 2500;
        for (int n = 1; n < argc; n++)
            if (string(argv[n]) == "-y"){
                y = atoi(argv[n+1]);
                if (y < 3 || y > x){
                    cerr << "Error: invalid y" << endl;
                    return 1;
                }
                break;
            }

        codec_video encoder(mode, x, y);
        cout << "Encoding file " << argv[2] << " to file "<< argv[3] << endl;
        encoder.encode_video(argv[2], argv[3]);
    }
    else if (string(argv[1]) == "decode"){
        codec_video decoder;
        cout << "Decoding file " << argv[2] << " to file "<< argv[3] << endl;
        decoder.decode_video(argv[2], argv[3]);
    }
    else{
        cerr << "Error: invalid option" << endl;
        return 1;
    }
    return 0;
}