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




			input[i * BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}


	}
	return input;
}


void createImage(int* image, int width, int height, int index, int rank, int worldsize, int* imageData, int origheigh)
{
	System::Drawing::Bitmap MyNewImage(width, height);
	System::Drawing::Bitmap outputimage(width, origheigh);
	int* output = new int[(height) * (width)];

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			if (i == height || j == width || i == 0 || j == 0)
			{
				output[i * width + j] = 0;
			}

			else
			{
				(output[i * width + j] = 4 * image[(i)*width + (j)]
					- 1 * image[(i)*width + (j + 1)]
					- 1 * image[(i)*width + (j - 1)]
					- 1 * image[(i + 1) * width + (j)]
					- 1 * image[(i - 1) * width + (j)]);
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
	MPI_Gather(output, width * height - 1, MPI_INT, imageData, width * height, MPI_INT, 0, MPI_COMM_WORLD);
	if (rank == 0)
	{
		for (int i = 0; i < outputimage.Height; i++)
		{
			for (int j = 0; j < MyNewImage.Width; j++)
			{
				System::Drawing::Color c = System::Drawing::Color::FromArgb(imageData[i * width + j], imageData[i * width + j], imageData[i * width + j]);
				outputimage.SetPixel(j, i, c);
			}
		}

		outputimage.Save("..//Data//Output//outputRes" + index + ".png");
		cout << "result Image Saved " << index << endl;
	}
}
int main()
{
	int rank = 0, world_size;

	MPI_Init(NULL, NULL);


	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int imageWidth = 4, imageHeight = 4;
	int start_s, stop_s, TotalTime = 0;
	int* working;
	if (rank == 0)
	{
		System::String^ imagePath;
		std::string img;
		img = "..//Data//Input//(328).jpg";
		imagePath = marshal_as<System::String^>(img);
		int* imageData = inputImage(&imageWidth, &imageHeight, imagePath);
		working = new int[(imageWidth) * ((imageHeight / world_size) + 1)];

		for (int i = 1; i < world_size; i++)
		{
			MPI_Send(&imageHeight, 1, MPI_INT, i, 1000 + i, MPI_COMM_WORLD);
			MPI_Send(&imageWidth, 1, MPI_INT, i, 2000 + i, MPI_COMM_WORLD);
		}
		for (int i = 0; i < (imageWidth) * ((imageHeight / world_size) + 1); i++)
			working[i] = imageData[i];
		for (int i = 1; i < world_size; i++)
		{
			if ((imageHeight) % world_size == 0)
			{
				MPI_Send(&imageData[(i * (imageHeight / world_size)) * imageWidth], (((imageHeight / world_size) + 1) * imageWidth), MPI_INT, i, i, MPI_COMM_WORLD);
			}
		}
		start_s = clock();
		createImage(working, imageWidth, (((imageHeight / world_size))), 0, rank, world_size, imageData, imageHeight);
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << "time: " << TotalTime << endl;
		free(working);
	}
	if (rank != 0)
	{
		MPI_Status status;
		MPI_Recv(&imageHeight, 1, MPI_INT, 0, rank + 1000, MPI_COMM_WORLD, &status);
		MPI_Recv(&imageWidth, 1, MPI_INT, 0, rank + 2000, MPI_COMM_WORLD, &status);
		working = new int[(imageWidth) * ((imageHeight / world_size) + 1)];
		MPI_Recv(working, (((imageHeight / world_size) + 1) * imageWidth), MPI_INT, 0, rank, MPI_COMM_WORLD, &status);
		start_s = clock();
		createImage(working, imageWidth, (imageHeight / world_size), 0, rank, world_size, working, imageHeight);

		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;

		free(working);
	}
	MPI_Finalize();
}

