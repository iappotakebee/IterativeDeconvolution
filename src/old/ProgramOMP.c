kjkk/*
  This program was written in C language 
  by Takenori Shimamura on January 4, 2019.
  Take modified trivial details on January 7, 2019.
  Take modified the display of dwell time in all.txt
  on March 31, 2019.
  Take added offset_hgt to adjust the imported target
  on October 12, 2019.
  Take modified the input of files 
  to avoid compiling this program each time 
  on November 23, 2019.

  The purpose of this program is to offer dwell time 
  in numerically controled (NC) fabrication,
  when the unit yield and the targeted shape are given.

  The sputtering machine will work on differential deposition 
  for elliptically curved mirrors, using this result.

  Only one-dimensional data can be deconvoluted.
  Parallel computation using OpenMP is available 
  (MacOSX, GCC).


// Usage
  Both files should be placed in the directory 
  of "./file/input/".
     The target shape: target.txt
     The unit sputter yield: unit.txt
  The imported files should have "\n" 
  at the end of the files.

  Follow the descritption of each parameter.

  The unit of height or descretization 
  is the same as the input file.

  You can change the iterative numbers in macro parameters.


// Descritption of each parameter 
  alpha: 
    Adjust alpha in the update section 
    when the error diverges.
  ls_alpha:
    Adjust ls_alpha when the ratio of decrease in alpha
    should be optimized. ls_alpha lessen alpha
    when the error in RMS exceed the previous value.
  lim_alpha:
    lim_alpha determines the lower limit of alpha.
    Even when alpha is repeatedly multiplied by ls_alpha,
    alpha cannot be less than lim_alpha.
  threshold:
    Change the threshold to escape from the loop calculation 
    when the shape errors are reduced to this value in RMS.
  offset_time:
    offset_time determines the lower limit of dwell time.
    This should be more than zero, and can consider
    the minimum duration in transfer of the stage.
  offset_hgt:
    offset_hgt adjust the height of the imported files.


// Some useful arguments
  addtime:
    When addtime is VALID, the year, day, and time 
    are added to the top of the output filenames.

    
// Descritption of some macro parameters 
  N_LOOPMAX:
    The maximum number of iterrative computations.
  N_LOOPREC:
    The interval between recording the results.
    The output files illustrate how data converge
    every N_LOOPREC times.
  N_LOOPDISP:
    The interval between displaying the results.
    The results include the iterative times,
    alpha, and error in rms.
 */

#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>
#include<omp.h>
#define DEBUG_PRINT_INT(a) (printf("%s: %d\n", #a, a))
#define DEBUG_PRINT_DBL(a) (printf("%s: %30.24lf\n", #a, a))
#define SPRINTF_FILENAME(filename, state,  a) \
  (sprintf(filename,"./output%s_%s.dat", state, #a))
#define VALID   0
#define INVALID 1
#define BUFF_SIZE  1024
#define N_LOOPMAX  5.0E5
#define N_LOOPREC  1E5
#define N_LOOPDISP 2E3
#define N_MARGIN   1
#define EPSILON    0.001
#define N_ARRAYS  5
#define N_TARGET  0
#define N_ERROR   1
#define N_DWELL   2
#define N_REALFIG 3
#define N_UNIT    4
#define N_INFO 4
#define N_rms  0
#define N_ntgt 1
#define N_nuni 2
#define N_nall 3
#define MS_TO_MIN 60000.0
// Initialize the number of data and all the arrays
int    InitDisplay(int n_tgt, int n_uni, int hn_uni, int n_all,
          double init_st, double init_en, int nthreads);
int    InitFileNames(const char *fpth, char *time, char *fnm, 
          char *newfpth);
int    ReadFilePaths(char *fpth_tgt, char *fpth_uni);
int    InitInputNum(int *n_tgt, int *n_uni, int *n_all, 
          char *fpth_tgt, char *fpth_uni);
int    ReadInputDat(double *target, int *n_tgt, 
          double *unit, int *n_uni, double *dwelltime, 
          const double offset_time, const double offset_hgt,
          int *n_all, int *hn_uni, 
          char *fpth_tgt, char *fpth_uni);
int    InitMatrixToDblZero(int ni, int nj, double **aa);
int    InitVectorToDblZero(int ni, double *a);
// Record or output arrays or data
int    RecordAllArrDat(int n_tgt, int n_uni, int n_all,
          double **aa, double *tgt, double *err, 
          double *dwell, double *real, double *unit);
int    RecordInfo(double rms, int n_tgt, int n_uni, int n_all, 
          double *a);
int    RecordColumns(char **aa);
int    MemorizeData(int ni, int cnt, double* a, double** hist);
int    WriteAllHistory(int ni, int nj, int cnt, 
           char* filename, char** info, double** hist);
int    Output(int ni, int nj, 
          char *filename, double **aa);
int    WriteAllAndHeader(int ni, int nj, char *filepath, 
          double **aa, char **bb, double *c);
int    WritePartAndHeader(int n_all, int n_tgt, int n_uni, 
          int nj, char *filepath, 
          double **aa, char **bb, double *c);
int    DisplaySumDwellTime(int n_all, double *dwelltime);
// Allocate & deallocate matrix
void   **AllocateMatrix(int size, int m, int n);
void   DeallocateMatrix(double **aa           );
void   DeallocateMatStr(char **aa             );
void   *AllocateVector (int size, int m       );
void   DeallocateVector(double *a             );
// Measure the computation time
double GetElapsedTime();
double GetCPUTime();
int    GetCurrentTime(char *str);

int main (int argc, char *argv[])
{
  /**************************************************************
    Parameters to adjust deconvolution performance
  **************************************************************/
  double       alpha     = 10.5;
  const double ls_alpha  = 0.95;
  //const double mr_alpha  = 1.05;
  const double lim_alpha = 0.5E-6;
  double       threshold = 0.1;
  const double offset_time = 1500.0; // in ms
  const double offset_hgt = 200.0; 
  /**************************************************************
    Some useful arguments (change them where necessary)
  **************************************************************/
  const int  addtime = VALID;
  const int  readfileonconsole = VALID;
  char infilepth_tgt[BUFF_SIZE]="./files/input/20201102/20201102VFMFirstTrialfromDS_filtered.txt";
  char infilepth_uni[BUFF_SIZE]="./files/input/20201102/20201102SputterYield_filtered.txt"; 
  //const char infilepth_tgt[BUFF_SIZE]="./files/input/target_test.txt";
  //const char infilepth_uni[BUFF_SIZE]="./files/input/unit_test.txt"; 
  const char outfilepath  [BUFF_SIZE]="./files/outputOMP/"; 
  char       outfilenm_all[BUFF_SIZE]="all.txt";
  char       outfilenm_pt [BUFF_SIZE]="extraction.txt";
  char       outfilenm_hst[BUFF_SIZE]="history.txt";
  char       outfilenm_err[BUFF_SIZE]="errors.txt";
  char       outfilenm_tm [BUFF_SIZE]="dwelltimes.txt";
  char       time         [BUFF_SIZE]="";
  char       ctmp         [BUFF_SIZE-10]="";
  int    n_tgt=0, n_uni=0, n_all, hn_uni; 
  //int    n_buff0, n_buffN;
  int    cnt, cnt_rec, i, j, tmp;
  int    s_cnt, e_cnt;
  int    nthreads;
  double rms_bef = 10E7, rms_aft = 10E7;
  double figerr_max = 0.0, figerr_min = 0.0;
  double st, en, st_omp, en_omp,  init_st, init_en;
  /**************************************************************
    Arrays to deconvolute the input data
   **************************************************************/
  double *target,*real_fig,*error,*dwelltime,*update,*unit,*info;
  double **memory,**fig_hst,**err_hst,**tm_hst;
  char   **columns,**hst_info;
  /**************************************************************
    Initialize the filename, the arrays and the parameters
  **************************************************************/
  st = GetCPUTime();
  st_omp = omp_get_wtime();
  init_st = GetCPUTime();
  // initialize filenames
  if (addtime == VALID)
  {
    GetCurrentTime(ctmp);
    sprintf(time, "%s_", ctmp);
  }
  if (readfileonconsole == VALID)
  {
    if (ReadFilePaths(infilepth_tgt, infilepth_uni) != VALID)
    {
      printf("Invalid file paths.\n");
    }
  }
  InitFileNames (outfilepath,time,outfilenm_all,outfilenm_all);
  InitFileNames (outfilepath,time,outfilenm_pt ,outfilenm_pt );
  InitFileNames (outfilepath,time,outfilenm_hst,outfilenm_hst);
  InitFileNames (outfilepath,time,outfilenm_err,outfilenm_err);
  InitFileNames (outfilepath,time,outfilenm_tm ,outfilenm_tm );
  InitInputNum  (&n_tgt, &n_uni, &n_all, 
    infilepth_tgt, infilepth_uni);
  // allocate and initialize the arrays
  target    = (double*) AllocateVector(sizeof(double), n_tgt );
  real_fig  = (double*) AllocateVector(sizeof(double), n_all );
  error     = (double*) AllocateVector(sizeof(double), n_tgt );
  dwelltime = (double*) AllocateVector(sizeof(double), n_all );
  update    = (double*) AllocateVector(sizeof(double), n_all );
  unit      = (double*) AllocateVector(sizeof(double), n_uni );
  info      = (double*) AllocateVector(sizeof(double), N_INFO);
  memory    = (double**)AllocateMatrix(sizeof(double),
                n_all, N_ARRAYS);
  fig_hst   = (double**)AllocateMatrix(sizeof(double), 
                n_all, 2 +  N_LOOPMAX / N_LOOPREC );
  err_hst   = (double**)AllocateMatrix(sizeof(double), 
                n_tgt, 2 +  N_LOOPMAX / N_LOOPREC );
  tm_hst    = (double**)AllocateMatrix(sizeof(double), 
                n_all, 2 +  N_LOOPMAX / N_LOOPREC );
  columns   = (char**)  AllocateMatrix(sizeof(char), 
                N_ARRAYS+N_INFO, BUFF_SIZE);
  hst_info  = (char**)  AllocateMatrix(sizeof(char), 
                2 + N_LOOPMAX / N_LOOPREC, BUFF_SIZE);
  InitVectorToDblZero(n_tgt,  target   );
  InitVectorToDblZero(n_all,  real_fig );
  InitVectorToDblZero(n_tgt,  error    );
  InitVectorToDblZero(n_all,  dwelltime);
  InitVectorToDblZero(n_all,  update   );
  InitVectorToDblZero(n_uni,  unit     );
  InitVectorToDblZero(N_INFO, info     );
  InitMatrixToDblZero(n_all, N_ARRAYS, memory);
  InitMatrixToDblZero(n_all, 2 +  N_LOOPMAX / N_LOOPREC, fig_hst);
  InitMatrixToDblZero(n_tgt, 2 +  N_LOOPMAX / N_LOOPREC, err_hst);
  InitMatrixToDblZero(n_all, 2 +  N_LOOPMAX / N_LOOPREC,  tm_hst);
  ReadInputDat(target, &n_tgt, unit, &n_uni, dwelltime, 
      offset_time, offset_hgt, &n_all, &hn_uni, 
      infilepth_tgt, infilepth_uni);
  init_en = GetCPUTime();
  nthreads = omp_get_max_threads();
  // display the initial conditions
  InitDisplay(n_tgt,n_uni,hn_uni,n_all,init_st,init_en,nthreads);
  /**************************************************************
    Calculate the dwell time in the loop
  **************************************************************/
  cnt = 0; cnt_rec = 0;
  s_cnt = hn_uni; e_cnt = n_all-hn_uni;
  while (rms_bef > threshold)
  {
    cnt++;
    // memorize the history of arrays
    if ( (cnt % (int)N_LOOPREC == 0) || cnt == 1)
    {
      MemorizeData(n_all, cnt_rec, real_fig , fig_hst);
      MemorizeData(n_tgt, cnt_rec,    error , err_hst);
      MemorizeData(n_all, cnt_rec, dwelltime,  tm_hst);
      sprintf(hst_info[cnt_rec], "Iteration-%d", cnt);
      cnt_rec++;
      printf("Have memorized arrays %d times\n", cnt_rec);
    } 
    // check how many times the loop has been computed
    if (cnt > N_LOOPMAX)
    {
      printf("Reached the maximum number of loops\n");
      break;
    }
    // display the current parameters
    if (cnt % (int)N_LOOPDISP == 0)
    {
      printf("loop: %9d, alpha: %9.4lf, rms: %9.4lf\n",
          cnt, alpha, rms_aft);
    } 
    // intialize the arrays
#pragma omp parallel for default(none) \
    private(i)                         \
    shared(n_all,real_fig,update)
    for (i=0; i<n_all; i++)
    {
      real_fig[i] = 0.0;
      update[i] = 0.0;
    }
    // convolute the unit sputter yield with the dwell time
#pragma omp parallel for default(none) \
    private(i,j,tmp)\
    shared(s_cnt, e_cnt, n_uni,hn_uni,dwelltime,unit)\
    reduction(+:real_fig[:n_all])
    for (i=s_cnt; i<e_cnt; i++)
    {
      for (j=0; j<n_uni; j++)
      {
        tmp            = i - hn_uni + j;
        real_fig[tmp] += (dwelltime[i]*unit[j]);
      }
    }
    // calculate the errors between the target and the figure
    rms_bef = rms_aft;  
    rms_aft = 0.0;
#pragma omp parallel for default(none)       \
    private(i,tmp)                           \
    shared(n_tgt,n_uni,error,target,real_fig)\
    reduction(+:rms_aft)
    for (i=0; i<n_tgt; i++)
    {
      tmp      = i + n_uni;
      error[i] = 0.0;
      error[i] = target[i] - real_fig[tmp];
      rms_aft += (error[i]*error[i]);
    }
    rms_aft = sqrt( rms_aft/n_tgt );
    // lessen alpha if the current errors worsen
    if (rms_bef < rms_aft)
    {
      if (alpha*ls_alpha > lim_alpha)
      {
        alpha *= ls_alpha;
        printf("Multiplied alpha by %9.4lf\n", ls_alpha);
        printf("loop: %9d, alpha: %9.4lf, rms: %9.4lf\n",
            cnt, alpha, rms_aft);
      }
    }
    else
    {
      // calculate the evaluation function (err x unit)
#pragma omp parallel for default(none) \
      private(i,j,tmp)\
      shared(n_tgt, n_uni, hn_uni,error,unit)\
      reduction(+:update[:n_all])
      for (i=0; i<n_tgt; i++)
      {
        for (j=0; j<n_uni; j++)
        {
          tmp = i + n_uni - hn_uni + j;
          update[tmp] += (error[i]*unit[j]); // why?
        }
      }
      // refresh the dwelltime using (t=t-alpha × (p-f))
#pragma omp parallel for default(none)       \
      private(i)                           \
      shared(s_cnt,e_cnt,alpha,update,dwelltime, offset_time)
      for (i=s_cnt; i<e_cnt; i++)
      {
        dwelltime[i] += (alpha*update[i]);
        // limit the minimum dwell time
        if (dwelltime[i] < offset_time)
        {
          dwelltime[i] = offset_time;
        }
      }
    }
  }
  for (i=0; i<n_tgt; i++)
  {
    if (figerr_max < error[i]) figerr_max = error[i];
    if (figerr_min > error[i]) figerr_min = error[i];
  }
/**************************************************************
  Record all data and write them down in files
**************************************************************/
  printf("\nDone.\n");
  printf("loop: %9d, alpha: %9.4lf, \nrms: %9.4lf, PV: %9.4lf\n",
      cnt, alpha, rms_aft, figerr_max-figerr_min);
  DisplaySumDwellTime(n_all, dwelltime);
  MemorizeData(n_all, cnt_rec, real_fig , fig_hst);
  MemorizeData(n_tgt, cnt_rec,    error , err_hst);
  MemorizeData(n_all, cnt_rec, dwelltime,  tm_hst);
  sprintf(hst_info[cnt_rec], "Iteration-%d", cnt);
  cnt_rec++;
  RecordInfo(rms_aft, n_tgt, n_uni, n_all, info);
  RecordColumns(columns);
  RecordAllArrDat(n_tgt, n_uni, n_all, 
      memory, target, error, dwelltime, real_fig, unit);
  WriteAllAndHeader(n_all, N_ARRAYS, outfilenm_all, 
      memory, columns, info);
  WritePartAndHeader(n_all, n_tgt, n_uni, N_ARRAYS, outfilenm_pt, 
      memory, columns, info);
  WriteAllHistory(n_all,N_LOOPMAX/N_LOOPREC+1, cnt_rec, 
    outfilenm_hst, hst_info, fig_hst);
  WriteAllHistory(n_tgt,N_LOOPMAX/N_LOOPREC+1, cnt_rec, 
    outfilenm_err, hst_info, err_hst);
  WriteAllHistory(n_all,N_LOOPMAX/N_LOOPREC+1, cnt_rec, 
    outfilenm_tm , hst_info,  tm_hst);
/**************************************************************
  Deallocate all the arrays
**************************************************************/
  DeallocateVector( target    );
  DeallocateVector( real_fig  );
  DeallocateVector( error     );
  DeallocateVector( dwelltime );
  DeallocateVector( update    );
  DeallocateVector( unit      );
  DeallocateVector( info      );
  DeallocateMatrix( memory    );
  DeallocateMatrix( fig_hst   );
  DeallocateMatrix( err_hst   );
  DeallocateMatrix( tm_hst    );
  DeallocateMatStr( columns   );
  DeallocateMatStr( hst_info  );
  en = GetCPUTime();
  en_omp = omp_get_wtime();
  printf("Serial Computation time: %9.4lf sec.\n", en-st);
  printf("OpenMP Computation time: %9.4lf sec.\n", 
      en_omp-st_omp);
}

/**************************************************************
   Initialize the number of data and all the arrays
**************************************************************/
int InitDisplay(int n_tgt, int n_uni, int hn_uni, int n_all,
    double init_st, double init_en, int nthreads)
{
  int i, myid;
  printf("\n                                       \n");
  printf("    Iterative Deconvolution Started   \n");
  printf("***************************************\n\n");
  // display the number of entries in the input data
  printf("The number of entries\n");
  printf("  # Target   : %7d\n",  n_tgt);
  printf("  # Unit     : %7d\n",  n_uni);
  printf("  # 1/2 Unit : %7d\n", hn_uni);
  printf("  # All      : %7d\n",  n_all);
  // display the number of PEs
  printf("The number of threads\n");
  printf("  # Total    : %7d\n",  nthreads);
#pragma omp parallel for default(none) \
    private(i,myid)                         \
    shared(nthreads)
  for (i=0; i<nthreads;i++)
  {
    myid = omp_get_thread_num();
    printf("  # Thread   : %7d\n",  myid);
  }
  printf("Computation time for initialization: %9.4lf\n", 
      init_en-init_st);
  printf("\n");
  return 0;
}
int InitFileNames(const char* fpth, char* time, char* fnm, 
    char* newfpth)
{
  char tmp[BUFF_SIZE];
  sprintf(tmp, "%s%s%s", fpth, time, fnm);
  strcpy(newfpth, tmp);
  return 0;
}
int ReadFilePaths(char *fpth_tgt, char *fpth_uni)
{
  printf("Input file path of targeted shape.\n");
  scanf("%s", fpth_tgt);
  printf("Input file path of unit shape.\n");
  scanf("%s", fpth_uni);
  return 0;
}
int InitInputNum(int* n_tgt, int* n_uni, int* n_all, 
    char* fpth_tgt, char* fpth_uni)
{
  FILE   *fp;
  char   buf[BUFF_SIZE];
  size_t i, read_size;
  if ( (fp = fopen(fpth_tgt, "r")) == NULL )
  {
    printf("Couldn't find a file for the target shape.\n");
    return -1;
  }
  while ( (read_size = fread(buf, 1, BUFF_SIZE, fp)) > 0)
  {
    for (i=0; i<read_size; i++)
    {
      if (buf[i] == '\n') (*n_tgt)++;    
    }
  }
  printf("Read the target shape in\n %s\n", fpth_tgt);
  if ( (fp = fopen(fpth_uni, "r")) == NULL )
  {
    printf("Couldn't find a file for the unit sputter yield.\n");
    return -1;
  }
  while ( (read_size = fread(buf, 1, BUFF_SIZE, fp)) > 0)
  {
    for (i=0; i<read_size; i++)
    {
      if (buf[i] == '\n') (*n_uni)++;    
    }
  }
  printf("Read the unit sputter yield in\n %s\n", fpth_uni);
  (*n_all) = (*n_tgt) + 2*N_MARGIN*(*n_uni);
  fclose(fp);
  return 0;
}
// read the input files and intialize the arrays
int ReadInputDat(double* target, int* n_tgt, 
    double* unit, int* n_uni, double* dwelltime, 
    const double offset_time, const double offset_hgt,
    int* n_all, int* hn_uni, 
    char* fpth_tgt, char* fpth_uni)
{
  int  i,cnt;
  FILE *fp;
  char buf[BUFF_SIZE];
  // read error files
  if ( (fp = fopen(fpth_tgt, "r")) == NULL )
  {
    printf("Couldn't find a file for the target shape.\n");
    exit(1);
  }
  cnt = 0;
  while ( fgets(buf, BUFF_SIZE, fp) != NULL)
  {
    sscanf(buf, "%lf", &target[cnt]);
    target[cnt]+=offset_hgt;
    cnt++;
  }
  if ( (*n_tgt) != cnt) 
  {
    printf("Inappropriate BUFF_SIZE for %s.\n", fpth_tgt); 
    return -1;
  }
  // read unit files
  if ( (fp = fopen(fpth_uni, "r")) == NULL )
  {
    printf("Couldn't find a file for the unit sputter yield.\n");
    exit(1);
  }
  cnt = 0;
  while ( fgets(buf, BUFF_SIZE, fp) != NULL)
  {
    sscanf(buf, "%lf", &unit[cnt]);
    cnt++;
  }
  if ( (*n_uni) != cnt) 
  {
    printf("Inappropriate BUFF_SIZE for %s.\n", fpth_uni); 
    return -1;
  }
  // Remove the last element to deconvolute data 
  //   if the number of elements in unit sputter yield is even
  (*n_uni) -= (1 - (*n_uni)%2);
  (*hn_uni) = ((*n_uni)-1) / 2;
  (*n_all)  = (*n_tgt) + 2*N_MARGIN*(*n_uni);
  // intialize the arrays for recording dwell time
#pragma omp parallel for default(none) \
  private(i)                           \
  shared(n_all, hn_uni, dwelltime, offset_time)     
  for (i=(*hn_uni); i<(*n_all)-(*hn_uni); i++)
  {
    dwelltime[i] = offset_time;
  }
  fclose(fp);
  return 0;
}
int InitMatrixToDblZero(int ni, int nj, double **aa)
{
  int i, j;
#pragma omp parallel for default(none) \
  private(i,j)                         \
  shared(ni, nj, aa)                   
  for (i=0; i<ni; i++)
  {
    for (j=0; j<nj; j++)
    {
      aa[i][j] = 0.0;
    }
  }
  return 0;
}
int InitVectorToDblZero(int ni, double *a)
{
  int i;
#pragma omp parallel for default(none) \
  private(i)                           \
  shared(ni,a)                         
  for (i=0; i<ni; i++)
  {
      a[i] = 0;
  }
  return 0;
}

  /**************************************************************
     Record or output arrays or data                     
  **************************************************************/
int RecordAllArrDat(int n_tgt, int n_uni, int n_all, double** aa, 
  double* tgt, double* err, double* dwell, 
  double* real, double* unit)
{
  int i, tmp;
#pragma omp parallel for default(none) \
  private(i,tmp)                       \
  shared(n_tgt, n_uni,tgt,err,aa)                         
  for (i=0; i<n_tgt; i++)
  {
    tmp = i + n_uni;
    aa[tmp][N_TARGET] = tgt[i];
    aa[tmp][N_ERROR]  = err[i];
  }
#pragma omp parallel for default(none) \
  private(i)                           \
  shared(n_all,real,dwell,aa)                         
  for (i=0; i<n_all; i++)
  {
    aa[i][N_REALFIG] = real[i];
    aa[i][N_DWELL]   = dwell[i];
  }
#pragma omp parallel for default(none) \
  private(i)                           \
  shared(n_uni,unit,aa)                         
  for (i=0; i<n_uni; i++)
  {
    aa[i][N_UNIT] = unit[i];
  }
  return 0;
}
int RecordInfo(double rms, int n_tgt, int n_uni, int n_all, 
    double* a)
{
  a[N_rms ] = rms  ;
  a[N_ntgt] = n_tgt;
  a[N_nuni] = n_uni;
  a[N_nall] = n_all;
  return 0;
}
int RecordColumns(char** aa)
{
  strcpy(aa[N_TARGET       ], "Target"         );
  strcpy(aa[N_ERROR        ], "Error"          );
  strcpy(aa[N_DWELL        ], "Dwell Time"     );
  strcpy(aa[N_REALFIG      ], "Expected Figure");
  strcpy(aa[N_UNIT         ], "Sputter Yield"  );
  strcpy(aa[N_UNIT+1+N_rms ], "Error in RMS"   );
  strcpy(aa[N_UNIT+1+N_ntgt], "Target Elements");
  strcpy(aa[N_UNIT+1+N_nuni], "Unit Elements"  );
  strcpy(aa[N_UNIT+1+N_nall], "Total Elements" );
  return 0;
}
int MemorizeData(int ni, int cnt, double* a, double** hist)
{
  int i;
#pragma omp parallel for default(none) \
  private(i)                           \
  shared(cnt,ni,hist, a)               
  for (i=0; i<ni; i++)
  {
      hist[i][cnt] = a[i];
  }
  return 0;
}
int WriteAllHistory(int ni, int nj, int cnt, 
    char* filename, char** info, double** hist)
{
  int i, j;
  FILE *fp;
  fp=fopen(filename, "w");
  fprintf(fp, "%10s ", "Count");
  for (j=0; j<cnt; j++)
  {
    fprintf(fp, "%25s ", info[j]);  
  }
  fprintf(fp, "\n");  
  for (i=0; i<ni; i++)
  {
    fprintf(fp, "%10d ", i);
    for (j=0; j<cnt; j++)
    {
      fprintf(fp, "%25.18e ", hist[i][j]);  
    }
    fprintf(fp, "\n");  
  }
  fclose(fp);
  return 0;
}
int Output(int ni, int nj, char* filename, double** aa)
{
  int i, j;
  FILE *fp;
  fp=fopen(filename, "w");
  for (i=0; i<ni; i++)
  {
    for (j=0; j<nj; j++)
    {
      fprintf(fp, "%25.18e ", aa[i][j]);  
    }
    fprintf(fp, "\n");  
  }
  fclose(fp);
  return 0;
}
int WriteAllAndHeader(int ni, int nj, char* filepath, 
    double** aa, char** bb, double* c)
{
  int i, j;
  FILE *fp;
  fp=fopen(filepath, "w");
  printf("filepath:\n %s\n", filepath);
  fprintf(fp, "%25s ", "Count");  
  for (i=0; i<(nj+N_INFO); i++)
  {
    fprintf(fp, "%25s ", bb[i]);  
  }
  fprintf(fp, "\n");  
  fprintf(fp, "%25d ", 0);  
  for (j=0; j<nj; j++)
  {
    fprintf(fp, "%25.18e ", aa[0][j]);  
  }
  for (j=0; j<N_INFO; j++)
  {
    fprintf(fp, "%25.18e ", c[j]);  
  }
  fprintf(fp, "\n");  
  for (i=1; i<ni; i++)
  {
    fprintf(fp, "%25d ", i);  
    for (j=0; j<nj; j++)
    {
      fprintf(fp, "%25.18e ", aa[i][j]);  
    }
    fprintf(fp, "\n");  
  }
  fclose(fp);
  return 0;
}
int WritePartAndHeader(int n_all, int n_tgt, int n_uni, int nj,
    char* filepath, double** aa, char** bb, double* c)
{
  int i, j;
  FILE *fp;
  fp=fopen(filepath, "w");
  printf("filepath:\n %s\n", filepath);
  // write down the columns
  fprintf(fp, "%25s ", "Count");  
  for (i=0; i<(nj+N_INFO); i++)
  {
    fprintf(fp, "%25s ", bb[i]);  
  }
  fprintf(fp, "\n");  
  fprintf(fp, "%25d ", n_uni);  
  // write down the first row
  fprintf(fp, "%25.18e ", aa[n_uni][N_TARGET]);  
  fprintf(fp, "%25.18e ", aa[n_uni][N_ERROR]);  
  fprintf(fp, "%25.18e ", aa[n_uni][N_DWELL]);  
  fprintf(fp, "%25.18e ", aa[n_uni][N_REALFIG]);  
  fprintf(fp, "%25.18e ", aa[0    ][N_UNIT]);  
  for (j=0; j<N_INFO; j++)
  {
    fprintf(fp, "%25.18e ", c[j]);  
  }
  fprintf(fp, "\n");  
  // write down the rest of the rows
  for (i=n_uni*N_MARGIN+1; i<(n_all-n_uni*N_MARGIN); i++)
  {
    fprintf(fp, "%25d ", i);  
    fprintf(fp, "%25.18e ", aa[i][N_TARGET]);  
    fprintf(fp, "%25.18e ", aa[i][N_ERROR]);  
    fprintf(fp, "%25.18e ", aa[i][N_DWELL]);  
    fprintf(fp, "%25.18e ", aa[i][N_REALFIG]);  
    fprintf(fp, "%25.18e ", aa[i-n_uni][N_UNIT]);  
    fprintf(fp, "\n");  
  }
  fclose(fp);
  return 0;
}
int DisplaySumDwellTime(int n_all, double *dwelltime)
{
  int i;
  double sum=0.0;
#pragma omp parallel for default(none)\
  private(i)                          \
  shared(n_all, dwelltime)            \
  reduction(+:sum)
  for (i=0; i<n_all; i++)
  {
    sum += dwelltime[i];
  }
  printf("Total fabrication time: %9.4lf minutes\n"
      , sum/MS_TO_MIN);
  return 0;
}
  /**************************************************************
     Allocate & deallocate matrix 
  **************************************************************/
void** AllocateMatrix(int size, int m, int n)
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
void DeallocateMatrix(double **aa)
{
  free( aa[0] );
  free( aa    );
}
void DeallocateMatStr(char **aa)
{
  free( aa[0] );
  free( aa    );
}
void* AllocateVector(int size, int m)
{ 
  void *a;
  if (( a = (void*)malloc( m * size )) == NULL ){
    printf("Errors in memory allocation of a. \n");
    exit(1);
  }  
  return a;
}
void DeallocateVector(double *a)
{
  free(a);
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
int GetCurrentTime(char *str)
{
  time_t    timer;
  struct tm *date;
  // get the elapsed time and convert it to the local time
  timer = time(NULL);    
  date  = localtime(&timer);
  strftime(str, 255, "%Y%m%d%H%M%S", date);
  return 0;
}
