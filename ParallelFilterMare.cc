// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2014 Qualcomm Technologies, Inc.  All rights reserved.
// Confidential & Proprietary – Qualcomm Technologies, Inc. ("QTI")
// 
// The party receiving this software directly from QTI (the "Recipient")
// may use this software as reasonably necessary solely for the purposes
// set forth in the agreement between the Recipient and QTI (the
// "Agreement").  The software may be used in source code form solely by
// the Recipient's employees (if any) authorized by the Agreement.
// Unless expressly authorized in the Agreement, the Recipient may not
// sublicense, assign, transfer or otherwise provide the source code to
// any third party.  Qualcomm Technologies, Inc. retains all ownership
// rights in and to the software.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>

#define NPIXELX 1000
#define NPIXELY 1000
#define GRAX 1
#define GRAY 1
#define ARRAYX (int)(ceil(NPIXELX/(double)GRAX))
#define ARRAYY (int)(ceil(NPIXELY/(double)GRAY))
#include <mare/mare.h>
#include <stdio.h>   

using namespace std;

mare::task_ptr task_im[ARRAYX][ARRAYY] = { NULL };

//Compilación con gcc -o filtro filter.c -fopenmp -lm
int main(int argc, char **argv){
    
    int px=NPIXELX;
    int py=NPIXELY;
    int i=0,j=0;
    double sum, promedio;
    
    double im[NPIXELX][NPIXELY];
    mare::group_ptr g = mare::create_group("Filtro");
    
    // "Lectura/Inicialización" de la imagen 
    for(i=0; i < px; i++)
        for(j=0; j < py; j++)
            im[i][j] = (double) (i*NPIXELX)+j;
    
    // Promedio inicial  (test de entrada)
    sum = 0.0;
    for(i=0; i < px; i++)
        for(j=0; j < py; j++)
            sum += im[i][j];
    
    promedio = sum /(px*py);
    printf("El promedio inicial es %g\n", promedio);
    
    
    /* Filtro a Paralelizar
    for(i=1; i < (px-1); i++)
        for(j=1; j < (py-1); j++)
            im[i][j] += 0.25 * sqrt((im[i-1][j]+im[i][j-1]));
    */
    i=0;
    j=0; 

    task_im[0][0] = mare::create_task([&im,&i,&j]{ 
	int shiftx = GRAX*i+1;
	int shifty = GRAY*j+1;
	int w,z;
	for (w = 0; (w<GRAX)&&((w+shiftx)<NPIXELX-1);w++)
	  for (z = 0;(z<GRAY)&&((z+shifty)<NPIXELY-1);z++)
	    im[w+shiftx][z+shifty] += 0.25 * sqrt((im[w+shiftx-1][z+shifty]+im[w+shiftx][z+shifty-1]));	
      });
    mare::launch(g,task_im[0][0]);
    
    for(i=1; i < ARRAYX; i++){ 
      task_im[i][j] = mare::create_task([&im,&i,&j]{ 
	int shiftx = GRAX*i+1;
	int shifty = GRAY*j+1;
	int w,z;
	for (w = 0; (w<GRAX)&&((w+shiftx)<NPIXELX-1);w++)
	  for ( z = 0;(z<GRAY)&&((z+shifty)<NPIXELY-1);z++)
	    im[w+shiftx][z+shifty] += 0.25 * sqrt((im[w+shiftx-1][z+shifty]+im[w+shiftx][z+shifty-1]));
      });
      mare::after(task_im[i-1][0],task_im[i][0]);
      mare::launch(g,task_im[i][0]);
    }
    
    i=0;
    j=0;

    for(j=1; j < ARRAYY; j++){
      task_im[i][j] = mare::create_task([&im,&i,&j]{ 
	int shiftx = GRAX*i+1;
	int shifty = GRAY*j+1;
	int w,z;
	for (w = 0; (w<GRAX)&&((w+shiftx)<NPIXELX-1);w++)
	  for ( z = 0;(z<GRAY)&&((z+shifty)<NPIXELY-1);z++)
	    im[w+shiftx][z+shifty] += 0.25 * sqrt((im[w+shiftx-1][z+shifty]+im[w+shiftx][z+shifty-1]));
      });
      mare::after(task_im[0][j-1],task_im[0][j]);
      mare::launch(g,task_im[0][j]);
    }
    for( i=1; i < ARRAYX;i++){
      for(j=1; j < ARRAYY; j++){
	task_im[i][j] = mare::create_task([&im,&i,&j]{ 
	    int shiftx = GRAX*i+1;
	    int shifty = GRAY*j+1;
	    int w,z;
	    for (w = 0; (w<GRAX)&&((w+shiftx)<NPIXELX-1);w++)
	      for ( z = 0;(z<GRAY)&&((z+shifty)<NPIXELY-1);z++)
		im[w+shiftx][z+shifty] += 0.25 * sqrt((im[w+shiftx-1][z+shifty]+im[w+shiftx][z+shifty-1]));
	  });
	mare::after(task_im[i-1][j],task_im[i][j]);
	mare::after(task_im[i][j-1],task_im[i][j]);
	mare::launch(g,task_im[i][j]);
      }
    }

    
    mare::wait_for(g);
    // Promedio tras el filtro (test de salida)
    sum = 0.0;
    for(i=0; i < px; i++)
        for(j=0; j < py; j++)
            sum += im[i][j];
    
    promedio = sum /(px*py);
    printf("El promedio tras el filtro es %g\n", promedio);
    
    return 0; 
}


