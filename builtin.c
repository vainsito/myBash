#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "strextra.h"
#include "command.h"
#include "parsing.h"
#include "builtin.h"
#include "tests/syscall_mock.h"

bool builtin_is_internal(scommand cmd){
    assert(cmd != NULL);
    bool result = false;
    char *string_to_compare = scommand_front(cmd);
    if (strcmp("cd",string_to_compare) == 0 || strcmp("help",string_to_compare) == 0 || strcmp("exit",string_to_compare) == 0){
        result = true;
    }
    return result;
}

bool builtin_alone(pipeline p){
    assert(p != NULL);
    bool result = false;
    result = (pipeline_length(p) == 1 && builtin_is_internal(pipeline_front(p)));
    return result;
}

void builtin_run(scommand cmd){
    assert(builtin_is_internal(cmd));
    char *command = scommand_front(cmd);
    if (!strcmp(command,"cd")){
        unsigned int command_length = scommand_length(cmd);
        if (command_length > 2){
            printf("Syntax Error - Too many arguments. See help for %s command with 'help'.\n", command);
        } else if (command_length == 2){
            scommand_pop_front(cmd);
            char *directory = strmerge("",scommand_front(cmd));
            int result = chdir(directory);
            if (result == -1){
                printf("Error - Directory does not exist.\n");
            }
        } else {
            printf("Syntax Error - Not enough arguments. See help for %s command with 'help'.\n", command);
        }
    } else if (!strcmp(command,"help")){
            printf("---------------------------------------------------------------------------------------------------------------------\n");
            printf("///////////////////////////////////////////////////////MyBASH////////////////////////////////////////////////////////\n");
            printf("Creators: Saharrea J. Manu, Ignacio A. Ramirez\n");
            printf("Ortiz J. Alexis, Ezequiel M. Reyna \n");
            printf("//////////////////////////////////////////////////Comandos_Internos/////////////////////////////////////////////////\n");
            printf("cd : Permite que el usuario se desplace entre directorios \n"
                            "Uso : cd 'Nombre del directorio a desplazarse' \n");
            printf("exit : Finaliza la ejecucion de MyBASH y todos los procesos asociados \n Uso: exit  \n");
            printf("help :  Envia un mensaje por salida estandar que indica"
                            "- Nombre de la Shell \n"
                            "- Nombre de sus autores  \n"
                            "- Lista de comandos internos implementados \n"
                            "Uso : help \n");
            printf("---------------------------------------------------------------------------------------------------------------------\n");
    } else if (!strcmp(command,"exit")){
        exit(EXIT_SUCCESS);
    }
}