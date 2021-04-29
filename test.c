#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>

void main()
{

  FILE *fp;
  char testread[1000];
  char testget[1000];
  int read_size;
  int i;
  char filename[100]= "./test.txt";
  if (( fp = fopen(filename, "r")) == NULL){
  
    printf("file not opend.\n");
  }
  else
  {
    printf("file opend.\n");
    while ( (read_size = fread(testread, 1, 5, fp)) > 0)
    {
      for (i=0; i<read_size; i++)
      {
        if (testread[i] == 10 || testread[i]== EOF)   
        {
          printf("testread[%d] = %d\n",i, testread[i]);
        }
      }
    }
    fclose(fp);
  }
  if (( fp = fopen(filename, "r")) == NULL){
  
    printf("file not opend.\n");
  }
  else
  {
    printf("file opend.\n");
    while(fgets(testget, 5, fp) != NULL)
    {
      for(i =0; i < 5; i++)
      {
        printf("fgetsi[%d]: %d\n", i,testget[i]);
      }
    }
    fclose(fp);
  }
}
