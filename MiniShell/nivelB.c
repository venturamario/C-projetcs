/*
PRACTICA 2  -   SISTEMAS OPERATIVOS
GRUPO: LA PEPA
Mario Ventura Burgos, Luis Miguel Vargas, Felip Toni Font
12-2021
*/

//__________________________________________________________________________________________________________________
//                                                      NIVEL B
//__________________________________________________________________________________________________________________


//---------------------------------------------------INCLUDE---------------------------------------------------
#include <errno.h>  //errno
#include <stdio.h>  //printf(), fflush(), fgets(), stdout, stdin, stderr, fprintf()
#include <stdlib.h> //setenv(), getenv()
#include <string.h> //strcmp(), strtok(), strerror()
#include <unistd.h> //NULL, getcwd(), chdir()
#include <sys/types.h> //pid_t
#include <sys/wait.h>  //wait()
#include <signal.h> //signal(), SIG_DFL, SIG_IGN, SIGINT, SIGCHLD
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR


//---------------------------------------------------DEFINE---------------------------------------------------------
#define _POSIX_C_SOURCE 200112L
#define DEBUGN1 1 //parse_args()
#define DEBUGN3 1 //execute_line()
#define PROMPT_PERSONAL 1 // si no vale 1 el prompt será solo el carácter de PROMPT
#define RESET_FORMATO "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS "\x1b[94m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 24 // cantidad de trabajos permitidos

//------------------------------------------DECLARACIÓN DE CONSTANTES------------------------------------------------
char const PROMPT = '$';            //char para el prompt del shell
int const MESSAGE_SIZE = 3000;    //Tamaño de los mensajes que se mostraán con sprintf

//------------------------------------------DECLARACIÓN DE VARIABLES------------------------------------------------
//variable global para guardar el nombre del minishell
static char mi_shell[COMMAND_LINE_SIZE]; 
//static pid_t foreground_pid = 0;
struct info_process {
    //STRUCT CON LOS ATRIBUTOS QUE DEFINEN UN PROCESO: pid, estado y línea de comando asociada
	pid_t pid; //pid del proceso
	char status; //'N','E','D','F': Ninguno, Ejecutándose, Detenido, Finalizado
	char cmd[COMMAND_LINE_SIZE]; //Línea de comando asociada
};
//Tabla de procesos. La posición 0 será para el foreground
static struct info_process jobs_list[N_JOBS];

//------------------------------------------DECLARACIÓN DE FUNCIONES------------------------------------------------
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs();
int internal_bg(char **args);
int internal_fg(char **args);
char *read_line(char *line);
int parse_args(char **args, char *line);
int execute_line(char *line);
void imprimir_prompt();
void reaper(int signum);
void ctrlc(int signum);


//---------------------------------------------DESARROLLO DE LAS FUNCIONES---------------------------------------------

int check_internal(char **args) {
    if (!strcmp(args[0], "cd"))
        return internal_cd(args);
    if (!strcmp(args[0], "export"))
        return internal_export(args);
    if (!strcmp(args[0], "source"))
        return internal_source(args);
    if (!strcmp(args[0], "jobs"))
        return internal_jobs(args);
    if (!strcmp(args[0], "bg"))
        return internal_bg(args);
    if (!strcmp(args[0], "fg"))
        return internal_fg(args);
    if (!strcmp(args[0], "exit"))
        exit(0);
    return 0; // no es un comando interno
}


int internal_cd(char **args) {
    printf("[internal_cd()→ comando interno no implementado]\n");
    return 1;
} 


int internal_export(char **args) {
    printf("[internal_export()→ comando interno no implementado]\n");
    return 1;
}


int internal_source(char **args) {
    printf("[internal_source()→ comando interno no implementado]\n");
    return 1;
}


int internal_jobs(char **args) {
    #if DEBUGN1 
        printf("[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n");
    #endif
    return 1;
}


int internal_fg(char **args) {
    #if DEBUGN1 
        printf("[internal_fg()→ Esta función enviará un trabajo detenido al foreground reactivando su ejecución, o uno del background al foreground ]\n");
    #endif
    return 1;
}


int internal_bg(char **args) {
    #if DEBUGN1 
        printf("[internal_bg()→ Esta función reactivará un proceso detenido para que siga ejecutándose pero en segundo plano]\n");
    #endif
    return 1;
}


void imprimir_prompt() {
#if PROMPT_PERSONAL == 1
    printf(NEGRITA ROJO_T "%s" BLANCO_T ":", getenv("USER"));
    printf(AZUL_T "MINISHELL" BLANCO_T "%c " RESET_FORMATO, PROMPT);
#else
    printf("%c ", PROMPT);

#endif
    fflush(stdout);
    return;
}


char *read_line(char *line) {
  
    imprimir_prompt();
    char *ptr=fgets(line, COMMAND_LINE_SIZE, stdin); // leer linea
    if (ptr) {
        // ELiminamos el salto de línea (ASCII 10) sustituyéndolo por el \0
        char *pos = strchr(line, 10);
        if (pos != NULL){
            *pos = '\0';
        } 
	}  else {   //ptr==NULL por error o eof
        printf("\r");
        if (feof(stdin)) { //se ha pulsado Ctrl+D
            fprintf(stderr,"Bye bye\n");
            exit(0);
        }   
    }
    return ptr;
}


int parse_args(char **args, char *line) {
    int i = 0;

    args[i] = strtok(line, " \t\n\r");
    #if DEBUGN1 
        //fprintf(stderr, GRIS "[parse_args()→ token %i: %s]\n" RESET_FORMATO, i, args[i]);
    #endif
    while (args[i] && args[i][0] != '#') { // args[i]!= NULL && *args[i]!='#'
        i++;
        args[i] = strtok(NULL, " \t\n\r");
        #if DEBUGN1 
            //fprintf(stderr, GRIS "[parse_args()→ token %i: %s]\n" RESET_FORMATO, i, args[i]);
        #endif
    }
    if (args[i]) {
        args[i] = NULL; // por si el último token es el símbolo comentario
        #if DEBUGN1 
            //fprintf(stderr, GRIS "[parse_args()→ token %i corregido: %s]\n" RESET_FORMATO, i, args[i]);
        #endif
    }
    return i;
}

//PERMITIR EJECUTAR COMANDOS EXTERNO
int execute_line(char *line) {
    char *args[ARGS_SIZE];
    pid_t pid, status;
    char command_line[COMMAND_LINE_SIZE];

    //copiamos la línea de comandos sin '\n' para guardarlo en el array de structs de los procesos
    memset(command_line, '\0', sizeof(command_line)); 
    strcpy(command_line, line); //antes de llamar a parse_args() que modifica line

    if (parse_args(args, line) > 0) {

        //COMPROBAR SI ES UN PROCESO INTERNO
        if (check_internal(args)) {
            return 1;
        } else {
            //HACER FORK PARA OBTENER PID DE PADRE E HIJOS
            pid = fork();
            //COMPROBAR SI ES EL PROCESO PADRE O HIJO
            if (pid == 0) {
                //ES EL PROCESO HIJO
                //>>>>>>>>>>>>>COMENTARIO DE DEBUGGING<<<<<<<<<<<<<<<<
                //fprintf(stderr, GRIS "[execute_line()→ PID hijo: %d]\n" RESET_FORMATO, getpid());
                //Asociar acción por defecto a SIGCHLD
                signal(SIGCHLD,SIG_DFL);
                //Ignorar SIGINT con signal (Cambiar comportamiento por defecto)
                signal(SIGINT,SIG_IGN); //Solo el padre hace uso de la función ctrlc(int signum)
                //Ejecutar comando externo con la función execvp()
                status = execvp(line, args);
                //Mostrar error en caso de que este suceda
                fprintf(stderr,ROJO_T "---ERROR AL HACER execvp() EN execute_line(char *line)---");
                //realizar un exit(-1);
                exit(-1);

            } else if (pid > 0) {
                //ES EL PROCESO PADRE
                //>>>>>>>>>>>>>COMENTARIO DE DEBUGGING<<<<<<<<<<<<<<<<
                //fprintf(stderr, GRIS "[execute_line()→ PID padre: %d nombre: (%s)]\n" RESET_FORMATO, getpid(),command_line);
                //ASOCIAR EL MANEJADOR ctrlc A LA SEÑAL SIGINT
                signal(SIGINT,ctrlc);
                //ACTUALIZAR LOS DATOS DE jobs_list[0] CON LOS DEL PROCESO HIJO EN FOREGROUND
                strcpy(jobs_list[0].cmd,command_line);  //Actualizar linea de comando
                jobs_list[0].status = 'E';              //Obligatorio establecer que está ejecutandose
                //Por si acaso, también establecer:
                jobs_list[0].pid = pid;

                //>>>>>>>>>>>>>COMENTARIO DE DEBUGGING<<<<<<<<<<<<<<<<
                fprintf(stderr, GRIS "[execute_line()→ PID padre: %d nombre: (%s)]\n" RESET_FORMATO, getpid(),command_line);
                fprintf(stderr, GRIS "[execute_line()→ PID hijo: %d nombre: (%s)]\n" RESET_FORMATO, jobs_list[0].pid,jobs_list[0].cmd);
                
                //SI HAY UN PROCESO HIJO EJECUTÁNDOSE EN PRIMER PLANO, EL PADRE ESPERA (FUNCIÓN pause())
                pause();

                //Esperar a recibir el cambio de informacion del hijo
                //wait(&status);------------------AHORA LO HACE reaper(int signum)
                //Resetear los datos de jobs_list[0] regresando el pid a 0 (indicar que no hay ningun proceso en foreground)
                //jobs_list[0].pid = 0; ----------AHORA LO HACE reaper(int signum)           

            } else {
                //HA HABIDO ERROR AL HACER FORK
                perror("ERROR: ");
            }
            #if DEBUGN3
            fprintf(stderr, GRIS "[execute_line()→ Proceso hijo %d (%s) finalizado con exit(), estado %d]\n" RESET_FORMATO, pid, command_line, WEXITSTATUS(status));
            #endif
        }
        
    }
    return 0;
}

//MÉTODO MAIN
int main(int argc, char *argv[]) {
    //argv[0] contiene el nombre del programa
    //argv[1] es un puntero a la primera línea de comandos proporcionada
    //argv[n] es el último argumento

    //Variable que representa una linea del minishell
    char line[COMMAND_LINE_SIZE];
    //Pone en el string line, 
    memset(line, 0, COMMAND_LINE_SIZE);

    //CREAR EL NOMBRE DEL MINISHELL
    strcpy(mi_shell,argv[0]);

    //MANEJADORES SIGCHLD,SIGINT
    signal(SIGCHLD,reaper); //Enviar señal a reaper(int signum)
    signal(SIGINT,ctrlc);   //Enviar señal a ctrlc(int signum)

    //INICIALIZAR LOS DATOS DE jobs_list[0] CON pid=0, status='N', Y cmd CON TODO A '\0'
    jobs_list[0].pid = 0;                           //pid a 0
    jobs_list[0].status = 'N';                      //status a 'N' (Ninguno)
    for(int i=0; i < COMMAND_LINE_SIZE; i++) {
        jobs_list[0].cmd[i] = '\0';                 //cmd = '\0'
    } 

    //ITERACIÓN PARA QUE EL SHELL SE EJECUTE
    while (1) {
        if (read_line(line)) { // !=NULL
            execute_line(line);
        }
    }
    return 0;
}

//MÉTODO REAPER (ENTERRADOR)
void reaper(int signum) {
    //MANEJADOR PROPIO DE SEÑAL ENVIADA A UN PROCESO CUANDO UNO DE SUS PROCESOS HIJOS TERMINA
    signal(SIGCHLD,reaper); //Volver a poner la instrucción
    //VARIABLES AUXILIARES
    int status;     //Status
    int acabado;    //Indica si ha acabado

    //COMPROBAR QUÉ PROCESO HIJO TERMINA SU EJECUCIÓN (si el hijo que acaba es el que está en primer plano)
    while ((acabado=waitpid(-1, &status, WNOHANG)) > 0) {
        //>>>>>>>>>>>>>COMENTARIOS DE DEBUGGING<<<<<<<<<<<<<<<<
        //VER SI EL PROCESO HIJO EN PRIMER PLANO HA ACABADO A CAUSA DE LA LLEGADA DE UNA SEÑAL
        //O SI HA FINALIZADO CON LA FUNCIÓN exit();
        if (WIFEXITED(status)) {
            //>>>>>>>>>>>>>COMENTARIOS DE DEBUGGING<<<<<<<<<<<<<<<<
            //HA FINALIZADO EL PROCESO HIJO EN PRIMER PLANO CON exit()
            fprintf(stderr, VERDE_T "reaper() --> Proceso hijo %d (%s) finalizado con exit(), en estado: %d]\n" RESET_FORMATO, jobs_list[0].pid,jobs_list[0].cmd,WEXITSTATUS(status));
        } else if(WIFSIGNALED(status)) {
            //>>>>>>>>>>>>>COMENTARIOS DE DEBUGGING<<<<<<<<<<<<<<<<
            //HA FINALIZADO AL RECIBIR UNA SEÑAL
            fprintf(stderr, VERDE_T "reaper() --> Proceso hijo %d (%s) finalizado al recibir señal %d]\n" RESET_FORMATO, jobs_list[0].pid,jobs_list[0].cmd,WTERMSIG(status));
        }

        //RESETEAR pid, PONER status EN 'F', ESTABLECER cmd[indice] A '\0'
        for(int indice = 0; indice < COMMAND_LINE_SIZE; indice++) {
            jobs_list[0].cmd[indice] = '\0';
        }
        jobs_list[0].pid = 0;
        jobs_list[0].status = 'F';
        
    }

}

//MÉTODO CTRLC
void ctrlc(int signum) {
    //MANEJADOR PROPIO PARA LA SEÑAL SIGINT (ctrl+C)
    signal(SIGINT,ctrlc); //Volver a poner la instrucción
    //DECLARAR EL MENSAJE PARA PODER HACER sprintf
    char mensaje[MESSAGE_SIZE];
    //DAR VALOR AL MENSAJE CREADO
    sprintf(mensaje, VERDE_T "\nctrlc()--> PROCESO CON 'PID' %d DE NOMBRE (%s), PROCESO EN FOREGROUND CON 'PID' %d DE NOMBRE (%s). MIRAR SI SE ENVIA SEÑAL %d\n"RESET_FORMATO,getpid(),mi_shell,jobs_list[0].pid,jobs_list[0].cmd,SIGTERM);
    //ESCRIBIR EL MENSAJE POR PANTALLA
    write(2,mensaje,strlen(mensaje));

    /*
    para que no afecte también a los de segundo plano, enviaremos a través del padre una señal diferente, SIGTERM,
    para controlar la terminación sólo del proceso en primer plano, siempre y cuando éste no sea nuestro propio mini shell
    */

    //HAY PROCESO EN FOREGROUND? (pid>0)
    if (jobs_list[0].pid > 0) {
        //EN CASO DE HABERLO, ES EL MINI SHELL? (nombre_proceso_actual == nombre_minishell)
        int iguales = strcmp(jobs_list[0].cmd, mi_shell);
        if (iguales == 0) {
            //EL PROCESO EN FOREGROUND ES EL SHELL
            sprintf(mensaje, VERDE_T "ctrlc() --> PROCESO EN FOREGOUND %d (%s) es el minishell: NO SE ENVÍA SEÑAL %d ]\n" RESET_FORMATO, getpid(),mi_shell,SIGTERM);
            write(2,mensaje,strlen(mensaje));

        } else {
            //SE ENVÍA LA SEÑAL SIGTERM MEDIANTE LA FUNCIÓN KILL Y EL pid DEL PROCESO DETECTADO
            kill(jobs_list[0].pid,SIGTERM);
            //EL PROCESO EN FOREGROUND NO ES EL SHELL
            sprintf(mensaje, VERDE_T "ctrlc() --> PROCESO EN FOREGOUND %d (%s) no es el minishell: SE ENVÍA LA SEÑAL %d CON KILL ]\n" RESET_FORMATO, jobs_list[0].pid,jobs_list[0].cmd,SIGTERM);
            write(2,mensaje,strlen(mensaje));
        }
    } else {
        sprintf(mensaje, VERDE_T "ctrlc() --> NO HAY PROCESO EN FOREGROUND. SOY EL PROCESO CON 'PID' %d (%s). SEÑAL %d NO ENVIADA ]\n" RESET_FORMATO, getpid(),mi_shell,SIGTERM);
        write(2,mensaje,strlen(mensaje));
        //PARA SIMULAR UNA CONSOLA
        imprimir_prompt();
    }
}


//__________________________________________________________FIN DEL NIVEL B______________________________________________________