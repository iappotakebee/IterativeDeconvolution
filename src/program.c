#include "input.h"
#include "func.h"

int main (int argc, char *argv[])
{
  /**************************************************************
    Parameters to adjust deconvolution performance
  **************************************************************/
  double alpha      = 1000.0,
         threshold  = 0.01;
  int    n_loopmax  = 200000, 
         n_looprec  = 50000, 
         n_loopdisp = 2000;
  double parameters[PARAMNUM] = {
    01.0, /* offset_dwell in ms */
    70.0,  /* offset_tgt in nm   */
    5.0,  /* mgnratio_row (int) */
    5.0   /* mgnratio_col (int) */
  };
  /**************************************************************
    Some useful arguments (change them where necessary)
  **************************************************************/
  int  flgs[FLGNUM] = {
    VALID,    /* addtime */
    VALID,    /* readfileonconsole */
    VALID,    /* readparameteronconsole */
    VALID,    /* silent */
    VALID,    /* readasfileshape */
    MIX       /* extendupdate      */
  };
  decnv_arr    data[IDX_NUM];
  int          i,j,cnt, cnt_rec, nthreads,
               gap_tr_i, gap_tr_j,gap_ud_i,gap_ud_j;
  double       tmp, rms_bef = 10E7, rms_aft = 10E7,
               figerr_max = 0.0, figerr_min = 10E7,
               st, en, st_omp=-1, en_omp=-2, init_st, init_en;
  const double ls_alpha  = 0.95,
               lim_alpha = 0.5E-6;
  /**************************************************************
    Initialize the filename, the arrays and the parameters
  **************************************************************/
  st = GetCPUTime();
#ifdef _OPENMP
  st_omp = omp_get_wtime();
#endif /* _OPENMP */
  init_st = GetCPUTime();
  printf("alpha       : %lf\n",alpha      );
  printf("threshold   : %lf\n",threshold  );
  printf("offset_time : %lf\n",parameters[OFFSET_DWELL]); // in ms
  printf("offset_hgt  : %lf\n",parameters[OFFSET_TGT  ]); 
  // initialize filenames
  if ( 
      (initDecnvArrays(data, flgs) != VALID)  ||
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
  importFiles(data, flgs);
  initCalculationRange(data, parameters, flgs);
  printDecnvArray(data, IDX_TGTORG  );
  printDecnvArray(data, IDX_UNITORG );
  printDecnvArray(data, IDX_DWELLORG);
  printDecnvArray(data, IDX_TGT     );
  printDecnvArray(data, IDX_UNIT    );
  printDecnvArray(data, IDX_DWELL   );
  printDecnvArray(data, IDX_REAL    );
  printDecnvArray(data, IDX_ERR     );
  printDecnvArray(data, IDX_UPDATE  );
  printDecnvArray(data, IDX_ALL     );
  printDecnvArray(data, IDX_PART    );
  printDecnvArray(data, IDX_DATPATH );
  printDecnvArray(data, IDX_TMSTMP  );
  printDecnvArray(data, IDX_LOG     );
  printDecnvArray(data, IDX_LOGPATH );
  init_en = GetCPUTime();
#ifdef _OPENMP
  nthreads = omp_get_max_threads();
#endif /* _OPENMP */
  /* display the initial conditions */
  initDisplay(data[IDX_TGT].ttl,data[IDX_UNIT].ttl,0,data[IDX_REAL].ttl,init_st, init_en,nthreads);
  /**************************************************************
    Calculate the dwell time in the loop
  **************************************************************/
  cnt = 0; cnt_rec = 0;
  gap_tr_i = data[IDX_TGT].i_st - data[IDX_REAL].i_st;
  gap_tr_j = data[IDX_TGT].j_st - data[IDX_REAL].j_st;
  gap_ud_i = data[IDX_UPDATE].i_st - data[IDX_DWELL].i_st;
  gap_ud_j = data[IDX_UPDATE].j_st - data[IDX_DWELL].j_st;
  /* initialize the figure profile */
  convoluteMatMat(
      data[IDX_REAL ].row, data[IDX_REAL ].col,
      data[IDX_REAL ].mat[0],
      data[IDX_DWELL].row, data[IDX_DWELL].col,
      data[IDX_DWELL].mat[0],
      data[IDX_UNIT ].row, data[IDX_UNIT].col,
      data[IDX_UNIT ].mat[0]
      );
  writeTmpDecnvArrays(data, cnt);
  cnt_rec++;
  printf("Have memorized arrays %d times\n", cnt_rec);
  while (rms_bef > threshold)
  {
    cnt++;
    /* check how many times the loop has been computed */
    if (cnt > n_loopmax)
    {
      printf("Reached the maximum number of loops\n");
      break;
    }
    // display the current parameters
    if (cnt % (int)n_loopdisp == 0)
    {
      printf("loop: %9d, alpha: %9.4lf, rms: %9.4lf\n",
          cnt, alpha, rms_aft);
    } 
    /* intialize the arrays */
    initVectorToDblZero
      (data[IDX_REAL  ].ttl, data[IDX_REAL  ].mat[0]);
    initVectorToDblZero
      (data[IDX_UPDATE].ttl, data[IDX_UPDATE].mat[0]);
    /* convolute the unit sputter yield with the dwell time */
    convoluteMatMat(
        data[IDX_REAL ].row, data[IDX_REAL ].col,
        data[IDX_REAL ].mat[0],
        data[IDX_DWELL].row, data[IDX_DWELL].col,
        data[IDX_DWELL].mat[0],
        data[IDX_UNIT ].row, data[IDX_UNIT].col,
        data[IDX_UNIT ].mat[0]
        );
    /* calculate the errors between the target and the figure */
    rms_bef = rms_aft;  
    rms_aft = 0.0;
    tmp = 0.0;
#ifdef _OPENMP
#pragma omp parallel for default(none)       \
    private(i,j, tmp)                           \
    shared(data, gap_tr_i,gap_tr_j)\
    reduction(+:rms_aft)
#endif /* _OPENMP */
    for (i=0; i<data[IDX_ERR].row; i++)
    {
      for (j=0; j<data[IDX_ERR].col; j++)
      {
        tmp = 
          data[IDX_TGT].mat[i][j] 
          - data[IDX_REAL].mat[gap_tr_i + i][gap_tr_j + j];
        data[IDX_ERR].mat[i][j] = tmp;
        rms_aft += (tmp*tmp);
      }
    }
    rms_aft = sqrt( rms_aft/data[IDX_TGT].ttl );
    /* reduce alpha if the current errors worsen */
    if (rms_bef < rms_aft)
    {
      if (alpha*ls_alpha > lim_alpha)
      {
        alpha *= ls_alpha;
        printf("Multiplied alpha by %9.4lf\n", ls_alpha);
        printf("loop: %9d, alpha: %9.4lf, rms: %9.4lf\n",
            cnt, alpha, rms_aft);
      }
      else
      {
        alpha = lim_alpha;
      }
    }
    else
    {
      /* calculate the evaluation function (err x unit) */
      convoluteMatMat(
          data[IDX_UPDATE].row, data[IDX_UPDATE].col,
          data[IDX_UPDATE].mat[0],
          data[IDX_ERR   ].row, data[IDX_ERR   ].col,
          data[IDX_ERR   ].mat[0],
          data[IDX_UNIT  ].row, data[IDX_UNIT  ].col,
          data[IDX_UNIT  ].mat[0]
          );
      if ( (cnt % (int)n_looprec == 0) || cnt == 1 || cnt == 2)
      {
        writeTmpDecnvArrays(data, cnt);
        cnt_rec++;
        printf("Have memorized arrays %d times\n", cnt_rec);
      } 
      /* refresh the dwelltime using (t=t-alpha × (p-f)) */
#ifdef _OPENMP
#pragma omp parallel for default(none)\
      private(i,j, tmp)\
      shared(alpha,parameters,data,gap_ud_i,gap_ud_j)
#endif /* _OPENMP */
      for (i=0; i<data[IDX_UPDATE].row; i++)
      {
        for (j=0; j<data[IDX_UPDATE].col; j++)
        {
          tmp =
            data[IDX_DWELL].mat[gap_ud_i+i][gap_ud_j+j] +
            (alpha*data[IDX_UPDATE].mat[i][j]);
          /* limit the minimum dwell time */
          if (tmp < parameters[OFFSET_DWELL])
          {
            data[IDX_DWELL].mat[gap_ud_i+i][gap_ud_j+j] = 
              parameters[OFFSET_DWELL];
          }
          else
          {
            data[IDX_DWELL].mat[gap_ud_i+i][gap_ud_j+j] = 
              tmp;
          }
        }
      }
    }
  }
#ifdef _OPENMP
#pragma omp parallel for default(none)\
      private(i,j)\
      shared(data)\
      reduction(max:figerr_max)\
      reduction( min:figerr_min)
#endif /* _OPENMP */
  for (i=0; i<data[IDX_ERR].row; i++)
  {
    for (j=0; j<data[IDX_ERR].col; j++)
    {
      if (figerr_max < data[IDX_ERR].mat[i][j]) 
        figerr_max = data[IDX_ERR].mat[i][j];
      if (figerr_min > data[IDX_ERR].mat[i][j]) 
        figerr_min = data[IDX_ERR].mat[i][j];
    }
  }
/**************************************************************
  Record all data and write them down in files
**************************************************************/
  printf("\nDone.\n");
  printf("loop: %9d, alpha: %9.4lf, \nrms: %9.4lf, PV: %9.4lf\n",
      cnt, alpha, rms_aft, figerr_max-figerr_min);
  DisplaySumDwellTime(data[IDX_DWELL].ttl, data[IDX_DWELL].mat[0]);
  exportFiles(data, flgs);
  terminateDecnvArrays(data, flgs);
  cnt_rec++;
/**************************************************************
  Deallocate all the arrays
**************************************************************/
  en = GetCPUTime();
#ifdef _OPENMP
  en_omp = omp_get_wtime();
#endif /* _OPENMP */
  printf("Serial Computation time: %9.4lf sec.\n", en-st);
  printf("OpenMP Computation time: %9.4lf sec.\n", 
      en_omp-st_omp);
}

