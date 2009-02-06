#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>

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

char buffer[MAXPATHLEN];

void process_run(int id, char input[], char output[], char source[]){
  printf("Run %d:\n", id);
  sprintf(buffer, "./verdugo %s %s %s", input, output, source);
  struct timeval begin, end;
  gettimeofday(&begin, NULL);
  int ret = system(buffer);
  if (ret == -1){
    printf("Problem executing file: %s\n", strerror(errno));
    return;
  }
  ret = WEXITSTATUS(ret);
  gettimeofday(&end, NULL);
  printf("\tRunning time: %lf\n",
         end.tv_sec + end.tv_usec / 1000000.0
         - begin.tv_sec - begin.tv_usec / 1000000.0);
}

int main(int argc, char * argv[]){
  struct timeval begin, end;
  int how_many, i;
  printf("How many runs do you want to run? ");
  scanf("%d", &how_many);

  gettimeofday(&begin, NULL);
  for (i=0; i<how_many; ++i){
    //Esperar_solicitud
    char *input, *output, *source;
    int run_id = i % 11;
    input = premade_runs[run_id][0];
    output = premade_runs[run_id][1];
    source = premade_runs[run_id][2];

    //atender_solicitud
    process_run(i, input, output, source);
  }
  gettimeofday(&end, NULL);
  printf("Total running time: %lf\n",
         end.tv_sec + end.tv_usec / 1000000.0
         - begin.tv_sec - begin.tv_usec / 1000000.0);

  return 0;
}

