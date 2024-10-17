#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//Funcion para recibir noticias desde el pipe
void recibirNoticias(char *pipeNoticias, char *categorias) {
    // Verificar si el pipe ya existe
    if (access(pipeNoticias, F_OK) == -1) {
        if (mkfifo(pipeNoticias, 0666) == -1) {
            perror("Error al crear pipe de noticias");
        }
    }

    int fd = open(pipeNoticias, O_RDONLY);  // Abrir pipe para leer noticias
    if (fd == -1) {
        perror("Error al abrir pipe de noticias");
        exit(1);
    }

    char buffer[256];
    while (1) {  // Bucle infinito para seguir leyendo noticias
        memset(buffer, 0, sizeof(buffer));  // Limpiar buffer
        int bytesRead = read(fd, buffer, sizeof(buffer) - 1);  // Leer del pipe
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';

            // Procesar las noticias recibidas
            char *noticia = strtok(buffer, "\n");
            while (noticia != NULL) {
                char categoriaNoticia = noticia[0];  // Filtrar por categorías
                if (strchr(categorias, categoriaNoticia)) {
                    printf("Noticia recibida por el suscriptor: %s\n", noticia);
                }
                noticia = strtok(NULL, "\n");
            }
        }
    }

    close(fd);  // Cerrar el pipe al finalizar
}


void enviarSuscripcion(char *pipeSSC, char *categorias) {
    // Verificar si el pipe ya existe
    if (access(pipeSSC, F_OK) == -1) {
        if (mkfifo(pipeSSC, 0666) == -1) {
            perror("Error al crear pipe de suscripción");
        }
    }

    int fd = open(pipeSSC, O_WRONLY);  // Abrir pipe para escribir
    if (fd == -1) {
        perror("Error al abrir pipe para enviar suscripción");
        exit(1);
    }

    write(fd, categorias, strlen(categorias));  // Enviar categorías al sistema
    close(fd);  // Cerrar pipe
}


int main(int argc, char *argv[]) {
    char *pipeSSC = NULL;
    char *pipeNoticias = "pipeNoticias";  //Pipe para recibir noticias
    char categorias[6] = {0};

    //Procesar los argumentos para obtener el nombre del pipe de suscripción
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            pipeSSC = argv[++i];
        }
    }

    if (pipeSSC) {
        printf("Ingrese las categorías a las que desea suscribirse (A, E, C, P, S): ");
        fgets(categorias, sizeof(categorias), stdin);

        enviarSuscripcion(pipeSSC, categorias);  //Enviar suscripción
        recibirNoticias(pipeNoticias, categorias);  //Recibir noticias
    } else {
        fprintf(stderr, "Uso: suscriptor -s pipeSSC\n");
    }

    return 0;
}




