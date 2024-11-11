#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

// Función para recibir noticias desde el pipe exclusivo
void recibirNoticias(char *pipeName, char *categorias) {
    if (access(pipeName, F_OK) == -1) {
        perror("Error al acceder al pipe de noticias");
        return;
    }

    int fd = open(pipeName, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("Error al abrir pipe de noticias");
        return;
    }

    char buffer[256];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            char *noticia = strtok(buffer, "\n");
            while (noticia != NULL) {
                char categoriaNoticia = noticia[0];
                if (strchr(categorias, categoriaNoticia)) {
                    printf("Noticia recibida por el suscriptor: %s\n", noticia);
                }
                noticia = strtok(NULL, "\n");
            }
        } else if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Error al leer del pipe de noticias");
            break;
        }
        
        usleep(500000);
    }

    close(fd);
}

// Función para enviar la suscripción y el nombre del pipe exclusivo
void enviarSuscripcion(char *pipeSSC, char *categorias, char *pipeName) {
    if (access(pipeSSC, F_OK) == -1) {
        if (mkfifo(pipeSSC, 0666) == -1) {
            perror("Error al crear pipe de suscripción");
            return;
        }
    }

    int fd = open(pipeSSC, O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir pipe para enviar suscripción");
        exit(1);
    }

    char mensaje[256];
    snprintf(mensaje, sizeof(mensaje), "%s:%s", categorias, pipeName);
    write(fd, mensaje, strlen(mensaje));
    close(fd);
}

int main(int argc, char *argv[]) {
    char *pipeSSC = NULL;
    char pipeName[20];
    char categorias[6] = {0};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            pipeSSC = argv[++i];
        }
    }

    if (pipeSSC) {
        snprintf(pipeName, sizeof(pipeName), "pipeNoticias%d", getpid());

        if (mkfifo(pipeName, 0666) == -1) {
            perror("Error al crear pipe exclusivo para noticias");
            exit(1);
        }

        printf("Ingrese las categorías a las que desea suscribirse (A, E, C, P, S): ");
        fgets(categorias, sizeof(categorias), stdin);

        enviarSuscripcion(pipeSSC, categorias, pipeName);
        recibirNoticias(pipeName, categorias);
    } else {
        fprintf(stderr, "Uso: suscriptor -s pipeSSC\n");
    }

    return 0;
}


