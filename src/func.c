#include"func.h"

int initDecnvArrays(decnv_arr *data, int *flgs)
{
  printf("initDecnvArrays:\n");
  int i;
  for(i=0; i<IDX_NUM; i++)
  {
    data[i].mat = NULL;
    data[i].row = -1;
    data[i].col = -1;
    data[i].ttl = -1;
    data[i].i_st= 0;
    data[i].j_st= 0;
    memset(data[i].name,     '\0', BUFF_SIZE);
    memset(data[i].imppth,   '\0', BUFF_SIZE);
    memset(data[i].exppth,   '\0', BUFF_SIZE);
    memset(data[i].tmpexppth,'\0', BUFF_SIZE);
    memset(data[i].histpth,  '\0', BUFF_SIZE);
  }
  strcpy(data[IDX_TGTORG  ].name,"target_org");
  strcpy(data[IDX_UNITORG ].name,"unit_org");
  strcpy(data[IDX_DWELLORG].name,"dwelltime_org");
  strcpy(data[IDX_TGT     ].name,"target");
  strcpy(data[IDX_UNIT    ].name,"unit");
  strcpy(data[IDX_DWELL   ].name,"dwelltime");
  strcpy(data[IDX_REAL    ].name,"realshape");
  strcpy(data[IDX_ERR     ].name,"error");
  strcpy(data[IDX_UPDATE  ].name,"update");
  strcpy(data[IDX_ALL     ].name,"all");
  strcpy(data[IDX_PART    ].name,"part");
  strcpy(data[IDX_DATPATH ].name,"data_part");
  strcpy(data[IDX_TMSTMP  ].name,"timestamp");
  strcpy(data[IDX_LOG     ].name,"log");
  strcpy(data[IDX_LOGPATH ].name,"log_path");
  return 0;
}
int terminateDecnvArrays(decnv_arr *data, int *flgs)
{
  int i;
  for(i=0; i<IDX_NUM; i++)
  {
    if (data[i].mat != NULL)
      deallocateMatrix((void**)data[i].mat);
  }
  return 0;
}
int printDecnvArray(decnv_arr *data, int index)
{

  FILE *fp;
  fp=fopen(data[IDX_LOG].exppth, "a");
  printf("printDecnvArray:\n");
  printf("-- mat       : %p\n",data[index].mat   );
  printf("-- row       : %d\n",data[index].row   );
  printf("-- col       : %d\n",data[index].col   );
  printf("-- ttl       : %d\n",data[index].ttl   );
  printf("-- i_st      : %d\n",data[index].i_st  );
  printf("-- j_st      : %d\n",data[index].j_st  );
  printf("-- name      : %s\n",data[index].name  );
  printf("-- imppth    : %s\n",data[index].imppth);
  printf("-- tmpexppth : %s\n",data[index].tmpexppth);
  printf("-- exppth    : %s\n",data[index].exppth);
  printf("-- histpth   : %s\n",data[index].histpth);
  fprintf(fp, "printDecnvArray:\n");
  fprintf(fp, "-- mat       : %p\n",data[index].mat   );
  fprintf(fp, "-- row       : %d\n",data[index].row   );
  fprintf(fp, "-- col       : %d\n",data[index].col   );
  fprintf(fp, "-- ttl       : %d\n",data[index].ttl   );
  fprintf(fp, "-- i_st      : %d\n",data[index].i_st  );
  fprintf(fp, "-- j_st      : %d\n",data[index].j_st  );
  fprintf(fp, "-- name      : %s\n",data[index].name  );
  fprintf(fp, "-- imppth    : %s\n",data[index].imppth);
  fprintf(fp, "-- tmpexppth : %s\n",data[index].tmpexppth);
  fprintf(fp, "-- exppth    : %s\n",data[index].exppth);
  fprintf(fp, "-- histpth   : %s\n",data[index].histpth);
  fclose(fp);
  return 0;
}
int writeDecnvArray(decnv_arr *data, int index)
{
#ifdef _DEBUG_
  printf("\nwriteDecnvArray:\n-- ");
  printDecnvArray(data, index);
#endif /* _DEBUG_ */
  FILE *fp;
  int i,j, cnt;
  char header_format[] = " ,%s,%s,%s,%s,%s\n";
  char value_format[]  = 
    "%d,%30.18lf,%30.18lf,%30.18lf,%30.18lf,%30.18lf\n";
  decnv_arr *p_datum = &data[index];
  /* write the current array */
  if(p_datum->mat != NULL && p_datum->exppth[0] != '\0')
  {
    if (p_datum->row == 1)
    {
      fp=fopen(p_datum->exppth, "w");
      faddSglPointer(fp, p_datum->col, p_datum->mat[0]);
#ifdef _DEBUG_
      printf("-- wrote (%d) arrays at\n    %s\n",
          p_datum->col,p_datum->exppth);
#endif /* _DEBUG_ */
    }
    else
    {
      fp=fopen(p_datum->exppth, "w");
      faddDblPointer(fp, p_datum->row, p_datum->col, p_datum->mat);
#ifdef _DEBUG_
      printf("-- wrote (%d x %d) arrays at\n    %s\n",
          p_datum->row, p_datum->col, p_datum->exppth);
#endif /* _DEBUG_ */
    }
    fclose(fp);
  }
  else
  {
#ifdef _DEBUG_
    printf("-- nothing to write.\n");
#endif /* _DEBUG_ */
  }
  /* add the current array to the history file */
  if(p_datum->mat != NULL && p_datum->histpth[0] != '\0')
  {
      fp=fopen(p_datum->histpth, "a");
      faddSglPointer(fp, p_datum->ttl, p_datum->mat[0]);
#ifdef _DEBUG_
      printf("-- wrote (1 x %d) arrays at\n    %s\n",
          p_datum->ttl, p_datum->histpth);
#endif /* _DEBUG_ */
      fclose(fp);
  }
  /* output all the data in a single csv */
  if(data[IDX_ALL].exppth[0] != '\0')
  {
    fp=fopen(data[IDX_ALL].exppth, "w");
    fprintf(fp, header_format,
        data[IDX_TGT   ].name,
        data[IDX_REAL  ].name,
        data[IDX_ERR   ].name,
        data[IDX_DWELL ].name,
        data[IDX_UNIT  ].name
        );
    cnt = 0;
    for(i=0; i<data[IDX_REAL].row; i++)
    {
      for(j=0; j<data[IDX_REAL].col; j++)
      {
        fprintf(fp, value_format,
            cnt,
            convertDecnvArray(&data[IDX_TGT   ],i,j),
            convertDecnvArray(&data[IDX_REAL  ],i,j),
            convertDecnvArray(&data[IDX_ERR   ],i,j),
            convertDecnvArray(&data[IDX_DWELL ],i,j),
            convertDecnvArray(&data[IDX_UNIT  ],i,j)
            );
        cnt++;
      }
    }
    fclose(fp);
  }
  /* output extracted the data in a single csv */
  if(data[IDX_PART].exppth[0] != '\0')
  {
    fp=fopen(data[IDX_PART].exppth, "w");
    fprintf(fp, header_format,
        data[IDX_TGT   ].name,
        data[IDX_REAL  ].name,
        data[IDX_ERR   ].name,
        data[IDX_DWELL ].name,
        data[IDX_UNIT  ].name
        );
    cnt = 0;
    for(i=data[IDX_TGT].i_st; 
        i<data[IDX_TGT].i_st+data[IDX_TGT].row; 
        i++)
    {
      for(j=data[IDX_TGT].j_st; 
          j<data[IDX_TGT].j_st+data[IDX_TGT].col; 
          j++)
      {
        fprintf(fp, value_format,
            cnt,
            convertDecnvArray(&data[IDX_TGT   ],i,j),
            convertDecnvArray(&data[IDX_REAL  ],i,j),
            convertDecnvArray(&data[IDX_ERR   ],i,j),
            convertDecnvArray(&data[IDX_DWELL ],i,j),
            convertDecnvArray(&data[IDX_UNIT  ],i,j)
            );
        cnt++;
      }
    }
    fclose(fp);
  }
  return 0;
}
int writeTmpDecnvArray(decnv_arr *data, int index, int tmpcnt)
{
#ifdef _DEBUG_
  printf("\nwriteTmpDecnvArray:\n-- ");
  printDecnvArray(data, index);
#endif /* _DEBUG_ */
  FILE *fp;
  char tmpexppth[BUFF_SIZE];
  char tmpnm[BUFF_SIZE];
  int flg_silent=VALID;
  decnv_arr *p_datum = &data[index];
  /* create a file path to temporary files */
  if(p_datum->mat != NULL && p_datum->tmpexppth[0] != '\0')
  {
    snprintf(tmpexppth, BUFF_SIZE, "%.*s%d/", 
        BUFF_SIZE-7, p_datum->tmpexppth,
        tmpcnt);
    snprintf(tmpnm, BUFF_SIZE, "%.*s.csv", 
        BUFF_SIZE-6, p_datum->name);
    initFileNames(tmpexppth,"",  
      tmpexppth, tmpnm, flg_silent);
    if (p_datum->row == 1)
    {
      fp=fopen(tmpexppth, "w");
      faddSglPointer(fp, p_datum->col, p_datum->mat[0]);
#ifdef _DEBUG_
      printf("-- wrote (%d) arrays at\n    %s\n",
          p_datum->col,tmpexppth);
#endif /* _DEBUG_ */
    }
    else
    {
      fp=fopen(tmpexppth, "w");
      faddDblPointer(fp, p_datum->row, p_datum->col, p_datum->mat);
#ifdef _DEBUG_
      printf("-- wrote (%d x %d) arrays at\n    %s\n",
          p_datum->row, p_datum->col, tmpexppth);
#endif /* _DEBUG_ */
    }
    fclose(fp);
  }
  else
  {
#ifdef _DEBUG_
    printf("-- nothing to write.\n");
#endif /* _DEBUG_ */
  }
  /* add the current array to the history file */
  if(p_datum->mat != NULL && p_datum->histpth[0] != '\0')
  {
      fp=fopen(p_datum->histpth, "a");
      faddSglPointer(fp, p_datum->ttl, p_datum->mat[0]);
#ifdef _DEBUG_
      printf("-- wrote (1 x %d) arrays at\n    %s\n",
          p_datum->ttl, p_datum->histpth);
#endif /* _DEBUG_ */
      fclose(fp);
  }
  return 0;
}
int writeTmpDecnvArrays(decnv_arr *data, int tmpcnt)
{
#ifdef _DEBUG_
  printf("\nwriteTmpDecnvArrays:\n-- ");
#endif /* _DEBUG_ */
  int i;
#ifndef _SERIAL_CALCULATION
#pragma omp parallel for default(none)\
      private(i)\
      shared(data, tmpcnt)
#endif /* _SERIAL_CALCULATION */
  for (i=0; i<IDX_NUM; i++)
  {
    writeTmpDecnvArray(data, i, tmpcnt);
  }
  return 0;
}
int initImpFilePaths(decnv_arr *data, int *flgs)
{
  FILE *fp_log;
  int flg_work   = VALID;
  int flg_silent = INVALID;
  flg_work += initFileNames(data[IDX_LOG  ].imppth,"",  
      data[IDX_LOGPATH].exppth,"log.txt"  , flg_silent);
  if (flgs[READFILE_ON_CONSOLE] != VALID)
  {
    flg_work += initFileNames(data[IDX_TGTORG  ].imppth,"",
        "./files/input/","target_test.txt"    , flg_silent);
    flg_work += initFileNames(data[IDX_UNITORG ].imppth,"",
        "./files/input/","unit_test.txt"      , flg_silent);
    flg_work += initFileNames(data[IDX_DWELLORG].imppth,"",
        "./files/input/","guesstime_test.txt" , flg_silent);
  }
  //data[IDX_TGTORG]  ="./files/input/20201102/20201102VFMFirstTrialfromDS_filtered.txt";
  //data[IDX_UNITORG] ="./files/input/20201102/20201102SputterYield_filtered.txt"; 
  else
  {
    if (scanFilePaths(data) != VALID)
    {
      printf("Opening the log file at %s\n", data[IDX_LOG].imppth);
      if ( (fp_log = fopen(data[IDX_LOG].imppth, "r")) == NULL )
      {
        printf("Couldn't open a log file.\n");
        flg_work = INVALID;
      }
      else
      {
        printf("Opened the log file.\n");
        if (fscanf(fp_log, "%s", data[IDX_TGTORG].imppth) == EOF)
        {
          printf("Invalid log file for the target.\n");
          flg_work = INVALID;
        }
        if (fscanf(fp_log, "%s", data[IDX_UNITORG].imppth) == EOF)
        {
          printf("Invalid log file for the unit.\n");
          flg_work = INVALID;
        }
        if (fscanf(fp_log, "%s", data[IDX_DWELLORG].imppth) == EOF)
        {
          printf("Invalid log file for the guess time.\n");
          flg_work = INVALID;
        }
        fclose(fp_log);
      }
    }
  }
  if ( (fp_log = fopen(data[IDX_LOG].imppth, "w")) == NULL )
  {
    printf("Couldn't open a log file.\n");
    flg_work = INVALID;
  }
  else if (fprintf(fp_log, "%s\n%s\n%s\n\n", 
        data[IDX_TGTORG].imppth, data[IDX_UNITORG].imppth, 
        data[IDX_DWELLORG].imppth) <0)
  {
    printf("Couldn't record the input files.\n");
    flg_work = INVALID;
  }
  fclose(fp_log);
  return flg_work;
}
int scanFilePaths(decnv_arr *data)
{
  char answer[10];
  printf("Use the previous files? [y/n]\n");
  scanf("%s", answer);
  if (strcmp(answer,"y")==0){
    return INVALID; 
  }
  else
  {
    printf("Input file path of targeted shape.\n");
    scanf("%s", data[IDX_TGTORG].imppth);
    printf("Input file path of unit shape.\n");
    scanf("%s", data[IDX_UNITORG].imppth);
    printf("Input file path of a guess dwell time.\n");
    scanf("%s", data[IDX_DWELLORG].imppth);
  }
  return VALID;
}
int  initExpFilePaths(decnv_arr *data, int *flgs)
{
  printf("initExpFilePaths:\n");
  int flg_silent = INVALID;
  int flg_work = VALID;
  char ctmp[BUFF_SIZE]="";
  /* initialize the export path */
  getCurrentTime(data[IDX_TMSTMP].exppth);
  flg_work += initFileNames
    (data[IDX_LOGPATH].exppth,"","../files/tmp/","", flg_silent);
  flg_work += initFileNames
    (data[IDX_DATPATH].exppth,"","../files/outputOMP/","", flg_silent);
  if (flgs[ADD_TIME] == VALID)
  {
    sprintf(ctmp, "%s", data[IDX_TMSTMP].exppth);
  }
  flg_work += initFileNames(data[IDX_TGTORG].exppth,ctmp,
      data[IDX_DATPATH].exppth,"target_org.csv"         ,
      flg_silent);
  flg_work += initFileNames(data[IDX_UNITORG].exppth,ctmp,
      data[IDX_DATPATH].exppth,"unit_org.csv"           ,
      flg_silent);
  flg_work += initFileNames(data[IDX_DWELLORG].exppth,ctmp,
      data[IDX_DATPATH].exppth,"guess_dwelltime.csv"     ,
      flg_silent);
  flg_work += initFileNames(data[IDX_TGT  ].exppth,ctmp,
      data[IDX_DATPATH].exppth,"target.csv"         ,
      flg_silent);
  flg_work += initFileNames(data[IDX_UNIT ].exppth,ctmp,
      data[IDX_DATPATH].exppth,"unit.csv"           ,
      flg_silent);
  flg_work += initFileNames(data[IDX_DWELL].exppth,ctmp,
      data[IDX_DATPATH].exppth,"dwelltime.csv"     ,
      flg_silent);
  flg_work += initFileNames(data[IDX_REAL ].exppth,ctmp,
      data[IDX_DATPATH].exppth,"realshape.csv"     ,
      flg_silent);
  flg_work += initFileNames(data[IDX_LOG  ].exppth,"",  
      data[IDX_LOGPATH].exppth,"log.txt"            ,
      flg_silent);
  flg_work += initFileNames(data[IDX_ALL  ].exppth,ctmp,
      data[IDX_DATPATH].exppth,"all.csv"            ,
      flg_silent);
  flg_work += initFileNames(data[IDX_PART ].exppth,ctmp,
      data[IDX_DATPATH].exppth,"extraction.csv"     ,
      flg_silent);
  flg_work += initFileNames(data[IDX_ERR  ].exppth,ctmp,
      data[IDX_DATPATH].exppth,"errors.csv"         ,
      flg_silent);
  /* initialize the history path */
  flg_work += initFileNames(data[IDX_TGT  ].histpth,ctmp,
      data[IDX_DATPATH].exppth,"target_history.csv"    ,
      flg_silent);
  flg_work += initFileNames(data[IDX_UNIT ].histpth,ctmp,
      data[IDX_DATPATH].exppth,"unit_history.csv"      ,
      flg_silent);
  flg_work += initFileNames(data[IDX_DWELL].histpth,ctmp,
      data[IDX_DATPATH].exppth,"dwelltime_history.csv",
      flg_silent);
  flg_work += initFileNames(data[IDX_REAL ].histpth,ctmp,
      data[IDX_DATPATH].exppth,"realshape_history.csv" ,
      flg_silent);
  flg_work += initFileNames(data[IDX_ERR  ].histpth,ctmp,
      data[IDX_DATPATH].exppth,"errors_history.csv"    ,
      flg_silent);
  flg_work += initFileNames(data[IDX_UPDATE].histpth,ctmp,
      data[IDX_DATPATH].exppth,"update_history.csv"    ,
      flg_silent);
  /* initialize the temporary export path */
  flg_work += initFileNames(data[IDX_LOGPATH ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_DATPATH ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_TGTORG  ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_UNITORG ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_DWELLORG].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_TGT     ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_UNIT    ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_DWELL   ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_REAL    ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_LOG     ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_ALL     ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_PART    ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  flg_work += initFileNames(data[IDX_ERR     ].tmpexppth,ctmp,
      data[IDX_DATPATH].exppth,"loop", flg_silent);
  return flg_work;
}
int importFiles(decnv_arr *data, int *flgs)
{
  data[IDX_TGTORG].mat = 
    readSeparatedValueFiles(&data[IDX_TGTORG].row, 
        &data[IDX_TGTORG].col, data[IDX_TGTORG].imppth,   
        flgs[READ_AS_FILESHAPE]);
  data[IDX_UNITORG].mat = 
    readSeparatedValueFiles(&data[IDX_UNITORG].row, 
        &data[IDX_UNITORG].col, data[IDX_UNITORG].imppth,   
        flgs[READ_AS_FILESHAPE]);
  data[IDX_DWELLORG].mat = 
    readSeparatedValueFiles(&data[IDX_DWELLORG].row, 
        &data[IDX_DWELLORG].col, data[IDX_DWELLORG].imppth,   
        flgs[READ_AS_FILESHAPE]);
  data[IDX_TGTORG].ttl = 
    data[IDX_TGTORG].row * data[IDX_TGTORG].col;
  data[IDX_UNITORG].ttl = 
    data[IDX_UNITORG].row * data[IDX_UNITORG].col;
  data[IDX_DWELLORG].ttl = 
    data[IDX_DWELLORG].row * data[IDX_DWELLORG].col;
  return 0;

}
int exportFiles(decnv_arr *data, int *flgs)
{
  int i;
  decnv_arr *tmp_decnv_arr;
  for (i=0;i<IDX_NUM;i++)
  {
    tmp_decnv_arr = &data[i];
    if (tmp_decnv_arr->exppth[0] != '\0')
      writeDecnvArray(data, i);
    if (tmp_decnv_arr->histpth[0] != '\0')
      transSeparatedFile(tmp_decnv_arr->histpth);
    if (tmp_decnv_arr->row == 1)
      transSeparatedFile(tmp_decnv_arr->exppth);
  }
  return 0;
}
int initImpMatrixShape(int (*numelem)[MATINFO_NUM], int *flgs)
{
  int i,j;
  char answer[20];
  for(i=0;i<IDX_NUM;i++)
  {
    for(j=0;j<MATINFO_NUM;j++)
    {
      numelem[i][j]=0;
    }
  }
  if(flgs[READ_AS_FILESHAPE] == VALID)
  {
  }
  else if(flgs[READ_AS_FILESHAPE] == INVALID)
  {
    printf("Read files as written? [y/n]\n");
    scanf("%s", answer);
    if (strcmp(answer,"n")==0){
      printf("Target shape [xx yy]\n");
      scanf("%d %d", &numelem[IDX_TGT][ROW], &numelem[IDX_TGT][COL]);
      printf("Unit shape [xx yy]\n");
      scanf("%d %d", &numelem[IDX_UNIT][ROW], &numelem[IDX_UNIT][COL]);
      return 0;
    }
    else
    {
      flgs[READ_AS_FILESHAPE] = VALID;
    }
  }
  return 0;
}
int initCalculationRange
   (decnv_arr *data, int mgnratio_row, int mgnratio_col) 
{
  printf("\ninitCalculationRange:\n");
  /* calculate the representatives of the unit mat*/
  printf("-- initializing the unit mat.\n");
  data[IDX_UNIT].row = 
    data[IDX_UNITORG].row+data[IDX_UNITORG].row%2-1;
  data[IDX_UNIT].col = 
    data[IDX_UNITORG].col+data[IDX_UNITORG].col%2-1;
  data[IDX_UNIT].ttl = data[IDX_UNIT].row * data[IDX_UNIT].col;
  data[IDX_UNIT].mat = 
    (double**) allocateMatrix
    (sizeof(double), data[IDX_UNIT].row, data[IDX_UNIT].col );
  if( scaleEventoOddMatrix(
      data[IDX_UNIT].row, data[IDX_UNIT].col, 
      data[IDX_UNIT].mat,
      data[IDX_UNITORG].row, data[IDX_UNITORG].col, 
      data[IDX_UNITORG].mat
      ) == INVALID)
  {
    printf("-- unable to shrink the unit matrix.\n");
  }
  /* calculate the representatives of the target mat*/
  printf("-- initializing the target mat.\n");
  data[IDX_TGT].row = data[IDX_TGTORG].row;
  data[IDX_TGT].col = data[IDX_TGTORG].col;
  data[IDX_TGT].ttl = data[IDX_TGT].row * data[IDX_TGT].col;
  if(data[IDX_UNIT].row != 1 && data[IDX_TGT].row != 1)
    data[IDX_TGT].i_st= data[IDX_UNIT].row*mgnratio_row;
  data[IDX_TGT].j_st= data[IDX_UNIT].col*mgnratio_col;
  data[IDX_TGT].mat = 
    (double**) allocateMatrix
    (sizeof(double), data[IDX_TGT].row, data[IDX_TGT].col );
  copyMatrix(
      data[IDX_TGT].row, data[IDX_TGT].col, 
      data[IDX_TGT].mat[0], 
      data[IDX_TGTORG].mat[0]);
  /* calculate the representatives of the dwelltime mat*/
  printf("-- initializing the dwell time mat.\n");
  if(data[IDX_UNIT].row == 1 && data[IDX_TGT].row == 1)
  {
    /* 1D deconvolution */
    data[IDX_DWELL].row = data[IDX_TGT].row;
    data[IDX_DWELL].col = (int)(
      data[IDX_TGT].col 
      + data[IDX_UNIT].col * (mgnratio_col-1) * 2
      + (data[IDX_UNIT].col + 1));
    data[IDX_DWELL].j_st = (int)(0.5*(data[IDX_UNIT].col -1));
  }
  else
  {
    /* 2D deconvolution */
    data[IDX_DWELL].row = (int)(
      data[IDX_TGT].row 
      + data[IDX_UNIT].row * (mgnratio_row-1) * 2
      + 0.5* (data[IDX_UNIT].row + 1));
    data[IDX_DWELL].col = (int)(
      data[IDX_TGT].col 
      + data[IDX_UNIT].col * (mgnratio_col-1) * 2
      + 0.5* (data[IDX_UNIT].col + 1));
    data[IDX_DWELL].i_st = (int)(0.5*(data[IDX_UNIT].row -1));
    data[IDX_DWELL].j_st = (int)(0.5*(data[IDX_UNIT].col -1));
  }
  data[IDX_DWELL].ttl =
    data[IDX_DWELL].row * data[IDX_DWELL].col;
  data[IDX_DWELL].mat = 
    (double**) allocateMatrix
    (sizeof(double), data[IDX_DWELL].row, data[IDX_DWELL].col );
  initVectorToDblZero
    (data[IDX_DWELL].ttl, data[IDX_DWELL].mat[0]);
  if (data[IDX_DWELLORG].mat != NULL)
  {
    if( scaleEventoOddMatrix(
        data[IDX_DWELL].row, data[IDX_DWELL].col, 
        data[IDX_DWELL].mat,
        data[IDX_DWELLORG].row, data[IDX_DWELLORG].col, 
        data[IDX_DWELLORG].mat
        ) == INVALID)
    {
      printf("-- unable to shrink the dwelltime matrix.\n");
      printf("-- initialized the dwell time matrix with 0.\n");
    }
  }
  /* calculate the range of the real shape mat*/
  printf("-- initializing the real shape mat.\n");
  if(data[IDX_UNIT].row == 1 && data[IDX_TGT].row == 1)
  {
    /* 1D deconvolution */
    data[IDX_REAL].row = data[IDX_TGT].row;
    data[IDX_REAL].col = 
      data[IDX_TGT].col + 
      data[IDX_UNIT].col * mgnratio_col * 2; 
  }
  else
  {
    /* 2D deconvolution */
    data[IDX_REAL].row = 
      data[IDX_TGT].row + data[IDX_UNIT].row * mgnratio_row * 2;
    data[IDX_REAL].col = 
      data[IDX_TGT].col + data[IDX_UNIT].col * mgnratio_col * 2;
  }
  data[IDX_REAL].ttl = data[IDX_REAL].row * data[IDX_REAL].col;
  data[IDX_REAL].mat = 
    (double**) allocateMatrix
    (sizeof(double), data[IDX_REAL].row, data[IDX_REAL].col);
  initVectorToDblZero
    (data[IDX_REAL].ttl, data[IDX_REAL].mat[0]);
  /* calculate the range of the error mat */
  printf("-- initializing the error mat.\n");
  data[IDX_ERR].row = data[IDX_TGT].row;
  data[IDX_ERR].col = data[IDX_TGT].col;
  data[IDX_ERR].ttl = data[IDX_ERR].row * data[IDX_ERR].col;
  data[IDX_ERR].i_st= data[IDX_TGT].i_st;
  data[IDX_ERR].j_st= data[IDX_TGT].j_st;
  data[IDX_ERR].mat = 
    (double**) allocateMatrix
    (sizeof(double), data[IDX_ERR].row, data[IDX_ERR].col);
  initVectorToDblZero
    (data[IDX_ERR].ttl, data[IDX_ERR].mat[0]);
  /* calculate the range of the update mat*/
  printf("-- initializing the update mat.\n");
  if(data[IDX_UNIT].row == 1 && data[IDX_TGT].row == 1)
  {
    /* 1D deconvolution */
    data[IDX_UPDATE].row = data[IDX_ERR].row;
    data[IDX_UPDATE].col = 
      data[IDX_ERR].col + data[IDX_UNIT].col - 1;
    data[IDX_UPDATE].i_st = data[IDX_ERR].i_st;
    data[IDX_UPDATE].j_st = 
      data[IDX_ERR].j_st - (int)(0.5*(data[IDX_UNIT].col - 1));
  }
  else
  {
    /* 2D deconvolution */
    data[IDX_UPDATE].row = 
      data[IDX_ERR].row + data[IDX_UNIT].row - 1;
    data[IDX_UPDATE].col = 
      data[IDX_ERR].col + data[IDX_UNIT].col - 1;
    data[IDX_UPDATE].i_st = 
      data[IDX_ERR].i_st - (int)(0.5*(data[IDX_UNIT].row - 1));
    data[IDX_UPDATE].j_st = 
      data[IDX_ERR].j_st - (int)(0.5*(data[IDX_UNIT].col - 1));
  }
    /* 2D deconvolution */
  data[IDX_UPDATE].ttl = 
    data[IDX_UPDATE].row * data[IDX_UPDATE].col;
  data[IDX_UPDATE].mat = 
    (double**) allocateMatrix
    (sizeof(double), data[IDX_UPDATE].row, data[IDX_UPDATE].col);
  initVectorToDblZero
    (data[IDX_UPDATE].ttl, data[IDX_UPDATE].mat[0]);
  return 0;
}
double convertDecnvArray
   (decnv_arr *datum,int glbl_i, int glbl_j)
{
  if(
      glbl_i-datum->i_st > -1         &&
      glbl_i-datum->i_st < datum->row &&
      glbl_j-datum->j_st > -1         &&
      glbl_j-datum->j_st < datum->col 
    )
  {
    return datum->mat\
      [glbl_i-datum->i_st][glbl_j-datum->j_st];
  }
  else
  {
    return 0.0;
  }
}
/**************************************************************
   Initialize the number of data and all the arrays
**************************************************************/
int scaleEventoOddMatrix(int m_dst, int n_dst, double **dest, 
    int m_src, int n_src, double **src)
{
  printf("\nscaleEventoOddMatrix:\n");
  int m_gap = m_dst - m_src, n_gap = n_dst - n_src;
  int i,j, flg_err = VALID;
  printf("-- dst: (%d, %d)\n", m_dst, n_dst);
  printf("-- src: (%d, %d)\n", m_src, n_src);
  printf("-- gap: (%d, %d)\n", m_gap, n_gap);
  if (m_gap  == 0 && n_gap == 0)
  {
    copyMatrix(m_dst, n_dst, dest[0], src[0]);
  }
  else if (m_gap == 0 && n_gap == -1)
  {
#ifndef _SERIAL_CALCULATION
#pragma omp parallel for default(none) \
  private(i,j)\
  shared(m_dst,n_dst,dest,src)
#endif /* _SERIAL_CALCULATION */
    for(i=0;i<m_dst;i++)
    {
      for(j=0;j<n_dst;j++)
      {
        dest[i][j] = 0.5 *(src[i][j]+src[i][j+1]);
      }
    }
  }
  else if (m_gap == -1 && n_gap == 0)
  {
#ifndef _SERIAL_CALCULATION
#pragma omp parallel for default(none) \
  private(i,j)\
  shared(m_dst,n_dst,dest,src)
#endif /* _SERIAL_CALCULATION */
    for(i=0;i<m_dst;i++)
    {
      for(j=0;j<n_dst;j++)
      {
        dest[i][j] = 0.5 *(src[i][j]+src[i+1][j]);
      }
    }
  }
  else if (m_gap == -1 && n_gap == -1)
  {
#ifndef _SERIAL_CALCULATION
#pragma omp parallel for default(none) \
  private(i,j)\
  shared(m_dst,n_dst,dest,src)
#endif /* _SERIAL_CALCULATION */
    for(i=0;i<m_dst;i++)
    {
      for(j=0;j<n_dst;j++)
      {
        dest[i][j] = 
          0.25 *
          (src[i][j]+src[i+1][j]+src[i][j+1]+src[i+1][j+1]);
      }
    }
  }
  else
  {
    printf("-- invalid arguments.\n");
    flg_err = INVALID;
  }
  return flg_err;
}
int initDisplay(int n_tgt, int n_uni, int hn_uni, int n_all,
    double init_st, double init_en, int nthreads)
{
  int i, myid=-1;
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
#ifndef _SERIAL_CALCULATION
#pragma omp parallel for default(none) \
    private(i,myid)                    \
    shared(nthreads)
#endif /* _SERIAL_CALCULATION */
  for (i=0; i<nthreads;i++)
  {
#ifdef _OPENMP
    myid = omp_get_thread_num();
#endif /* _OPENMP */
    printf("  # Thread   : %7d\n",  myid);
  }
  printf("Computation time for initialization: %9.4lf\n", 
      init_en-init_st);
  printf("\n");
  return 0;
}
  /**************************************************************
     Record or output arrays or data                     
  **************************************************************/
int MemorizeData(int ni, int cnt, double* a, double** hist)
{
  int i;
#ifndef _SERIAL_CALCULATION
#pragma omp parallel for default(none) \
  private(i)                           \
  shared(cnt,ni,hist, a)               
#endif /* _SERIAL_CALCULATION */
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
int DisplaySumDwellTime(int n_all, double *dwelltime)
{
  int i;
  double sum=0.0;
#ifndef _SERIAL_CALCULATION
#pragma omp parallel for default(none)\
  private(i)                          \
  shared(n_all, dwelltime)            \
  reduction(+:sum)
#endif /* _SERIAL_CALCULATION */
  for (i=0; i<n_all; i++)
  {
    sum += dwelltime[i];
  }
  printf("Total fabrication time: %9.4lf minutes\n"
      , sum/MS_TO_MIN);
  return 0;
}
