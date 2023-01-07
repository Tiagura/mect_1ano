#ifndef CODEC_VIDEO_H
#define CODEC_VIDEO_H

#include <fstream>
#include <string>
#include <bitset>
#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>


#include "codec_image.h"

using namespace std;

class codec_video{
    private:
        int mode;
        uint32_t x, y;

        //  Function to write the encoded data to a file
        int write_bin_to_file(const char* fileOut, string encoded){
            ofstream filew(fileOut,ios::out | ios::binary);
            char buffer = 0;
            int count = 0;
            for (uint32_t i = 0; i < encoded.length(); i++){
                buffer <<= 1;
                if (encoded[i] == '1') {
                    buffer |= 1;
                }
                count++;
                if(count == 8) {
                    //print the buffer
                    //cout << "BUFFER VAL: "<< buffer << endl;
                    filew.write(&buffer, 1);
                    buffer = 0;
                    count = 0;
                }
            }
            //write last buffer
            //if buffer is not empty
            if(count != 0) {
                //write buffer + padding
                buffer <<= (8-count);
                filew.write(&buffer, 1);
                count = 0;
                buffer = 0;
            }
            filew.close();
            return 0;
        }

        //  Function to read the encoded data from a file
        string read_bin_from_file(const char* fileIn){
            //read file
            ifstream filer(fileIn, ios::in | ios::binary);

            //read file
            char c;
            string encoded;
            while(filer.get(c)){
                //cout << "BUFFER VAL: "<< (int)c << endl;
                //convert char to binary
                bitset<8> x(c);
                //cout << "BUFFER VAL: "<< x << endl;
                encoded += x.to_string();
            }
            filer.close();
            return encoded;
        }

        // Function to read file to frames
        string read_file_to_frames(const char* fileIn, uint32_t n_frames, uint32_t height, uint32_t width, uint32_t frame_type, Mat[] to_store ){
            //open file
            ifstream filer(fileIn, ios::in | ios::binary);
            //ignore first line
            string header = getline(filer, line);
            cout << "HEADER: " << header << endl;
            string line;                            //line to read
            uint32_t size = height*width*1.5;       //read height*width*1,5 bytes
            char* buffer = new char[size];
            for(uint32_t i = 0; i < n_frames; i++){
                //read next line and check if contains "FRAME"
                getline(filer, line);
                if(line.find("FRAME") == std::string::npos){       //check if frame start
                    cout << "Error reading file" << endl;
                    return "";
                }
                //read frame
                filer.read(buffer, size);
                //write frame to Mat
                Mat tmp(height, width, frame_type);
                //write y plane
                for(uint32_t j = 0; j < height; j++){
                    for(uint32_t k = 0; k < width; k++){
                        tmp.at<Vec3b>(j,k)[0] = buffer[j*width+k];
                    }
                }
                //write Cb plane for 4 pixels at a time (2x2)
                for(uint32_t j = 0; j < height; j+=2){
                    for(uint32_t k = 0; k < width; k+=2){
                        tmp.at<Vec3b>(j,k)[1] = buffer[height*width + (j/2)*(width/2) + (k/2)];
                        tmp.at<Vec3b>(j+1,k)[1] = buffer[height*width + (j/2)*(width/2) + (k/2)];
                        tmp.at<Vec3b>(j,k+1)[1] = buffer[height*width + (j/2)*(width/2) + (k/2)];
                        tmp.at<Vec3b>(j+1,k+1)[1] = buffer[height*width + (j/2)*(width/2) + (k/2)];
                    }
                }
                //write Cr plane for 4 pixels at a time (2x2)
                for(uint32_t j = 0; j < height; j+=2){
                    for(uint32_t k = 0; k < width; k+=2){
                        tmp.at<Vec3b>(j,k)[2] = buffer[height*width + (width/2)*(height/2) + (j/2)*(width/2) + (k/2)];
                        tmp.at<Vec3b>(j+1,k)[2] = buffer[height*width + (width/2)*(height/2) + (j/2)*(width/2) + (k/2)];
                        tmp.at<Vec3b>(j,k+1)[2] = buffer[height*width + (width/2)*(height/2) + (j/2)*(width/2) + (k/2)];
                        tmp.at<Vec3b>(j+1,k+1)[2] = buffer[height*width + (width/2)*(height/2) + (j/2)*(width/2) + (k/2)];
                    }
                }
                //add frame to array
                to_store[i] = new Mat(tmp);
            }
            filer.close();
            return header;
        }

    public:
        codec_video(int mode, uint32_t x, uint32_t y){
            this->mode = mode;
            this->x = x;
            this->y = y;
        }

        codec_video(){
            this->mode = 8;
            this->x = 2500;
            this->y = 2500;
        }

        //  Function to encode a video file
        int encode_video(const char* fileIn, const char* fileOut){
            
            clock_t time_req;                           //for time measurement
            time_req = clock();                         //start time measurement
            
            Mat frame;                  //frame to store video frame
            VideoCapture cap(fileIn);   //read video

            //check if video is opened
            if(!cap.isOpened()){
                cout << "Error opening video stream or file" << endl;
                return -1;
            }

            VideoCapture cap2(fileIn);
            cap2 >> frame;


            //get video info
            uint32_t width = cap.get(CAP_PROP_FRAME_WIDTH);
            uint32_t height = cap.get(CAP_PROP_FRAME_HEIGHT);
            uint32_t fps = cap.get(CAP_PROP_FPS);
            uint32_t frames = cap.get(CAP_PROP_FRAME_COUNT);
            uint32_t frame_type = frame.type();
            int fourcc = cap.get(CAP_PROP_FOURCC);
            
            //close cap2   
            cap2.release();

            //create image codec
            image_codec encoder(mode, x, y);
        
            string encoded=""; //string to store encoded data

            //print header info
            cout << "Video info: " << endl;
            cout << "\tWidth: " << width << endl;
            cout << "\tHeight: " << height << endl;
            cout << "\tFPS: " << fps << endl;
            cout << "\tFrames: " << frames << endl;
            cout << "\tFrame type: " << frame_type << endl;
            cout << "\tFourCC: " << fourcc << endl;
            cout << "\tMode: " << this->mode << endl;
            cout << "\tX: " << this->x << endl;
            cout << "\tY: " << this->y << endl;

            //alloc space for frames
            Mat* to_store = new Mat[frames];
            //read video
            string header = read_file_to_frames(fileIn, frames, height, width, frame_type, to_store);

            //create header to store video info
            //width 32 bits, height 32 bits, fps 32 bits, frames 32 bits, frame type 32 bits, fourcc 32 bits, mode 4 bits, x 32 bits, y 32 bits
            encoded += bitset<32>(width).to_string();
            encoded += bitset<32>(height).to_string();
            encoded += bitset<32>(fps).to_string();
            encoded += bitset<32>(frames).to_string();
            encoded += bitset<32>(frame_type).to_string();
            encoded += bitset<32>(fourcc).to_string();
            encoded += bitset<4>(this->mode).to_string();
            encoded += bitset<32>(this->x).to_string();
            encoded += bitset<32>(this->y).to_string();

            namedWindow("rgb", WINDOW_AUTOSIZE);

            //encode each frame
            for(uint32_t i = 0; i < frames; i++){

                imshow("rgb", to_store[i]);
                waitKey(1);

                //encode frame
                encoded += encoder.encode_video_frame(to_store[i]);

                cout << "Frame " << i << " encoded" << endl;
                if(i==0) { imwrite("encoded_frame" + to_string(i) + ".ppm", to_store[i]); }
            }


            //write encoded data to file
            write_bin_to_file(fileOut, encoded);
            
            //print execution time
            time_req = clock() - time_req;
            cout << "Execution time: " << (float)time_req/CLOCKS_PER_SEC << " seconds" << endl;
            //compare size of original and encoded file
            ifstream file1(fileIn, ios::binary | ios::ate);
            ifstream file2(fileOut, ios::binary | ios::ate);
            cout << "Original file size: " << file1.tellg() << " bytes" << endl;
            cout << "Encoded file size: " << file2.tellg() << " bytes" << endl;
            //compression ratio
            cout << "Compression ratio: " << (float)file1.tellg()/(float)file2.tellg() << endl;
            file1.close();
            file2.close();

            return 0;
        }

        //  Function to decode a video file
        int decode_video(const char* fileIn, const char* fileOut){
            clock_t time_req;                           //for time measurement
            time_req = clock();                         //start time measurement

            //read encoded data from file
            string encoded = read_bin_from_file(fileIn);

            //get video info from header
            //width 32 bits, height 32 bits, fps 32 bits, frames 32 bits, frame type 32 bits, fourcc 32 bits, mode 4 bits, x 32 bits, y 32 bits
            uint32_t width = bitset<32>(encoded.substr(0, 32)).to_ulong();
            uint32_t height = bitset<32>(encoded.substr(32, 32)).to_ulong();
            uint32_t fps = bitset<32>(encoded.substr(64, 32)).to_ulong();
            uint32_t frames = bitset<32>(encoded.substr(96, 32)).to_ulong();
            uint32_t frame_type = bitset<32>(encoded.substr(128, 32)).to_ulong();
            int fourcc = bitset<32>(encoded.substr(160, 32)).to_ulong();
            this->mode = bitset<4>(encoded.substr(192, 4)).to_ulong();
            this->x = bitset<32>(encoded.substr(196, 32)).to_ulong();
            this-> y = bitset<32>(encoded.substr(228, 32)).to_ulong();
            
            //print video info
            cout << "Video info: " << endl;
            cout << "\tWidth: " << width << endl;
            cout << "\tHeight: " << height << endl;
            cout << "\tFPS: " << fps << endl;
            cout << "\tFrames: " << frames << endl;
            cout << "\tFrame type: " << frame_type << endl;
            cout << "\tFourCC: " << fourcc << endl;
            cout << "\tMode: " << mode << endl;
            cout << "\tX: " << x << endl;
            cout << "\tY: " << y << endl;


            encoded = encoded.substr(260);          //remove header from encoded string
            char* encoded_ptr = &encoded[0];    //pointer encoded string  

            //create image codec
            image_codec codec(mode, x, y);      

            //width is cols, height is rows
            VideoWriter video(fileOut, fourcc, fps, Size(height, width)); //create video writer

            Mat* decoded_frame = new Mat(height, width, frame_type);   //pointer to decoded frame

            //decode video
            for (uint32_t i = 0; i < frames; i++){
                encoded_ptr = codec.decode_video_frame(encoded_ptr, decoded_frame, height, width, frame_type);

                //convert YUV to RGB
                //cvtColor(frame, frame, COLOR_YUV2BGR_I420);
                
                //write decoded frame to video
                video.write(*decoded_frame);

                //write decoded frame as image 
                if(i==150) { imwrite("frame" + to_string(i) + ".ppm", *decoded_frame); }

                //break;
            }

            time_req = clock() - time_req;
            cout << "Execution time: " << (float)time_req/CLOCKS_PER_SEC << " seconds" << endl;

            return 0;
        }
};

#endif