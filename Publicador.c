#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void enviarNoticias(char *pipePSC, char *file, int timeN) {
    if (access(pipePSC, F_OK) == -1) {
        if (mkfifo(pipePSC, 0666) == -1) {
            perror("Error al crear pipe");
        }
    }

    int fd = open(pipePSC, O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir pipe");
        exit(1);
    }

    FILE *fp = fopen(file, "r");
    if (!fp) {
        perror("Error al abrir archivo de noticias");
        exit(1);
    }

    char linea[100];
    while (fgets(linea, sizeof(linea), fp)) {
        write(fd, linea, strlen(linea));
        printf("Noticia enviada: %s\n", linea);
        sleep(timeN);
    }

    close(fd);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    char *pipePSC = NULL;
    char *file = NULL;
    int timeN = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            pipePSC = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0) {
            file = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0) {
            timeN = atoi(argv[++i]);
        }
    }

    if (pipePSC && file) {
        enviarNoticias(pipePSC, file, timeN);
    } else {
        fprintf(stderr, "Uso: publicador -p pipePSC -f file -t timeN\n");
    }

    return 0;
}


