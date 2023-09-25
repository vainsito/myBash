#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <glib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "tests/syscall_mock.h"
#include "strextra.h"
#include "command.h"
#include "parsing.h"
#include "builtin.h"
#include "execute.h"



// Definimos la funcion que convierte un comando simple en un arreglo de cadenas con sus argumentos
static char **scommand_to_argv(scommand cmd) {
    // Inicializamos el contador de argumentos a cero
    int argc = 0;

    // Creamos un puntero a un arreglo de cadenas
    char **argv = NULL;

    // Obtenemos el número de argumentos en el comando simple
    int num_args = scommand_length(cmd);

    // Iteramos a través de los argumentos y los copiamos al arreglo
    for (int i = 0; i < num_args; i++) {
        // Obtenemos el argumento actual
        char *arg = scommand_front(cmd);
        // Incrementamos el contador de argumentos
        argc++;
        // Realocamos la memoria para el arreglo de cadenas
        argv = realloc(argv, argc * sizeof(char));
        if (argv == NULL) {
            perror("Error al asignar memoria para argv");
            exit(EXIT_FAILURE);
        }
        // Copiamos el argumento en la posición correspondiente del arreglo
        argv[argc - 1] = strdup(arg);
        // Movemos el puntero de argumentos al siguiente
        scommand_pop_front(cmd);
    }

    // Añadimos un elemento NULL al final del arreglo como marcador
    argv = realloc(argv, (argc + 1) * sizeof(char *));
    if (argv == NULL) {
        perror("Error al asignar memoria para argv");
        exit(EXIT_FAILURE);
    }
    argv[argc] = NULL;

    return argv;
}

// Funcion que realiza el redireccionamiento de la entrada de un comando simple
static void scommand_redirect_in(scommand cmd) {
    // Obtenemos el archivo de entrada
    char *in = scommand_get_redir_in(cmd);
    // Si hay un archivo de entrada, lo abrimos y redireccionamos la entrada estándar
    if (in != NULL) {
        // Abrimos el archivo de entrada
        int fd_in = open(in, O_RDONLY,O_CREAT); // fd = file descriptor
        if (fd_in < 0) { // Si el archivo no existe
            perror("Error al abrir archivo de entrada");
            exit(EXIT_FAILURE);
        }
        // Redireccionamos la entrada estándar
        int in_redir = dup2(fd_in, STDIN_FILENO); // STDIN_FILENO = 0
        if (in_redir < 0) { // Si no se pudo redireccionar
            perror("Error al redireccionar la entrada ");
            exit(EXIT_FAILURE);
        }
        // Cerramos el archivo de entrada
        int close_fd = close(fd_in); 
        if (close_fd < 0) {
            perror("Error al cerrar archivo de entrada");
            exit(EXIT_FAILURE);
        }
    }
}

// Funcion que realiza el redireccionamiento de la salida de un comando simple
static void scommand_redirect_out(scommand cmd){
    // Obtenemos el archivo de salida
    char *out = scommand_get_redir_out(cmd);
    // Si hay un archivo de salida, lo abrimos y redireccionamos la salida estándar
    if (out != NULL) {
        // Abrimos el archivo de salida, 
        int fd_out = open(out, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // O_WRONLY = write only, O_CREAT = crea el archivo si no existe, S_IRUSR = permisos de lectura para el usuario, S_IWUSR = permisos de escritura para el usuario
        if (fd_out < 0) { // Si el archivo no existe
            perror("Error al abrir archivo de salida");
            exit(EXIT_FAILURE);
        }
        // Redireccionamos la salida estándar
        int out_redir = dup2(fd_out, STDOUT_FILENO); // STDOUT_FILENO = 1
        if ( out_redir < 0) {
            perror("Error al redireccionar salida estándar");
            exit(EXIT_FAILURE);
        }
        // Cerramos el archivo de salida
        int close_fdout = close(fd_out);
        if ( close_fdout < 0) {
            perror("Error al cerrar archivo de salida");
            exit(EXIT_FAILURE);
        }
    }
}

// Definimos la funcion que ejecuta un comando simple interno y externo con redireccionamiento o sin redireccionamiento
static void execute_simple(scommand cmd, pipeline apipe){
    assert(cmd != NULL);
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error al crear proceso hijo");
        exit(EXIT_FAILURE);
    } else if (pid == 0){
        // Aca debemos redireccionar la entrada y salida del comando si es necesario
        if (scommand_get_redir_in(cmd) != NULL){
            scommand_redirect_in(cmd); // Redireccionamos la entrada
        }
        if (scommand_get_redir_out(cmd) != NULL){
            scommand_redirect_out(cmd); // Redireccionamos la salida
        }
        // Una vez redireccionada la entrada y salida, ejecutamos el comando
        char *argc = scommand_front(cmd); // Obtenemos el comando
        char **argv = scommand_to_argv(cmd); // Obtenemos los argumentos del comando
        int err_return = execvp(argc, argv); // Ejecutamos el comando
        if (err_return < 0) { // Si hubo un error al ejecutar el comando
            perror("Error al ejecutar comando");
            exit(EXIT_FAILURE);
        }
    } else {
        if (pipeline_get_wait(apipe) == true) {
            wait(NULL);
        }
    }
}
// Definimos una funcion que ejecuta en foreground un pipeline
static void exec_fob(pipeline apipe, bool is_fore){
    scommand fst_cmd = pipeline_front(apipe);
    pipeline_pop_front(apipe);
     scommand lst_cmd = pipeline_front(apipe);
    // Empezamos la plomeria
    int fd[2];
    int err_pipe = pipe(fd);
    if(err_pipe < 0){
        perror("Error al crear el pipe");
        exit(EXIT_FAILURE);
    }
    /* -------- Primer comando ------------*/
    pid_t pid_frg = fork();
    if(pid_frg < 0){
        perror("Error al crear proceso hijo en foreground");
        exit(EXIT_FAILURE);
    }
    else if( pid_frg == 0){ // Caso hijo
        int err_dup = dup2(fd[1],STDOUT_FILENO); // Abrimos el pipe para escribir
        if(err_dup < 0){
            perror("Error al duplicar el pipe");
            exit(EXIT_FAILURE);
        }
        else{ // Ejecutamos el primer comando
            close(fd[0]); // Cerramos el extremo de lectura del pipe
            close(fd[1]); // Cerramos el extremo de escritura del pipe
            execute_simple(fst_cmd,apipe);
        }
    }
    // Comando 2
    pid_t pid_frg2 = fork();
    if(pid_frg2 < 0){
        perror("Error al crear proceso hijo en foreground");
        exit(EXIT_FAILURE);
    }
    else if( pid_frg2 == 0){ // Caso hijo
        int err_dup = dup2(fd[0],STDIN_FILENO); // Abrimos el pipe para leer
        if(err_dup < 0){
            perror("Error al duplicar el pipe");
            exit(EXIT_FAILURE);
        }
        else{ // Ejecutamos el segundo comando
            close(fd[1]); // Cerramos el extremo de escritura del pipe
            execute_simple(lst_cmd,apipe);
        }
    }
    else{ // Caso padre
        close(fd[0]); // Cerramos el extremo de lectura del pipe
        close(fd[1]); // Cerramos el extremo de escritura del pipe
        if(is_fore == true){
            wait(NULL);
            wait(NULL);
        }
    }
}
// Definimos la funcion que ejecuta un pipeline
void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);
    if (!pipeline_is_empty(apipe)){
        // ---------------------------- FOREGROUND ----------------------------- // 
        if (pipeline_get_wait(apipe)){ 
            if (pipeline_length(apipe) == 1){ // Si el pipeline tiene un solo comando
                if (builtin_alone(apipe)){ // Si el comando es interno
                    builtin_run(pipeline_front(apipe));
                } 
                else { // Si el comando es externo
                    execute_simple(pipeline_front(apipe),apipe);
                }
            } 
            else if (pipeline_length(apipe) == 2){ // Si el pipeline tiene mas de un comando
                exec_fob(apipe, true);
            }
        }
         // ---------------------------- BACKGROUND ----------------------------- //
        else{ 
            exec_fob(apipe, false);
            printf(" ///////// El proceso con pid %d Termino /////////\n", getpid());
            exit(EXIT_SUCCESS);
        }
    }   
}
