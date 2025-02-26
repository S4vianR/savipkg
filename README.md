# SaviPKG - Un manejador de paquetes de ArchLinux (por el momento)

Bienvenid@s seán a mi manejador de paquetes, es una versión sencilla utilizando C, no es un frontend para pacman, es algo nuevo y desde cero

## Flags del programa

Flags de uso:
[ ]: "-r" Esto es para especificar algún repositorio, como "core", "extra", "multilib", etc. **Por el momento no se soporta el AUR**.

- "-s" Esto es para buscar el paquete en específico.
- "-i" Esta flag es para instalar el paquete con el archivo especificado.
- "-R" Esta flag es para eliminar un paquete del sistema

## Instrucciones de uso para instalar

### Paso 1 - Buscar el paquete a instalar

En este ejemplo se instalará "fish"

Se usa la flag "-r" para especificar "extra" que es el repositorio en el que está y la flag "-s" para buscar entre todos los archivos.

```shell-session
user on pc at [ ~/CLionProjects/savipkg ]
 > $ ./savipkg -r extra -s fish
La fecha actual Feb 25 2025

[==================================================] 100%

Paquete encontrado: angelfish-24.12.2-1-x86_64.pkg.tar.zst
Paquete encontrado: bluefish-2.2.15-3-x86_64.pkg.tar.zst
Paquete encontrado: catfish-4.20.0-2-any.pkg.tar.zst
Paquete encontrado: cutefish-calculator-0.4-2-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-core-0.8-2-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-dock-0.8-1-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-filemanager-0.8-2-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-icons-0.8-2-any.pkg.tar.zst
Paquete encontrado: cutefish-launcher-0.8-2-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-qt-plugins-0.5-4-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-screenlocker-0.5-2-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-settings-0.8-9-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-statusbar-0.7-1-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-terminal-0.7-1-x86_64.pkg.tar.zst
Paquete encontrado: cutefish-wallpapers-1.0-2-any.pkg.tar.zst
Paquete encontrado: fish-3.7.1-2-x86_64.pkg.tar.zst
Paquete encontrado: fisher-4.4.5-1-any.pkg.tar.zst
Paquete encontrado: fishui-0.8-2-x86_64.pkg.tar.zst
Paquete encontrado: libcutefish-0.7-5-x86_64.pkg.tar.zst
Paquete encontrado: libfishsound-1.0.1-2-x86_64.pkg.tar.zst
Paquete encontrado: perl-crypt-blowfish-2.14-16-x86_64.pkg.tar.zst
Paquete encontrado: python-babelfish-0.6.1-2-any.pkg.tar.zst
Paquete encontrado: python-jellyfish-1.1.3-1-x86_64.pkg.tar.zst
```

### Paso 2 - Instalar el paquete

Se usa la flag "-r" para especificar "extra" que es el repositorio en el que está y la flag "-i" instalar el archivo espeficiado.

```shell-session
use on pc at [ ~/CLionProjects/savipkg ]
 > $ sudo ./savipkg -r extra -i fish-3.7.1-2-x86_64.pkg.tar.zst
La fecha actual Feb 25 2025
Descargando el archivo: fish-3.7.1-2-x86_64.pkg.tar.zst
[==================================================] 100%
Paquete a instalar: fish-3.7.1-2-x86_64.pkg.tar.zst
¿Desea instalar el paquete? (s/n): s
Paquete instalado
```

### Paso 3 - Ejecutar el binario

Ya se instaló la shell de "fish"

```shell-session
user on pc at [ ~/CLionProjects/savipkg ]
 > $ fish
Welcome to fish, the friendly interactive shell
Type help for instructions on how to use fish
user@pc ~/C/savipkg (main)> 
```

## Instrucciones de uso para desinstalar un paquete

Se usa la flag "-R" para especificar el paquete a eliminar y en este caso no se necesita usar la flag para especificar el repositorio.

```shell-session
user on pc at [ ~/CLionProjects/savipkg ]
 > $ sudo ./savipkg -R fish
La fecha actual Feb 25 2025
El paquete fish ha sido eliminado.
Refrescando la sesión de la terminal...

user on pc at [ ~/CLionProjects/savipkg ]
 > $ fish
bash: fish: orden no encontrada
```
