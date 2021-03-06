#include <mpi.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 

#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int* Red = new int[BM.Height * BM.Width];
	int* Green = new int[BM.Height * BM.Width];
	int* Blue = new int[BM.Height * BM.Width];
	input = new int[(BM.Height + 1) * (BM.Width + 1)];


	for (int i = 0; i <= BM.Height; i++)
	{

		for (int j = 0; j <= BM.Width; j++)
		{
			System::Drawing::Color c;
			if (i == BM.Height || j == BM.Width || i == 0 || j == 0)
			{
				c = System::Drawing::Color::FromArgb(0, 0, 0);
			}
			else
			{
				c = BM.GetPixel(j, i);

			}


			/*Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;*/

			input[i * BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values
			//input[i * BM.Width + j] = ((c.R + c.B + c.G) ); //gray scale value equals the average of RGB values


			//cout << input[i * BM.Width + j] << " ";
		}

		//cout << endl;
	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width , height );

	//System::Drawing::Bitmap temp(width+1, height+1);

	int* output = new int[(height ) * (width )];

	int kernal[3][3] = { {0,-1,0},
						{-1,4,-1},
						{0,-1,0} };

	/*width -= 1;
	height -= 1;*/
	float result = 0;

	for (int i = 0; i < MyNewImage.Height; i++)
	{

		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j

			//float kernelResult = 0; 
			if (i == height || j == width || i == 0 || j == 0)
			{
				output[i * width + j] = 0;
			}

			else
			{
				(output[i  * width + j ] =   4 * image[(i)*width + (j )]
				 -1 * image[(i ) * width + (j + 1)]
				-1 * image[(i ) * width + (j-1)]
				-1* image[(i + 1) * width + (j )]
				-1 * image[(i -1) * width + (j )] ) ;
			}
			

			//output[i * width + j] = image[(i + 1) * width + (j + 1)];

			if (output[i * width + j] < 0)
			{
				output[i * width + j] = 0;
			}
			if (output[i * width + j] > 255)
			{
				output[i * width + j] = 255;
			}


			System::Drawing::Color c = System::Drawing::Color::FromArgb(output[i * width + j], output[i * width + j], output[i * width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//outputRes" + index + ".png");
	cout << "result Image Saved " << index << endl;
}


int main()
{
	int rank = 0, world_size;

	/*MPI_Init(NULL, NULL);


	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);*/




	int ImageWidth = 4, ImageHeight = 4;

	int start_s, stop_s, TotalTime = 0;

	System::String^ imagePath;
	std::string img;
	img = "..//Data//Input//f.jpg";

	imagePath = marshal_as<System::String^>(img);
	int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);


	start_s = clock();
	createImage(imageData, ImageWidth, ImageHeight, 0);
	stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	cout << "time: " << TotalTime << endl;

	system("pause");


	free(imageData);



	return 0;

}



