#include"input.h"

  /**********************************************************
    Allocate & deallocate matrix 
  **********************************************************/
void** allocateMatrix(int size, int m, int n)
{ 
  void **aa;
  int i;
  if (( aa = (void**)malloc( m * sizeof(void*) )) == NULL ){
    printf("Errors in memory allocation of aa. \n");
    exit(1);
  }  
  if (( aa[0] = (void*)malloc( m * n *  size )) == NULL ){
    printf("Errors in memory allocation of aa[0]. \n");
    exit(1);
  }  
  for(i=1; i<m; i++) aa[i]=(char*)aa[i-1] + size * n;
  return aa;
}
void deallocateMatrix(void **aa)
{
  free( aa[0] );
  free( aa    );
}
void* allocateVector(int size, int m)
{ 
  void *a;
  if (( a = (void*)malloc( m * size )) == NULL ){
    printf("Errors in memory allocation of a. \n");
    exit(1);
  }  
  return a;
}
void deallocateVector(void *a)
{
  free(a);
}
double **readSeparatedValueFiles
   (int *row, int *col, char *fpth, int flg_readasfile)
{
#ifdef _DEBUG_
  printf("readSeparatedValueFiles:\n");
#endif /* _DEBUG_ */
  FILE *fp;
  char buf[BUFF_SIZE], *line;
  int  i,cnt_row,cnt_col,cnt,
       read_size=0, read_size_bef=0,
       tmp_line_max=1, line_max=1, /* '\0' */
       tmp_row=0, tmp_col_init=0, tmp_col=0, 
       flg_work = VALID, flg_head = INVALID;
  char delimiter[2] = " ";
  double **p_arr;
  memset(buf,'\0',sizeof(char)*BUFF_SIZE);
  /* open a file */
  if ( (fp = fopen(fpth, "r")) == NULL )
  {
    printf("-- couldn't find a file.\n (%s)\n",fpth);
    flg_work = INVALID;
  }
  /* read characters by 1 byte */
  tmp_col_init=1;
  while ( flg_work==VALID && (read_size=fread(buf, 1, BUFF_SIZE, fp)) > 0)
  {
    for (i=0; i<read_size; i++)
    {
      tmp_line_max++;
      if (tmp_row == 0)
      {
        if (buf[i] != ' ')  flg_head = VALID;
        if (buf[i] == '\t' && flg_head == VALID) 
        {
          delimiter[0] = '\t';
          tmp_col_init++;    
        }
        else if (buf[i] == ',' && flg_head == VALID)
        {
          delimiter[0] = ','  ;
          tmp_col_init++;    
        }
        else if ((buf[i] == ' ') 
            && (delimiter[0] == ' ') 
            && (flg_head == VALID)
            )
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
        tmp_col++;
        if (
            tmp_col != tmp_col_init && 
            tmp_row > 0 
           )
        { 
          printf("-- not matrix data (%d out of %d) at row %d.\n",
              tmp_col, tmp_col_init, tmp_row);
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
      printf("-- invalid data.\n");
      flg_work = INVALID;
    }
    else
    {
      if (buf[read_size_bef -1] != '\n') 
      {
        tmp_row++;
        tmp_col++;
      }
      else 
      {
        printf("-- detected an extra new line at the end.\n"
            "-- tmp bef  : (%5d x %5d)\n",
            tmp_row, tmp_col);
        tmp_col = tmp_col_init;
        printf("-- tmp aft  : (%5d x %5d)\n",
            tmp_row, tmp_col);
      }
      if (tmp_col != tmp_col_init && tmp_row > 1)
      { 
        printf("-- not matrix data (%d out of %d) "
            "at the last row %d.\n",
            tmp_col, tmp_col_init, tmp_row);
        flg_work = INVALID;
      }
      if (tmp_line_max > line_max) line_max = tmp_line_max;
    }
    tmp_col = tmp_col_init;
  }
#ifdef _DEBUG_
  printf("-- read (%d x %d) items in\n %s\n", tmp_row,tmp_col, fpth);
#endif /* _DEBUG_ */
  if (flg_readasfile == VALID)
  {
    (*row) = tmp_row;
    (*col) = tmp_col;
#ifdef _DEBUG_
    printf("-- modified the number of rows and columns.\n");
#endif /* _DEBUG_ */
  }
  fclose(fp);
  /* verify the data can be reshaped in the designated size */
  if (flg_work == VALID && tmp_row * tmp_col == (*row)*(*col))
  {
    fp = fopen(fpth, "r");
    if ((*col)==1)
    {
      (*col) = (*row);
      (*row) = 1;
    }
#ifdef _DEBUG_
    printf("-- preparing for reading the data.\n"
           "    line : (%d) arrays\n"
           "    p_arr: (%d x %d) arrays\n",
        line_max, *row, *col
        );
#endif /* _DEBUG_ */
    line = (char*) allocateVector(sizeof(char), line_max);
    p_arr = (double**) allocateMatrix(sizeof(double), (*row), (*col));
    cnt_row = 0;
    cnt_col = 0;
    memset(buf,'\0',sizeof(char)*BUFF_SIZE);
#ifdef _DEBUG_
    printf("-- starting to scan each line.\n");
#endif /* _DEBUG_ */
    while ( fscanf(fp, "%s", line) != EOF)
    {
      cnt=0;
      for(i=0;i<line_max;i++)
      {
        if(
            (buf[cnt]=line[i])==delimiter[0] ||
             buf[cnt]         == '\0'        
          )
        {
          if(buf[0] != '\0') 
          {
            p_arr[cnt_row][cnt_col] = atof(buf);
            if(cnt_col+1 == (*col))
            {
              if (cnt_row +1 != (*row)) 
              {
                cnt_col = 0;
                cnt_row++;
              }
            }
            else
            {
              cnt_col++;
            }
          }
          memset(buf,'\0',sizeof(char)*BUFF_SIZE);
          cnt = 0;
        }
        else
        {
          cnt++;
        }
        if( line[i] == '\0')
        {
          memset(line,'\0',sizeof(char)*line_max);
          break;
        }
      }
    }
    if(cnt_row+1 != (*row) || cnt_col+1 !=(*col))
    {
      printf("-- conflicting number of rows or columns "
          "(%dx%d out of %dx%d)\n",
          cnt_row+1,cnt_col+1,(*row),(*col));
      flg_work = INVALID;
    }
    else
    {
#ifdef _DEBUG_
      printf("-- finished reading as expected.\n");
#endif /* _DEBUG_ */
    }
    deallocateVector((void*)line);
    fclose(fp);
  }
  else
  {
    flg_work = INVALID;
  }
  if(flg_work == VALID)
  {
    return p_arr;
  }
  else
  {
    return NULL;
  }
}

  /**********************************************************
    Initialize arrays or data
  **********************************************************/
int initMatrixToDblZero(int ni, int nj, double **aa)
{
  int i, j;
  for (i=0; i<ni; i++)
  {
    for (j=0; j<nj; j++)
    {
      aa[i][j] = 0.0;
    }
  }
  return 0;
}
int  initMatrixToRodriguesRotation
   (double* dest, double theta, double *n)
{
  printf("initMatrixToRodriguesRotation:\n");
  double c, s, n0, n1, n2;
  int PX=0,PY=1,PZ=2,DoF3=3;
  c = cos(theta);
  s = sin(theta);

  n0 = n[PX];
  n1 = n[PY];
  n2 = n[PZ];

  dest[PX+DoF3*PX] = c     + n0*n0*(1.0-c);
  dest[PX+DoF3*PY] = -n2*s + n0*n1*(1.0-c);
  dest[PX+DoF3*PZ] = n1*s  + n0*n2*(1.0-c);

  dest[PY+DoF3*PX] = n2*s  + n0*n1*(1.0-c);
  dest[PY+DoF3*PY] = c     + n1*n1*(1.0-c);
  dest[PY+DoF3*PZ] = -n0*s + n1*n2*(1.0-c);

  dest[PZ+DoF3*PX] = -n1*s + n0*n2*(1.0-c);
  dest[PZ+DoF3*PY] = n0*s  + n1*n2*(1.0-c);
  dest[PZ+DoF3*PZ] = c     + n2*n2*(1.0-c);
  //mir->rotmat_err[PX][PX] =  1.0; 
  //mir->rotmat_err[PX][PY] =  0.0; 
  //mir->rotmat_err[PX][PZ] =  0.0; 
  //mir->rotmat_err[PY][PX] =  0.0; 
  //mir->rotmat_err[PY][PY] =  cos(pitch_err); 
  //mir->rotmat_err[PY][PZ] =  sin(pitch_err); 
  //mir->rotmat_err[PZ][PX] =  0.0; 
  //mir->rotmat_err[PZ][PY] = -sin(pitch_err); 
  //mir->rotmat_err[PZ][PZ] =  cos(pitch_err); 
  
  //tmpmat[PX][PX] =  cos(roll_err); 
  //tmpmat[PX][PY] =  sin(roll_err); 
  //tmpmat[PX][PZ] =  0.0;
  //tmpmat[PY][PX] = -sin(roll_err); 
  //tmpmat[PY][PY] =  cos(roll_err); 
  //tmpmat[PY][PZ] =  0.0; 
  //tmpmat[PZ][PX] =  0.0; 
  //tmpmat[PZ][PY] =  0.0; 
  //tmpmat[PZ][PZ] =  1.0; 

  //tmpmat[PX][PX] =  cos(yaw_err); 
  //tmpmat[PX][PY] =  0.0;
  //tmpmat[PX][PZ] = -sin(yaw_err); 
  //tmpmat[PY][PX] =  0.0;
  //tmpmat[PY][PY] =  1.0;
  //tmpmat[PY][PZ] =  0.0;
  //tmpmat[PZ][PX] =  sin(yaw_err); 
  //tmpmat[PZ][PY] =  0.0;
  //tmpmat[PZ][PZ] =  cos(yaw_err); 
  return 0;
}
int initVectorToDblZero(int ni, double *a)
{
  int i;
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none) \
  private(i)\
  shared(ni, a)
#endif /*_SERIAL_CALCULATION */
#endif /* _OPENMP */
  for (i=0; i<ni; i++)
  {
      a[i] = 0.0;
  }
  return 0;
}
int initVectorToDblIdentity(int nj, double *a, int n_cols)
{
  int i, max_i=(int)nj/(n_cols+1);
  initVectorToDblZero(nj, a);
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none) \
  private(i)\
  shared(n_cols, a, max_i)
#endif /*_SERIAL_CALCULATION */
#endif /* _OPENMP */
  for (i=0; i<max_i; i++)
  {
      a[n_cols*i+i] = 1.0;
  }
  return 0;
}
int initVectorToIntZero(int ni, int *a)
{
  int i;
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none) \
  private(i)\
  shared(ni, a)
#endif /*_SERIAL_CALCULATION */
#endif /* _OPENMP */
  for (i=0; i<ni; i++)
  {
      a[i] = 0.0;
  }
  return 0;
}
int copyMatrix(int m, int n, double *dest, double *src)
{
#ifndef LAPACK_H
  int i, n_tot = m*n ;
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none) \
  private(i)\
  shared(n_tot,dest,src)
#endif /* _SERIAL_CALCULATION */
#endif /* _OPENMP */
  for(i=0; i<n_tot; i++)
  {
    dest[i]=src[i];
  }
#endif /* LAPACK_H */
#ifdef LAPACK_H
  int num=m*n,incx=1,incy=1;
  dcopy_(&num,src,&incx,dest,&incy);
#endif /* LAPACK_H */
  return 0;
}
int  trimMatrix
   (int m_dst, int n_dst, double *dest, 
    int i_st, int j_st, int m_src, int n_src, double *src)
{
  int i, j;
  printf("trimMatrix:\n");
  if(i_st + m_dst -1 < m_src && j_st + n_dst -1 < n_src)
  {
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none) \
  private(i,j)\
  shared(m_dst,n_dst,n_src,i_st,j_st,dest,src)
#endif /* _SERIAL_CALCULATION */
#endif /* _OPENMP */
    for(i=0; i<m_dst; i++)
    {
      for(j=0; j<n_dst; j++)
      {
        dest[n_dst*i+j]=src[(i_st + i)*n_src + (j_st + j)];
      }
    }
  }
  else
  {
    printf("-- invalid matrix size.\n");
    printf(
        "-- dst: (%5d x %5d)\n"\
        "-- src: (%5d x %5d)\n"\
        "-- st : (%5d , %5d)\n",
        m_dst, n_dst,
        m_src, n_src,
        i_st , j_st  
        );
  }
  return 0;
}
int convoluteMatMat (int m_dest, int n_dest, double *dest,
    int m_src,  int n_src,  double *src,
    int m_ker,  int n_ker,  double *ker)
{
#ifdef _DEBUG_
  printf("\nconvoluteMatMat:\n");
  printf("-- dst: (%d x %d)\n", m_dest, n_dest);
  printf("-- src: (%d x %d)\n", m_src, n_src);
  printf("-- ker: (%d x %d)\n", m_ker, n_ker);
#endif /*_DEBUG_ */
  int i,j,m,n;
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
  int ttl_dest=m_dest*n_dest;
#endif /*_SERIAL_CALCULATION*/
#endif /* _OPENMP */
  if(m_src + m_ker - 1 != m_dest 
      || n_src + n_ker - 1 != n_dest)
  {
    printf("-- warning: extra cells exist\n");
  }
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none) \
  private(i,j,m,n)\
  shared(n_dest,m_src,n_src,m_ker,n_ker,src,ker)\
  reduction(+:dest[:ttl_dest])
#endif /*_SERIAL_CALCULATION*/
#endif /* _OPENMP */
  for (i=0; i<m_src; i++)
  {
    for (j=0; j<n_src; j++)
    {
      for (m=0; m<m_ker; m++)
      {
        for (n=0; n<n_ker; n++)
        {
          dest[(i+m)*n_dest + j+n] 
            += src[i*n_src+ j]*ker[m*n_ker + n];
        }
      }
    }
  }
  return 0;
}
int convoluteVecVec (int n_dest, double *dest,
    int n_src,  double *src, int n_ker,  double *ker)
{
#ifdef _DEBUG_
  printf("\nconvoluteVecVec:\n");
  printf("-- dst: (1 * %d)\n", n_dest);
  printf("-- src: (1 * %d)\n", n_src);
  printf("-- ker: (1 * %d)\n", n_ker);
#endif /*_DEBUG_ */
  int j,n;
  if(n_src + n_ker - 1 != n_dest)
  {
    printf("-- warning: extra cells exist\n");
  }
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none) \
  private(j, n)\
  shared(n_dest,n_src,n_ker,src,ker)\
  reduction(+:dest[:n_dest])
#endif /*_SERIAL_CALCULATION*/
#endif /* _OPENMP */
  for (j=0; j<n_src; j++)
  {
    for (n=0; n<n_ker; n++)
    {
      dest[j+n] 
        += src[j]*ker[n];
    }
  }
  return 0;
}
int calcMaxMin 
   (int n, double *mat, double *max, double *min)
{
  printf("calcMaxMin:\n"
      "-- initial values:\n"
      "---- max: %lf\n"
      "---- min: %lf\n",
      *max, *min
      );
  int    i; 
  double tmp_max=*max, tmp_min=*min;
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none)\
      private(i)\
      shared(mat, n)\
      reduction(max:tmp_max)\
      reduction(min:tmp_min)
#endif /* _SERIAL_CALCULATION */
#endif /* _OPENMP */
  for (i=0; i<n; i++)
  {
      if (tmp_max < mat[i]) tmp_max = mat[i];
      if (tmp_min > mat[i]) tmp_min = mat[i];
  }
  *max = tmp_max;
  *min = tmp_min;
  printf("-- results :\n"
      "---- max: %lf\n"
      "---- min: %lf\n",
      *max, *min
      );
  return 0;
}
  /**********************************************************
    Output data
  **********************************************************/
int initFileNames(char* newfpth, 
    char* time, const char* fpth, char* fnm, int flg_silent)
{
  int  tmp_byte;
  char tmp_path[BUFF_SIZE];
  struct stat statbuf;
  if(flg_silent != VALID) printf("initFileNames:\n");
  // create a new directory
  if(time[0] == '\0')
  {
    if(flg_silent != VALID) printf("-- skipped date information.\n");
    tmp_byte = snprintf(tmp_path,sizeof(tmp_path)-1,
        "%.*s", 
        BUFF_SIZE, fpth
        );
  }
  else
  {
    tmp_byte = snprintf(tmp_path,sizeof(tmp_path)-1,
        "%.*s%.*s/", 
        BUFF_SIZE, fpth, 
        BUFF_SIZE, time 
        );
  }
  if (tmp_byte>=BUFF_SIZE || tmp_byte <0)
  {
    printf("-- invalid file path.\n");
  }
  if (stat(tmp_path, &statbuf) != 0)
  {
    mkdir(tmp_path, 
        S_IRUSR | S_IWUSR | S_IXUSR |  /* rwx */
        S_IRGRP | S_IWGRP | S_IXGRP |  /* rwx */
        S_IROTH | S_IXOTH | S_IXOTH    /* rwx */
        );
  }
  // change the file name
  if(time[0] == '\0')
  {
    tmp_byte = snprintf(tmp_path,sizeof(tmp_path)-1,
        "%.*s%.*s", 
        BUFF_SIZE, fpth, 
        BUFF_SIZE, fnm 
        );
  }
  else
  {
    tmp_byte = snprintf(tmp_path,sizeof(tmp_path)-1,
        "%.*s%.*s/%.*s", 
        BUFF_SIZE, fpth, 
        BUFF_SIZE, time,
        BUFF_SIZE, fnm 
        );
  }
  if (tmp_byte>=BUFF_SIZE || tmp_byte <0)
  {
    printf("-- invalid file name.\n");
  }
  strcpy(newfpth, tmp_path);
  if(flg_silent != VALID) 
  {
    printf("-- output directory : %s\n", newfpth);
    printf("-- time stamp       : %s\n", time);
    printf("\n");
  }
  return 0;
}
int writeOutDblPointer(char* filename, int ni, int nj, double** aa)
{
  int i, j;
  FILE *fp;
  fp=fopen(filename, "w");
  for (i=0; i<ni; i++)
  {
    for (j=0; j<nj; j++)
    {
      fprintf(fp, "%30.18e", aa[i][j]);  
      if (j+1 != nj) fprintf(fp, ",");  
    }
    fprintf(fp, "\n");  
  }
  fclose(fp);
  return 0;
}
int faddDblPointer(FILE *fp, int ni, int nj, double** aa)
{
  int i, j;
  for (i=0; i<ni; i++)
  {
    for (j=0; j<nj; j++)
    {
      fprintf(fp, "%30.18e", aa[i][j]);  
      if (j+1 != nj) fprintf(fp, ",");  
    }
    fprintf(fp, "\n");  
  }
  return 0;
}
int writeOutSglPointer(char* filename, int ni, double* a)
{
  int i;
  FILE *fp;
  fp=fopen(filename, "w");
  for (i=0; i<ni; i++)
  {
    fprintf(fp, "%30.18e", a[i]);  
    if (i+1 != ni) fprintf(fp, ",");  
  }
  fprintf(fp, "\n");  
  fclose(fp);
  return 0;
}
int faddSglPointer(FILE *fp, int ni, double* a)
{
  int i;
  for (i=0; i<ni; i++)
  {
    fprintf(fp, "%30.18e", a[i]);  
    if (i+1 != ni) fprintf(fp, ",");  
  }
  fprintf(fp, "\n");  
  return 0;
}
int transSeparatedFile(char* fpth)
{
//#ifdef _DEBUG_
  printf("\ntransSeparatedFile:\n-- ");
//#endif /* _DEBUG_ */
  int i,j,row, col, flg_work = VALID;
  double **tmp_arr, **trans_arr;
  if ((tmp_arr 
        = readSeparatedValueFiles(&row, &col, fpth, VALID)) 
      == NULL) 
  {
//#ifdef _DEBUG_
    printf("-- null pointer at tmp_arr: %p\n", tmp_arr); 
//#endif /* _DEBUG_ */
    flg_work = INVALID;
  }
  else
  {
    trans_arr = 
      (double**) allocateMatrix(sizeof(double), col, row);
#ifdef _OPENMP
#ifndef _SERIAL_CALCULATION 
#pragma omp parallel for default(none) \
    private(i,j)                   \
    shared(trans_arr, tmp_arr,row,col)
#endif /* _SERIAL_CALCULATION */
#endif /* _OPENMP */
    for (i=0;i<row;i++)
    {
      for (j=0;j<col;j++)
      {
        trans_arr[j][i]=tmp_arr[i][j];
      }
    }
//#ifdef _DEBUG_
    printf("-- writing the transposed data.\n");
//#endif /* _DEBUG_ */
    writeOutDblPointer(fpth, col, row, trans_arr);
  }
  return flg_work;
}
  /**************************************************************
     Calculate the computation time  
  **************************************************************/
double GetElapsedTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1.0e-6;
}
double GetCPUTime()
{
  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);
  return ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec*1.0e-6;
}
int getCurrentTime(char *str)
{
  time_t    timer;
  struct tm *date;
  // get the elapsed time and convert it to the local time
  timer = time(NULL);    
  date  = localtime(&timer);
  strftime(str, 255, "%Y%m%d_%H%M%S", date);
  return 0;
}
int getSerialParallelCompTime(double *t_st)
{
  t_st[SER]   = GetCPUTime();
#ifdef _OPENMP
  t_st[PAR]   = omp_get_wtime();
#endif /* _OPENMP */
  return 0;
}
