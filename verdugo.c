#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>

/* Return codes */
enum { RET_ACCEPTED, RET_TIMELIMIT_EXCEEDED, RET_RUNTIME_ERROR, RET_WRONG_ANSWER,
       RET_PRESENTATION_ERROR, RET_COMPILE_ERROR, RET_INTERNAL_ERROR };

char input_file[MAXPATHLEN], output_file[MAXPATHLEN];

char buffer[MAXPATHLEN];
int timelimit = 5;

void print_exit_message(int ret_code){
  char * names[] = {"ACCEPTED", "TIMELIMIT EXCEEDED", "RUNTIME ERROR", "WRONG ANSWER", "PRESENTATION ERROR", "COMPILE ERROR", "INTERNAL ERROR"};
  fprintf(stdout, "%d - %s\n", ret_code, names[ret_code]);
}


void usage(int argc, char * argv[]){
  printf("Usage: %s judge-input-file judge-output-file source-code.c", argv[0]);
  printf("\n");
  printf("Available options:\n");
  printf("\t-t num\tThe subprocess will be killed after num seconds of execution (Default: 5 seconds)\n");
  printf("\t-h\tPrint this help and quit.\n");
  printf("\n");
  printf("Return value:\n");
  printf("\t0\tAccepted\n");
  printf("\t1\tTimelimit exceeded\n");
  printf("\t2\tRuntime error\n");
  printf("\t3\tWrong answer\n");
  printf("\t4\tPresentation error\n");
  printf("\t5\tCompile error\n");
  printf("\t6\tInternal error while executing the program\n");
}



int main(int argc, char * argv[]){
  //parse options
  int opt, ret;
  while ((opt = getopt(argc, argv, "ht:")) != -1){
    switch (opt){
    case 't':
      timelimit = atoi(optarg);
      if (timelimit <= 0){
        fprintf(stderr, "Timelimit must be a positive integer!\n");
        print_exit_message(RET_INTERNAL_ERROR);
        exit(RET_INTERNAL_ERROR);
      }
      break;
    case 'h':
      usage(argc, argv);
      exit(RET_INTERNAL_ERROR);
      break;
    case '?':
      usage(argc, argv);
      exit(RET_INTERNAL_ERROR);
      break;
    }
  }

  argc -= optind;
  argv += optind;

  sprintf(buffer, "gcc -O1 %s", argv[2]);
  ret = system(buffer);
  if (ret == -1){
    fprintf(stderr, "FATAL ERROR: Cannot invoke gcc compiler. Panic!\n");
    exit(RET_INTERNAL_ERROR);
  }
  ret = WEXITSTATUS(ret);
  if (ret != 0){
    print_exit_message(RET_COMPILE_ERROR);
    exit(RET_COMPILE_ERROR);
  }

  sprintf(buffer, "./jaula -t %d %s %s %s > /dev/null 2> /dev/null", timelimit, argv[0], "output.tmp", "a.out");
  ret = system(buffer);

  if (ret == -1){
    fprintf(stderr, "FATAL ERROR: Cannot execute program. Panic!\n");
    exit(RET_INTERNAL_ERROR);
  }

  ret = WEXITSTATUS(ret);
  if (ret == RET_TIMELIMIT_EXCEEDED){
    print_exit_message(RET_TIMELIMIT_EXCEEDED);
    exit(RET_TIMELIMIT_EXCEEDED);
  }

  if (ret == RET_RUNTIME_ERROR){
    print_exit_message(RET_RUNTIME_ERROR);
    exit(RET_RUNTIME_ERROR);
  }

  sprintf(buffer, "diff %s %s\n", argv[1], "output.tmp");
  ret = system(buffer);
  if (ret == -1){
    fprintf(stderr, "FATAL ERROR: Cannot compare files. Panic!\n");
    exit(RET_INTERNAL_ERROR);
  }
  ret = WEXITSTATUS(ret);
  if(ret == 0){
    print_exit_message(RET_ACCEPTED);
    exit(RET_ACCEPTED);
  }


  sprintf(buffer, "diff -q -i -b -B -w %s %s\n", argv[1], "output.tmp");
  ret = system(buffer);

  if (ret == -1){
    fprintf(stderr, "FATAL ERROR: Cannot compare files. Panic!\n");
    exit(RET_INTERNAL_ERROR);
  }
  ret = WEXITSTATUS(ret);
  if(ret == 0){
    print_exit_message(RET_PRESENTATION_ERROR);
    exit(RET_PRESENTATION_ERROR);
  }

  print_exit_message(RET_WRONG_ANSWER);
  exit(RET_WRONG_ANSWER);
}
