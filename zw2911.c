// must compile with -std=c99 -Wall -o checkdiv

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mpi.h>


int main(int argc, char *argv[]){

unsigned int x, n;
unsigned int i; //loop index
FILE * fp; //for creating the output file
char filename[100]=""; // the file name
int * numbers; //the numbers in the range [2, N]

clock_t start_p1, start_p2, start_p3, end_p1, end_p2, end_p3;

/////////////////////////////////////////
// start of part 1

start_p1 = clock();
// Check that the input from the user is correct.
if(argc != 3){

  printf("usage:  ./checkdiv N x\n");
  printf("N: the upper bound of the range [2,N]\n");
  printf("x: divisor\n");
  exit(1);
}



// Process 0 must send the x and n to each process.
// Other processes must, after receiving the variables, calculate their own range.

// initialize the MPI communicators
int my_rank, comm_sz;

MPI_Init (&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

if (my_rank != 0) {
  MPI_Recv(argv, argc, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
} else {
  for (int q = 1; q < comm_sz; q++) {
    MPI_Send(argv, argc, MPI_CHAR, q, 0, MPI_COMM_WORLD);
  }
}

n = (unsigned int)atoi(argv[1]);
x = (unsigned int)atoi(argv[2]);

end_p1 = clock();
//end of part 1
/////////////////////////////////////////

/////////////////////////////////////////
//start of part 2
start_p2 = clock();

// The main computation part starts here
if (my_rank == 0) {
  numbers = (int*)malloc(sizeof(int) * (n - 2 + 1));
  for (i = 0; i < (n-2+1); i++) {
    numbers[i] = 2 + i;
  }
}

unsigned int num = n - 2 + 1;
unsigned int local_n = (num%comm_sz == 0) ? num/comm_sz : (num/comm_sz+1);
int * local_a = malloc(sizeof(int) * local_n);

MPI_Scatter(numbers, local_n, MPI_UNSIGNED, local_a, local_n, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

int * divi_list = NULL;
int * mylist = malloc(sizeof(int) * local_n);
int count = 0;

if (my_rank == comm_sz - 1) {
  for (i = 0; i < local_n; i++) {
    if(i < (num - local_n * (comm_sz-1))) 
    {
      if (local_a[i] % x == 0)
       mylist[count] = local_a[i];
      else 
       mylist[count] = 0;
    }
    else {
      mylist[count] = 0;
    }
    count++;
  }
} else {
  for (i = 0; i < local_n; i++) {
    if (local_a[i] % x == 0)
     mylist[count] = local_a[i];
    else 
     mylist[count] = 0;
    count++;  
  }
}


if (my_rank == 0) {
  divi_list = malloc(sizeof(int) * (comm_sz * local_n));
} 
MPI_Gather(mylist, local_n, MPI_UNSIGNED, divi_list, local_n, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

end_p2 = clock();
// end of the main compuation part
//end of part 2
/////////////////////////////////////////


/////////////////////////////////////////
//start of part 3
// Writing the results in the file

//forming the filename

start_p3 = clock();

strcpy(filename, argv[1]);
strcat(filename, ".txt");

if( !(fp = fopen(filename,"w+t")))
{
  printf("Cannot create file %s\n", filename);
  exit(1);
}

//Write the numbers divisible by x in the file as indicated in the lab description.
if (my_rank == 0) {
  for (i = 0; i < (comm_sz*local_n); i++) {
    if (divi_list[i] != 0) {
      char str[100];
      sprintf(str, "%d", divi_list[i]);
      fputs(str, fp);
      fputs("\n", fp);
    }
  }
}

fclose(fp);

end_p3 = clock();
//end of part 3
/////////////////////////////////////////

double time_p1 = (double)(end_p1-start_p1)/CLOCKS_PER_SEC;
double time_p2 = (double)(end_p2-start_p2)/CLOCKS_PER_SEC;
double time_p3 = (double)(end_p3-start_p3)/CLOCKS_PER_SEC;

double p1;
double p2;
double p3;

MPI_Reduce(&time_p1, &p1, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
MPI_Reduce(&time_p2, &p2, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
MPI_Reduce(&time_p3, &p3, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

/* Print  the times of the three parts */
if (my_rank == 0) {
printf("time of part1 = %lf s part2 = %lf s part3 = %lf s\n", p1, p2, p3);
}
MPI_Finalize();

return 0;
}
