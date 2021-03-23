#include <stdio.h>
#include <stdlib.h>
#include "Et_main.h"

double main(int argc, char *argv[]) {

  if(argc<=1) {
    printf("You did not feed me arguments, I will die now :( ...");
    exit(1);
  }  //otherwise continue on our merry way....
  int method_integer = atoi(argv[1]);  //argv[0] is the program name
                              //atoi = ascii to int
  
  return run_et_main(method_integer);

}
