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
#include <mare/mare.h>
#include <stdio.h>

using namespace std;
    

mare::task_ptr task_im[1000][1000] = { NULL };

//Compilación con gcc -o filtro filter.c -fopenmp -lm
int main(int argc, char **argv){
    
    int px=NPIXELX;
    int py=NPIXELY;
    int i,j;
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
    task_im[1][1] = mare::create_task([&im]{ im[1][1] += 0.25 * sqrt((im[0][1]+im[1][0]));});
    mare::launch(g,task_im[1][1]);
    for(i=2; i < px-1; i++){ 
      task_im[i][1] = mare::create_task([&im,i]{ im[i][1] += 0.25 * sqrt((im[i-1][1]+im[i][0]));});
      mare::after(task_im[i-1][1],task_im[i][1]);
      mare::launch(g,task_im[i][1]);
    }
    
    for(j=2; j < py-1; j++){
      task_im[1][j] = mare::create_task([&im,j]{ im[1][j] += 0.25 * sqrt((im[0][j]+im[1][j-1]));});
      mare::after(task_im[1][j-1],task_im[1][j]);
      mare::launch(g,task_im[1][j]);
    }

    for( i=2; i < px-1;i++)
      for(j=2; j < py-1; j++){
	task_im[i][j] = mare::create_task([&im,i,j]{  im[i][j] += 0.25 * sqrt((im[i-1][j]+im[i][j-1])); });
	mare::after(task_im[i-1][j],task_im[i][j]);
	mare::after(task_im[i][j-1],task_im[i][j]);
	mare::launch(g,task_im[i][j]);
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
