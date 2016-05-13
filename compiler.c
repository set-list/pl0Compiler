//Sam Verma

#include "scanner.h"
#include "parser.h"
#include "vm.h"
#include "header.h"

int main(int argc, char *argv[]){
    int l=0,a=0,v=0,i=0;
    strcpy(inputFileName,"input.pl0");
    if(argc != 0){
        for(i=1; i<argc; i++){
            if(strcmp(argv[i], "-l") == 0)
                l = 1;
            else if(strcmp(argv[i], "-a") == 0)
                a = 1;
            else if(strcmp(argv[i], "-v") == 0)
                v = 1; 
        }
    }
    scanner(l);
    parser(a);
    vm(v);
    return 0;
}
