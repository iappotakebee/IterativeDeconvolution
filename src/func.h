#ifndef _FUNC_H
#define _FUNC_H
//#define _SERIAL_CALCULATION

#include "input.h"
/*
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
  Take added the guess value for the dwell time.

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
  flg_addtime:
    When flg_addtime is VALID, the year, day, and time 
    are added to the top of the output filenames.

    
// Descritption of some macro parameters 
  n_loopmax:
    The maximum number of iterrative computations.
  n_looprec:
    The interval between recording the results.
    The output files illustrate how data converge
    every n_looprec times.
  n_loopdisp:
    The interval between displaying the results.
    The results include the iterative times,
    alpha, and error in rms.
 */

#ifndef BUFF_SIZE
#define BUFF_SIZE 1024
#endif /* BUFF_SIZE */
#ifndef VALID
#define VALID 0
#endif /* VALID */
#ifndef INVALID
#define INVALID 1
#endif /* INVALID */

#define EPSILON   0.001
#define N_INFO 4
#define N_rms  0
#define N_ntgt 1
#define N_nuni 2
#define N_nall 3
#define MS_TO_MIN 60000.0
/* file import/export */
#define IDX_NUM      17
#define N_ALL        6
#define IDX_TGTORG   0 
#define IDX_UNITORG  1 
#define IDX_DWELLORG 2   
#define IDX_TGT      3
#define IDX_UNIT     4
#define IDX_DWELL    5  
#define IDX_REAL     6
#define IDX_ERR      7
#define IDX_UPDATE   8
#define IDX_ALL      9
#define IDX_PART     10
#define IDX_DATPATH  11
#define IDX_TMSTMP   12
#define IDX_LOG      13
#define IDX_LOGPATH  14
/* martix information */
#define MATINFO_NUM 3
#define ROW 0
#define COL 1
#define TTL 2
/* options/flags */
#define FLGNUM               6
#define ADD_TIME             0
#define READFILE_ON_CONSOLE  1
#define READPARAM_ON_CONSOLE 2
#define SILENT               3
#define READ_AS_FILESHAPE    4
#define EXTENDUPDATE         5
#define COS2                 2
#define EXP                  3
#define MIX                  4
/* control flags */
#define CONTFLGNUM           2
#define CONTINUELOOP         0
#define RMSDECREASE          1
/* parameters */
#define PARAMNUM     16
#define ALPHA        0
#define THRESHOLD    1
#define OFFSET_DWELL 2
#define OFFSET_TGT   3
#define N_LOOPMAX    4
#define N_LOOPREC    5
#define N_LOOPDISP   6
#define MGNRATIO_ROW 7
#define MGNRATIO_COL 8
#define RMS_BEF      9
#define RMS_AFT      10
#define FIGERR_MAX   11
#define FIGERR_MIN   12
#define LS_ALPHA     13
#define LIM_ALPHA    14
#define COUNT        15

/* time stamps */
#define TYPES    2
#define SER      0
#define PAR      1
#define N_TIMEST 6
#define ST_TOTAL 0
#define EN_TOTAL 1
#define ST_INIT  2
#define EN_INIT  3
#define ST_EXP   4
#define EN_EXP   5

typedef struct decnv_arr{
  double **mat;
  int  row;
  int  col;
  int  ttl;
  int  i_st; /* relative to real shpae */
  int  j_st;
  char name     [BUFF_SIZE];
  char imppth   [BUFF_SIZE];
  char exppth   [BUFF_SIZE];
  char tmpexppth[BUFF_SIZE];
  char histpth  [BUFF_SIZE];
} decnv_arr;
typedef double (*df_3dblarg)(double, double, double *);
typedef int    (*if_3dblarg)(double *, double *, double *);

extern int initDecnvArrays(decnv_arr *data, int *flgs);
extern int terminateDecnvArrays(decnv_arr *data, int *flgs);
extern int printDecnvArray(decnv_arr *data, int index);
extern int writeDecnvArray(decnv_arr *data, int index);
extern int writeTmpDecnvArray
          (decnv_arr *data, int index, int tmpcnt);
extern int writeTmpDecnvArrays
          (decnv_arr *data, int tmpcnt);
extern int initImpFilePaths(decnv_arr *data, int *flgs);
extern int scanFilePaths(decnv_arr *data);
extern int initExpFilePaths(decnv_arr *data, int *flgs);
extern int importFiles(decnv_arr *data, int *flgs);
extern int initParameters(double *parameters);
extern int initCalculationRange
          (decnv_arr *data, double *parameters, int *flgs);
extern double  convertDecnvArray
          (decnv_arr *datum,int glbl_i, int glbl_j);
extern int applyWindowFunction
          (int m_dst, int n_dst, double **dest, 
           int m_src, int n_src, double **src, char *mode);
extern double  squareCos
          (double k, double x, double *param);
extern double  minusExp
          (double inv_t_const, double x, double *param);
extern double  squareCosMinusExp
          (double arg1, double x, double *param);
extern int exportFiles(decnv_arr *data, int *flgs);
extern int initArgsSquareCos
          (double *arg1_i, double *arg1_j, double *param);
extern int initArgsMinusExp 
          (double *arg1_i, double *arg1_j, double *param);
extern int initArgsMix 
          (double *arg1_i, double *arg1_j, double *param);
extern int deconvoluteMatrices
          (decnv_arr *data, double *parameters);
extern int deconvoluteMatricesParallel
          (decnv_arr *data, double *parameters);
extern  int deconvoluteVectors
          (decnv_arr *data, double *parameters);
extern  int deconvoluteVectorsParallel
          (decnv_arr *data, double *parameters);
// Initialize the number of data and all the arrays
extern int scaleEventoOddMatrix
          (int m_dst, int n_dst, double **dest, 
           int m_src, int n_src, double **src);
extern int initDisplay
          (decnv_arr *data, double *parameters, 
           double (*timestamps)[TYPES]);

// Record or output arrays or data
extern int displaySumDwellTime(int n_all, double *dwelltime);
#endif /*  _FUNC_H */
