#include <opencv2/opencv.hpp> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//OpenCV 4.1.1
using namespace cv;

void changeClasses(int** segMat, int i, int j, int newClass,int N, int M);
int nearestMean(unsigned char*, int**, int);
int** randomMatrixGenerator(int);
int** calculateNewMeans(long*, long*, long*, long*, int);
int** createSegmentationMatrix(int**, int, int);
int calculateMistake(int**, int**, int);
Mat readImage();
Mat kMeans(Mat);
Mat changeImageColorsWithMeanValues(Mat, int**, int**);
Mat changeImageColorsWithSegMatValues(Mat, int**);

int** pixelClasses;
int segmentCount = 0;

int main() {

	Mat image;
	Mat imageKMeans;
	Mat imageSeg;
	int** segMat;
	srand(time(NULL));
	
	printf("IMAGE PROCESSING - HW1\n");
	printf("---------------------------------------------------\n");
	image = readImage();

	imageKMeans = kMeans(image);
	printf("K-MEANS COMPLETED!\n");
	namedWindow("After K Means", WINDOW_AUTOSIZE);
	imshow("After K Means", imageKMeans);
	imwrite("imageK.png", imageKMeans);

	segMat = createSegmentationMatrix(pixelClasses,image.rows,image.cols);
	imageSeg = changeImageColorsWithSegMatValues(image, segMat);
	printf("SEGMENTATION COMPLETED!\n");
	namedWindow("After Segmentation", WINDOW_AUTOSIZE);
	imshow("After Segmentation", imageSeg);
	imwrite("imageSeg.png", imageSeg);

	waitKey(0);
	return 0;
}

//Dosyadan görseli okur.
Mat readImage() {
	Mat image;
	char imageName[] = "image5.jpg";
	image = imread(imageName, IMREAD_COLOR);
	if (!image.data) {
		printf("Okuma Hatasi!\n ");
		exit(0);
	}
	else {
		printf("Image Name : %s\n", imageName);
		printf("IMAGE ROWS : %d\nIMAGE COLS : %d\n", image.rows, image.cols);
	}

	return image;
}

//Random renk matrisi oluþturur.
int** randomMatrixGenerator(int K) {
	
	int** mat;
	mat = (int**)malloc(K * sizeof(int*));
	for (int i = 0; i < K; i++) {
		mat[i] = (int *)malloc(3 * sizeof(int));
		for (int j = 0; j < 3 ; j++)
			mat[i][j] = rand() % 255;
	}

	return mat;
}

//Segmentation matrisini oluþturur.
int** createSegmentationMatrix(int** mat,int N,int M) {

	int** segMat = NULL;
	int myClass = 1;


	//Segmentation Matrisini oluþturma
	segMat = (int**)malloc(N * sizeof(int*));
	for (int i = 0; i < N; i++)
		segMat[i] = (int *)calloc(M, sizeof(int));

	//Tek tek matris elemanlarýný gezer.
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {

			//Her matris elemaný için çevresindeki deðerleri kontrol eder.
			for (int x = i - 1; x < i + 1; x++) {
				for (int y = j - 1; y < j + 2; y++)
					//Sýnýrlarýn içindeyse ve þu an kontrol ettiðimiz matris elemaný deðilse
					if (x >= 0 && y >= 0 && x < N && y < M && (x != i || y != j)) {
						// Ýkisi de ayný renk mi kontrolü
						if (mat[i][j] == mat[x][y]) {
							//Daha önce bir classý yoksa
							if (segMat[i][j] == 0 && segMat[x][y] != 0)
								segMat[i][j] = segMat[x][y];
							else {
								//Büyük olan küçük olana dönüþmeli
								if (segMat[i][j] > segMat[x][y] && segMat[x][y] != 0)
									changeClasses(segMat, i, j, segMat[x][y],N,M);
								else if (segMat[i][j] < segMat[x][y])
									changeClasses(segMat, x, y, segMat[i][j], N, M);
							}
						}
					}
			}
			//Ýþlemin sonunda çevresinde hiç benzeri eleman yoksa yeni class atanýr.
			if (segMat[i][j] == 0) {
				segMat[i][j] = myClass++;
			}
		}
	}
	printf("SEGMANTATION CLASS COUNT : %ld \n", myClass);
	segmentCount = myClass;
	return segMat;
}

//Görsele k means uygular.
Mat kMeans(Mat image) {

	int K; //Mean sayýsý.
	int mean;
	int** means; //Mean deðerleri tutulur.
	int** newMeans = NULL;
	int** pixelMeans; // Her pikselin sýnýfýný tutacak.
	long* sumMeanRed;
	long* sumMeanGreen;
	long* sumMeanBlue;
	long* count;
	unsigned char* p;

	printf("ENTER THE NUMBER K : ");
	scanf_s("%d", &K);
	means = randomMatrixGenerator(K);

	
	//Sýnýf matrisini oluþturur.
	pixelMeans = (int**)calloc(image.rows, sizeof(int*));
	for (int i = 0; i < image.rows; i++)
		pixelMeans[i] = (int *)calloc(image.cols, sizeof(int));


	int iter = 0;
	do {
		//Mean deðerlerinin sýfýrlanmasý
		sumMeanRed = (long*)calloc(K, sizeof(long));
		sumMeanGreen = (long*)calloc(K, sizeof(long));
		sumMeanBlue = (long*)calloc(K, sizeof(long));
		count = (long*)calloc(K, sizeof(long));

		if (newMeans != NULL)
			means = newMeans;

		//En yakýn Mean bulunmasý
		for (int i = 0; i < image.rows; i++)
			for (int j = 0; j < image.cols; j++) {
				mean = nearestMean(image.ptr(i, j), means, K);
				pixelMeans[i][j] = mean;

				p = p = image.ptr(i, j);
				sumMeanRed[mean] += p[0];
				sumMeanGreen[mean] += p[1];
				sumMeanBlue[mean] += p[2];
				count[mean]++;
			}
			
		//Ortalamalara göre yeni mean deðerlerinin hesaplanmasý
		newMeans = calculateNewMeans(sumMeanRed, sumMeanGreen, sumMeanBlue, count, K);
		iter++;

	} while	(calculateMistake(newMeans,means,K) > 25 && iter<15);

	image = changeImageColorsWithMeanValues(image, newMeans, pixelMeans);
	pixelClasses = pixelMeans;
	return image;

}

//Verilen renk deðerine en yakýn mean sýnýfý deðerini döndürür.
int nearestMean(unsigned char *color, int**means, int K) {
	int min = INT_MAX;
	int minMeanClass;
	int tmp;
	for (int i = 0; i < K; i++) {
		tmp = (int) sqrt(pow(means[i][0] - color[0], 2) + pow(means[i][1] - color[1], 2) + pow(means[i][2] - color[2], 2));
		if (tmp < min) {
			min = tmp;
			minMeanClass = i;
		}
	}

	return minMeanClass;
}

//Mean sýnýflarýný yeni elemanlarýnýn ortalamasý ile yeniden oluþturur.
int** calculateNewMeans(long* sumMeanRed, long*sumMeanGreen, long*sumMeanBlue, long* count,int K) {	
	
	int** newMeans;	
	newMeans = (int**)calloc(K, sizeof(int*));
	for(int i=0 ; i<K ; i++)
		newMeans[i] = (int*)calloc(K, sizeof(int));

	for (int i = 0; i < K; i++)
		if (count[i] > 0) {
			newMeans[i][0] = sumMeanRed[i] / count[i];
			newMeans[i][1] = sumMeanGreen[i] / count[i];
			newMeans[i][2] = sumMeanBlue[i] / count[i];
		}

	return newMeans;
}

//Mean deðerleri arasýndaki fark ile hata miktarýný hesaplar.
int calculateMistake(int** newMeans, int** means, int K) {

	int mistake = 0;
	for (int i = 0; i < K; i++)
		for (int j = 0; j < 3; j++)
			mistake += (int) fabs(newMeans[i][j] - means[i][j]);
	printf("MISTAKE : %d\n", mistake);
	return mistake;
}

//Mean sýnýf deðerlerine göre görseli boyar.
Mat changeImageColorsWithMeanValues(Mat image, int** newMeans, int**pixelMeans) {
	
	int mean,i,j;
	for (i = 0; i < image.rows; i++)
		for (j = 0; j < image.cols; j++) {
			mean = pixelMeans[i][j];
			image.ptr(i, j)[0] = newMeans[mean][0];
			image.ptr(i, j)[1] = newMeans[mean][1];
			image.ptr(i, j)[2] = newMeans[mean][2];
		}

	return image;
}

//Segmentation Matrisindeki deðerlere göre görseli boyar.
Mat changeImageColorsWithSegMatValues(Mat image, int** segMat) {

	int** colors = randomMatrixGenerator(segmentCount);
	
	int seg, i, j;
	for (i = 0; i < image.rows; i++)
		for (j = 0; j < image.cols; j++) {
			seg = segMat[i][j];
			image.ptr(i, j)[0] = colors[seg][0];
			image.ptr(i, j)[1] = colors[seg][1];
			image.ptr(i, j)[2] = colors[seg][2];
		}

	return image;
}

//seg[i][j] deðerindekileri newClass yapar.
void changeClasses(int** segMat, int i, int j, int newClass,int N,int M) {

	//printf("ASAMA i:%d , j:%d , nc : %d\n", i, j, newClass);
	int oldClass = segMat[i][j];
	segMat[i][j] = newClass;

	if (oldClass != newClass) {
		if (i - 1 >= 0 && j - 1 >= 0 && segMat[i - 1][j - 1] == oldClass)
			changeClasses(segMat, i - 1, j - 1, newClass, N, M);

		if (i - 1 >= 0 && segMat[i - 1][j] == oldClass)
			changeClasses(segMat, i - 1, j, newClass, N, M);

		if (i - 1 >= 0 && j + 1 < M && segMat[i - 1][j + 1] == oldClass)
			changeClasses(segMat, i - 1, j + 1, newClass, N, M);

		if (j - 1 >= 0 && segMat[i][j - 1] == oldClass)
			changeClasses(segMat, i, j - 1, newClass, N, M);

		if (j + 1 < M && segMat[i][j + 1] == oldClass)
			changeClasses(segMat, i, j + 1, newClass, N, M);

		if (i + 1 < N && j - 1 >= 0 && segMat[i + 1][j - 1] == oldClass)
			changeClasses(segMat, i + 1, j - 1, newClass, N, M);

		if (i + 1 < N && segMat[i + 1][j] == oldClass)
			changeClasses(segMat, i + 1, j, newClass, N, M);

		if (i + 1 < N && j + 1 < M && segMat[i + 1][j + 1] == oldClass)
			changeClasses(segMat, i + 1, j + 1, newClass, N, M);
	}
}
