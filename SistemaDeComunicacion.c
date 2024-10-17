#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_SUSCRIPTORES 10 //Maximo suscriptores actual

//Definicion estructura suscriptor
typedef struct {
    int fd;
    char categorias[6];
} Suscriptor;

Suscriptor suscriptores[MAX_SUSCRIPTORES];
int num_suscriptores = 0;

//Funcion para agregar suscriptor
void agregarSuscriptor(int fd, char *categorias) {
    suscriptores[num_suscriptores].fd = fd;
    strcpy(suscriptores[num_suscriptores].categorias, categorias);
    num_suscriptores++;
}

//Funcion para distribuir noticias a los suscriptores
void distribuirNoticias(char *noticia) {
    // Abrir el pipe para enviar las noticias
    int fdNoticias = open("pipeNoticias", O_WRONLY);  // Abre el pipe para escribir las noticias
    if (fdNoticias == -1) {
        perror("Error al abrir pipe para noticias");
        return;
    }

    // Enviar la noticia a través del pipe
    write(fdNoticias, noticia, strlen(noticia));  
    close(fdNoticias);  // Cerrar el pipe después de enviar
}

//Funcion principal del SC
void gestionarSistema(char *pipePSC, char *pipeSSC, int timeF) {
    // Verificar si las pipes ya existen
    if (access(pipePSC, F_OK) == -1) {
        if (mkfifo(pipePSC, 0666) == -1) {
            perror("Error al crear pipe de publicador");
        }
    }
    if (access(pipeSSC, F_OK) == -1) {
        if (mkfifo(pipeSSC, 0666) == -1) {
            perror("Error al crear pipe de suscriptor");
        }
    }

    int fdPublicador = open(pipePSC, O_RDONLY);  // Abrir pipe de publicador
    if (fdPublicador == -1) {
        perror("Error al abrir pipe de publicador");
        exit(1);
    }

    int fdSuscriptor = open(pipeSSC, O_RDONLY);  // Abrir pipe de suscriptor
    if (fdSuscriptor == -1) {
        perror("Error al abrir pipe de suscriptor");
        exit(1);
    }

    char buffer[256];
    char noticia[256];
    int noticiaIndex = 0;

    while (1) {
        memset(buffer, 0, sizeof(buffer));  //Limpiar buffer

        //Leer de publicadores
        int bytesReadPub = read(fdPublicador, buffer, sizeof(buffer) - 1);
        if (bytesReadPub > 0) {
            buffer[bytesReadPub] = '\0';
            for (int i = 0; i < bytesReadPub; i++) {
                if (buffer[i] == '.') {  //Detectar fin de noticia
                    noticia[noticiaIndex] = '\0';
                    printf("Noticia recibida en SC: %s\n", noticia);
                    distribuirNoticias(noticia);  //Distribuir noticia
                    noticiaIndex = 0;
                } else {
                    noticia[noticiaIndex++] = buffer[i];
                }
            }
        }

        //Leer de suscriptores
        int bytesReadSub = read(fdSuscriptor, buffer, sizeof(buffer) - 1);
        if (bytesReadSub > 0) {
            buffer[bytesReadSub] = '\0';
            agregarSuscriptor(fdSuscriptor, buffer);  //Agregar suscriptor
        }
    }

    sleep(timeF);
    close(fdPublicador);
    close(fdSuscriptor);
}

int main(int argc, char *argv[]) {
  //Creacion De Pipes
    char *pipePSC = NULL;
    char *pipeSSC = NULL;
    int timeF = 1;

    //Procesar argumentos de entrada
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            pipePSC = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            pipeSSC = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0) {
            timeF = atoi(argv[++i]);
        }
    }

    if (pipePSC && pipeSSC) {
        gestionarSistema(pipePSC, pipeSSC, timeF);  //Gestionar el sistema
    } else {
        fprintf(stderr, "Uso: sistema -p pipePSC -s pipeSSC -t timeF\n");
    }

    return 0;
}


