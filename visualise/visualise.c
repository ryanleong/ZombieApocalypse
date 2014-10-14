#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>

/**
 * Fills an image pixel with a color based on properties of the cell and entity.
 * Humans are green, infected are blue and zombies are red.
 */
void setRGB(png_byte * ptr, char type, char gender, int age) {
	// TODO make the color depend on age; this is low priority
	// I tried it but the difference was not noticeable

	switch (type) {
	case '_':
		ptr[0] = 255;
		ptr[1] = 255;
		ptr[2] = 255;
		break;
	case 'H':
		if (gender == 'F') {
			ptr[0] = 0;
			ptr[1] = 200;
		} else if (gender == 'f') {
			ptr[0] = 100;
			ptr[1] = 200;
		} else {
			ptr[0] = 0;
			ptr[1] = 150;
		}
		ptr[2] = 0;
		break;
	case 'I':
		if (gender == 'F') {
			ptr[0] = 0;
			ptr[2] = 200;
		} else if (gender == 'f') {
			ptr[0] = 100;
			ptr[2] = 200;
		} else {
			ptr[0] = 0;
			ptr[2] = 150;
		}
		ptr[1] = 0;
		break;
	case 'Z':
		ptr[0] = 200;
		ptr[1] = 0;
		ptr[2] = 0;
		break;
	}
}

/**
 * Generates an image for the world mapping each cell to a pixel.
 */
int printWorld(FILE * in, FILE * out) {
	int width;
	int height;
	int time;

	fscanf(in, "Width %d; Height %d; Time %d\n", &width, &height, &time);

	int code = 0;
	png_structp png_ptr;
	png_infop info_ptr;

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		code = 1;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, out);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, width, height, 8,
	PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
	PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for row pointers
	png_bytep * image = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (int y = 0; y < height; y++) {
		// Allocate memory for one row (3 bytes per pixel - RGB)
		image[y] = (png_bytep) malloc(3 * width * sizeof(png_byte));
		for (int x = 0; x < width; x++) {
			setRGB(image[y] + x * 3, '_', '_', 0);
		}
	}

	// Prepare image data; this can be done in parallel
	do {
		int x;
		int y;
		char type;
		char gender;
		int age;
		int whatsGoingOn = fscanf(in, "[%d %d] %c %c %d\n", &x, &y, &type,
				&gender, &age);
		if (whatsGoingOn == EOF) {
			break;
		}
		setRGB(image[y] + x * 3, type, gender, age);
	} while (1);
	png_write_image(png_ptr, image);

	// End write
	png_write_end(png_ptr, NULL);

	finalise: if (info_ptr != NULL)
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL)
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
	if (image != NULL) {
		for (int i = 0; i < height; i++) {
			free(image[i]);
		}
		free(image);
	}

	return code;
}

int main(int argc, char ** argv) {
	if (argc != 2 && argc != 3) {
		printf("I want input file and possibly output file.\n");
		exit(1);
	}

	FILE * in = fopen(argv[1], "r");

	char * outName;
	if (argc == 3) {
		outName = argv[2];
	} else {
		outName = argv[1];
		int len = strlen(outName);
		outName[len - 3] = 'p';
		outName[len - 2] = 'n';
		outName[len - 1] = 'g';
	}

	FILE * out = fopen(outName, "wb");

	printWorld(in, out);

	fclose(out);
	fclose(in);

	exit(0);
}
