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
enum { RET_SUCCESS, RET_TIMELIMIT_EXCEEDED, RET_RUNTIME_ERROR, RET_INTERNAL_ERROR };
char input_file[MAXPATHLEN], output_file[MAXPATHLEN];
int timelimit = 5, children_pid;

void print_exit_message(int ret_code){
  char * names[] = {"SUCCESS", "TIMELIMIT EXCEEDED", "RUNTIME ERROR", "INTERNAL ERROR"};
  fprintf(stdout, "%d - %s\n", ret_code, names[ret_code]);
}

void timeout_handler(int sig){
  fprintf(stdout, "Subprocess timed out after %d seconds\n", timelimit);
  kill(children_pid, SIGKILL);
  print_exit_message(RET_TIMELIMIT_EXCEEDED);
  exit(RET_TIMELIMIT_EXCEEDED);
}

void usage(int argc, char * argv[]){
  printf("Usage: %s [options] input-file output-file program [arg1 arg2 ...]\n", argv[0]);
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
  int opt;
  while ((opt = getopt(argc, argv, "ht:")) != -1){
    switch (opt){
    case 't':
      timelimit = atoi(optarg);
      if (timelimit <= 0){
        fprintf(stderr, "ERROR: Timelimit must be a positive integer (received %s)!\n", optarg);
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

  if (argc - optind < 3){
    fprintf(stderr, "FATAL ERROR: wrong number of parameters.\n");
    usage(argc, argv);
    exit(RET_INTERNAL_ERROR);
  }

  children_pid = fork();
  if (children_pid == -1){
    fprintf(stdout, "ERROR forking sub-process. Panic!\n");
    print_exit_message(RET_INTERNAL_ERROR);
    exit(RET_INTERNAL_ERROR);
  }

  if (children_pid != 0){
    /* parent process */
    signal(SIGALRM, timeout_handler);
    alarm(timelimit);
    printf("Time limit is %d seconds\n", timelimit);

    int status;
    wait(&status);
    /* if we get here, the subprocess ended within the time-limit. Disable the alarm. */
    alarm(0);

    struct rusage usage;
    getrusage(RUSAGE_CHILDREN, &usage);
    double used_time =
      usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0 +
      usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;

    printf("Running time: %lf\n", used_time);
    if (WIFEXITED(status)){
      fprintf(stdout, "Subprocess ended SUCCESSFULLY and returned %d\n", WEXITSTATUS(status));
      print_exit_message(RET_SUCCESS);
      exit(RET_SUCCESS);
    }else if (WIFSIGNALED(status)){
      if (WTERMSIG(status) == SIGUSR1){
        fprintf(stdout, "Couldn't execute program.\n");
        print_exit_message(RET_INTERNAL_ERROR);
        exit(RET_INTERNAL_ERROR);
      }else{
        fprintf(stdout, "Subprocess ended ABNORMALLY with signal code %d\n", WTERMSIG(status));
        print_exit_message(RET_RUNTIME_ERROR);
        exit(RET_RUNTIME_ERROR);
      }
    }else{
      fprintf(stdout, "Unrecognized ending situation.\n");
      print_exit_message(RET_RUNTIME_ERROR);
      exit(RET_RUNTIME_ERROR);
    }
  }else{
    /* child process */
    argc -= optind;
    argv += optind;

    if (freopen(argv[0], "r", stdin) == NULL){
      fprintf(stderr, "FATAL ERROR: Cannot open input file %s: %s\n", argv[0], strerror(errno));
      raise(SIGUSR1);
    }
    if (freopen(argv[1], "w", stdout) == NULL){
      fprintf(stderr, "FATAL ERROR: Cannot open output file: %s\n", argv[1], strerror(errno));
      raise(SIGUSR1);
    }

    /* attempt to exec the child process */
    if (execv(argv[2], argv + 2) == -1){
      fprintf(stderr, "FATAL ERROR: Cannot execute program %s: %s\n", argv[2], strerror(errno));
    }
    raise(SIGUSR1);
  }
  /* This is limbo, should never get here */
  exit(RET_INTERNAL_ERROR);
}
