#ifndef CODEC_GOLOMB_H
#define CODEC_GOLOMB_H

#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <fstream>
#include <sndfile.hh>
#include <math.h>
#include <iostream>

#include <opencv2/opencv.hpp>
#include "golomb.h"

using namespace std;
using namespace cv;

int prints = 1;

class image_codec{
    private:
        int mode;
        uint32_t x, y;

        int calculate_prediction(int mode, int a, int b, int c){
            if(mode == 8){
                if (c >= max(a,b)){
                    return min(a,b);
                }else if (c <= min(a,b)){
                    return max(a,b);
                }else{
                    return a+b-c;
                }
            }else if(mode==2){
                //b
                return b;
            }else if(mode==3){
                //c
                return c;
            }else if(mode==4){
                //a+b-c
                return a+b-c;
            }else if(mode==5){
                return a+(b-c)/2;
            }else if(mode==6){
                //b+(a-c)/2
                return b+(a-c)/2;
            }else if(mode==7){
                return (a+b)/2;
            }else if(mode==1){
                //a
                return a;
            }else{
                //error
                cout << "ERROR: Wrong mode" << endl;
                // system exit
                exit(0);
            }
        }

        uint32_t calc_m(double avg){
            double alfa = avg / (avg + 1);
            double res = ceil(-1/log2(alfa));
            if (res == 0){
                return 1;
            }
            return res;
        }

    public:
        image_codec(int n_mode, uint32_t x_in, uint32_t y_in){
            mode = n_mode;
            x = x_in;
            y = y_in;
        } 
        image_codec(){
            mode = 8;
            x = 0;
            y = 0;
        }       

        //  Function to encode a frame, returns the string with the encoded frame data
        string encode_video_frame(Mat image){
            if (!image.data){
                printf("No image data");
                return "";
            }

            int prediction;         //int to store the prediction
            string encoded = "";    //string to store the encoded frame

            //print values of pixels (0,0) (0,1) (0,2) (0,3) (1,0) (1,1) (1,2) (1,3) (2,0) (2,1) (2,2) (2,3) (3,0) (3,1) (3,2) (3,3)
            cout << "PIXEL (0,0)\t1: " << (int)image.at<Vec3b>(0,0)[0] << " 2: " << (int)image.at<Vec3b>(0,0)[1] << " 3: " << (int)image.at<Vec3b>(0,0)[2] << endl;
            cout << "PIXEL (0,1)\t1: " << (int)image.at<Vec3b>(0,1)[0] << " 2: " << (int)image.at<Vec3b>(0,1)[1] << " 3: " << (int)image.at<Vec3b>(0,1)[2] << endl;
            // cout << "PIXEL (0,2)\t1: " << (int)image.at<Vec3b>(0,2)[0] << " 2: " << (int)image.at<Vec3b>(0,2)[1] << " 3: " << (int)image.at<Vec3b>(0,2)[2] << endl;
            // cout << "PIXEL (0,3)\t1: " << (int)image.at<Vec3b>(0,3)[0] << " 2: " << (int)image.at<Vec3b>(0,3)[1] << " 3: " << (int)image.at<Vec3b>(0,3)[2] << endl;
            cout << "PIXEL (1,0)\t1: " << (int)image.at<Vec3b>(1,0)[0] << " 2: " << (int)image.at<Vec3b>(1,0)[1] << " 3: " << (int)image.at<Vec3b>(1,0)[2] << endl;
            cout << "PIXEL (1,1)\t1: " << (int)image.at<Vec3b>(1,1)[0] << " 2: " << (int)image.at<Vec3b>(1,1)[1] << " 3: " << (int)image.at<Vec3b>(1,1)[2] << endl;
            // cout << "PIXEL (1,2)\t1: " << (int)image.at<Vec3b>(1,2)[0] << " 2: " << (int)image.at<Vec3b>(1,2)[1] << " 3: " << (int)image.at<Vec3b>(1,2)[2] << endl;
            // cout << "PIXEL (1,3)\t1: " << (int)image.at<Vec3b>(1,3)[0] << " 2: " << (int)image.at<Vec3b>(1,3)[1] << " 3: " << (int)image.at<Vec3b>(1,3)[2] << endl;
            // cout << "PIXEL (2,0)\t1: " << (int)image.at<Vec3b>(2,0)[0] << " 2: " << (int)image.at<Vec3b>(2,0)[1] << " 3: " << (int)image.at<Vec3b>(2,0)[2] << endl;
            // cout << "PIXEL (2,1)\t1: " << (int)image.at<Vec3b>(2,1)[0] << " 2: " << (int)image.at<Vec3b>(2,1)[1] << " 3: " << (int)image.at<Vec3b>(2,1)[2] << endl;
            // cout << "PIXEL (2,2)\t1: " << (int)image.at<Vec3b>(2,2)[0] << " 2: " << (int)image.at<Vec3b>(2,2)[1] << " 3: " << (int)image.at<Vec3b>(2,2)[2] << endl;
            // cout << "PIXEL (2,3)\t1: " << (int)image.at<Vec3b>(2,3)[0] << " 2: " << (int)image.at<Vec3b>(2,3)[1] << " 3: " << (int)image.at<Vec3b>(2,3)[2] << endl;
            // cout << "PIXEL (3,0)\t1: " << (int)image.at<Vec3b>(3,0)[0] << " 2: " << (int)image.at<Vec3b>(3,0)[1] << " 3: " << (int)image.at<Vec3b>(3,0)[2] << endl;
            // cout << "PIXEL (3,1)\t1: " << (int)image.at<Vec3b>(3,1)[0] << " 2: " << (int)image.at<Vec3b>(3,1)[1] << " 3: " << (int)image.at<Vec3b>(3,1)[2] << endl;
            // cout << "PIXEL (3,2)\t1: " << (int)image.at<Vec3b>(3,2)[0] << " 2: " << (int)image.at<Vec3b>(3,2)[1] << " 3: " << (int)image.at<Vec3b>(3,2)[2] << endl;
            // cout << "PIXEL (3,3)\t1: " << (int)image.at<Vec3b>(3,3)[0] << " 2: " << (int)image.at<Vec3b>(3,3)[1] << " 3: " << (int)image.at<Vec3b>(3,3)[2] << endl;



            //store the values to encode of the image    
            vector<uint32_t> image_values;

            //image_values[0]...image_values[rows-1] = Y channel of first row
            //image_values[rows]...image_values[rows+cols-1] = Y channel of first column
            //image_values[rows+cols]...image_values[(rows*cols)-1] = Y channel of remaining pixels

            //store the first column for Y channel
            for(uint32_t i = 0; i<image.rows; i++){
                image_values.push_back(image.at<Vec3b>(i,0)[0] * 2);   //this way we store the original values for first row
            }
            
            //store the first row for Y channel
            for(uint32_t i = 1; i<image.cols; i++){
                image_values.push_back(image.at<Vec3b>(0,i)[0] * 2);   //this way we store the original values for first column
            }

            //process Y channel for all pixels
            for(uint32_t i = 1; i < image.rows; i++){
                for(uint32_t j = 1; j < image.cols; j++){
                    prediction = image.at<Vec3b>(i,j)[0] - calculate_prediction(mode, image.at<Vec3b>(i,j-1)[0], image.at<Vec3b>(i-1,j)[0], image.at<Vec3b>(i-1,j-1)[0]);    //calculate prediction
                    if(prediction < 0){
                        image_values.push_back((prediction * -2) + 1);  //map negative values to positive odd numbers
                    } else {
                        image_values.push_back(prediction * 2);         //map positive values to positive even numbers
                    }
                    //cout << "PREDICTION FOR Y: " << prediction << endl;
                }
            }

            //image_values[(rows*cols)]...image_values[(rows*cols)+(rows/2)-1] = values of Cb and Cr channels of first row [Cb,Cr,Cb,Cr...]
            //image_values[(rows*cols)+(rows/2)]...image_values[(rows*cols)+(rows/2)+(cols/2)-1] = values of Cb and Cr channels of first column [Cb,Cr,Cb,Cr...]
            //image_values[(rows*cols)+(rows/2)+(cols/2)]...image_values[end] = values of Cb and Cr channels of remaining pixels

            //store the first column for Cb and Cr channels
            for(uint32_t i = 0; i<image.rows; i+=2){
                image_values.push_back(image.at<Vec3b>(i,0)[1] * 2);   //this way we store the original values for first row
                image_values.push_back(image.at<Vec3b>(i,0)[2] * 2);   //this way we store the original values for first row
            }

            //store the first row for Cb and Cr channels
            for(uint32_t i = 2; i<image.cols; i+=2){ 
                //cout << "PIXEL (" << 0 << "," << i << ")\t1: " << (int)image.at<Vec3b>(0,i)[0] << " 2: " << (int)image.at<Vec3b>(0,i)[1] << " 3: " << (int)image.at<Vec3b>(0,i)[2] << endl;
                image_values.push_back(image.at<Vec3b>(0,i)[1] * 2);   //this way we store the original values for first column
                image_values.push_back(image.at<Vec3b>(0,i)[2] * 2);   //this way we store the original values for first column
            }

            //process Cb and Cr channels for all pixels  cause -> number values of Cb = number of values of Cr
            for(uint32_t i = 2; i < image.rows; i+=2){
                for(uint32_t j = 2; j < image.cols; j+=2){
                    prediction = image.at<Vec3b>(i,j)[1] - calculate_prediction(mode, image.at<Vec3b>(i,j-1)[1], image.at<Vec3b>(i-1,j)[1], image.at<Vec3b>(i-1,j-1)[1]);    //calculate prediction
                    if(prediction < 0){
                        image_values.push_back((prediction * -2) + 1);  //map negative values to positive odd numbers
                    } else {
                        image_values.push_back(prediction * 2);         //map positive values to positive even numbers
                    }
                    //cout << "PREDICTION FOR Cb: " << prediction << endl;
                    prediction = image.at<Vec3b>(i,j)[2] - calculate_prediction(mode, image.at<Vec3b>(i,j-1)[2], image.at<Vec3b>(i-1,j)[2], image.at<Vec3b>(i-1,j-1)[2]);    //calculate prediction
                    if(prediction < 0){
                        image_values.push_back((prediction * -2) + 1);  //map negative values to positive odd numbers
                    } else {
                        image_values.push_back(prediction * 2);         //map positive values to positive even numbers
                    }
                    //cout << "PREDICTION FOR Cr: " << prediction << endl;
                }
            }

            //check image_values size
            //cout << "image_values size: " << image_values.size() << endl;
            //cout << "MUST HAVE: " << image.cols*image.rows*1.5 << endl;

            golomb codec_alg;   //init codec_alg with golomb constructor
            codec_alg.change_m_encode(10);  //set m to 10

            //encode image_values
            for(uint32_t i = 0; i < image_values.size(); i++){
                encoded += codec_alg.encode_number(image_values[i], 0);
            }

            if(prints){
                int i=1;
                int j=1;
                //print used points for prediction
                cout << "PIXEL (" << i << "," << j << ")\t1: " << (int)image.at<Vec3b>(i,j)[0] << " 2: " << (int)image.at<Vec3b>(i,j)[1] << " 3: " << (int)image.at<Vec3b>(i,j)[2] << endl;
                cout << "PIXEL (" << i << "," << j-1 << ")\t1: " << (int)image.at<Vec3b>(i,j-1)[0] << " 2: " << (int)image.at<Vec3b>(i,j-1)[1] << " 3: " << (int)image.at<Vec3b>(i,j-1)[2] << endl;
                cout << "PIXEL (" << i-1 << "," << j << ")\t1: " << (int)image.at<Vec3b>(i-1,j)[0] << " 2: " << (int)image.at<Vec3b>(i-1,j)[1] << " 3: " << (int)image.at<Vec3b>(i-1,j)[2] << endl;
                cout << "PIXEL (" << i-1 << "," << j-1 << ")\t1: " << (int)image.at<Vec3b>(i-1,j-1)[0] << " 2: " << (int)image.at<Vec3b>(i-1,j-1)[1] << " 3: " << (int)image.at<Vec3b>(i-1,j-1)[2] << endl;
                //print prediction
                cout << "PREDICTION FOR Y: " << calculate_prediction(mode, image.at<Vec3b>(i,j-1)[0], image.at<Vec3b>(i-1,j)[0], image.at<Vec3b>(i-1,j-1)[0]) << endl;

                int index = 0;
                int index2 = 0;
                cout << "COL:" << index << endl;
                //print index column of newImage
                for(uint32_t i = 0; i < image.rows; i++){
                    cout << "("<< i <<","<<index <<") " << (int)image.at<Vec3b>(i,index)[0] << " " << (int)image.at<Vec3b>(i,index)[1] << " " << (int)image.at<Vec3b>(i,index)[2] << endl;
                } 
                cout << "ROW :"<< index2 << endl;
                //print first row of newImage
                for(uint32_t i = 0; i < image.cols; i++){
                    cout << "("<<index2 <<","<< i <<") " << (int)image.at<Vec3b>(index2,i)[0] << " " << (int)image.at<Vec3b>(index2,i)[1] << " " << (int)image.at<Vec3b>(index2,i)[2] << endl;
                }
            }       

            return encoded;
        }

        char* decode_video_frame(char* p, Mat *ptr, uint32_t rows, uint32_t cols, uint32_t type){

            //size of string with pointer p
            // cout << "Encoded size in bytes: " << (double)strlen(p)/8 << endl;
            // cout << "Encoded size in Mbytes: " << (double)strlen(p)/8/1024/1024 << endl;

            long * tmp_val = (long*)malloc(sizeof(long));    //pointer to store decoded number

            //init codec_alg with golomb constructor
            golomb codec_alg;
            codec_alg.change_m_decode(10);
            
            //vector to store mapped values
            vector<uint32_t> mapped_values;

            Mat newImage = Mat(rows, cols, type);  //create new image with same size as original image

            //decode channel Y
            //decode first column
            for(uint32_t i = 0; i < rows; i++){
                p = codec_alg.decode_string(p, tmp_val, 0);
                mapped_values.push_back(*tmp_val);          //store mapped value in vector
                newImage.at<Vec3b>(i,0)[0] = (*tmp_val/2);      //store decoded value in image
            }
            //decode first row
            for(uint32_t i = 1; i < cols; i++){
                p = codec_alg.decode_string(p, tmp_val, 0);
                mapped_values.push_back(*tmp_val);          //store mapped value in vector
                newImage.at<Vec3b>(0,i)[0] = (*tmp_val/2);      //store decoded value in image
            }

            //decode remaining values
            for(uint32_t i = 1; i < rows; i++){
                for(uint32_t j = 1; j < cols; j++){
                    p = codec_alg.decode_string(p, tmp_val, 0);
                    mapped_values.push_back(*tmp_val);         //store mapped value in vector   
                    if(*tmp_val % 2 == 0){
                        *tmp_val = *tmp_val/2;
                    }else{
                        *tmp_val = (*tmp_val-1)/-2;
                    }
                    newImage.at<Vec3b>(i,j)[0] = *tmp_val + calculate_prediction(mode, newImage.at<Vec3b>(i,j-1)[0], newImage.at<Vec3b>(i-1,j)[0], newImage.at<Vec3b>(i-1,j-1)[0]);    //store decoded value in image
                }
            }

            //decode channel Cb and Cr
            //decode first columns of both channels
            for(uint32_t i = 0; i<rows; i+=2){
                p = codec_alg.decode_string(p, tmp_val, 0);
                mapped_values.push_back(*tmp_val);          //store mapped value in vector
                
                // cout << "(" << i << ")" << endl;
                // cout << "INDEX: " << i << endl;
                // cout <<"INDEX AFFECTED: (" << i <<",0) / " << "(" << i+1 <<",0) / " << "(" << i <<",1) / " << "(" << i+1 <<",1)" << endl;
                // cout << "DECODED VALUE: " << *tmp_val/2 << endl;
                
                newImage.at<Vec3b>(i,0)[1] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(i+1,0)[1] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(i,1)[1] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(i+1,1)[1] = (*tmp_val/2);      //store decoded value in image

                p = codec_alg.decode_string(p, tmp_val, 0);
                mapped_values.push_back(*tmp_val);          //store mapped value in vector

                newImage.at<Vec3b>(i,0)[2] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(i+1,0)[2] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(i,1)[2] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(i+1,1)[2] = (*tmp_val/2);      //store decoded value in image

                // cout << "DECODED VALUE: " << *tmp_val/2 << endl;
                // cout <<"--------------------------------------------"<<endl;
            }

            //decode first rows of both channels
            for(uint32_t i=2; i<cols; i+=2){
                p = codec_alg.decode_string(p, tmp_val, 0);
                mapped_values.push_back(*tmp_val);          //store mapped value in vector

                // cout << "(" << i << ")" << endl;
                // cout << "INDEX: " << i << endl;
                // cout <<"INDEX AFFECTED: (" << 0 <<"," << i << ") / " << "(" << 0 <<"," << i+1 << ") / " << "(" << 1 <<"," << i << ") / " << "(" << 1 <<"," << i+1 << ")" << endl;
                // cout << "DECODED VALUE: " << *tmp_val/2 << endl;

                newImage.at<Vec3b>(0,i)[1] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(0,i+1)[1] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(1,i)[1] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(1,i+1)[1] = (*tmp_val/2);      //store decoded value in image

                p = codec_alg.decode_string(p, tmp_val, 0);
                mapped_values.push_back(*tmp_val);          //store mapped value in vector

                newImage.at<Vec3b>(0,i)[2] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(0,i+1)[2] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(1,i)[2] = (*tmp_val/2);      //store decoded value in image
                newImage.at<Vec3b>(1,i+1)[2] = (*tmp_val/2);      //store decoded value in image

                // cout << "DECODED VALUE: " << *tmp_val/2 << endl;
                // cout <<"--------------------------------------------"<<endl;
            }

            //decode remaining values
            for(uint32_t i = 2; i < rows; i+=2){
                for(uint32_t j = 2; j < cols; j+=2){
                    for(uint8_t n = 1; n<3; n++){
                        p = codec_alg.decode_string(p, tmp_val, 0);
                        mapped_values.push_back(*tmp_val);         //store mapped value in vector  

                        if(*tmp_val % 2 == 0){
                            *tmp_val = (*tmp_val/2);                //undo mapping
                        }else{
                            *tmp_val = ((*tmp_val-1)/-2);           //undo mapping
                        }

                        newImage.at<Vec3b>(i,j)[n] = (*tmp_val) + calculate_prediction(mode, newImage.at<Vec3b>(i,j-1)[n], newImage.at<Vec3b>(i-1,j)[n], newImage.at<Vec3b>(i-1,j-1)[n]);     //store decoded value in image
                        newImage.at<Vec3b>(i,j+1)[n] = (*tmp_val) + calculate_prediction(mode, newImage.at<Vec3b>(i,j-1)[n], newImage.at<Vec3b>(i-1,j)[n], newImage.at<Vec3b>(i-1,j-1)[n]);     //store decoded value in image
                        newImage.at<Vec3b>(i+1,j)[n] = (*tmp_val) + calculate_prediction(mode, newImage.at<Vec3b>(i,j-1)[n], newImage.at<Vec3b>(i-1,j)[n], newImage.at<Vec3b>(i-1,j-1)[n]);     //store decoded value in image
                        newImage.at<Vec3b>(i+1,j+1)[n] = (*tmp_val) + calculate_prediction(mode, newImage.at<Vec3b>(i,j-1)[n], newImage.at<Vec3b>(i-1,j)[n], newImage.at<Vec3b>(i-1,j-1)[n]);     //store decoded value in image
        
                    }
                }
            }

            if(prints){
                int i = 1;
                int j = 1;
                //print used points for prediction
                cout << "PIXEL (" << i << "," << j << ")\t1: " << (int)newImage.at<Vec3b>(i,j)[0] << " 2: " << (int)newImage.at<Vec3b>(i,j)[1] << " 3: " << (int)newImage.at<Vec3b>(i,j)[2] << endl;
                cout << "PIXEL (" << i << "," << j-1 << ")\t1: " << (int)newImage.at<Vec3b>(i,j-1)[0] << " 2: " << (int)newImage.at<Vec3b>(i,j-1)[1] << " 3: " << (int)newImage.at<Vec3b>(i,j-1)[2] << endl;
                cout << "PIXEL (" << i-1 << "," << j << ")\t1: " << (int)newImage.at<Vec3b>(i-1,j)[0] << " 2: " << (int)newImage.at<Vec3b>(i-1,j)[1] << " 3: " << (int)newImage.at<Vec3b>(i-1,j)[2] << endl;
                cout << "PIXEL (" << i-1 << "," << j-1 << ")\t1: " << (int)newImage.at<Vec3b>(i-1,j-1)[0] << " 2: " << (int)newImage.at<Vec3b>(i-1,j-1)[1] << " 3: " << (int)newImage.at<Vec3b>(i-1,j-1)[2] << endl;
                //print prediction
                cout << "PREDICTION FOR Y: " << calculate_prediction(mode, newImage.at<Vec3b>(i,j-1)[0], newImage.at<Vec3b>(i-1,j)[0], newImage.at<Vec3b>(i-1,j-1)[0]) << endl;
                //print prediction for Cb
                cout << "PREDICTION FOR Cb: " << calculate_prediction(mode, newImage.at<Vec3b>(i,j-1)[1], newImage.at<Vec3b>(i-1,j)[1], newImage.at<Vec3b>(i-1,j-1)[1]) << endl;
                //print prediction for Cr
                cout << "PREDICTION FOR Cr: " << calculate_prediction(mode, newImage.at<Vec3b>(i,j-1)[2], newImage.at<Vec3b>(i-1,j)[2], newImage.at<Vec3b>(i-1,j-1)[2]) << endl;

                int pos_x = 0;
                int pos_y = 0;
                cout << "COL:" << pos_x << endl;
                //print index column of newImage
                for(uint32_t i = 0; i < rows; i++){
                    cout << "("<< i <<","<<pos_x <<") " << (int)newImage.at<Vec3b>(i,pos_x)[0] << " " << (int)newImage.at<Vec3b>(i,pos_x)[1] << " " << (int)newImage.at<Vec3b>(i,pos_x)[2] << endl;
                } 

                cout << "ROW :"<< pos_y << endl;
                //print first row of newImage
                for(uint32_t i = 0; i < cols; i++){
                    cout << "("<<pos_y <<","<< i <<") "<< (int)newImage.at<Vec3b>(pos_y,i)[0] << " " << (int)newImage.at<Vec3b>(pos_y,i)[1] << " " << (int)newImage.at<Vec3b>(pos_y,i)[2] << endl;
                }
            }           

            *ptr = newImage;
            return p;
        }
};

#endif