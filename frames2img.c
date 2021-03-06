#include <stdlib.h>
#include <stdint.h>
#include <png.h>
#include <omp.h>

typedef uint8_t uint8;
typedef uint32_t uint32;

const int bytesPerPixel = 3;

void usage();
uint32 averagergb(png_image *, uint8 *);

int main(int argc, char **argv){
	if(argc == 1) usage();
	int height = 1080;
	int width = argc-1;
	int percent = 0;

	uint32 *frames = malloc(width*sizeof(uint32));

#pragma omp parallel for
	for(int i =1;i<argc;i++) {
		png_image image = {.version = PNG_IMAGE_VERSION, .opaque = NULL, };
		if(!png_image_begin_read_from_file(&image, argv[i])) usage();
		image.format = PNG_FORMAT_RGB;

		uint8 *buffer = malloc(PNG_IMAGE_SIZE(image));

		png_image_finish_read(&image, NULL, buffer, 0, NULL);

		frames[i-1] = averagergb(&image, buffer);

		free(buffer);
		png_image_free(&image);

		fprintf(stderr, "%d%% Complete\r", ++percent * 100 / argc);
		fflush(stderr);
	}
	uint8 *imgPointer = malloc(height*width*3*sizeof(uint8 *));

	for(int i=0; i<height; i++){
		for(int j=0; j<width; j++){
			int index = i+j*height;
			imgPointer[j*3+i*width*3+2] = ((frames[j] >> 16)&0xff);
			imgPointer[j*3+i*width*3+1] = ((frames[j] >> 8)&0xff);
			imgPointer[j*3+i*width*3+0] = ((frames[j])&0xff);
		}
	}

	png_image output = {0};
	output.format = PNG_FORMAT_RGB;
	output.version = PNG_IMAGE_VERSION;
	output.width = width;
	output.height = height;

	png_image_write_to_stdio(&output, stdout, 0, imgPointer, 0, NULL);

	free(imgPointer);
	free(frames);
}

void
usage() {
	fprintf(stderr, "USAGE: ./frames2img PNGs...");
	exit(1);
}

uint32
averagergb(png_image *img, uint8 *buffer) {
	png_image image = *img;
	double sumR = 0;
	double sumG = 0;
	double sumB = 0;

	double fact = image.width * image.height;

	for(int y =0;y<image.height;y++) {
		for(int x=0;x<image.width;x++) {
			sumR += buffer[3*(x+y*image.width)+2] / fact;
			sumG += buffer[3*(x+y*image.width)+1] / fact;
			sumB += buffer[3*(x+y*image.width)+0] / fact;
		}
	}

	return (((uint8) sumR & 0xff) << 16) |
		(((uint8)sumG & 0xff) << 8) |
		((uint8) sumB & 0xff);
}
