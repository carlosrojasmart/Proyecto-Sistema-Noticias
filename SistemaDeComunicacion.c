#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#define MAX_SUSCRIPTORES 10

typedef struct {
    int fd;
    char categorias[6];
    char pipeName[20];
} Suscriptor;

Suscriptor suscriptores[MAX_SUSCRIPTORES];
int num_suscriptores = 0;

void agregarSuscriptor(int fd, char *categorias, char *pipeName) {
    if (num_suscriptores < MAX_SUSCRIPTORES) {
        suscriptores[num_suscriptores].fd = fd;
        strcpy(suscriptores[num_suscriptores].categorias, categorias);
        strcpy(suscriptores[num_suscriptores].pipeName, pipeName);
        num_suscriptores++;
        printf("Suscriptor agregado: FD=%d, Categorías=%s, Pipe=%s\n", fd, categorias, pipeName);
    } else {
        printf("Máximo de suscriptores alcanzado.\n");
    }
}

void distribuirNoticias(char *noticia) {
    char categoria = noticia[0];
    for (int i = 0; i < num_suscriptores; i++) {
        if (strchr(suscriptores[i].categorias, categoria) != NULL) {
            int fd = open(suscriptores[i].pipeName, O_WRONLY | O_NONBLOCK);
            if (fd != -1) {
                write(fd, noticia, strlen(noticia));
                close(fd);
                printf("Noticia enviada al suscriptor %d (%s): %s\n", suscriptores[i].fd, suscriptores[i].pipeName, noticia);
            } else {
                perror("Error al abrir pipe del suscriptor");
            }
        }
    }
}

void gestionarSistema(char *pipePSC, char *pipeSSC, int timeF) {
    if (access(pipePSC, F_OK) == -1 && mkfifo(pipePSC, 0666) == -1) {
        perror("Error al crear pipe de publicador");
        return;
    }
    if (access(pipeSSC, F_OK) == -1 && mkfifo(pipeSSC, 0666) == -1) {
        perror("Error al crear pipe de suscriptor");
        return;
    }

    int fdPublicador = open(pipePSC, O_RDONLY | O_NONBLOCK);
    int fdSuscriptor = open(pipeSSC, O_RDONLY | O_NONBLOCK);

    if (fdPublicador == -1 || fdSuscriptor == -1) {
        perror("Error al abrir pipe de publicador o suscriptor");
        exit(1);
    }

    char buffer[256];
    char noticia[256];
    int noticiaIndex = 0;

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fdPublicador, &read_fds);
        FD_SET(fdSuscriptor, &read_fds);
        int max_fd = fdPublicador > fdSuscriptor ? fdPublicador : fdSuscriptor;

        struct timeval timeout;
        timeout.tv_sec = timeF;
        timeout.tv_usec = 0;

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity == -1) {
            perror("Error en select");
            break;
        }

        if (FD_ISSET(fdPublicador, &read_fds)) {
            int bytesReadPub = read(fdPublicador, buffer, sizeof(buffer) - 1);
            if (bytesReadPub > 0) {
                buffer[bytesReadPub] = '\0';
                strcpy(noticia, buffer);
                distribuirNoticias(noticia);
            }
        }

        if (FD_ISSET(fdSuscriptor, &read_fds)) {
            int bytesReadSus = read(fdSuscriptor, buffer, sizeof(buffer) - 1);
            if (bytesReadSus > 0) {
                buffer[bytesReadSus] = '\0';
                char *categorias = strtok(buffer, ":");
                char *pipeName = strtok(NULL, ":");
                agregarSuscriptor(fdSuscriptor, categorias, pipeName);
            }
        }
    }

    close(fdPublicador);
    close(fdSuscriptor);
}

int main(int argc, char *argv[]) {
    char *pipePSC = NULL;
    char *pipeSSC = NULL;
    int timeF = 5;

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
        gestionarSistema(pipePSC, pipeSSC, timeF);
    } else {
        fprintf(stderr, "Uso: sc -p pipePSC -s pipeSSC -t timeF\n");
    }

    return 0;
}
