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

#define workPart (imageHeight / world_size)
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


void createImage(int* image, int width, int height, int index, int rank, int worldsize, int* imageData, int origheigh, int offset)
{

	System::Drawing::Bitmap outputimage(width, origheigh);
	MPI_Status status;
	int* output = new int[(height) * (width)];

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{

			output[i * width + j] = 4 * image[(i + 1) * width + (j + 1)] // self 
				- 1 * image[(i)*width + (j + 1)] //left
				- 1 * image[(i)*width + (j)]  //up
				- 1 * image[(i + 1) * width + (j)]  //down
				- 1 * image[(i)*width + (j)]; //up

			if (output[i * width + j] < 0)
			{
				output[i * width + j] = 0;
			}
			if (output[i * width + j] > 255)
			{
				output[i * width + j] = 255;
			}


		}
	}
	if (rank == 0)
	{
		
		if (worldsize != 1)
		{
			for (int i = 1; i < worldsize; i++)
			{
				MPI_Recv(&image[(offset)+(i * (origheigh / worldsize * width))], origheigh / worldsize * width, MPI_INT, i, i * 1000000, MPI_COMM_WORLD, &status);
			}
		}

		for (int i = 0; i < outputimage.Height; i++)
		{
			for (int j = 0; j < outputimage.Width; j++)
			{

				if (i < height)
				{
					image[i * width + j] = output[i * width + j];
				}

				System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i * width + j], image[i * width + j], image[i * width + j]);
				outputimage.SetPixel(j, i, c);
			}
			//cout << endl;
		}
		outputimage.Save("..//Data//Output//outputRes00" + index  + ".jpg");
		cout << "result Image Saved " << index << endl;

		
		

		
	}
	else
	{

		MPI_Send(output, origheigh / worldsize * width, MPI_INT, 0, 1000000 * rank, MPI_COMM_WORLD);

		free(output);
	}
}
int main()
{
	int rank, world_size;
	int times = 10;

	MPI_Init(NULL, NULL);


	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	while (times--)
	{


		int imageWidth = 4, imageHeight = 4;
		int start_s, stop_s, TotalTime = 0;
		int* working   = NULL;
		if (rank == 0)
		{

			System::String^ imagePath;
			std::string img;
			//cin >> img;
			img = "10N.png";

			start_s = clock();

			img = "..//Data//Input//" + img;
			imagePath = marshal_as<System::String^>(img);
			int* imageData = inputImage(&imageWidth, &imageHeight, imagePath);

			for (int i = 1; i < world_size; i++)
			{
				MPI_Send(&imageHeight, 1, MPI_INT, i, 1000 + i, MPI_COMM_WORLD);
				MPI_Send(&imageWidth, 1, MPI_INT, i, 2000 + i, MPI_COMM_WORLD);
			}
			if (world_size == 1)
			{

				createImage(imageData, imageWidth, imageHeight, times, rank, world_size, imageData, imageHeight, (imageHeight % world_size) * (imageWidth));
				stop_s = clock();
				TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
				cout << "time: " << TotalTime << endl;
				free(imageData);
				
				continue;
			}
			

			for (int i = 1; i < world_size; i++)
			{
				MPI_Send(&imageData[(i * (workPart)) * imageWidth],  (  (workPart + 1) * (imageWidth + 2) ), MPI_INT, i, i, MPI_COMM_WORLD);

			}
			

			createImage(imageData, imageWidth, imageHeight, times, rank, world_size, imageData, imageHeight, (imageHeight % world_size) * (imageWidth));
			stop_s = clock();
			TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
			cout << "time: " << TotalTime << endl;
			


			free(imageData);
		}
		if (rank != 0)
		{
			MPI_Status status;
			MPI_Recv(&imageHeight, 1, MPI_INT, 0, rank + 1000, MPI_COMM_WORLD, &status);
			MPI_Recv(&imageWidth, 1, MPI_INT, 0, rank + 2000, MPI_COMM_WORLD, &status);

			working = new int[(imageWidth + 2) * ((workPart) + 1)];

			MPI_Recv(working, ((2 + (workPart)) * (imageWidth + 2)), MPI_INT, 0, rank, MPI_COMM_WORLD, &status);
			start_s = clock();
			createImage(working, imageWidth, (workPart), times, rank, world_size, working, imageHeight, (imageHeight % world_size) * (imageWidth));

			stop_s = clock();
			TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;

			free(working);
		}


		
	
	}
	MPI_Finalize();

	return 0;
}
