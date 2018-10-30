// Lena.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <vector>
using namespace std;
#include "./gdal/gdal_priv.h"
#pragma comment(lib, "gdal_i.lib")

float kernel1[3][3] = {
	0, 0.2, 0,
	0.2, 0.2, 0.2,
	0, 0.2, 0 
};

float kernel2[5][5] = {
	0.2, 0, 0, 0, 0,
	0, 0.2, 0, 0, 0,
	0, 0, 0.2, 0, 0,
	0, 0, 0, 0.2, 0,
	0, 0, 0, 0, 0.2 
};

int kernel3[3][3] = {
	-1, -1, -1,
	-1, 8, -1,
	-1, -1, -1
};

int kernel4[3][3] = {
	-1, -1, -1,
	-1, 9, -1,
	-1, -1, -1
};

int kernel5[3][3] = {
	-1, -1, 0,
	-1, 0, -1,
	0, 1, 1
};

float kernel6[5][5] = {
	0.0120, 0.1253, 0.2736, 0.1253, 0.0120,
	0.1253, 1.3054, 2.8514, 1.3054, 0.1253,
	0.2736, 2.8514, 6.2279, 2.8514, 0.2736,
	0.1253, 1.3054, 2.8514, 1.3054, 0.1253,
	0.0120, 0.1253, 0.2736, 0.1253, 0.0120
};

GByte* buffTmp, **buff;

int cou[6] = { 1,2,1,1,1,2 };

int calc(int num, int row, int col,int buffnum,int tmpXlen) {
	int n = cou[num - 1];
	float sum = 0;

	for (int i = 0; i < 2*n+1; i++) {
		for (int j = 0; j < 2*n+1; j++) {
			sum += buff[buffnum][(row-n+i)*tmpXlen+(col-n+j)] * kernel3[i][j];
		}
	}
	return int(sum);
}

int main()
{
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			kernel6[i][j] /= 25;
		}
	}

	GDALDataset* poSrcDS;
	GDALDataset* poDstDS;
	int imgXlen, imgYlen;
	char* srcPath = "lena.jpg";
	char* dstPath = "lena.tif";
	int bandNum;

	GDALAllRegister();

	poSrcDS = (GDALDataset*)GDALOpenShared(srcPath, GA_ReadOnly);

	imgXlen = poSrcDS->GetRasterXSize();
	imgYlen = poSrcDS->GetRasterYSize();
	bandNum = poSrcDS->GetRasterCount();

	buffTmp = (GByte*)CPLMalloc(imgXlen * imgYlen * sizeof(GByte));

	poDstDS = GetGDALDriverManager()->GetDriverByName("GTiff")->Create(
		dstPath, imgXlen, imgYlen, bandNum, GDT_Byte, NULL
	);

	buff = (GByte**)CPLMalloc(bandNum*imgXlen*imgYlen * sizeof(GByte));
	for (int i = 0; i < bandNum; i++) {
		buff[i] = (GByte*)CPLMalloc(imgXlen*imgYlen * sizeof(GByte));
	}

	for (int i = 0; i < bandNum; i++) {
		poSrcDS->GetRasterBand(i+1)->RasterIO(
			GF_Read, 0, 0, imgXlen, imgYlen, buff[i], imgXlen, imgYlen, GDT_Byte, 0, 0
		);
	}

	for (int k = 0; k < bandNum; k++) {
		for (int j = 1; j < imgYlen-1 ; j++) {
			for (int i = 1; i < imgXlen-1; i++) {
				buff[k][j*imgXlen + i] = (GByte)calc(3, j, i,k,imgXlen);
			}
		}
	}

	for (int i = 0; i < bandNum; i++) {
		poDstDS->GetRasterBand(i + 1)->RasterIO(
			GF_Write, 0, 0, imgXlen, imgYlen, buff[i], imgXlen, imgYlen, GDT_Byte, 0, 0
		);
		printf("... ... band %d processing ... ...\n", i);
	}


	CPLFree(buffTmp);
	for (int i = 0; i < bandNum; i++) {
		CPLFree(buff[i]);
	}
	CPLFree(buff);
	GDALClose(poDstDS);
	GDALClose(poSrcDS);

	system("PAUSE");
	return 0;
}

