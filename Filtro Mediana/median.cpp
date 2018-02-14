#include <QtGui/QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>

#define COLOUR_DEPTH 4

/*
  ***********************************************************
  RODRIGO DÍAZ-HELLÍN VALERA
  CARLOS SOBRINO PÉREZ
  LABORATORIO  C2
  ***********************************************************
*/

/*
  ***********************************************************
  Método secuencial original del programa.
  ***********************************************************
*/
double medianFilterSecuencial(QImage* image, QImage* result) {
  double start_time = omp_get_wtime();
  
  QRgb* pixelPtr = (QRgb*) image->bits();
  QRgb* resultPtr = (QRgb*) result->bits();
  
  
  for (int h = 1; h < image->height() - 1; h++)
    
    for (int w = 1; w < image->width() - 1; w++) {

      QRgb window[9];	// para guardar los 9 píxeles en torno al pixel de coordenadas (h,w)
	  int k = 0;
      for (int y = -1; y < 2; y++){	
		for (int x = -1; x < 2; x++){
			window[k++] = pixelPtr[(h + y) * image->width() + (w + x)];
			/* El pixel de coordenadas (a,b) ocupa en memoria la posición, 
			a*image->width()+b (relativa al inicio de la imagen), ya que antes de ese pixel hay:
			a filas de píxeles completas más b pixeles en la propia fila */
			
		}
	}
      
      //   Ordenar los 5 primeros elementos para obtener la mediana
      for (int j = 0; j < 5; ++j) {
		//   Encontrar el elemento que ocuparía la posición j en la lista ordenada
		int min = j;
		for (int l = j + 1; l < 9; ++l)
			if (window[l] < window[min]) min = l;
		//   Poner el elmento encontrado en la posición j de window
		QRgb temp = window[j];
		window[j] = window[min];
		window[min] = temp;
      }

      //   La mediana es window[4] (el 5.º de los 9 elementos)
      resultPtr[h * image->width() + w] = window[4];
    }
  
  return omp_get_wtime() - start_time;    
}

/*
  ***********************************************************
    Método Secuencial paralelizado.
  ***********************************************************
*/
double medianFilterParalelizado(QImage* image, QImage* result) {
  double start_time = omp_get_wtime();
  
  QRgb* pixelPtr = (QRgb*) image->bits();
  QRgb* resultPtr = (QRgb*) result->bits();
  
  #pragma omp parallel for schedule(dynamic, 4)  
  for (int h = 1; h < image->height() - 1; h++){
  	 
    for (int w = 1; w < image->width() - 1; w++) {

      QRgb window[9];	// para guardar los 9 píxeles en torno al pixel de coordenadas (h,w)
	  int k = 0;
      for (int y = -1; y < 2; y++){	
		for (int x = -1; x < 2; x++){
			window[k++] = pixelPtr[(h + y) * image->width() + (w + x)];
			/* El pixel de coordenadas (a,b) ocupa en memoria la posición, 
			a*image->width()+b (relativa al inicio de la imagen), ya que antes de ese pixel hay:
			a filas de píxeles completas más b pixeles en la propia fila */
			
		}
	}
      //   Ordenar los 5 primeros elementos para obtener la mediana
      for (int j = 0; j < 5; ++j) {
		//   Encontrar el elemento que ocuparía la posición j en la lista ordenada
		int min = j;
		for (int l = j + 1; l < 9; ++l)
			if (window[l] < window[min]) min = l;
		//   Poner el elmento encontrado en la posición j de window
		QRgb temp = window[j];
		window[j] = window[min];
		window[min] = temp;
		
      }

      //   La mediana es window[4] (el 5.º de los 9 elementos)
      resultPtr[h * image->width() + w] = window[4];
    }
  }
  return omp_get_wtime() - start_time;    
}

/*
  ***********************************************************
    Método Con uso de Localidad de datos.
  ***********************************************************
*/

double medianFilterLocal(QImage* image, QImage* result) {
  double start_time = omp_get_wtime();
  
  QRgb* pixelPtr = (QRgb*) image->bits();
  QRgb* resultPtr = (QRgb*) result->bits();
   
  for (int h = 1; h < image->height() - 1; h++){
    
    /********************************************************
    Declaramos la variable ValorPixel que es donde vamos a tener los valores
    temporales de los pixeles.

    Luego realizaremos un doble for donde en el inializamos los valores 
    iniciales de las esquinas que no son redundantes[0][0] [0][1] [1][0] [1][1]
    [2][0] [2][1].

    *********************************************************/
    QRgb ValorPixel[9]; // para guardar los 9 píxeles en torno al pixel de coordenadas (h,w)
    int k = 0;
      for (int x = 0; x < 2; x++)
        for (int y = -1; y < 2; y++) 
        ValorPixel[k++] = pixelPtr[(h + y) * image->width() + x];
    
    for (int w = 1; w < image->width() - 1; w++) {

      /****************************************************************
      Realizaremos la introducción de los valores que van cambiando en cada da iteracion del h=X, es decir, [0][2] [1][2]  [2][2] 
      y una vez introducidos estos valores ya estaria la array ValorPixel 

      *****************************************************************/
    ValorPixel[6]=pixelPtr[(h - 1)*image->width()+(w + 1)];
    ValorPixel[7]=pixelPtr[(h)*image->width()+(w + 1)];
    ValorPixel[8]=pixelPtr[(h + 1)*image->width()+(w + 1)];
      
    QRgb window[9];
    memcpy(window,ValorPixel,sizeof(QRgb)*9);
      /******************************************************************
      La funcion memcpy en esta parte del código realiza lo siguiente:
      Copia los valores de núm bytes desde la ubicación apuntada por la fuente directamente al bloque de memoria apuntado por el destino.
      es decir copiamos el valor de matriz Valor Pixel en la matriz windows
  
    *******************************************************************/
      //   Ordenar los 5 primeros elementos para obtener la mediana
    for (int j = 0; j < 5; ++j) {
    //   Encontrar el elemento que ocuparía la posición j en la lista ordenada
      int min = j;
      for (int l = j + 1; l < 9; ++l)
      
       if (window[l] < window[min]) 
        min = l;
    //   Poner el elemento encontrado en la posición j de window
    
      QRgb temp = window[j];
      window[j] = window[min];
      window[min] = temp;
      }

      /******************************************************************
      Ahora lo que hacemos es pasar los valores de pixeles de [0][1] [1][1] [2][1](que son de la variable ValorPixel el [3][4][5]) 
      a los valores [X][0](que son de la variable ValorPixel el [0][1][2]) y tambien;
      pasamos los valores de pixeles de [0][2] [1][2] [2][2](que son de la variable ValorPixel el [6][7][8]) 
      a los valores [X][1](que son de la variable ValorPixel el [3][4][5])
      Que son los valores que se van repitiendo

    *******************************************************************/
      ValorPixel[0]=ValorPixel[3];
      ValorPixel[1]=ValorPixel[4];
      ValorPixel[2]=ValorPixel[5];
      ValorPixel[3]=ValorPixel[6];
      ValorPixel[4]=ValorPixel[7];
      ValorPixel[5]=ValorPixel[8];
      //   La mediana es window[4] (el 5.º de los 9 elementos)
      resultPtr[h * image->width() + w] = window[4];
   }
  }
  return omp_get_wtime() - start_time; 
}

/*
  ***********************************************************
    Método Con uso de Localidad de datos Paralelizado
  ***********************************************************
*/
double medianFilterLocalParalelizado(QImage* image, QImage* result) {
 double start_time = omp_get_wtime();
  
  QRgb* pixelPtr = (QRgb*) image->bits();
  QRgb* resultPtr = (QRgb*) result->bits();
   
  #pragma omp parallel for schedule (dynamic, 4)
  for (int h = 1; h < image->height() - 1; h++){
    QRgb temp[9]; 
    int k = 0;
      for (int x = 0; x < 2; x++)
        for (int y = -1; y < 2; y++) 
        temp[k++] = pixelPtr[(h + y) * image->width() + x];
    
    for (int w = 1; w < image->width() - 1; w++) {
    temp[6]=pixelPtr[(h - 1)*image->width()+(w + 1)];
    temp[7]=pixelPtr[(h)*image->width()+(w + 1)];
    temp[8]=pixelPtr[(h + 1)*image->width()+(w + 1)];
      
    QRgb window[9];
    memcpy(window,temp,sizeof(QRgb)*9);
      
      
    for (int j = 0; j < 5; ++j) {
    
      int min = j;
      for (int l = j + 1; l < 9; ++l)
      
       if (window[l] < window[min]) 
        min = l;
    
      QRgb temp = window[j];
      window[j] = window[min];
      window[min] = temp;
      }

      temp[0]=temp[3];
      temp[1]=temp[4];
      temp[2]=temp[5];
      temp[3]=temp[6];
      temp[4]=temp[7];
      temp[5]=temp[8];

      resultPtr[h * image->width() + w] = window[4];
   }
  }
  return omp_get_wtime() - start_time;  
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QPixmap qp = QPixmap("test_ruido.bmp");
    if(qp.isNull())
    {
        printf("image not found\n");
	return -1;
    }
    
    /*
      Creamos una imágen por cada método que vamos a utilizar.
   */
    QImage image = qp.toImage();
    QImage resulsecuencial(image);
    QImage resulLocal(image);
    QImage resulParalelizado(image);
    QImage resulLocalParalelizado(image);

    /*
      Recibimos el tiempo que ha tardado en ejecutarse los diferentes métodos.
    */
    
    double computeTime = medianFilterSecuencial(&image, &resulsecuencial);
    printf("Tiempo ejecución Secuencial: %0.9f segundos\n", computeTime);

    double computeTime3 = medianFilterParalelizado(&image, &resulParalelizado);
    printf("Tiempo ejecución Paralelizado: %0.9f segundos\n", computeTime3);

    double computeTime2 = medianFilterLocal(&image, &resulLocal);
    printf("Tiempo ejecución Local: %0.9f segundos\n", computeTime2);

    double computeTime4 = medianFilterLocalParalelizado(&image, &resulLocalParalelizado);
    printf("Tiempo ejecución Local Paralelizado: %0.9f segundos\n", computeTime4);

     /*
      Comprobamos que las imágenes resultantes de los cuatro 
      métodos son iguales entre sí.
    */ 

    if(resulsecuencial==resulParalelizado && resulsecuencial==resulLocal && resulsecuencial==resulLocalParalelizado)
    	printf("Las imágenes resultantes de los métodos son iguales\n");
    else
    	printf("Alguna/s imágene/s resultante de los métodos es distinta\n");
	
     /*
    **********************************************************************************
              CONCLUSIÓN
    **********************************************************************************
    
    Tras realizar la práctica podemos observar que paralelizar mejora el rendimiento
    de los métodos pero si además se realiza la localidad de los datos se aumenta
    más el rendimiento.
    Por tanto podemos concluir que el mejor tiempo es la opción de la localidad
    de datos y además paralelizado.

    ***********************************************************************************
      */

    return 0;

}

