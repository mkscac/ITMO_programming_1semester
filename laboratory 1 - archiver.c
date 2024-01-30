#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define COUNT_MAX_FILES 50


void Create_fileArchive(const char *nameArchive, char *masFile[], const int countFiles) {
    FILE* myARCHIVE = fopen(nameArchive, "wb");
    if(myARCHIVE == NULL) {
        printf("Ошибка создания архива\n");
        return;
    }

    for(int i = 0; i < countFiles; i++) {
        FILE *fileOpen = fopen(masFile[i], "rb");
        if(fileOpen == NULL) {
            printf("Ошибка открытия файла\n");
            continue;
        }

        fseek(fileOpen, 0, SEEK_END);
        long long sizeFile = ftell(fileOpen);
        rewind(fileOpen);
        fwrite(&sizeFile, sizeof(long long), 1, myARCHIVE);


        int fileNameLength = strlen(masFile[i]);
        fwrite(&fileNameLength, sizeof(int), 1, myARCHIVE);
        fwrite(masFile[i], sizeof(char), fileNameLength, myARCHIVE);


        char* buffer = (char*)malloc(sizeFile * sizeof(char));

        fread(buffer, 1, sizeFile, fileOpen);
        fwrite(buffer, 1, sizeFile, myARCHIVE);
        free(buffer);
        fclose(fileOpen);
    }
    fclose(myARCHIVE);
}



void Extract_fileArchive(const char *nameArchive) {
    FILE *myARCHIVE = fopen(nameArchive, "rb");
    if (myARCHIVE == NULL) {
        printf("Ошибка открытия архива\n");
        return;
    }


    int numbFile = 1;
    while(!feof(myARCHIVE)) {
        long long sizeFile;
        fread(&sizeFile, sizeof(long long), 1, myARCHIVE);
        if(feof(myARCHIVE)) {
            break;
        }

        char* buffer = (char*)malloc(sizeFile);
        if(buffer == NULL) {
            printf("Ошибка выделения памяти\n");
            fclose(myARCHIVE);
            return;
        }

        fread(buffer, 1, sizeFile, myARCHIVE);

        char fileName[50];
        sprintf(fileName, "file_%d.txt", numbFile++);

        FILE* file = fopen(fileName, "wb");
        if(file == NULL) {
            printf("Ошибка открытия файла для записи: %s\n", fileName);
            free(buffer);
            continue;
        }

        fwrite(buffer, 1, sizeFile, file);

        free(buffer);
        fclose(file);
    }
    fclose(myARCHIVE);
}


void List_fileArchive(const char *nameArchive) {
    FILE* myARCHIVE = fopen(nameArchive, "rb");
    if(myARCHIVE == NULL) {
        printf("Ошибка открытия архива\n");
        return;
    }

    int numbFile = 1;
    while(!feof(myARCHIVE)) {
        long sizeFile = 0;

        fread(&sizeFile, sizeof(long long), 1, myARCHIVE);
        if(feof(myARCHIVE)) {
            break;
        }

        int fileNameLength;
        fread(&fileNameLength, sizeof(int), 1, myARCHIVE);

        char filename[fileNameLength+1];
        fread(filename, sizeof(char), fileNameLength, myARCHIVE);
        filename[fileNameLength] = '\0';

        printf("File %d: %s, size = %ld bytes\n", numbFile++, filename, sizeFile);
        fseek(myARCHIVE, sizeFile, SEEK_CUR);
    }
    fclose(myARCHIVE);
}


int main(int argc, char *argv[]) {
    if(argc < 4) {
        printf("Недопустимое количество аргументов\n");
        return 1;
    }


    int i, j;
    char *fileNameArchive = NULL;
    char *masFile[COUNT_MAX_FILES];
    int currentNumberFile = 0;

    int flag_extract = 0;
    int flag_list = 0;


    /// парсинг команд
    for(i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--file") == 0) { /// задает название файловому архиву
            fileNameArchive = argv[i+1];
            i++;
        }

        else if(strcmp(argv[i], "--create") == 0) { /// создает файловый архив
            for(j = i+1; j < argc; j++) {
                if(argv[j][0] != '-') {
                    masFile[currentNumberFile] = argv[j];
                    currentNumberFile++;
                } else {
                    break;
                }
            }
        }

        else if(strcmp(argv[i], "--extract") == 0) { /// извлекает из файлового архива данные
            flag_extract = 1;
        }

        else if(strcmp(argv[i], "--list") == 0) { /// список файлов
            flag_list = 1;
        }
    }



    if(fileNameArchive != NULL && currentNumberFile > 0) {
        Create_fileArchive(fileNameArchive, masFile, currentNumberFile);
    }
    else if(fileNameArchive != NULL && flag_extract > 0) {
        Extract_fileArchive(fileNameArchive);
    }
    else if(fileNameArchive != NULL && flag_list > 0) {
        List_fileArchive(fileNameArchive);
    }
    else {
        printf("Укажите имя архива!\n");
        return 1;
    }
    return 0;
}