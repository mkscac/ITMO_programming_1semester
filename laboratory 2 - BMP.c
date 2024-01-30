#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct imageInfo {
    unsigned long height;
    unsigned long width;
    unsigned long size;
    unsigned long image_offset;
    unsigned char bmp_header[54];
};


int countNeighbors(int **grid, int x, int y) { /// считает количество соседей
    int summa = 0;
    for(int i = x - 1; i <= x + 1; i++) {
        for(int j = y - 1; j <= y + 1; j++) {
            summa += grid[i][j];
        }
    }
    if(grid[x][y] == 1) {
        summa--;
    }
    return summa;
}


unsigned char *convert_to_bmp(int **matrix, unsigned long height, unsigned long length) {
    unsigned char *pixelData = (unsigned char*)malloc(height * length * 3); /// храним пиксели (3 бита отвечает за 1 пиксель)
    int index = 0;

    for(int i = height - 1; i >= 0; i--) {
        for(int j = 0; j < length; j++) {
            if(matrix[i][j] == 1) {
                pixelData[index] = 0; /// тк rgb - на 1 пиксель 3 бита, 0 - черный,
                pixelData[index + 1] = 0;
                pixelData[index + 2] = 0;
            } else {
                pixelData[index] = 255;
                pixelData[index + 1] = 255;
                pixelData[index + 2] = 255;
            }
            index += 3;
        }
    }
    return pixelData;
}


int** allocateArray(unsigned long height, unsigned long width) {
    /// выделяет память под игровое поле - двумерный масиив
    int** array = (int**)malloc(height * sizeof(int*));

    for(int i = 0; i < height; i++) {
        array[i] = (int*)malloc(width * sizeof(int));
        if(array[i] == NULL) {
            return NULL;
        }
    }
    return array;
}





int main(int argc, char* argv[]) {
    if(argc < 4) {
        printf("Недопустимое количество аргументов\n");
        return 1;
    }

    struct imageInfo info;
    FILE* MY_image;

    long dump_freq = 1;
    long max_iter = 1;
    char* dirName;
    int** cur_gen;
    int** next_gen;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "--input") == 0) {
            MY_image = fopen(argv[i + 1], "rb");
            if(MY_image == NULL) {
                printf("Не удалось открыть файл bmp");
                return (0);
            }
        }
        else if (strcmp(argv[i], "--output") == 0) {
            dirName = argv[i + 1];
        }
        else if (strcmp(argv[i], "--max_iter") == 0) {
            max_iter = strtol(argv[i + 1], NULL, 10);
        }
        else if (strcmp(argv[i], "--dump_freq") == 0) {
            dump_freq = strtol(argv[i + 1], NULL, 10);
        }
    }

    fread(info.bmp_header, sizeof(unsigned char), 54, MY_image);
    /// из литтл эндин переводим в бигэндинг
    info.image_offset = info.bmp_header[0xD] << 24 | info.bmp_header[0xC] << 16 | info.bmp_header[0xB] << 8 | info.bmp_header[0xA];
    info.size = info.bmp_header[0x5] << 24 | info.bmp_header[0x4] << 16 | info.bmp_header[0x3] << 8 | info.bmp_header[0x2];
    info.width = info.bmp_header[0x15] << 24 | info.bmp_header[0x14] << 16 | info.bmp_header[0x13] << 8 | info.bmp_header[0x12];
    info.height = info.bmp_header[0x19] << 24 | info.bmp_header[0x18] << 16 | info.bmp_header[0x17] << 8 | info.bmp_header[0x16];

    printf("смещение = %lu\n", info.image_offset);
    printf("размер = %lu\n", info.size);
    printf("высота = %lu\n", info.height);
    printf("ширина = %lu\n", info.width);
    printf("\n");


    cur_gen = allocateArray(info.height, info.width * 3); /// создаем игровое поле
    next_gen = allocateArray(info.height, info.width * 3);
    if((cur_gen == NULL) || (next_gen == NULL)){
        return 1;
    }


    fseek(MY_image, info.image_offset, SEEK_SET);
    char buffer[3]; /// 3 цвета - RGB - 3 байта цвета - red green blue

    for(int i = info.height - 1; i >= 0; i--) {
        for(int j = 0; j < info.width; j++) {
            buffer[0] = fgetc(MY_image); /// считываем изображение
            buffer[1] = fgetc(MY_image);
            buffer[2] = fgetc(MY_image);
            if (buffer[0] != 0 && buffer[1] != 0 && buffer[2] != 0) {
                cur_gen[i][j] = 0;
            } else {
                cur_gen[i][j] = 1;
            }
        }
    }


    char fileName[10];
    char directory[256];
    for(unsigned long i = 0; i < info.height; i++) {
        for(unsigned long j = 0; j < info.width; j++) {
            next_gen[i][j] = cur_gen[i][j];
        }
    }


    /// описание правил самой игры, 1 - закрашенная черная black клетка, 0 - белая
    unsigned char *pixelInfo;
    int countOfNeighbors;
    for(int gameIteration = 0; gameIteration < max_iter; gameIteration++) {
        for(unsigned long i = 1; i < info.height - 1; i++) {
            for(unsigned long j = 1; j < info.width - 1; j++) {
                countOfNeighbors = countNeighbors(cur_gen, i, j);

                if(cur_gen[i][j] == 0 && countOfNeighbors == 3) {
                    next_gen[i][j] = 1;
                }
                else if(cur_gen[i][j] == 1) {
                    if(countOfNeighbors < 2 || countOfNeighbors > 3) {
                        next_gen[i][j] = 0;
                    }
                }
            }
        }

        for(unsigned long i = 0; i < info.height; i++) {
            for(unsigned long j = 0; j < info.width; j++) {
                cur_gen[i][j] = next_gen[i][j];
            }
        }


        pixelInfo = convert_to_bmp(cur_gen, info.height, info.width); /// все биты изображение

        if (gameIteration % dump_freq == 0) {
            sprintf(fileName, "%d", gameIteration);
            strcpy(directory, dirName);
            strcat(directory, "/");
            strcat(directory, fileName);
            strcat(directory, ".bmp");

            FILE* new_bmp = fopen(directory, "wb");
            if (new_bmp != NULL) {
                printf("File %d создан\n", gameIteration);
            }
            else {
                printf("File %d не получилось открыть\n", gameIteration);
            }
            fwrite(info.bmp_header, 1, 54, new_bmp); /// дампинг (скачиваем)
            fwrite(pixelInfo, 1, 3 * info.width * info.height, new_bmp);
        }
    }

    free(cur_gen);
    free(next_gen);
    free(pixelInfo);
    return 0;
}

