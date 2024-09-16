#include "input.h"
#include "func.h"

int main (int argc, char *argv[])
{
  /**************************************************************
    Parameters to adjust deconvolution performance
  **************************************************************/
  static double parameters[PARAMNUM] = {
    1000,     /* ALPHA              */
    0.01,       /* THRESHOLD          */
    01.0,       /* OFFSET_DWELL in ms */
    70.0,      /* OFFSET_TGT in nm   */
    100000,     /* N_LOOPMAX   (int)  */
    50000,      /* N_LOOPREC   (int)  */
    2000,       /* N_LOOPDISP  (int)  */
    1.0,        /* MGNRATIO_ROW (int) */
    1.0,        /* MGNRATIO_COL (int) */
    10E7,       /* RMS_BEF            */
    10E7,       /* RMS_AFT            */
    -1E8,       /* FIGERR_MAX         */
     1E8,       /* FIGERR_MIN         */
    0.95,       /* LS_ALPHA           */
    1.0E-4,     /* LIM_ALPHA          */
    0           /* COUNT     (int)    */
  };
  /**************************************************************
    Some useful arguments (change them where necessary)
  **************************************************************/
  static int  flgs[FLGNUM] = {
    VALID,    /* addtime */
    VALID,    /* readfileonconsole */
    VALID,    /* readparameteronconsole */
    VALID,    /* silent */
    VALID,    /* readasfileshape */
    INVALID   /* extendupdate      */
  };
  static double timestamps[N_TIMEST][TYPES] = {
    {-1.0,-2.0},  /*  ST_TOTAL */ 
    {-1.0,-2.0},  /*  EN_TOTAL */ 
    {-1.0,-2.0},  /*  ST_INIT  */
    {-1.0,-2.0},  /*  EN_INIT  */
    {-1.0,-2.0},  /*  ST_EXP   */
    {-1.0,-2.0}   /*  EN_EXP   */
  };
  static decnv_arr    data[IDX_NUM];
  /**************************************************************
    Initialize the filename, the arrays and the parameters
  **************************************************************/
  getSerialParallelCompTime(timestamps[ST_TOTAL]);
  getSerialParallelCompTime(timestamps[ST_INIT ]);
  // initialize filenames
  if ( 
      (initDecnvArrays (data, flgs) != VALID) ||
      (initExpFilePaths(data, flgs) != VALID) ||
      (initImpFilePaths(data, flgs) != VALID)
     )
  {
    printf("...unable to initialize the files.\n");
    exit(1);
  }
  else
  {
    printf("...initialized the data.\n");
  }
  if (
      importFiles(data, flgs)    != VALID ||
      initParameters(parameters) != VALID ||
      initCalculationRange(data, parameters, flgs) != VALID
     )
  {
    printf("...unable to initialize the ranges.\n");
    exit(1);
  }
  else
  {
    printf("...initialized the range.\n");
  }
  getSerialParallelCompTime(timestamps[EN_INIT ]);
  /* display the initial conditions */
  initDisplay(data, parameters, timestamps);
  if (data[IDX_TGTORG ].row == 1 &&
      data[IDX_UNITORG].row == 1
     )
  {
    printf("...started 1D deconvolution ");
#if defined( _OPENMP) && !defined( _SERIAL_CALCULATION )
    printf("(parallel).\n");
    deconvoluteVectorsParallel(data, parameters);
#else
    printf("(serial).\n");
    deconvoluteVectors(data, parameters);
#endif /*_SERIAL_CALCULATION, _OPENMP */
  }
  else
  {
    printf("...started 2D deconvolution ");
#if defined( _OPENMP) && !defined( _SERIAL_CALCULATION )
    printf("(parallel).\n");
    deconvoluteMatricesParallel(data, parameters);
#else
    printf("(serial).\n");
    deconvoluteMatrices(data, parameters);
#endif /*_SERIAL_CALCULATION, _OPENMP */
  }
  calcMaxMin(data[IDX_ERR].ttl, data[IDX_ERR].mat[0],
      &parameters[FIGERR_MAX], &parameters[FIGERR_MIN]);
/**************************************************************
  Record all data and write them down in files
**************************************************************/
  printf("\n...done.\n");
  getSerialParallelCompTime(timestamps[ST_EXP  ]);
  displaySumDwellTime
    (data[IDX_DWELL].ttl, data[IDX_DWELL].mat[0]);
  exportFiles(data, flgs);
/**************************************************************
  Deallocate all the arrays
**************************************************************/
  terminateDecnvArrays(data, flgs);
  getSerialParallelCompTime(timestamps[EN_EXP  ]);
  getSerialParallelCompTime(timestamps[EN_TOTAL]);
  printf("loop: %9d, alpha: %9.4lf, \n"\
         "rms: %9.4lf, PV: %9.4lf\n",
         (int)parameters[COUNT], parameters[ALPHA], 
         parameters[RMS_AFT], 
         parameters[FIGERR_MAX]-parameters[FIGERR_MIN]);
  printf("Serial Computation time:\n"\
     "-- total: %9.4lf sec.\n"\
     "-- init : %9.4lf sec.\n"\
     "-- iter : %9.4lf sec.\n"\
     "-- exp  : %9.4lf sec.\n", 
     timestamps[EN_TOTAL][SER]-timestamps[ST_TOTAL][SER],
     timestamps[EN_INIT ][SER]-timestamps[ST_INIT ][SER],
     timestamps[ST_EXP  ][SER]-timestamps[EN_INIT ][SER],
     timestamps[EN_EXP  ][SER]-timestamps[ST_EXP  ][SER]
     );
  printf("OpenMP Computation time:\n"\
     "-- total: %9.4lf sec.\n"\
     "-- init : %9.4lf sec.\n"\
     "-- iter : %9.4lf sec.\n"\
     "-- exp  : %9.4lf sec.\n", 
     timestamps[EN_TOTAL][PAR]-timestamps[ST_TOTAL][PAR],
     timestamps[EN_INIT ][PAR]-timestamps[ST_INIT ][PAR],
     timestamps[ST_EXP  ][PAR]-timestamps[EN_INIT ][PAR],
     timestamps[EN_EXP  ][PAR]-timestamps[ST_EXP  ][PAR]
     );
}

