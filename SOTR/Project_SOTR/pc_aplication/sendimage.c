/* *******************************************************************
 *                           BRUNO LEMOS                             *
 *                           TIAGO MARQUES    
 * pronto                                                              *
 *********************************************************************/

// C library headers
#include <stdio.h>
#include <string.h>
#include<time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include<sys/time.h>

#define IMGWIDTH 128
const char start[] = "START";
// #define CLOCKS_PER_SEC 1000000
#define START_SIZE 5

int readImage(uint8_t *img1D, char *filename, int SIZE);
int main()
{

  // -------------------------------- configuration --------------------------------

  // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
  int serial_port = open("/dev/ttyACM0", O_RDWR);

  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if (tcgetattr(serial_port, &tty) != 0)
  {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    return 1;
  }

  tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
  tty.c_cflag |= CS8;            // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;                                                        // Disable echo
  tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
  tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
  tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 115200
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
  {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    return 1;
  }

  // Variables
  char read_buf[256];                             // Buffer to store the data received
  int num_bytes;                                  // Number of bytes read by the read() system call
  int error = 0;                                  // Error flag
  struct timeval start, end;                      // timer
  double elapsedTime;                             // time in seconds
  int maxImages= 100;                              // number of images to send
  int index = 0;                                  // index 64 by 64 bytes of an image
  int ret = 0;                                    // return value of write
  int flag = 0;                                   // flag to check if the image was sent
  int imageindex = 0;                             // index of the image
  char image_name [50];                           // name of the image
  uint8_t img1D[IMGWIDTH*IMGWIDTH];               // image in 1D
  int totalimagessent = maxImages;                // total number of images sent


  gettimeofday(&start, NULL);  // start timer

  printf("Starting to send images ...\n"); // start send images


  
  for(int i = 0; i < maxImages; i++){                 // loop to send number of 'maxImages' images
    imageindex = 0;                                   // reset image index
    sprintf(image_name, "imagens/img%d.raw", i);      // create image name

    error = readImage(img1D, image_name,IMGWIDTH);   // read image
    if (error == -2){                                // very if image was not read
      totalimagessent = i;
      break;
    }else{
      // sent success
      // no code ...
    }

    while(index < ((IMGWIDTH*IMGWIDTH)/64) ){                         // loop to send 64 bytes of an image at a time
      ret = write(serial_port, &img1D[imageindex], 64);               // send 64 bytes of an image
      imageindex+=64;                                                 // update image index
      flag = 0;                                                       // reset flag
      // us sleep 0.01s
      // usleep(10000);
      while(flag == 0){                                              // loop to wait for the image to be sent
        num_bytes = read(serial_port, &read_buf, sizeof(read_buf));  // read serial port
        for (int i = 0; i < num_bytes; i++) {                        // loop to print the received data
          if (read_buf[i]== 0x24) {                                  // receive "." is ACK
            flag = 1;
          }else{
            putchar(read_buf[i]);                                    // received string is not ACK so print it
          }
        }
        memset(read_buf, 0, sizeof(read_buf));                       // reset read buffer
      }

      if (num_bytes <= 0) {                                          // if no data was read
        // printf("\tError sending image %3d\n",index);
      }else{                                                         // if data was read
        // printf("\tSucess\n");
      }
      index++;                                                       // update index
    }
    index = 0;                                                       // reset index
  }

  
  gettimeofday(&end, NULL);                                 // end timer
  elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0;       // sec to ms
  elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0;    // us to ms
  printf("\n\n\n"); 
  if (totalimagessent != maxImages){                        // if not all images were sent
    printf("Missing img%d.raw\n", totalimagessent);         // print missing image
  }
  else{                                                                               // if all images were sent                  
    printf("Total image sent %d\n",maxImages);                                        // print total images sent
    printf("Execution time: %f s\n", elapsedTime/1000);                               // print exec time in seconds
    printf("Execution time per image: %f s\n", (elapsedTime/1000)/(totalimagessent)); // print exec time per image in seconds
  }


  return 0;
};

// function to print array of chars in hex
void print_hex(char *data, int len){
  printf("\nReceived message: ");
  for (int i = 0; i < len; i++)
  {
    printf("%02x ", data[i]);
  }
  printf("\n\r");
}

// IMGWIDTH*IMGWIDTH as 1D array of uint8_t in parameter , and change the values of the array
int readImage(uint8_t *img1D, char *filename, int SIZE){
  uint8_t img1D2[SIZE*SIZE];
    FILE *file = fopen(filename, "rb");

    if (!file || fread( img1D2, sizeof(uint8_t), SIZE*SIZE, file) != SIZE*SIZE) {
        // fprintf(stderr, "Failure to read file magic.\n");
        return -2;
    }

    // Change the structure of the image
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            img1D[i + j * SIZE] = img1D2[i * SIZE + j];
        }
    }
    return 0;
}