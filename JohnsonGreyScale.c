////////////////////////////////////////////////////////////////////////////////
//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>




////////////////////////////////////////////////////////////////////////////////
//MACRO DEFINITIONS

#pragma warning(disable: 4996)

//problem assumptions
#define BMP_HEADER_SIZE_BYTES 14
#define BMP_DIB_HEADER_SIZE_BYTES 40
#define MAXIMUM_IMAGE_SIZE 256
static const int REDCON = 299;
static const int GRNCON = 587;
static const int BLUCON = 114;

//bmp compression methods
//none:
#define BI_RGB 0


////////////////////////////////////////////////////////////////////////////////
//DATA STRUCTURES

struct BMP_Header {
    char signature[2];		//ID field
    int size;		//Size of the BMP file
    short reserved1;		//Application specific
    short reserved2;		//Application specific
    int offset_pixel_array;  //Offset where the pixel array (bitmap data) can be found
};


struct DIB_Header {
    int headerSize;
    int width;
    int height;
    short planes;
    short bitsPerPixel;
    int compression;
    int imageSize;
    int xPixPMeter;
    int yPixPMeter;
    int colors;
    int importantColorCount;
    
};


struct Pixel_Data {
    unsigned char R;
    unsigned char G;
    unsigned char B;
    
};
int width, height;
typedef struct threadData {
    int tNUm;
    int width;
    int height;
    struct Pixel_Data pixToPass[MAXIMUM_IMAGE_SIZE][MAXIMUM_IMAGE_SIZE] ;
} threadData;

////////////////////////////////////////////////////////////////////////////////
//MAIN PROGRAM CODE

void* boxBlur(void* arguments);


struct Pixel_Data toPrint[256][256];

int main(int argc, char* argv[]) {
    int numThreadsAvail = (int)sysconf(_SC_NPROCESSORS_ONLN);
    printf("Thread available: %d\n", numThreadsAvail);
    char* inputFile= argv[1];
    char* outputFile= argv[2];
    
    //sample code to read first 14 bytes of BMP file format
    FILE* file = fopen(inputFile, "rb");
    struct BMP_Header header;
    struct DIB_Header DIB;
    
    int i, ii, j;
    
   
    //read bitmap file header (14 bytes)
    fread(&header.signature, sizeof(char)*2, 1, file);
    fread(&header.size, sizeof(int), 1, file);
    fread(&header.reserved1, sizeof(short), 1, file);
    fread(&header.reserved2, sizeof(short), 1, file);
    fread(&header.offset_pixel_array, sizeof(int), 1, file);
    
    fread(&DIB.headerSize, sizeof(int), 1, file);
    fread(&DIB.width, sizeof(int), 1, file);
    width = DIB.width;
    fread(&DIB.height, sizeof(int), 1, file);
    height = abs(DIB.height);
    fread(&DIB.planes, sizeof(short), 1, file);
    fread(&DIB.bitsPerPixel, sizeof(short), 1, file);
    fread(&DIB.compression, sizeof(int), 1, file);
    fread(&DIB.imageSize, sizeof(int), 1, file);
    fread(&DIB.xPixPMeter, sizeof(int), 1, file);
    fread(&DIB.yPixPMeter, sizeof(int), 1, file);
    fread(&DIB.colors, sizeof(int), 1, file);
    fread(&DIB.importantColorCount, sizeof(int), 1, file);
    
    
    char padArray[width*((3*(sizeof(char))))%4];
    int padNum =(width*(3*(sizeof(char))))%4;
    
    //reading in my pixel data
    fseek(file, header.offset_pixel_array, SEEK_SET);
    threadData data;
    data.height = height;
    data.width = width;
    for(i = 0; i< height; i++){
        for(j = 0; j< width; j++){
            
            
            fread(&data.pixToPass[i][j].B, sizeof(char), 1, file);
            fread(&data.pixToPass[i][j].G, sizeof(char), 1, file);
            fread(&data.pixToPass[i][j].R, sizeof(char), 1, file);
        }
        
        //padding when needed
        for(ii = 0; ii< padNum; ii++){
            fread(&padArray[i], sizeof(char), 1, file);
        }
        
    }
    
    
    printf("Width: %d Height: %d \n",DIB.width, DIB.height);
    printf("signature: %c%c\n", header.signature[0], header.signature[1]);
    printf("size: %d\n", header.size);
    printf("reserved1: %d\n", header.reserved1);
    printf("reserved2: %d\n", header.reserved2);
    printf("offset_pixel_array: %d\n", header.offset_pixel_array);
    
    
    
    
    ///////////////////////////////////////////multi-threading my blur function//////////////////////////////
    threadData dataArray[4] = {data, data, data, data};
    pthread_t tids[numThreadsAvail];
    for (int i = 0; i < numThreadsAvail; i++) {
        dataArray[i].tNUm = i;
        
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[i], &attr, boxBlur, &dataArray[i]);
        printf("threadID: %d\n",(int)tids[i]);
    }
    
    
    for (i = 0; i < numThreadsAvail; ++i) {
        pthread_join(tids[i], NULL);
    }
    
    ///////////////////////////////////////////multi-threading my blur function//////////////////////////////
    
    fclose(file); //closing my input file
    
    //opening my output file and writing all of the header, DIB and pixel info
    
    file = fopen(outputFile,"wb");
    
    fwrite(&header.signature, sizeof(char)*2, 1, file);
    fwrite(&header.size, sizeof(int), 1, file);
    fwrite(&header.reserved1, sizeof(short), 1, file);
    fwrite(&header.reserved2, sizeof(short), 1, file);
    fwrite(&header.offset_pixel_array, sizeof(int), 1, file);
    
    fwrite(&DIB.headerSize, sizeof(int), 1, file);
    fwrite(&DIB.width, sizeof(int), 1, file);
    fwrite(&DIB.height, sizeof(int), 1, file);
    fwrite(&DIB.planes, sizeof(short), 1, file);
    fwrite(&DIB.bitsPerPixel, sizeof(short), 1, file);
    fwrite(&DIB.compression, sizeof(int), 1, file);
    fwrite(&DIB.imageSize, sizeof(int), 1, file);
    fwrite(&DIB.xPixPMeter, sizeof(int), 1, file);
    fwrite(&DIB.yPixPMeter, sizeof(int), 1, file);
    fwrite(&DIB.colors, sizeof(int), 1, file);
    fwrite(&DIB.importantColorCount, sizeof(int), 1, file);
    
    
    fseek(file, header.offset_pixel_array, SEEK_SET);
    for(i = 0; i< height; i++){
        for(j = 0; j< width; j++){
            
            fwrite(&toPrint[i][j].B, sizeof(char), 1, file);
            fwrite(&toPrint[i][j].G, sizeof(char), 1, file);
            fwrite(&toPrint[i][j].R, sizeof(char), 1, file);
            // printf("Red: %d Green: %d Blue: %d\n",pixel[i][j].R, pixel[i][j].G, pixel[i][j].B);
        }
        
        for(ii = 0; ii< padNum; ii++){
            fwrite(&padArray[i], sizeof(char), 1, file);
        }
    }
    
    fclose(file);
    return 0;
    
}



//my blur function
void* boxBlur(void* arguments){
    struct threadData *args = (struct threadData* )arguments;
    
    int h = args->height;
    int w = args->width;
    int threadNum = args->tNUm;
    printf("Thread %d\n",threadNum);
    struct Pixel_Data pixArray[256][256];
    struct Pixel_Data pixelBlur[w][h];
    int i, j;
    
    //these two lines are how I divide the work between the 4 processors
    int begin = (h*threadNum)/4;
    int end = (h*(threadNum + 1))/4;
    
    for(i = 0; i< h; i++){
        for(j = 0; j< w; j++){
            pixArray[i][j] = args->pixToPass[i][j];
        }
    }
    
    //taking averages with conditionals that depend on the place the pixel is in the array.
    for(i = begin; i< end; i++){
        for(j = 0; j< w; j++){

			pixelBlur[i][j].R = ((pixArray[i][j].R * REDCON + pixArray[i][j].G * GRNCON + pixArray[i][j].B * BLUCON)/1000);
			pixelBlur[i][j].G = ((pixArray[i][j].R * REDCON + pixArray[i][j].G * GRNCON + pixArray[i][j].B * BLUCON)/1000);
			pixelBlur[i][j].B = ((pixArray[i][j].R * REDCON + pixArray[i][j].G * GRNCON + pixArray[i][j].B * BLUCON)/1000);

            
        }
    }
    
//putting blurred pixel into another array to print. I know this is a bit ugly, but I ran out of time to make it nicer. It does the trick.
    for(i = begin; i< end; i++){
        for(j = 0; j< w; j++){
            toPrint[i][j] = pixelBlur[i][j];
        }
    }
    
    pthread_exit(0);
}
