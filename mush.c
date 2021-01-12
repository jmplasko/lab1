#include "Functions.h"

int main(int argc, char *argv[]){
  FILE *infile = NULL;
  char buffer[BUFFERSIZE] = {0};
    
  if (2 < argc){
    fprintf(stderr, "usage: %s [ scriptfile ]\n", argv[0]);
    exit(1);
  }
  
  if (argc == 1){
    run_mush();
  }
  
  if (argc == 2){
    infile = fopen(argv[1], "r");
    
    if (infile == NULL){
      perror(argv[1]);
      exit(EXIT_FAILURE);
    }
    
    while(feof(infile) == 0){
      if ( fgets(buffer, BUFFERSIZE, infile) != NULL ){
        interpret_script(buffer);
      }
      memset(buffer, '\0', BUFFERSIZE);
    }
    fclose(infile);
  }
  
  return 1;
}