// Indiaminah Lawrence



#include <stdio.h>
#include <stdlib.h>

struct jpgHeader{
	unsigned short fileMarker; // byte 0
	unsigned short APPMarker; // byte 2
	unsigned short APP1Block; // byte 4
	char exifString[5]; // byte 6	
	char endianess[3]; // byte 12
	};

struct tiff {
	unsigned short tag;       // byte 0 1
	unsigned short dataType;      // byte 2 3
	unsigned short numOfItems;    // byte 4 5
	unsigned short nullHold;      // byte 6 7  and 10 11
	unsigned short offsetOrValue; // byte 8 9	
	};


int main(int argc, char **argv){
	unsigned short num;
	unsigned short tagList[] = {0x010f, 0x0110, 0x829a, 0x829d, 0x8827, 0x9003, 0x920a, 0xa002, 0xa003};
	FILE *f = fopen(argv[1], "r");
	struct jpgHeader start;
	struct tiff find;
	int i; // for loop incrementer 
	long int goBack; // return to pointer location	
	fseek(f, 0, SEEK_END);
	long int endOfFile = ftell(f); // get byte number for end of file
	fseek(f, 0, SEEK_SET);
		

	if(fread(&start.fileMarker, 2, 1, f) != 1){ //Start of file marker
		fprintf(stderr, "Error while reading file marker\n");
		return 1;
	}
	
	if(fread(&start.APPMarker, 2, 1, f) != 1){ //APP1 marker
		fprintf(stderr, "Error while reading APP1 marker\n");
		return 1;
	}
	if(start.APPMarker != 0xe1ff){ // check fo APP1
		fprintf(stderr, "Error, exifviewer only works with APP1. Program terminated.\n");
		return 1;
	}	
	fread(&start.APP1Block, 2, 1, f);
	if(fread(&start.exifString, 1, 4, f) != 4){ //Exif string
		fprintf(stderr, "Error while reading Exif string.\n");
		return 1;
	}
	start.exifString[4] = '\0';
	if(strcmp(start.exifString, "Exif")){
		fprintf(stderr, "Error. Exif tag not found.\n");
		return 1;
	}
	fseek(f, 2, SEEK_CUR);
	fread(&num, 2, 1, f);
	if(num != 0x4949){		
		fprintf(stderr, "Error. Endianess not supported.\n");
		return 1;	
	}
	
	// move to postion of first tag
	fseek(f, 8, SEEK_CUR);

	for(i = 0; i < (sizeof(tagList)/sizeof(unsigned short)); i++){

		fread(&find.tag, 2, 1, f);

		// find the next tag in the list
		if(find.tag != tagList[i]){
			if(ftell(f) == endOfFile){
				printf("Tag does not exist\n");
				continue;
			}
			i--;
			continue;
	
		}				
	// get info
	fread(&find.dataType, 2, 1, f);
	fread(&find.numOfItems, 2, 1, f);
	fread(&find.nullHold, 2, 1, f);
	fread(&find.offsetOrValue, 2, 1, f);
	fread(&find.nullHold, 2, 1, f);
	fseek(f, 12, SEEK_SET);
	fseek(f, find.offsetOrValue, SEEK_CUR);

	// read integers
	if(find.dataType == 4){
		if(find.tag == 0xa002){
			printf("Width:\t\t %d pixels\n", find.offsetOrValue);
		} else
		if(find.tag == 0xa003){
			printf("Height:\t\t %d pixels\n", find.offsetOrValue);
		}
	} else
	// read ascii characters
	if(find.dataType == 2){	
		char tempStr[find.numOfItems + 1];
		fread(&tempStr, 1, find.numOfItems, f);
		tempStr[find.numOfItems] = '/0';

		if(find.tag == 0x9003){	
			printf("Date Taken:\t %s\n", tempStr);
		} else
		if(find.tag == 0x010f){
			printf("Manufacturer:\t %s\n", tempStr);
		} else
		if(find.tag == 0x0110){
			printf("Model:\t\t %s\n", tempStr);
		}
	} else 
	// read 16-bit integer
	if(find.dataType == 3){
		if(find.tag == 0x8827){
		printf("ISO:\t\t ISO %d\n", find.offsetOrValue);
		}
	} else 
	/// read integer fractions
	if(find.dataType == 5){
		unsigned int tempInt[3];
		fread(&tempInt, 4, 2, f);
		tempInt[2] = '\0';
		if(find.tag == 0x829d){
			float deci = (float)tempInt[0]/tempInt[1];
			printf("F-stop:\t\t f/%.1f\n", deci);
		} else 
		if(find.tag == 0x0920a){
			printf("Focal Length:\t %d mm\n", tempInt[0]/tempInt[1]);
		} else 
		{
			printf("Exposure Time:\t %d/%d second\n", tempInt[0],tempInt[1]);
		}

	}

	//return to start to find the next tag
	fseek(f, 22, SEEK_SET);		
	}
	
	return 0;
}
