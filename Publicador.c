#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//Funcion para enviar noticias a través del pipe
void enviarNoticias(char *pipePSC, char *file, int timeN) {
    if (access(pipePSC, F_OK) == -1) {
        if (mkfifo(pipePSC, 0666) == -1) {
            perror("Error al crear pipe");
        }
    }

    int fd = open(pipePSC, O_WRONLY);  // Abrir pipe para escribir
    if (fd == -1) {
        perror("Error al abrir pipe");
        exit(1);
    }

    FILE *fp = fopen(file, "r");  // Abrir archivo de noticias
    if (!fp) {
        perror("Error al abrir archivo de noticias");
        exit(1);
    }

    char linea[100];
    while (fgets(linea, sizeof(linea), fp)) {
        write(fd, linea, strlen(linea));  // Enviar noticia por pipe
        printf("Noticia enviada: %s\n", linea);  // Confirmación de envío
        sleep(timeN);  // Esperar antes de enviar la siguiente noticia
    }

    close(fd);  // Cerrar pipe
    fclose(fp);  // Cerrar archivo de noticias
}



int main(int argc, char *argv[]) {
    char *pipePSC = NULL;
    char *file = NULL;
    int timeN = 1;

    //Procesar argumentos de entrada
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            pipePSC = argv[++i];  //Obtener el nombre del pipe para el publicador
        } else if (strcmp(argv[i], "-f") == 0) {
            file = argv[++i];  //Obtener el archivo con las noticias
        } else if (strcmp(argv[i], "-t") == 0) {
            timeN = atoi(argv[++i]);  //Tiempo de espera entre noticias
        }
    }

    //Validar los parámetros
    if (pipePSC && file) {
        enviarNoticias(pipePSC, file, timeN);  //Enviar noticias
    } else {
        fprintf(stderr, "Uso: publicador -p pipePSC -f file -t timeN\n");
    }

    return 0;
}

