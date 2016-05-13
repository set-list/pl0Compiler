//Sam Verma

#include "header.h"


symTable symTab[MAXSYMTABSIZE];
Token currentToken;
int symTablePos = 0, tokenTablePos = 0;
instruction MCode[MAXCODELENGTH];
int mcodePos = 0, currentM = 0, lexLevel = 0;
int col = 0, row = 1;
int counted = 0, numProcedures = 0;
int varLevel = 0, constLevel = 0, varNum = 0, constNum = 0;
char currentProc[identMax];
void analyze();
void runBlock();
void constFound();
void varFound();
void statementFound();
void expressionFound();
void termFound();
void factorFound();
void operationFound();
void procedureFound();
void voidSyms(int level);
void pushSymTable(int kind, Token t, int L, int M, int num);
int searchSym(char *name, int level);
void fetchToken();
void printMCode(int flag);
void toCode(int OP, int L, int M);
void toFile();
int toInt(char *num);

void parser(int flag){
    int i;

    fileMCode = fopen(nameMCode,"w");
    if(fileMCode == NULL)
        printError(1);
    analyze();
    //printf("No errors, program is syntactically correct.\n");
    toFile();
    fclose(fileMCode);
    printMCode(flag);
}

void analyze(){
    fetchToken(); //get first token 
    runBlock(); //block
    if(currentToken.type != periodsym){ //"."
        printf("\nError: Line:%d, Column:%d :: ",row,col);
        printError(24);
    }
}

void runBlock(){
    counted = 0;
    int tempBlockPos = mcodePos, temPos;
    currentM = 0;

    toCode(7,0,0); 

    while(currentToken.type == constsym || currentToken.type == varsym){
    if(currentToken.type == constsym)
        constFound();
    if(currentToken.type == varsym)
        varFound();
    counted++;
    }

    temPos = currentM; //current mcode pos

    while(currentToken.type == procsym)
        procedureFound();

    MCode[tempBlockPos].M = mcodePos;

    toCode(6,0,temPos + 4);
    statementFound();
    if(currentToken.type != periodsym && currentToken.type == semicolonsym){
        toCode(2,0,0); //return proc
        voidSyms(lexLevel);
    }
    else
        toCode(9,0,2);
}

void constFound(){
    Token tempT;
    int returner = 0;
    if(constNum >= 1){
        if(constLevel == lexLevel){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(25);
        }
    }
    constNum++;
    constLevel = lexLevel;
    do{
        fetchToken();
        if(currentToken.type != identsym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(8); //const/int/proc must have ident after
        }
        returner = searchSym(currentToken.name, lexLevel);
        if(returner != -1 && symTab[returner].level == lexLevel){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(26); //semicolon needed between statements
        }

        strcpy(tempT.name, currentToken.name); //copy into temp

        fetchToken();
        if(currentToken.type != eqlsym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(4); //equals wanted after const declaration
        }

        fetchToken();
        if(currentToken.type != numbersym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(5); //number wanted after equals in const
        }

        pushSymTable(1, tempT, constLevel, -5, toInt(currentToken.name));
        fetchToken();
    } while(currentToken.type == commasym);

    if(currentToken.type != semicolonsym){
        printf("\nError: Line:%d, Column:%d :: ",row,col);
        printError(13); //semicolon needed between statements
    }

    fetchToken();
}

void varFound(){
    int returner = 0;
    if(varNum >= 1){
        if(varLevel == lexLevel){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(25);
        }
    }
    varNum++;
    varLevel = lexLevel;

    do{
        fetchToken();
        if(currentToken.type != identsym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(8); //const/int/proc must have ident after
        }//see if exists already

        returner = searchSym(currentToken.name, lexLevel);
        if(returner != -1 && symTab[returner].level == lexLevel){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(26); //missing semicolon
        }
        pushSymTable(2, currentToken, lexLevel, currentM+4, 0);
        fetchToken();
    } while(currentToken.type == commasym);

    if(currentToken.type != semicolonsym){
        printf("\nError: Line:%d, Column:%d :: ",row,col);
        printError(13); //missing semicolon
    }

    fetchToken();
}

void statementFound(){
    int symPos, identPos, tempBPos, temPos, temPos2;

    if(currentToken.type == identsym){
        symPos = searchSym(currentToken.name, lexLevel);

        if(symPos == -1){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printf("Identifier '%s': ", currentToken.name);
            printError(15); //undeclared variable
        }
        else if(symTab[symPos].kind == 1){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(12); //invalid assignment
        }

        identPos = symTab[symPos].addr;

        fetchToken();
        if(currentToken.type != becomessym){
            if(currentToken.type == eqlsym){
                printf("\nError: Line:%d, Column:%d :: ",row,col);
                printError(3); //use := not =
            }
            else{
                printf("\nError: Line:%d, Column:%d :: ",row,col);
                printError(9); //missing :=
            }
        }

        fetchToken();
        expressionFound();

        if(currentToken.type != semicolonsym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(13); 
        }

        toCode(4, lexLevel-symTab[symPos].level, identPos);
    }
    else if(currentToken.type == callsym){
        fetchToken();

        if(currentToken.type != identsym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(10);
        }

        symPos = searchSym(currentToken.name, lexLevel);

        if(symPos == -1){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printf("Identifier '%s': ", currentToken.name);
            printError(15); 
        }
        else if(symTab[symPos].kind == 1){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(12); 
        }

        fetchToken();

        toCode(5, lexLevel, symTab[symPos].addr);

    }
    else if(currentToken.type == beginsym){
        fetchToken();
        statementFound();

        while(currentToken.type == semicolonsym){
            fetchToken();
            statementFound();
        }
        if(currentToken.type != endsym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(14); //endsym expected
        }

        fetchToken();
    }
    else if(currentToken.type == ifsym){
        fetchToken();
        operationFound();
        if(currentToken.type != thensym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(6); //missing then
        }

        fetchToken();
        tempBPos = mcodePos;

        toCode(8,0,0);

        statementFound();
        MCode[tempBPos].M = mcodePos;

        fetchToken();

        if(currentToken.type != elsesym){
            tokenTablePos--;
            tokenTablePos--;
            currentToken.type = tokenList[tokenTablePos].type;
            strcpy(currentToken.name,tokenList[tokenTablePos].name);
            while(currentToken.type == newlinesym){
                tokenTablePos--;
                currentToken.type = tokenList[tokenTablePos].type;
                strcpy(currentToken.name,tokenList[tokenTablePos].name);
            }
            col--;
        }

        if(currentToken.type == elsesym){
            MCode[tempBPos].M = mcodePos+1;

            tempBPos = mcodePos;

            toCode(7,0,0);

            fetchToken();
            statementFound();
            MCode[tempBPos].M = mcodePos;
        }
    }
    else if(currentToken.type == whilesym){
        temPos = mcodePos;

        fetchToken();
        operationFound();

        temPos2 = mcodePos;

        toCode(8,0,0);

        if(currentToken.type != dosym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(7); //missing do
        }

        fetchToken();
        statementFound();

        toCode(7,0,temPos);

        MCode[temPos2].M = mcodePos;
    }
    else if(currentToken.type == readsym){
        fetchToken();

        if(currentToken.type == identsym){
            symPos = searchSym(currentToken.name, lexLevel);
            if(symPos == -1){
                printf("\nError: Line:%d, Column:%d :: ",row,col);
                printf("Identifier '%s': ", currentToken.name);
                printError(15); 
            }
            fetchToken();

            toCode(9,0,1); //read from screen

            toCode(4,0,symTab[symPos].addr); //increment mcode
        }
    }
    else if(currentToken.type == writesym){
        fetchToken();

        if(currentToken.type == identsym){
            symPos = searchSym(currentToken.name, lexLevel);
            if(symPos == -1){
                printf("\nError: Line:%d, Column:%d :: ",row,col);
                printf("Identifier '%s': ", currentToken.name);
                printError(15); 
            }
            fetchToken();
            if(symTab[symPos].kind == 1)
                toCode(1,0,symTab[symPos].val); //if constant
            else
                toCode(3,0,symTab[symPos].addr); //read from screen

            toCode(9,0,0); 
        }
        else{
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(10);
        }
    }
}

void expressionFound(){
    int thisOp;

    if(currentToken.type == plussym || currentToken.type == minussym){
        thisOp = currentToken.type;

        if(thisOp == minussym){
            fetchToken();
            termFound();
            toCode(2,0,1);
        }
    }
    else{
        termFound();
    }

    while(currentToken.type == plussym || currentToken.type == minussym){
        thisOp = currentToken.type;
        fetchToken();
        termFound();

        if(thisOp == plussym)
            toCode(2,0,2);
        else
            toCode(2,0,3);
    }
}

void termFound(){
    int thisOp;

    factorFound();

    while(currentToken.type == multsym || currentToken.type == slashsym){
        thisOp = currentToken.type;
        fetchToken();
        factorFound();

        if(thisOp == multsym)
            toCode(2,0,4);
        else
            toCode(2,0,5);
    }
}

void factorFound(){
    int symPos;

    if(currentToken.type == identsym){
        symPos = searchSym(currentToken.name, lexLevel);

        if(symPos == -1){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printf("Identifier '%s': ", currentToken.name);
            printError(15); 
        }

        if(symTab[symPos].kind == 1)
            toCode(1, 0, symTab[symPos].val);
        else
            toCode(3, lexLevel-symTab[symPos].level, symTab[symPos].addr);

        fetchToken();
    }
    else if(currentToken.type == numbersym){
        toCode(1, 0, toInt(currentToken.name));
        fetchToken();
    }
    else if (currentToken.type == lparentsym){
        fetchToken();
        expressionFound();

        if(currentToken.type != rparentsym){
            printf("\nError: Line:%d, Column:%d :: ",row,col);
            printError(16); //) missing
        }

        fetchToken();
    }
    else{
        printf("\nError: Line:%d, Column:%d :: '%s'",row,col,currentToken.name);
        printError(14); //cannot begin with this sym
    }
}

void operationFound(){
    int thisOp;
    if(currentToken.type == oddsym){
        toCode(2,0,6);
        fetchToken();
        expressionFound();
    }
    else{
        expressionFound();
        thisOp = currentToken.type;

        switch (currentToken.type) {
            case becomessym:
                printf("\nError: Line:%d, Column:%d :: ",row,col);
                printError(2);
                break;
            case eqlsym:
                thisOp = 8;
                break;

            case neqsym:
                thisOp = 9;
                break;

            case lessym:
                thisOp = 10;
                break;

            case leqsym:
                thisOp = 11;
                break;

            case gtrsym:
                thisOp = 12;
                break;

            case geqsym:
                thisOp = 13;
                break;

            default:
                printf("\nError: Line:%d, Column:%d :: ",row,col);
                printError(11); //missing rel op
                break;
        }
        fetchToken();
        expressionFound();
        toCode(2,0,thisOp);
    }
}

void procedureFound(){
    numProcedures++;
    fetchToken();
    if(currentToken.type != identsym){
        printf("\nError: Line:%d, Column:%d :: ",row,col);
        printError(8); //missing ident
    }
    strcpy(currentProc,currentToken.name);

    pushSymTable(3, currentToken, lexLevel, mcodePos, -1);

    lexLevel++;
    if(lexLevel > MAXLEXLEVELS){
        printf("\nError: :: ");
        printError(23);
    }
    numProcedures++;
    varNum = 0;

    fetchToken();
    if(currentToken.type != semicolonsym){
        printf("\nError: Line:%d, Column:%d :: ",row,col);
        printError(13); //missing semicolon
    }

    fetchToken();
    runBlock(); //run for the proc's insides
    lexLevel--;

    if(currentToken.type != semicolonsym){
        printf("\nError: Line:%d, Column:%d :: ",row,col);
        printError(13); //missing semicolon
    }
    strcpy(currentProc," ");
    fetchToken();
}

int searchSym(char *name, int level){
    int i;
    while(level != -1){
        for(i=symTablePos-1; i >= 0; i--){
            if((strcmp(name,symTab[i].name) == 0) && (symTab[i].addr != -1) && (symTab[i].level == level)){
                return i;
            }
        }
        level--;
    }
    return -1; //not found 
}

void voidSyms(int level){
    int i;
    for(i=symTablePos-1; i >= 0; i--){
        if(symTab[i].level == level && symTab[i].kind != 3 && symTab[i].addr != -1){
            symTab[i].addr = -1;
        }
    }
}

void pushSymTable(int kind, Token t, int L, int M, int num){
    symTab[symTablePos].kind = kind;
    strcpy(symTab[symTablePos].name,t.name);
    symTab[symTablePos].level = L;
    symTab[symTablePos].addr = M;
    if(kind == 1)
        symTab[symTablePos].val = num;
    else if (kind == 2)
        currentM++;
    else if (kind == 3){
        procedures[procPos][0] = M;
        procedures[procPos][1] = L+1;
        procPos++;
    }
    symTablePos++;
}

void fetchToken(){
    currentToken = tokenList[tokenTablePos];
    tokenTablePos++;
    if(currentToken.type == newlinesym){
        while(currentToken.type == newlinesym){
            row++;
            col = 0;
            fetchToken();
        }
    }
    else
		col++;
}

void printMCode(int flag){
    char c;
    fileMCode = fopen(nameMCode,"r");
    if(fileMCode == NULL)
        printError(1);
    if(flag){
        printf("Machine Code\n");
        c = fgetc(fileMCode);
        while(c != EOF){
            printf("%c",c);
            c = fgetc(fileMCode);
        }
        printf("\n\n");
    }
    fclose(fileMCode);
}

int toInt(char *num){
    int returner = 0, i = 0;
    while(num[i] != '\0'){
        returner *= 10;
        returner += num[i] - '0';
        i++;
    }
    return returner;
}

void toCode(int OP, int L, int M){
    MCode[mcodePos].OP = OP;
    MCode[mcodePos].M = M;
    MCode[mcodePos].L = L;
    mcodePos++;
}

void toFile(){
    int i;
    for(i=0; i< mcodePos; i++){
        fprintf(fileMCode,"%d %d %d\n",MCode[i].OP, MCode[i].L, MCode[i].M);
    }
}
