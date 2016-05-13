# pl0Compiler
pl0 Compiler for Systems Software

##Building and running the compiler
Keep the C source and header files in the same directory

Change over to that directory in the terminal and run the following command
  
	gcc -o main compiler.c
  
This will generate an executable named main. To run it use the command
  
	./main 
  
followed by any of the following flags as arguments in any order

	-l
	-a
	-v
	
-l tells the compiler to print the lexeme list to the console

-a tells the compiler to print the generated machine code to the console

-v tells the compiler to print a stack trace to the console

	read
or

	write
statements in the code will cause the console to either prompt for or print out a value
	

