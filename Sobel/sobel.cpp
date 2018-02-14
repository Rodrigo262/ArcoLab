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

int weight[3][3] = {{ 1,  2,  1 },
		    { 0,  0,  0 },
		    { -1,  -2,  -1 }};

/*
	***********************************************************
 	Método secuencial original del programa.
	***********************************************************
*/

double computeSobelSecuencial(QImage *srcImage, QImage *dstImage) {
  double start_time = omp_get_wtime();
  int pixelValue;
  int ii, jj, blue;

  for (ii = 1; ii < srcImage->height() - 1; ii++) {  	// Recorremos la imagen, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
      pixelValue = 0;
      for (int j = -1; j <= 1; j++) {					// Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
	    blue = qBlue(srcImage->pixel(jj+i, ii+j));
		

            pixelValue += weight[j + 1][i + 1] * blue;	// En pixelValue se calcula el componente y del gradiente
          }
	
      }
	

      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
	
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());	// Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}

/*
	***********************************************************
 		Método Con uso de Localidad de datos.
	***********************************************************
*/

double computeSobelLocalidad(QImage *srcImage, QImage *dstImage) {
	double start_time = omp_get_wtime();
	int pixelValue;
	int ii, jj;
	int valoresRedun[2][3];

	for (ii = 1; ii < srcImage->height() - 1; ii++) { // Recorremos la imagen, excepto los bordes
		for (jj = 1; jj < srcImage->width() - 1; jj++) {
			
/*
			**********************************************************************
				Este if lo usamos para inicializar la matriz de valores
				redundantes, es decir, los que se repiten. Este proceso se repite
				cada vez que llega al extremo de la foto, es decir, la anchura.
			**********************************************************************
*/
			if(jj==1){
				valoresRedun[0][0] = qBlue(srcImage->pixel(jj-1, ii-1));
				valoresRedun[1][0] = qBlue(srcImage->pixel(jj, ii-1));
 				//1
				valoresRedun[0][1] = qBlue(srcImage->pixel(jj-1, ii));
				valoresRedun[1][1] = qBlue(srcImage->pixel(jj, ii));
				//2
				valoresRedun[0][2] = qBlue(srcImage->pixel(jj-1, ii+1));
				valoresRedun[1][2] = qBlue(srcImage->pixel(jj, ii+1));

			}
/*
			**********************************************************************
				Creamos un vector de tres posiciones. Calculamos los valores
				nuevos de las posiciones del vector que posteriormente vamos 
				a insertar en la matriz.

				Además en esta parte del código, realizamos de manera
				manual el intercambio de datos entre las mátrices.
			**********************************************************************
*/

			int pixelesCambiados[3];
			pixelValue = 0;
			pixelesCambiados[0]=qBlue(srcImage->pixel(jj+1, ii-1));//Valores nuevos
			pixelesCambiados[1]=qBlue(srcImage->pixel(jj+1, ii));//Valores nuevos
			pixelesCambiados[2]=qBlue(srcImage->pixel(jj+1, ii+1));//Valores nuevos

			pixelValue+=valoresRedun[0][0]*weight[0][0];
			pixelValue+=valoresRedun[1][0]*weight[0][1];
			pixelValue+=valoresRedun[0][1]*weight[1][0];
			pixelValue+=valoresRedun[1][1]*weight[1][1];
			pixelValue+=valoresRedun[0][2]*weight[2][0];
			pixelValue+=valoresRedun[1][2]*weight[2][1];
			pixelValue+=pixelesCambiados[0]*weight[0][2];
			pixelValue+=pixelesCambiados[1]*weight[1][2];
			pixelValue+=pixelesCambiados[2]*weight[2][2];

			int new_value = pixelValue;
			if (new_value > 255) new_value = 255;
			if (new_value < 0) new_value = 0;

			dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());
			
			valoresRedun[0][0]=valoresRedun[1][0];
			valoresRedun[0][1]=valoresRedun[1][1];
			valoresRedun[0][2]=valoresRedun[1][2];
			valoresRedun[1][0]=pixelesCambiados[0];
			valoresRedun[1][1]=pixelesCambiados[1];
			valoresRedun[1][2]=pixelesCambiados[2];

		}
		
	}
	return omp_get_wtime() - start_time; 
}

/*
	***********************************************************
 		Método Con Localidad de datos Paralelizado.
	***********************************************************
*/


double computeSobelLocalParalelizado(QImage *srcImage, QImage *dstImage) {
	double start_time = omp_get_wtime();
	int pixelValue;
	int ii, jj;
	int valoresRedun[2][3];

	/*
  	***********************************************************************
		Para paralelizar el código hemos utilizado la cláusula schedule
		(tal y cómo indica en las transparencias), dynamic ya que el static
		es menos óptimo.
	************************************************************************
  */
	
	#pragma omp parallel for schedule(dynamic)
	for (ii = 1; ii < srcImage->height() - 1; ii++) { // Recorremos la imagen, excepto los bordes
		for (jj = 1; jj < srcImage->width() - 1; jj++) {

/*
			**********************************************************************
				Este if lo usamos para inicializar la matriz de valores
				redundantes, es decir, los que se repiten. Este proceso se repite
				cada vez que llega al extremo de la foto, es decir, la anchura.
			**********************************************************************
*/
			if(jj==1){
				valoresRedun[0][0] = qBlue(srcImage->pixel(jj-1, ii-1));
				valoresRedun[1][0] = qBlue(srcImage->pixel(jj, ii-1));
 				//1
				valoresRedun[0][1] = qBlue(srcImage->pixel(jj-1, ii));
				valoresRedun[1][1] = qBlue(srcImage->pixel(jj, ii));
				//2
				valoresRedun[0][2] = qBlue(srcImage->pixel(jj-1, ii+1));
				valoresRedun[1][2] = qBlue(srcImage->pixel(jj, ii+1));

			}
/*
			**********************************************************************
				Creamos un vector de tres posiciones. Calculamos los valores
				nuevos de las posiciones del vector que posteriormente vamos 
				a insertar en la matriz.

				Además en esta parte del código, realizamos de manera
				manual el intercambio de datos entre las mátrices.
			**********************************************************************
*/

			int pixelesCambiados[3];
			pixelValue = 0;
			pixelesCambiados[0]=qBlue(srcImage->pixel(jj+1, ii-1));//Valores nuevos
			pixelesCambiados[1]=qBlue(srcImage->pixel(jj+1, ii));//Valores nuevos
			pixelesCambiados[2]=qBlue(srcImage->pixel(jj+1, ii+1));//Valores nuevos

			pixelValue+=valoresRedun[0][0]*weight[0][0];
			pixelValue+=valoresRedun[1][0]*weight[0][1];
			pixelValue+=valoresRedun[0][1]*weight[1][0];
			pixelValue+=valoresRedun[1][1]*weight[1][1];
			pixelValue+=valoresRedun[0][2]*weight[2][0];
			pixelValue+=valoresRedun[1][2]*weight[2][1];
			pixelValue+=pixelesCambiados[0]*weight[0][2];
			pixelValue+=pixelesCambiados[1]*weight[1][2];
			pixelValue+=pixelesCambiados[2]*weight[2][2];

			int new_value = pixelValue;
			if (new_value > 255) new_value = 255;
			if (new_value < 0) new_value = 0;

			dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());
			

			
			valoresRedun[0][0]=valoresRedun[1][0];
			valoresRedun[0][1]=valoresRedun[1][1];
			valoresRedun[0][2]=valoresRedun[1][2];
			valoresRedun[1][0]=pixelesCambiados[0];
			valoresRedun[1][1]=pixelesCambiados[1];
			valoresRedun[1][2]=pixelesCambiados[2];

		}
		
	}
	return omp_get_wtime() - start_time; 
}

/*
	***********************************************************
 		Método Secuencial paralelizado.
	***********************************************************
*/

double computeSobelParalelo(QImage *srcImage, QImage *dstImage) {
  double start_time = omp_get_wtime();
  int pixelValue;
  int ii, jj, blue;

  /*
  	***********************************************************************
		Para paralelizar el código hemos utilizado la cláusula schedule
		(tal y cómo indica en las transparencias), dynamic ya que el static
		es menos óptimo.
	************************************************************************
  */

  #pragma omp parallel for schedule(dynamic)
  for (ii = 1; ii < srcImage->height() - 1; ii++) {  	// Recorremos la imagen, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
      pixelValue = 0;
      for (int j = -1; j <= 1; j++) {					// Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
	    blue = qBlue(srcImage->pixel(jj+i, ii+j));
            pixelValue += weight[j + 1][i + 1] * blue;	// En pixelValue se calcula el componente y del gradiente
          }
      }

      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
	
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());	// Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QPixmap qp = QPixmap("test_1080p.bmp");
    if(qp.isNull())
    {	printf("image not found\n");
		return -1;
    }
    
    QImage image = qp.toImage();

	/*
		Creamos una imágen por cada método que vamos a utilizar.
	 */

    QImage sobelImageSecuencial(image);
    QImage sobelImageLocal(image);
    QImage sobelImageParalelizado(image);
    QImage sobelImageLocalParalelizado(image);

    /*
		Recibimos el tiempo que ha tardado en ejecutarse los diferentes métodos.
    */
    
    double computeTimeSecuencial = computeSobelSecuencial(&image, &sobelImageSecuencial);
    printf("Tiempo ejecución secuencial: %0.9f segundos\n", computeTimeSecuencial);

    double computeTimeLocal= computeSobelLocalidad(&image, &sobelImageLocal);
    printf("Tiempo ejecución localidad: %0.9f segundos\n", computeTimeLocal);

    double computeTimeParalelo= computeSobelParalelo(&image, &sobelImageParalelizado);
    printf("Tiempo ejecución paralelizado: %0.9f segundos\n", computeTimeParalelo);

    double computeTimeLocalParalelizado= computeSobelLocalParalelizado(&image, &sobelImageLocalParalelizado);
    printf("Tiempo ejecución localidad paralelizado: %0.9f segundos\n", computeTimeLocalParalelizado);    

    /*
    	Comprobamos que las imágenes resultantes de los cuatro 
    	métodos son iguales entre sí.
    */
    if(sobelImageSecuencial==sobelImageParalelizado && sobelImageSecuencial==sobelImageLocal && sobelImageSecuencial==sobelImageLocalParalelizado)
    	printf("Las imágenes resultantes de los métodos son iguales\n");
    else
    	printf("Alguna/s imágene/s resultante de los métodos es distinta\n");
    return 0;

    /*
	**********************************************************************************
						CONCLUSIÓN
	**********************************************************************************
	
	Tras realizar la práctica podemos observar que paralelizar mejora el rendimiento
	de los métodos pero si además se realiza la localidad de los datos se aumenta
	más el rendimiento.
	Por tanto podemos concluir que el mejor tiempo es la opción es la localidad
	de datos y además paralelizado.

	***********************************************************************************
    */
}
