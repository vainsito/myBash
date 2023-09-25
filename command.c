#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <assert.h>
#include "strextra.h"
#include "command.h"

struct scommand_s {
    GQueue *queue; 
    GString *in;
    GString *out;
};

scommand scommand_new(void){
    scommand result = malloc(sizeof(struct scommand_s));
    result->queue = g_queue_new();
    result->in = NULL;
    result->out = NULL;
    assert(result != NULL 
    && scommand_is_empty (result) 
    && scommand_get_redir_in (result) == NULL 
    && scommand_get_redir_out (result) == NULL
    );
    return result;
}

scommand scommand_destroy(scommand self){  
    assert(self != NULL);
    g_queue_free_full(self->queue, free);
    // Si hay un archivo de entrada, lo borramos
    if (self->in != NULL){
        g_string_free(self->in, true);
    }
    // Si hay un archivo de salida, lo borramos
    if (self->out != NULL){
        g_string_free(self->out, true);
    }
    free(self);
    self = NULL;
    assert(self == NULL);
    return self;
}

void scommand_push_back(scommand self, char * argument){
    assert(self != NULL && argument != NULL);
    g_queue_push_tail(self->queue, argument);
    assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
    assert(self != NULL && !scommand_is_empty(self));
    g_queue_delete_link(self->queue, self->queue->head);
}

void scommand_set_redir_in(scommand self, char * filename){
    assert(self != NULL);
    // Si ya hay un archivo de entrada, lo borramos
    if (self->in != NULL){
        g_string_free(self->in, true);
    }
    // Seteamos el nuevo archivo de entrada
    if (filename != NULL){
        self->in = g_string_new(filename);
    }
    else{
        self->in = NULL;
    }
}

void scommand_set_redir_out(scommand self, char * filename){
    assert(self != NULL);
    if (self->out != NULL){
        g_string_free(self->out, true);
    }
    // Seteamos el nuevo archivo de entrada
    if (filename != NULL){
        self->out = g_string_new(filename);
    }
    else{
        self->out = NULL;
    }
}

bool scommand_is_empty(const scommand self){
    assert(self != NULL);
    return ( self->queue->head == NULL && self->queue->tail == NULL && self->in == NULL && self->out == NULL);
}

unsigned int scommand_length(const scommand self){
    assert(self != NULL);
    unsigned int tam = g_queue_get_length(self->queue);
    assert((tam == 0) == scommand_is_empty(self));
    return tam;
}

char * scommand_front(const scommand self){
    assert(self != NULL && !scommand_is_empty(self));
    char* result = g_queue_peek_head(self->queue);
    assert(result != NULL);
    return result;
}

char *scommand_get_redir_in(const scommand self) {
   assert(self != NULL); // Asegurarse de que self no sea NULL
    
    if (self->in != NULL && self->in->str != NULL) {
        return self->in->str;
    } else {
        return NULL;
    }
}

char * scommand_get_redir_out(const scommand self){
    assert(self != NULL);
    if (self->out == NULL){
        return NULL;
    } else {
        return self->out->str;
    }
}

char * scommand_to_string(const scommand self){
    assert(self != NULL);
    char *result = NULL;
    result = strdup("");
    if (scommand_length(self) != 0){
        result = strdup("");
        unsigned int queue_length = scommand_length(self);
        for (unsigned int i = 0u; i < queue_length; i++){
            result = strmerge(result,scommand_front(self));
            result = strmerge(result, " ");
            scommand_pop_front(self);
        }
    }
    if (self->in != NULL){
        result = strmerge(result," < ");
        result = strmerge(result,scommand_get_redir_in(self));
    }
    if (self->out != NULL){
        result = strmerge(result," > ");
        result = strmerge(result,scommand_get_redir_out(self));
    }
    assert(scommand_is_empty(self) || scommand_get_redir_in(self)==NULL || scommand_get_redir_out(self)==NULL || strlen(result) > 0);
    return result;
}

struct pipeline_s{
    GList *pipe;
    bool state;
};

pipeline pipeline_new(void){
    pipeline self = malloc(sizeof(struct pipeline_s));
    self->pipe = NULL;
    self->state = true;
    assert(pipeline_is_empty(self) && pipeline_get_wait(self)); // Resto de las aserciones
    return self;
}


pipeline pipeline_destroy(pipeline self){
    assert(self != NULL);
    if (!pipeline_is_empty(self)){
        while (self->pipe != NULL){
            self->pipe->data = scommand_destroy(self->pipe->data);
            self->pipe = self->pipe->next;
        }
        g_list_free_full(self->pipe,free);
    }
    free(self);
    self = NULL;
    assert(self == NULL);
    return self;
}

void pipeline_push_back(pipeline self, scommand sc){
    assert(self != NULL && sc != NULL);
    self->pipe = g_list_append(self->pipe, sc);
    assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self){
    assert(self != NULL && !pipeline_is_empty(self));
    self->pipe = g_list_remove(self->pipe, self->pipe->data);
}

void pipeline_set_wait(pipeline self, const bool w){
    assert(self != NULL);
    self->state = w;
}

bool pipeline_is_empty(const pipeline self){
    assert(self != NULL);
    return (self->pipe == NULL);
}

unsigned int pipeline_length(const pipeline self){
    assert(self != NULL);
    unsigned int tam = g_list_length(self->pipe);
    assert((tam == 0) == pipeline_is_empty(self));
    return tam;
}

scommand pipeline_front(const pipeline self) {
    assert(self != NULL && !pipeline_is_empty(self));
    GList *firstElement = g_list_first(self->pipe);
    scommand result = firstElement->data;
    assert(result != NULL); // Verificar que el elemento sea una instancia válida de scommand
    return result;
}

bool pipeline_get_wait(const pipeline self){
    assert(self != NULL);
    return (self->state == true);
}

char * pipeline_to_string(const pipeline self){
    assert(self != NULL);
    char *result = malloc(sizeof(char));
    result = strdup("");
    if (!pipeline_is_empty(self)){
        result = strmerge(scommand_to_string(pipeline_front(self)),result); 
        pipeline_pop_front(self);

        if (pipeline_length(self) != 0){
            unsigned int pipe_length = pipeline_length(self);
            for(unsigned int i = 0u ; i < pipe_length; i++){ 
                result = strmerge(result, " | ");
                result = strmerge(result,scommand_to_string(pipeline_front(self))); 
                pipeline_pop_front(self);
            }
        }
    }
    if (!pipeline_get_wait(self)){
        result = strmerge(result, " &");
    } 
    assert(pipeline_is_empty(self) || pipeline_get_wait(self) || strlen(result) > 0);
    return result;
}