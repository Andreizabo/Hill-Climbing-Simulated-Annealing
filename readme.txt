To compile "code.cpp" use:
g++ -pthread -o <output-name> code.cpp
It is important to include "-pthread" because the program uses threads and it won't compile without that argument.
Also, in order for the program to work, 12 folders need to be created:
"DeJong", "Michalewicz", "Schwefel" and "Rastrigin" need to be in the same folder with the program.
Inside each of these 4 folders need to be the following 3 folders:
"hc_best", "hc_first", "sa".
I have included an archive with the folders for easier use of the program.
The program is meant to be run with 12 threads.