#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "parsing.h"
#include "parser.h"
#include "command.h"


static scommand parse_scommand(Parser p){
    assert(p != NULL);

    scommand result = scommand_new();

    arg_kind_t type;
    char *arg = parser_next_argument(p, &type); //Toma el primer argumento 
    
    while(arg != NULL){ //Si el argumento no es nulo, comenzamos a revisar el resto
    //Dependiendo del tipo se lo asignamos a un campo de nuestro command 
        if (type == ARG_NORMAL){
            scommand_push_back(result, arg);
        } else if (type == ARG_INPUT){
            scommand_set_redir_in(result, arg);
        } else if (type == ARG_OUTPUT){
            scommand_set_redir_out(result, arg);
        }
    //Ahora tomamos un nuevo argumento para asi poder analizar todos los que conforman nuestro comando simple 
        arg = NULL;
        arg = parser_next_argument(p, &type);
    }
    //Analisis de errores de parseo 
    if (scommand_is_empty(result) || type == ARG_INPUT){ //|| type == ARG_OUTPUT){
        result = scommand_destroy(result);
        result = NULL;
    }
    return result;
}

pipeline parse_pipeline(Parser p){
    assert(p != NULL && !parser_at_eof(p));

    pipeline result = pipeline_new(); 
    scommand cmd = NULL;
    bool error = false, another_pipe=true, background, garbage;

    
    cmd = parse_scommand(p); // (sort -r "example") | ls -l | grep "model"
    parser_skip_blanks(p);
    error = (cmd==NULL); //Comando inválido al empezar 

    while (another_pipe && !error) {
        parser_op_pipe(p, &another_pipe); //(sort -r "example" |) ls -l | grep "model"
        pipeline_push_back(result ,cmd); //Introduce el comando a nuestro pipeline 
        if (another_pipe == true){
            cmd = parse_scommand(p); // Agarramos el otro comando simple, (sort -r "example" | ls -l) | grep "model & \n"
            error = (cmd==NULL);
        }
    }

    if(!error){ //si no hay error luego de realizar el proceso de analisis, verificamos si se solicita operar en background 
        parser_op_background(p, &background);
        pipeline_set_wait(result, (!background)); 

        parser_garbage(p, &garbage); //verificamos si existen simbolos basura, si los hay sabemos que existe un error dentro de la sintaxis del codigo
        error = garbage;
    } else { //Para consumir la totalidad incluido el \n
         parser_garbage(p, &garbage); 
    }

    if (error) { //Si tiene un error lo destruimos 
        result = pipeline_destroy(result);
        result = NULL;
        printf("Command syntax Error\n");
    }
    return result; // MODIFICAR
}
