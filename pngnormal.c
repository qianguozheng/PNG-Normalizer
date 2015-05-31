#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <zlib.h>
//#include <zlib/crc32.h>

char oldpng[50*1024]; //50KB
char newpng[50*1024]; //50KB

//png is big-endian byte-orlder
//unsigned short pngheader[8]={0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
unsigned short pngheader[8]={0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
//unsigned short pngheader[5]={0x5089,0x474E,0x0A0D,0x0A1A};


/*PyDoc_STRVAR(compress__doc__,
"compress(string[, level]) -- Returned compressed string.\n"
"\n"
"Optional arg level is the compression level, in 0-9.");
*/

//static PyObject *
//PyZlib_compress(PyObject *self, PyObject *args)
char *compress_own(char *data, int *output_len, int input_len)
{
    //PyObject *ReturnVal = NULL;
    char *input, *output;
    int length, level=Z_DEFAULT_COMPRESSION, err;
    z_stream zst;

    /* require Python string object, optional 'level' arg */
    /*if (!PyArg_ParseTuple(args, "s#|i:compress", &input, &length, &level))
        return NULL;*/
	//level = 6;
	length = input_len;
	input = data;
    zst.avail_out = length + length/1000 + 12 + 1;

	printf("zst.avail_out=%d\n", zst.avail_out);
    output = (char*)malloc(zst.avail_out);
    if (output == NULL) {
        //PyErr_SetString(PyExc_MemoryError,
        printf("Can't allocate memory to compress data\n");
        return NULL;
    }

    /* Past the point of no return.  From here on out, we need to make sure
       we clean up mallocs & INCREFs. */

    //zst.zalloc = (alloc_func)NULL;
    //zst.zfree = (free_func)Z_NULL;
    zst.zalloc = NULL;
    zst.zfree = Z_NULL;
    zst.next_out = (Byte *)output;
    zst.next_in = (Byte *)input;
    zst.avail_in = length;
    err = deflateInit(&zst, level);

    switch(err) {
    case(Z_OK):
        break;
    case(Z_MEM_ERROR):
        //PyErr_SetString(PyExc_MemoryError,
        printf("Out of memory while compressing data\n");
        goto error;
    case(Z_STREAM_ERROR):
        //PyErr_SetString(ZlibError,
        printf("Bad compression level\n");
        goto error;
    default:
        deflateEnd(&zst);
        //zlib_error(zst, err, "while compressing data");
        printf("while compressing data\n");
        goto error;
    }

    //Py_BEGIN_ALLOW_THREADS;
    err = deflate(&zst, Z_FINISH);
    //Py_END_ALLOW_THREADS;

    if (err != Z_STREAM_END) {
        //zlib_error(zst, err, "while compressing data");
        printf("while compressing data\n");
        deflateEnd(&zst);
        goto error;
    }

    err=deflateEnd(&zst);
    if (err == Z_OK)
    {    //ReturnVal = PyString_FromStringAndSize((char *)output,
        //                                       zst.total_out);
        *output_len = zst.total_out;
        //return zst.next_out;
	return output;
    }
    else
		printf("while finishing compression\n");
        //zlib_error(zst, err, "while finishing compression");

 error:
    free(output);

    return NULL;
}

/*
PyDoc_STRVAR(decompress__doc__,
"decompress(string[, wbits[, bufsize]]) -- Return decompressed string.\n"
"\n"
"Optional arg wbits is the window buffer size.  Optional arg bufsize is\n"
"the initial output buffer size.");
*/

//static PyObject *
//PyZlib_decompress(PyObject *self, PyObject *args)
char *decompress(char *compressed, int wsize, int input_len, int bufsize)
{
	printf("wsize=%d, input_len=%d, bufsize=%d\n", wsize, input_len, bufsize);
    Byte *result_str;
    Byte *input = compressed;
    int length, err;
    //int wsize=DEF_WBITS;
    //Py_ssize_t r_strlen=DEFAULTALLOC;
    int r_strlen = bufsize;
    length = input_len;
    z_stream zst;

    //if (!PyArg_ParseTuple(args, "s#|in:decompress",
    //                      &input, &length, &wsize, &r_strlen))
    //    return NULL;

    if (r_strlen <= 0)
        r_strlen = 1;

    zst.avail_in = length;
    zst.avail_out = r_strlen;

    //if (!(result_str = PyString_FromStringAndSize(NULL, r_strlen)))
    //    return NULL;
    result_str = (char *)malloc(r_strlen);
	memset(result_str, 0, r_strlen)
	;
    zst.zalloc = (alloc_func)NULL;
    zst.zfree = (free_func)Z_NULL;
    zst.next_out = (Byte *)(result_str);
    zst.next_in = (Byte *)input;
    err = inflateInit2(&zst, wsize);

    switch(err) {
    case(Z_OK):
        break;
    case(Z_MEM_ERROR):
        //PyErr_SetString(PyExc_MemoryError,
        printf("Out of memory while decompressing data\n");
        goto error;
    default:
        inflateEnd(&zst);
        //zlib_error(zst, err, "while preparing to decompress data");
        printf("while preparing to decompress data\n");
        goto error;
    }

    do {
        //Py_BEGIN_ALLOW_THREADS
        err=inflate(&zst, Z_FINISH);
        //Py_END_ALLOW_THREADS
		printf(" inflate: err=%d\n", err);
        switch(err) {
        case(Z_STREAM_END):
            break;
        case(Z_BUF_ERROR):
            /*
             * If there is at least 1 byte of room according to zst.avail_out
             * and we get this error, assume that it means zlib cannot
             * process the inflate call() due to an error in the data.
             */
            if (zst.avail_out > 0) {
                //zlib_error(zst, err, "while decompressing data");
                printf("while decompressing data\n");
                inflateEnd(&zst);
                goto error;
            }
            /* fall through */
        case(Z_OK):
            /* need more memory */
            /*if (_PyString_Resize(&result_str, r_strlen << 1) < 0) {
                inflateEnd(&zst);
                goto error;
            }*/
            //zst.next_out = (unsigned char *)PyString_AS_STRING(result_str)
            zst.next_out = (unsigned char *)(result_str) \
                + r_strlen;
            zst.avail_out = r_strlen;
            r_strlen = r_strlen << 1;
            break;
        default:
            inflateEnd(&zst);
            //zlib_error(zst, err, "while decompressing data");
            printf("%d: while decompressing data, %d\n", __LINE__, err);
            goto error;
        }
    } while (err != Z_STREAM_END);

    err = inflateEnd(&zst);
    if (err != Z_OK) {
        //zlib_error(zst, err, "while finishing data decompression");
        printf("while finishing data decompression\n");
        goto error;
    }

    //_PyString_Resize(&result_str, zst.total_out);
    return result_str;

 error:
    //Py_XDECREF(result_str);
    if (result_str)
    {
		free(result_str);
	}
    return NULL;
}


static int write_png_data(const char *filename, const char *pngdata, uint32_t size)
{
	size_t amount = 0, written = 0;
	FILE *fp = fopen(filename, "wb+");
	
	while(written < size)
	{
		amount = fwrite((pngdata+written), 1, size-written, fp);
		written += amount;
		if (written == size)
		{
			printf("written = size\n");
			break;
		}
		//printf("amount=%ld\n", amount);
	}
	
	fflush(fp);
	fclose(fp);
	
	return written;
}

int pngnormal(long int size)
{
	printf("OldPNG size: %ldKB\n", size/1024);
	
	int i = 0;
	char *poldpng = oldpng;
	char *pnewpng = newpng;
	int total_len = 0;
	int old_len = size;
	int new_len = 0;
	
	int chunkLength = 0;
	char chunkType[4] = {0};
	char *chunkData = NULL;
	char *newdata = NULL;
	char * compressed_data = NULL;
	int chunkCRC = 0;
	int width = 0;
	int height = 0;
	int bufsize = 0;
	
	/*printf("PNG[0]=%02x, PNG[1]=%02x, PNG[2]=%02x, PNG[3]=%02x\n"
			"PNG[4]=%02x, PNG[5]=%02x, PNG[6]=%02x, PNG[7]=%02x\n",
			poldpng[0], poldpng[1], poldpng[2], poldpng[3],
			poldpng[4], poldpng[5], poldpng[6], poldpng[7]);*/
	for (i = 1; i< 8; i++)
	{
		if (poldpng[i] != pngheader[i])
		{
			printf("i=%d failed, poldpng=%02x\n", i, poldpng[i]);
			return -1;
		}
	}
	printf("Valid PNG File\n");
	
	//Copy first 8 bytes to Newpng;
	memcpy(pnewpng, poldpng, 8);
	
	printf("PNG[0]=%02x, PNG[1]=%02x, PNG[2]=%02x, PNG[3]=%02x\n"
			"PNG[4]=%02x, PNG[5]=%02x, PNG[6]=%02x, PNG[7]=%02x\n",
			pnewpng[0], pnewpng[1], pnewpng[2], pnewpng[3],
			pnewpng[4], pnewpng[5], pnewpng[6], pnewpng[7]);
	new_len = 8;
	poldpng += new_len;
	pnewpng += new_len;
	total_len = new_len;
	while(new_len < old_len)
	{
		
		
		//Get chunkLength
		memcpy(&chunkLength, poldpng, 4);
		chunkLength = ntohl(chunkLength);
		printf("ChunkLength=%d\n", chunkLength);
		
		//Get chunkType
		memcpy(chunkType, poldpng+4, 4);
		
		printf("chunkType: %02x, %02x, %02x, %02x,\n "
				"%c%c%c%c\n", 
			chunkType[0], chunkType[1], chunkType[2], chunkType[3], 
			chunkType[0], chunkType[1], chunkType[2], chunkType[3]);
			
		//Get chunkData
		if (chunkLength > 0)
		{
			chunkData = (char *)malloc(chunkLength+1);
			memcpy(chunkData, poldpng+8, chunkLength);	
		}
		
		//Get chunkCRC
		memcpy(&chunkCRC, poldpng+chunkLength+8, 4);
		chunkCRC = ntohl(chunkCRC);
		
		//Copy all the old data from oldpng to newpng
		//memcpy(pnewpng, poldpng, chunkLength+12);
		new_len += chunkLength + 12;
		poldpng += chunkLength + 12;
		
		printf("new_len=%d\n", new_len);
		//pnewpng += new_len;
		
		
		//Parsing the header chunk [IHDR]
		if (chunkType[0] == 'I' && chunkType[1] == 'H' &&
			chunkType[2] == 'D' && chunkType[3] == 'R')
		{
			printf("This is IHDR chunk\n");
			memcpy(&width, chunkData, 4);
			width = ntohl(width);
			memcpy(&height, chunkData+4, 4);
			height = ntohl(height);
			
			printf("Height x Width: %dx%d\n", height, width);
		}
		
		//Parsing the image chunk [IDAT]
		if (chunkType[0] == 'I' && chunkType[1] == 'D' &&
			chunkType[2] == 'A' && chunkType[3] == 'T')
		{
			printf("This is IDAT chunk\n");
			bufsize = width * height * 4 + height;
			char *decompressed_data = (char *)malloc(bufsize+1);
			memset(decompressed_data, 0, bufsize);
			//FIXME: if the Image is right or decompress occurred error, what should we do?
			memcpy(decompressed_data, decompress(chunkData, -8, chunkLength, bufsize), bufsize);
			
			//Swapping red & blue bytes for each pixel
			newdata = (char *)malloc(bufsize+1);
			int x, y, i, len_newdata = 0;
			
			memset(newdata, 0, bufsize);
			
			for (y=0; y< height; y++)
			{
				i = len_newdata;
				memcpy(newdata+len_newdata, decompressed_data+i, 1);
				len_newdata++;
				for (x=0; x< width; x++)
				{
					i = len_newdata;
					memcpy(newdata+len_newdata, decompressed_data+i+2, 1);
					len_newdata++;
					memcpy(newdata+len_newdata, decompressed_data+i+1, 1);
					len_newdata++;
					memcpy(newdata+len_newdata, decompressed_data+i+0, 1);
					len_newdata++;
					memcpy(newdata+len_newdata, decompressed_data+i+3, 1);
					len_newdata++;
				}						
			}
			
			//Compressing the image chunk
			if (chunkData)
			{
				free(chunkData);
				chunkData = NULL;
			}
			if (decompressed_data)
			{
				free(decompressed_data);
				decompressed_data = NULL;
			}
			
			
			chunkData = (char *) malloc(bufsize+1);
			memset(chunkData, 0, bufsize);
			memcpy(chunkData, newdata, bufsize);
			
			
			char *tmp = NULL;
			int output_len = -1;
			
			printf("bufsize=%d\n", bufsize);
			tmp = compress_own(newdata, &output_len, bufsize);
			
			compressed_data = (char *)malloc(output_len+1);
			memset(compressed_data, 0, output_len+1);
			/*char buf[50*1024];
			memset(buf, 0, 50*1024);
			output_len = 50*1024;
			printf("newdata=%p, bufsize=%d, buf=%p, output_len=%d\n", newdata, bufsize, buf, output_len);
			int ret = compress(buf, &output_len,newdata, bufsize);
			printf("ret=%d, output_len=%d\n", ret, output_len);*/
			compressed_data = (char *)malloc(output_len+1);
			memset(compressed_data, 0, output_len+1);
			memcpy(compressed_data, tmp, output_len);
			
			memset(chunkData, 0, chunkLength);
			memcpy(chunkData, compressed_data, output_len);
			chunkLength = output_len;
			chunkCRC = crc32(0, chunkType, 4);
			chunkCRC = crc32(chunkCRC, compressed_data, output_len);
			chunkCRC = (chunkCRC + 0x100000000) % 0x100000000;
			
			printf("output_len=%d\n", output_len);
		}
			
		//Removing CgBI chunk [CgBI]
		if (chunkType[0] != 'C' || chunkType[1] != 'g' ||
			chunkType[2] != 'B' || chunkType[3] != 'I')
		{
			
			printf("This NOT CgBI chunk\n");
			int tmp;
			tmp = htonl(chunkLength);
			memcpy(pnewpng, &tmp, 4);
			
			memcpy(pnewpng+4, chunkType, 4);
			
			if (chunkLength > 0)
			{
				memcpy(pnewpng+8, chunkData, chunkLength);
			}
			
			tmp = htonl(chunkCRC);
			memcpy(pnewpng+chunkLength+8, &tmp, 4);
			
			pnewpng += chunkLength + 12;
			total_len += chunkLength + 12;
			
			printf("total_len=%d\n", total_len);
			//if (chunkData)
				//free(chunkData);
		}
		//free data used in IDAT
		if (compressed_data)
		{
			free(compressed_data);
			compressed_data = NULL;
		}
		if (chunkData)
		{
			free(chunkData);
			chunkData = NULL;
		}
		//Stopping the PNG file parsing [IEND]
		if (chunkType[0] == 'I' && chunkType[1] == 'E' &&
			chunkType[2] == 'N' && chunkType[3] == 'D')
		{
			printf("This is IEND chunk\n");
			break;
		}
		//break;
	}
	
	write_png_data("test.png", newpng, total_len);
	
	return 0; 
}

/*  
 * @param:  filename, file to open
 * @return: size of file
 */
long int readpng(char *filename)
{
	FILE *fp = NULL;
	long int size = 0;
	
	fp = fopen(filename, "rb");
	if (NULL == fp)
	{
		perror("File open failed");
		return -1;
	}
	memset(oldpng, 0, sizeof(oldpng));
	memset(newpng, 0, sizeof(newpng));
	
	/*while((size = fread(oldpng, sizeof(oldpng), 1, fp)))
	{
		if (feof(fp))
		{
			printf("End OF FILE\n");
			break;
		}
		printf("Something happened, cause the file not read correctly\n");
	}*/
	
	while(!feof(fp))
	{
		size += fread(oldpng+size, 1, 1, fp);
	}
	
	printf("size=%ld, strlen(oldpng)=%d", size, strlen(oldpng));
	
	fclose(fp);
	return size;
}

int main(int argc, char *argv[])
{
	if (argc <= 1 || argc > 2)
	{
		printf("Usage: ./pngnormal <PNG_EXTRACT_FROM_IPA>.png\n");
		return -1;
	}
	
	/*printf("unsigned short:%d\n" ,sizeof(unsigned short));
	printf("short:%d\n" ,sizeof(short));
	printf("int:%d\n" ,sizeof(int));
	printf("char:%d\n" ,sizeof(char));*/
	pngnormal(readpng(argv[1]));
}
