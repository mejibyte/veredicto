#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>

//constants
#define MAXRUNS 1000

/* Return codes */
enum { RET_ACCEPTED, RET_TIMELIMIT_EXCEEDED, RET_RUNTIME_ERROR, RET_WRONG_ANSWER,
       RET_PRESENTATION_ERROR, RET_COMPILE_ERROR, RET_INTERNAL_ERROR };


char * premade_runs[11][3] = {
  //accepted
  {"data/a.in", "data/a.out", "solutions/a.c"},
  {"data/c.in", "data/c.out", "solutions/c.c"},

  //presentation error
  {"data/b.in", "data/b.out", "solutions/b.c"},

  //wrong answer
  {"data/a.in", "data/a.out", "solutions/b.c"},
  {"data/a.in", "data/a.out", "solutions/c.c"},
  {"data/b.in", "data/b.out", "solutions/a.c"},
  {"data/b.in", "data/b.out", "solutions/c.c"},
  {"data/c.in", "data/c.out", "solutions/a.c"},
  {"data/c.in", "data/c.out", "solutions/b.c"},

  //time limit exceeded
  {"data/c.in", "data/c.out", "solutions/infinite.c"},

  //runtime error
  {"data/c.in", "data/c.out", "solutions/runtime_error.c"}
};

//struct used to pass parameters to threads
struct thread_parameters_t{
  char input[MAXPATHLEN], output[MAXPATHLEN], source[MAXPATHLEN], buffer[MAXPATHLEN];
  int run_id;
};

//global variables
char buffer[MAXPATHLEN]; //All-use buffer
struct thread_parameters_t thread_parameters[MAXRUNS];
pthread_t thread_ids[MAXRUNS];
sem_t sem_screen; //Semaphore to avoid two threads writing to the screen at the same time

//function prototypes
void process_run(int id, char input[], char output[], char source[], char buffer[]);
void * thread_main(void * arg);
void print_verdict_message(int ret_code);
int main(int argc, char * argv[]);


int main(int argc, char * argv[]){
  struct timeval begin, end;
  int how_many, i;

  printf("How many runs do you want to run? ");
  scanf("%d", &how_many);
  if (how_many < 0 || how_many > MAXRUNS){
    fprintf(stderr, "ERROR: Number of runs must be between 0 and %d (both inclusive)\n", MAXRUNS);
    exit(1);
  }


  sem_init(&sem_screen, 0, 1); //One screen available

  gettimeofday(&begin, NULL);
  for (i=0; i<how_many; ++i){
    //Esperar_solicitud
    char *input, *output, *source;
    int run_id = i % 11;
    input = premade_runs[run_id][0];
    output = premade_runs[run_id][1];
    source = premade_runs[run_id][2];

    //atender_solicitud
    thread_parameters[i].run_id = i;
    memcpy(thread_parameters[i].input, input, strlen(input) + 1);
    memcpy(thread_parameters[i].output, output, strlen(output) + 1);
    memcpy(thread_parameters[i].source, source, strlen(source) + 1);
    int error_code = pthread_create(&thread_ids[i], NULL, thread_main, (void *) &thread_parameters[i]);
    //pthread_detach(thread_ids[i]); /* Create a detached thread, so its memory is freed as soon as it ends */
    //printf("Thread number %d has id = %lu. pthread_create returned %d\n", i, thread_ids[i], error_code);
    if (error_code != 0){
      thread_ids[i] = 0;
      sprintf(buffer, "ERROR: Creating thread %d: ", i);
      perror(buffer);
    }
  }

  for (i=0; i<how_many; ++i){
    if (thread_ids[i]){
        int error_code = pthread_join(thread_ids[i], NULL);
        if (error_code != 0){
          sprintf(buffer, "ERROR: pthread_join returned error code %d: \n", error_code);
          perror(buffer);
        }
      }
      }

    gettimeofday(&end, NULL);
    printf("Total running time: %lf\n",
           end.tv_sec + end.tv_usec / 1000000.0
           - begin.tv_sec - begin.tv_usec / 1000000.0);

    return 0;
  }


  void process_run(int id, char input[], char output[], char source[], char buffer[]){
    sprintf(buffer, "./verdugo %s %s %s > /dev/null", input, output, source);
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    int ret = system(buffer);
    if (ret == -1){
      printf("Problem executing file: %s\n", strerror(errno));
      return;
    }
    ret = WEXITSTATUS(ret);
    gettimeofday(&end, NULL);

    /* lock the screen using the semaphore (i.e. do a P) */
    sem_wait(&sem_screen);

    printf("Run %d:\n", id);
    print_verdict_message(ret);
    printf("\tRunning time: %lf\n",
           end.tv_sec + end.tv_usec / 1000000.0
           - begin.tv_sec - begin.tv_usec / 1000000.0);

    /* unlock the screen using the semaphore (i.e. do a V) */
    sem_post(&sem_screen);
  }


  void * thread_main(void * arg){
    struct thread_parameters_t * params = (struct thread_parameters_t *) arg;
    /* printf("I am thread %d.\n" */
    /*        "input = %s\n" */
    /*        "output = %s\n" */
    /*        "source = %s\n", params->run_id, params->input, params->output, params->source); */
    process_run(params->run_id, params->input, params->output, params->source, params->buffer);

    pthread_exit(NULL);
  }

  void print_verdict_message(int ret_code){
    char * names[] = {"ACCEPTED", "TIMELIMIT EXCEEDED", "RUNTIME ERROR", "WRONG ANSWER", "PRESENTATION ERROR", "COMPILE ERROR", "INTERNAL ERROR"};
    fprintf(stdout, "%d - %s\n", ret_code, names[ret_code]);
  }
