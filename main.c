#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <curl/curl.h>
#include <unistd.h> // Para la función geteuid()
#include <errno.h>
#include <sys/ioctl.h> // Para obtener el tamaño de la terminal

/**
 * Este es savipkg un administrador de paquetes para sistemas operativos basados en linux
 * Se deberá de ejecutar con permisos de superusuario
 * Este no utiliza el sistema de paquetes de la distribución como pacman o apt
 * Y este deberá de usar los mirrors de la distribución
 */

// Variables globales
char *urls[20];
char *package_names[30000]; // Aumentar el tamaño del arreglo para manejar más paquetes

// Estructura para almacenar los datos de la respuesta
struct MemoryStruct
{
    char *memory;
    size_t size;
};

int fetch_urls(char *repositorio)
{
    // Variables
    const char *nombre_archivo = "/etc/pacman.d/mirrorlist";
    int url_count = 0;

    // Del archivo solo mostrar los mirrors, aparece como: Server = algo
    FILE *archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL)
    {
        printf("No se pudo abrir el archivo %s\n", nombre_archivo);
        return 1;
    }

    char linea[256];
    while (fgets(linea, sizeof(linea), archivo))
    {
        if (strncmp(linea, "Server", 6) == 0)
        {
            // Extraer solo la URL
            char *url = strchr(linea, '=');
            if (url != NULL)
            {
                url++; // Mover el puntero para saltar el '='
                // Eliminar espacios en blanco al inicio
                while (*url == ' ')
                    url++;

                // Reemplazar $repo y $arch
                const char *repo = repositorio; // Repositorio
                const char *arch = "x86_64";    // Arquitectura
                char modified_url[256];

                // Reemplazar $repo y $arch en la URL
                snprintf(modified_url, sizeof(modified_url), "%s", url);
                char *pos_repo = strstr(modified_url, "$repo");
                if (pos_repo)
                {
                    memmove(pos_repo + strlen(repo), pos_repo + 5, strlen(pos_repo + 5) + 1);
                    memcpy(pos_repo, repo, strlen(repo));
                }
                char *pos_arch = strstr(modified_url, "$arch");
                if (pos_arch)
                {
                    memmove(pos_arch + strlen(arch), pos_arch + 5, strlen(pos_arch + 5) + 1);
                    memcpy(pos_arch, arch, strlen(arch));
                }

                urls[url_count] = strdup(modified_url);
                // Eliminar el salto de línea
                urls[url_count][strcspn(urls[url_count], "\n")] = 0;
                url_count++;
            }
        }
    }
    fclose(archivo);

    return 0;
}

// Callback para manejar los datos recibidos
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, struct MemoryStruct *userp)
{
    size_t realsize = size * nmemb;
    char *ptr = realloc(userp->memory, userp->size + realsize + 1);

    if (ptr == NULL)
    {
        // Si realloc falla, no se pierde el puntero original
        printf("No se pudo asignar memoria\n");
        return 0; // Out of memory
    }

    userp->memory = ptr; // Asignar el nuevo puntero
    memcpy(&(userp->memory[userp->size]), contents, realsize);
    userp->size += realsize;
    userp->memory[userp->size] = 0; // Null-terminate
    return realsize;
}

void extract_package_names(const char *html)
{
    const char *start_tag = "<a href=\"";
    const char *end_tag = "\">";
    const char *current_pos = html;

    while ((current_pos = strstr(current_pos, start_tag)) != NULL)
    {
        current_pos += strlen(start_tag); // Mover el puntero después de <a href="
        char *end_pos = strstr(current_pos, end_tag);
        if (end_pos)
        {
            size_t length = end_pos - current_pos;
            char *package_name = malloc(length + 1);
            strncpy(package_name, current_pos, length);
            package_name[length] = '\0'; // Null-terminate

            // Ignorar archivos terminados en ".sig"
            if (strstr(package_name, ".sig") == NULL)
            {
                // Poner el nombre del paquete en la lista
                for (int i = 0; i < 30000; i++) // Asegurarse de que el índice esté dentro del rango
                {
                    if (package_names[i] == NULL)
                    {
                        package_names[i] = package_name; // Guardar el nombre del paquete
                        break;
                    }
                }
            }
            else
            {
                free(package_name); // Liberar memoria si es un archivo .sig
            }
            current_pos = end_pos + strlen(end_tag); // Mover el puntero después de ">
        }
        else
        {
            break; // No se encontró el final
        }
    }
}

int fetch_packages(char *urls[])
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1); // Inicializar puntero
    chunk.size = 0;           // Tamaño inicial

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        if (urls[0] != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_URL, urls[0]);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Seguir redirecciones

            // Realizar la solicitud
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() falló: %s\n", curl_easy_strerror(res));
            }
            else
            {
                // Extraer nombres de paquetes del HTML
                extract_package_names(chunk.memory);
            }

            // Limpiar la memoria
            free(chunk.memory);
        }

        // Limpiar
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}

size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// Función de progreso
int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    if (dltotal == 0)
        return 0;

    int bar_width = 50;
    float progress = (float)dlnow / (float)dltotal;
    int pos = bar_width * progress;

    printf("\r[");
    for (int i = 0; i < bar_width; ++i)
    {
        if (i < pos)
            printf("=");
        else if (i == pos)
            printf(">");
        else
            printf(" ");
    }
    printf("] %d%%", (int)(progress * 100.0));
    fflush(stdout);

    return 0;
}

int fetch_and_save_package(const char *url, const char *output_path)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        fp = fopen(output_path, "wb");
        if (fp == NULL)
        {
            fprintf(stderr, "No se pudo abrir el archivo %s para escritura\n", output_path);
            return 1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Seguir redirecciones
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // Habilitar la función de progreso
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback); // Establecer la función de progreso

        // Realizar la solicitud
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() falló: %s\n", curl_easy_strerror(res));
        }

        // Limpiar
        fclose(fp);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}

void install_package(const char *package_name)
{
    char output_path[256];
    snprintf(output_path, sizeof(output_path), "/tmp/savipkg/%s", package_name);

    // Crear el directorio temporal
    system("mkdir -p /tmp/savipkg");

    // Descargar el paquete
    for (int i = 0; i < 20 && urls[i] != NULL; i++)
    {
        char package_url[1024];
        snprintf(package_url, sizeof(package_url), "%s/%s", urls[i], package_name);
        if (fetch_and_save_package(package_url, output_path) == 0)
        {
            printf("Paquete %s descargado y guardado en: %s\n", package_name, output_path);
            break;
        }
    }

    // Instalar el paquete desempaquetando el archivo y moviendo los archivos a sus ubicaciones
    system("tar -xvf /tmp/savipkg/*.tar.zst -C /");

    // Preguntar al usuario si desea instalar el paquete
    printf("¿Desea instalar el paquete? (s/n): ");
    char response;
    scanf(" %c", &response); // Nota: Espacio antes de %c para ignorar espacios en blanco
    if (tolower(response) == 's')
    {
        printf("Paquete instalado\n");
    }
    else
    {
        printf("Paquete no instalado\nSaliendo del programa...\n");
    }

    // Eliminar el directorio temporal
    system("find /tmp/savipkg -mindepth 1 -delete && rmdir /tmp/savipkg");
}

// Eliminar los paquetes descargados
int remove_packages(const char *package_name)
{
    char paths[4][256];
    int found = 0;

    // Definir las rutas a eliminar
    snprintf(paths[0], sizeof(paths[0]), "/etc/%s", package_name);
    snprintf(paths[1], sizeof(paths[1]), "/usr/lib/%s", package_name);
    snprintf(paths[2], sizeof(paths[2]), "/usr/bin/%s", package_name);
    snprintf(paths[3], sizeof(paths[3]), "/usr/share/%s", package_name);

    // Intentar eliminar cada ruta
    for (int i = 0; i < 4; i++)
    {
        if (access(paths[i], F_OK) != -1)
        {
            found = 1;
            char command[512];
            snprintf(command, sizeof(command), "rm -rf %s", paths[i]);
            system(command);
        }
    }

    if (!found)
    {
        printf("El paquete %s no existe o ya ha sido eliminado.\n", package_name);
    }
    else
    {
        printf("El paquete %s ha sido eliminado.\n", package_name);
        // Refrescar la sesión de la terminal
        printf("Refrescando la sesión de la terminal...\n");
        system("hash -r");
    }

    return 0; // Éxito
}

// Buscar paquetes que coincidan con el término de búsqueda
int search_packages(const char *query)
{
    int found = 0;
    int total_packages = 0;
    char found_packages[30000][256]; // Buffer para almacenar los nombres de los paquetes encontrados
    int found_count = 0;

    // Contar el número total de paquetes
    for (int i = 0; i < 30000 && package_names[i] != NULL; i++)
    {
        total_packages++;
    }

    // Buscar paquetes y mostrar barra de progreso
    printf("\n");
    for (int i = 0; i < total_packages; i++)
    {
        // Actualizar la barra de progreso
        int bar_width = 50;
        float progress = (float)(i + 1) / (float)total_packages;
        int pos = bar_width * progress;

        printf("\r[");
        for (int j = 0; j < bar_width; ++j)
        {
            if (j < pos)
                printf("=");
            else if (j == pos)
                printf(">");
            else
                printf(" ");
        }
        printf("] %d%%", (int)(progress * 100.0));
        fflush(stdout);

        if (strstr(package_names[i], query) != NULL)
        {
            snprintf(found_packages[found_count], sizeof(found_packages[found_count]), "Paquete encontrado: %s", package_names[i]);
            found_count++;
            found = 1;
        }
    }
    printf("\n\n"); // Nueva línea después de la barra de progreso

    // Mostrar los nombres de los paquetes encontrados
    for (int i = 0; i < found_count; i++)
    {
        printf("%s\n", found_packages[i]);
    }

    return found;
}

int main(int argc, char *argv[])
{
    // Mostrarle al usuario la fecha actual
    printf("La fecha actual %s\n", __DATE__);

    char *search_query = NULL;
    char *repo = NULL;
    char *install_package_name = NULL;

    // Procesar argumentos de línea de comandos
    int opt;
    char *remove_package_name = NULL;
    while ((opt = getopt(argc, argv, "s:r:i:R:")) != -1)
    {
        switch (opt)
        {
        case 's':
            search_query = optarg; // Guardar el término de búsqueda
            break;
        case 'r':
            repo = optarg; // Guardar el repositorio seleccionado
            break;
        case 'i':
            // Verificar si el programa se está ejecutando con permisos de superusuario
            if (geteuid() != 0)
            {
                fprintf(stderr, "Este programa debe ejecutarse con permisos de superusuario.\n");
                exit(EXIT_FAILURE);
            }

            install_package_name = optarg; // Guardar el nombre del paquete a instalar
            break;
        case 'R':
            // Verificar si el programa se está ejecutando con permisos de superusuario
            if (geteuid() != 0)
            {
                fprintf(stderr, "Este programa debe ejecutarse con permisos de superusuario.\n");
                exit(EXIT_FAILURE);
            }

            remove_package_name = optarg; // Guardar el nombre del paquete a eliminar
            break;
        default:
            fprintf(stderr, "Uso: %s [-s search_query] [-r repo] [-i package_name] [-R package_name]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Si se especifica un repositorio, se hace el fetch de los paquetes
    if (repo != NULL)
    {
        if (fetch_urls(repo) == 0)
        {
            // Hacer un fetch de los paquetes
            fetch_packages(urls); // Pasar el arreglo completo de URLs

            if (search_query != NULL)
            {
                if (!search_packages(search_query))
                {
                    printf("No se encontró ningún paquete con el término de búsqueda: %s\n", search_query);
                }
            }

            if (install_package_name != NULL)
            {
                printf("Instalando paquete: %s\n", install_package_name);
                install_package(install_package_name);
            }
        }
    }

    // Si se especifica un paquete para eliminar, se llama a remove_packages
    if (remove_package_name != NULL)
    {
        remove_packages(remove_package_name);
    }

    return 0;
}