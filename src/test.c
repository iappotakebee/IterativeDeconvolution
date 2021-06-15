#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>

int  readSeparatedValueFiles(double **p_arr, int *row, int *col, char *fpth, int flg_silent)
{
  FILE *fp;
  char buf[BUFF_SIZE] *line;
  int  i,cnt=0, read_size=0, read_size_bef=0, 
  int  tmp_line_max=1, line_max=1;
  int  tmp_row=0, tmp_col_init=0, tmp_col=0, cnt_row,cnt_col;
  int  flg_work = VALID;
  char delimiter[2] = " ";
  *col = 0;
  /* open a file */
  if ( (fp = fopen(fpth, "r")) == NULL )
  {
    printf("Couldn't find a file.\n");
    flg_work = INVALID;
  }
  /* read characters by 1 byte */
  while ( (read_size=fread(buf, 1, BUFF_SIZE, fp)) > 0)
  {
    for (i=0; i<read_size; i++)
    {
      tmp_line_max++;
      if (tmp_row == 0)
      {
        if (buf[i] == '\t') 
        {
          delimiter[0] = '\t';
          tmp_col_init++;    
        }
        else if (buf[i] == ',') 
        {
          delimiter[0] = ','  ;
          tmp_col_init++;    
        }
        if ((buf[i] == ' ')&&(delimiter[0] == ' '))
        {
          tmp_col_init++;    
        }
      }
      else
      {
        if (buf[i]==delimiter[0]) tmp_col++;    
      }
      if (buf[i] == '\n') 
      {
        if (tmp_col != tmp_col_init)
        { 
          printf("Not matrix data.\n");
          flg_work = INVALID;
          break;
        }
        if (tmp_line_max > line_max) line_max = tmp_line_max;
        tmp_row++;    
        tmp_line_max = 1;
        tmp_col = 0;
      }
    }
    read_size_bef = read_size;
  }
  /* modify the number of rows according to the last letter */
  if (flg_work == VALID)
  {
    if (feof(fp) == 0)
    {
      printf("Invalid data.\n");
      flg_work = INVALID;
    }
    else if (buf[read_size_bef -1] != '\n' && feof(fp)!=0)
    {
      tmp_row++;
    }
    tmp_col = tmp_col_init;
  }
  printf("Read the %d items in\n %s\n", tmp_row*tmp_col, fpth[IDX_TGT]);
  fclose(fp);
  /* verify the data can be reshaped in the designated size */
  if (tmp_row * tmp_col == (*row)*(*col))
  {
    fp = fopen(fpth, "r");
    if ((*col)==1)
    {
      (*col) = (*row);
      (*row) = 1;
    }
    line = (char*) allocateVector(sizeof(char), line_max);
    p_arr = (double**) allocateMatrix(sizeof(double), (*row), (*col));
    cnt_row = 0;
    cnt_col = 0;
    while ( fscanf(fp, "%s", line) != EOF)
    {
      {
        p_arr[cnt_row][cnt_col] = atof(line);
        if(cnt_col == (*col)-1)
        {
          cnt_row++;
          cnt_col = 0;
        }
        else
        {
          cnt_col++;
        }
        memset(buf,'0',sizeof(line));
      }
    }
    if(cnt_row != (*row)-1 || cnt_col !=(*col)-1)
    {
      printf("conflicting number of rows or columns\n")
      flg_work == INVALID;
    }
    fclose(fp);
  }
  else
  {
    flg_work == INVALID;
  }
  return flg_work;

}
void main()
{

  double **test;
  char filename[100]= "./test.txt";
  readSeparatedValueFiles(testr, int *row, int *col, char *fpth, int flg_silent)
}
