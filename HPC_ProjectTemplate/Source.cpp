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
#define chunck (imageWidth) * ((imageHeight / world_size) + 1)

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;

	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width +2;
	*h = BM.Height +2;
	
	input = new int[(BM.Height + 2) * (BM.Width + 2)];


	for (int i = 0; i <= BM.Height+1; i++)
	{

		for (int j = 0; j <= BM.Width+1; j++)
		{
			System::Drawing::Color c;
			if (i == BM.Height+1 || j == BM.Width+1 || i == 0 || j == 0)
			{
				c = System::Drawing::Color::FromArgb(0, 0, 0);
			}
			else
			{
				c = BM.GetPixel(j-1, i-1);

			}




			input[i * (BM.Width+2) + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}


	}
	return input;
}


void createImage(int* image, int width, int height, int index, int rank, int worldsize, int* imageData, int origheigh)
{
	/*width -= 1;
	height -= 1;*/

	System::Drawing::Bitmap MyNewImage(width, height);
	int* output = new int[(height) * (width)];

	int h = height;

	if (rank == 0 || rank == worldsize - 1)
	{
		h += 1;
	}

	else
	{
		h += 2;
	}

	
	for (int i = 1; i < h-1; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			if (i == h || j == width || i == 0 || j == 0)
			{
				output[i * width + j] = 0;
			}

			else
			{
				output[(i-1) * width + j] = 4 * image[(i)*width + (j)]
					- 1 * image[(i)*width + (j + 1)]
					- 1 * image[(i)*width + (j - 1)]
					- 1 * image[(i + 1) * width + (j)]
					- 1 * image[(i - 1) * width + (j)];
			}


			//output[i * width + j] = image[(i + 1) * width + (j + 1)];

			if (output[(i - 1) * width + (j )] < 0)
			{
				output[(i - 1) * width + (j )] = 0;
			}
			if (output[(i - 1) * width + (j )] > 255)
			{
				output[(i - 1) * width + (j )] = 255;
			}

			
		}
	}


	if (rank != 0)
	{
		MPI_Send(output, width * height, MPI_INT, 0, rank, MPI_COMM_WORLD);

		//delete output;
	}
		

	//MPI_Gather(output, width * height, MPI_INT, imageData, width * height, MPI_INT, 0, MPI_COMM_WORLD);



	//to be revised.
	

	if (rank == 0)
	{
		System::Drawing::Bitmap outputimage(width, origheigh);

		int size = width * height;
		for (int j = 0; j < size; j++)
		{
			imageData[j] = output[j];
		}


		MPI_Status status;
		//process the extra part
		int i;
		for ( i = 1; i < worldsize; i++)
		{
			MPI_Recv(&imageData[ i * width * height  ], height * width,MPI_INT, i, i, MPI_COMM_WORLD, &status);
		}


		/* 
		* if (origheigh % worldsize != 0)
		{

			image = &imageData[(11 * ((origheigh / worldsize) + origheigh % worldsize)) * MyNewImage.Width];

		
			output = new int[origheigh % worldsize * MyNewImage.Width];
			cout << "got in ";
			for (int i = 0; i < origheigh % worldsize; i++)
			{
				for (int j = 0; j < MyNewImage.Width; j++)
				{
					if (i == height || j == width || i == 0 || j == 0)
					{
						output[i * width + j] = 0;
					}

					else
					{
						output[i * width + j] = 4 * image[(i)*width + (j)]
							- 1 * image[(i)*width + (j + 1)]
							- 1 * image[(i)*width + (j - 1)]
							- 1 * image[(i + 1) * width + (j)]
							- 1 * image[(i - 1) * width + (j)];
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
		}
		*/

		//int lastIndex = outputimage.Height - origheigh % worldsize;
		for (int i = 0; i < outputimage.Height; i++)
		{
			for (int j = 0; j < MyNewImage.Width; j++)
			{


				if (imageData[i * width + j] < 0)
				{
					imageData[i * width + j] = 0;
				}
				if (imageData[i * width + j] > 255)
				{
					imageData[i * width + j] = 255;
				}


				System::Drawing::Color c = System::Drawing::Color::FromArgb(imageData[i * width + j], imageData[i * width + j], imageData[i * width + j]);
				outputimage.SetPixel(j, i, c);
			}
		}


		
		/*if (origheigh % worldsize != 0)
		{
			cout << "got in ";
			for (int i = 0; i < origheigh % worldsize; i++)
			{
				for (int j = 0; j < MyNewImage.Width; j++)
				{
					System::Drawing::Color c = System::Drawing::Color::FromArgb(output[i * width + j], output[i * width + j], output[i * width + j]);
					outputimage.SetPixel(j, lastIndex-1 +i, c);
				}
			}
		}*/


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
		std::string img, imageName;
		cin >> imageName;
		img = "..//Data//Input//" + imageName;
		imagePath = marshal_as<System::String^>(img);
		int* imageData = inputImage(&imageWidth, &imageHeight, imagePath);
		working = new int[chunck];

		for (int i = 1; i < world_size; i++)
		{
			MPI_Send(&imageHeight, 1, MPI_INT, i, 1000 + i, MPI_COMM_WORLD);
			MPI_Send(&imageWidth, 1, MPI_INT, i, 2000 + i, MPI_COMM_WORLD);
		}

		bool equalDivide = ((imageHeight) % world_size == 0);

		int size = chunck;
		for (int i = 0; i < size; i++)
		{
			working[i] = imageData[i+imageWidth+1];
			//cout << imageData[i] <<" ";
		}
			

		cout << "chunck rank 0 :" << size<<endl;
		

		
		
			int i;
			for ( i = 1; i < world_size-1; i++)
			{


				MPI_Send(&imageData[(i * (imageHeight / world_size)) * (imageWidth) - imageWidth], chunck+imageWidth, MPI_INT, i, i, MPI_COMM_WORLD);

			}

			MPI_Send(&imageData[(i * (imageHeight / world_size)) * (imageWidth)-imageWidth], chunck , MPI_INT, i, i, MPI_COMM_WORLD);


			createImage(working, imageWidth, (((imageHeight / world_size))), 0, rank, world_size, imageData, imageHeight);
		

		//else

		//{
		//	int i;
		//	for (i = 1; i < world_size; i++)
		//	{

		//		MPI_Send(&imageData[(i * ((imageHeight / world_size))) * imageWidth], chunck, MPI_INT, i, i, MPI_COMM_WORLD);


		//	}

		//	//work on extra in  the Master processor

		//	createImage(working, imageWidth, (imageHeight / world_size), 0, rank, world_size, imageData, imageHeight);

		//}
		//start_s = clock();
		/*stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << "time: " << TotalTime << endl;*/

		
		free(working);
	}
	if (rank != 0 )
	{
		MPI_Status status;
		MPI_Recv(&imageHeight, 1, MPI_INT, 0, rank + 1000, MPI_COMM_WORLD, &status);
		MPI_Recv(&imageWidth, 1, MPI_INT, 0, rank + 2000, MPI_COMM_WORLD, &status);

		if (rank == world_size - 1)
		{
			working = new int[chunck ];
			MPI_Recv(working, chunck , MPI_INT, 0, rank, MPI_COMM_WORLD, &status);

			cout << "chunck: " << chunck  << endl;

		}

		else
		{
			working = new int[chunck + imageWidth];
			MPI_Recv(working, chunck + imageWidth, MPI_INT, 0, rank, MPI_COMM_WORLD, &status);


			cout << "Extra: " << chunck + imageWidth << endl;
		}
		

		start_s = clock();
		createImage(working, imageWidth, (imageHeight / world_size), 0, rank, world_size, working, imageHeight);

		cout << "Height  " << (imageHeight / world_size) * imageWidth << endl;
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;


		free(working);
	}

	
	MPI_Finalize();
}

