#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>


char input_file[MAXPATHLEN], output_file[MAXPATHLEN];

/* Return codes */
#define RET_ACCEPTED 0
#define RET_TIMELIMIT_EXCEEDED 1
#define RET_RUNTIME_ERROR 2
#define RET_WRONG_ANSWER 3
#define RET_PRESENTATION_ERROR 4
#define RET_COMPILE_ERROR 5
#define RET_INTERNAL_ERROR 6

char input_file[MAXPATHLEN], output_file[MAXPATHLEN];

char buffer[MAXPATHLEN];
int timelimit = 5;

void print_exit_message(int ret_code){
  char * names[] = {"ACCEPTED", "TIMELIMIT EXCEEDED", "RUNTIME ERROR", "WRONG ANSWER", "PRESENTATION ERROR", "COMPILE ERROR", "INTERNAL ERROR"};
  fprintf(stdout, "%d - %s\n", ret_code, names[ret_code]);
}


void usage(int argc, char * argv[]){
  printf("Usage: %s judge-input-file judge-output-file source-code.c [inputfile][test.txt][source-code]", argv[0]);
  printf("\n");
  printf("Available options:\n");
  printf("\t-t num\tThe subprocess will be killed after num seconds of execution (Default: 5 seconds)\n");
  printf("\t-h\tPrint this help and quit.\n");
  printf("\n");
  printf("Return value:\n");
  printf("\t0\tSubprocess finished correctly\n");
  printf("\t1\tSubprocess timed out\n");
  printf("\t2\tSubprocess finished abnormally\n");
  printf("\t3\tSubprocess was not executed because an error occured\n");
}



int main(int argc, char * argv[]){
  //parse options
  int opt, ret;
  while ((opt = getopt(argc, argv, "ht:")) != -1){
    switch (opt){
    case 't':
      timelimit = atoi(optarg);
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
  //check ret != -1
  if (ret != 0){
    print_exit_message(RET_COMPILE_ERROR);
    exit(RET_COMPILE_ERROR);
  }

  sprintf(buffer, "./jaula %s %s %s", argv[0], argv[1], "a.out");
  ret = system(buffer);
  //check ret != -1
  if (ret == RET_TIMELIMIT_EXCEEDED){
    print_exit_message(RET_TIMELIMIT_EXCEEDED);
    exit(RET_TIMELIMIT_EXCEEDED);
  }

  if (ret == RET_RUNTIME_ERROR){
    print_exit_message(RET_RUNTIME_ERROR);
    exit(RET_RUNTIME_ERROR);
  }
  
  //here we attempt to execute the compiled program called a.out and save the text in the file test.txt
  if(system("./a.out> test.txt"!=0)){
    //by this command we compare the text file generated by a.out with test.txt
     sprintf(buffer,"diff -s %s %s\n",argv[1],"test.txt");
     int res= system(buffer);
     if(res==0){
       fprintf(stderr, "the files are exacly the same\n");
     }
     else{
       fprintf(stderr, "the files are diferent\n");
     }
    }
  else{
    fprintf("there was an error executing the the compiled program");
  }

  exit(RET_INTERNAL_ERROR);
}
