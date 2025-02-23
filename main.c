#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Este es savipkg un administrador de paquetes para sistemas operativos basados en linux
 * Se deberá de ejecutar con permisos de superusuario
 * Este no utiliza el sistema de paquetes de la distribución como pacman o apt
 * Y este deberá de usar los mirrors de la distribución
 */

// Variables globales
char *urls[20];

int fetch_urls(void) {
    // Variables
    const char *nombre_archivo = "/etc/pacman.d/mirrorlist";
    int url_count = 0;

    // Del archivo solo mostrar los mirrors, aparece como: Server = algo
    FILE *archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo %s\n", nombre_archivo);
        return 1;
    }

    char linea[256];
    while (fgets(linea, sizeof(linea), archivo)) {
        if (strncmp(linea, "Server", 6) == 0) {
            // Extraer solo la URL
            char *url = strchr(linea, '=');
            if (url != NULL) {
                url++; // Mover el puntero para saltar el '='
                // Eliminar espacios en blanco al inicio
                while (*url == ' ') url++;
                urls[url_count] = strdup(url);
                // Eliminar el salto de línea
                urls[url_count][strcspn(urls[url_count], "\n")] = 0;
                url_count++;
            }
        }
    }
    fclose(archivo);

    // Mostrar la lista de URLs al usuario
    printf("Lista de URLs\n");

    for (int i = 0; i < url_count; i++) {
        printf("%s\n", urls[i]);
        free(urls[i]);
    }

    return 0;
}

int main(void) {
    // Mostrarle al usuario la fecha actual
    printf("La fecha actual %s\n", __DATE__);

    // Mostrar los mirrors de la distribución
    fetch_urls();

    return 0;
}