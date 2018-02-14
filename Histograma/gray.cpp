/*
	***********************************************************+
	RODRIGO DÍAZ-HELLÍN VALERA
	CARLOS SOBRINO PÉREZ
	LABORATORIO  C2
	***********************************************************
*/
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>

#define COLOUR_DEPTH 4
#define NH 4

void inicializarHistograma (int h[])
{
	for (int i = 0; i< 256; i++)
	h [i] = 0;
}
double computeGraySequential(QImage *image, int vector[]) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
     
   for (int i = 0; i < image->byteCount(); i += COLOUR_DEPTH) {
    	QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + i);
    	int gray = qGray(*rgbpixel);
    	*rgbpixel = QColor(gray, gray, gray).rgba();
		vector[gray] ++; 
   }
  	
  return omp_get_wtime() - start_time;  
}

/*
	************************************************
			Código utilizando critical
	************************************************
*/

double computeGrayParallel(QImage *image, int v[]) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  
#pragma omp parallel for num_threads(NH)

  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {

    	QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    	int gray = qGray(*rgbpixel);
    	*rgbpixel = QColor(gray, gray, gray).rgba();
    	#pragma omp critical
    		v[gray] ++;
  	}
  	return omp_get_wtime() - start_time;  
	}

/*
	************************************************
			Código utilizando atomic
	************************************************
*/

/*
double computeGrayParallel(QImage *image, int v[]) {
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  
#pragma omp parallel for num_threads(NH)

  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {

    	QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    	int gray = qGray(*rgbpixel);
    	*rgbpixel = QColor(gray, gray, gray).rgba();
    	#pragma omp atomic
    		v[gray] ++;
  	}
  	return omp_get_wtime() - start_time;  
	}
*/


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QPixmap qp = QPixmap("test_1080p.bmp"); // ("c:\\test_1080p.bmp");
    if(qp.isNull())
    {
        printf("image not found\n");
	return -1;
    }
    
	int HS[256];
    int HP [256];
    inicializarHistograma(HS);
    inicializarHistograma(HP);
	QImage image = qp.toImage();
    double computeTime;

    computeTime = computeGraySequential(&image, HS);
    printf("Tiempo de ejecución secuencial: %0.9f segundos\n", computeTime);
    
    computeTime = computeGrayParallel(&image, HP);
    printf("Tiempo de ejecución paralelo: %0.9f segundos\n", computeTime);
	
	if(memcmp(HS,HP,256)==0)
		printf("Los histogramas son iguales\n");

    return 0;
}

/*
	****************************************************************************
									CONCLUSIÓN
	****************************************************************************
	Una vez realizadas las pruebas pertinentes podemos observar que el
	tiempo de ejecución en paralelo puede ser igual o mayor que el secuencial
	pero nunca puede ser menor. Esto se debe a que la ejecución en paralelo
	realiza las acciones de manera paralela pero se encuentra un problema
	al escribir en el vector. No pueden escribir varios hilos a la vez en 
	el mismo vector de ahí que el tiempo se incremente. 
*/