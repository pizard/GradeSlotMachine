#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include "Model_OBJ.h"
#define POINTS_PER_VERTEX 3
#define TOTAL_FLOATS_IN_TRIANGLE 9
#define VERTEX_PER_TRIANGLE 3
using namespace std;

Model_OBJ::Model_OBJ()
{
	this->TotalConnectedTriangles = 0;
	this->TotalConnectedPoints = 0;
	this->TotalConnectedVertexNormals = 0;
}

Model_OBJ::~Model_OBJ()
{
}

float* Model_OBJ::calculateNormal(float * coord1, float * coord2, float * coord3)
{
	/* calculate Vector1 and Vector2 */
	float va[3], vb[3], vr[3], val;
	va[0] = coord1[0] - coord2[0];
	va[1] = coord1[1] - coord2[1];
	va[2] = coord1[2] - coord2[2];

	vb[0] = coord1[0] - coord3[0];
	vb[1] = coord1[1] - coord3[1];
	vb[2] = coord1[2] - coord3[2];

	/* cross product */
	vr[0] = va[1] * vb[2] - vb[1] * va[2];
	vr[1] = vb[0] * va[2] - va[0] * vb[2];
	vr[2] = va[0] * vb[1] - vb[0] * va[1];

	/* normalization factor */
	val = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);

	float norm[3];
	norm[0] = vr[0] / val;
	norm[1] = vr[1] / val;
	norm[2] = vr[2] / val;


	return norm;
}

int Model_OBJ::Load(char * filename)
{
	string line;
	ifstream objFile(filename);
	if (objFile.is_open())                                       // If obj file is open, continue
	{
		objFile.seekg(0, ios::end);                              // Go to end of the file, 
		long fileSize = objFile.tellg();                           // get file size
		objFile.seekg(0, ios::beg);                              // we'll use this to register memory for our 3d model

		vertexBuffer = (float*)malloc(fileSize);                     // Allocate memory for the verteces
		normalBuffer = (float*)malloc(fileSize);
		Faces_Triangles = (float*)malloc(fileSize * sizeof(float));         // Allocate memory for the triangles
		Faces_Triangles_vertex_normal = (float*)malloc(fileSize * sizeof(float));
		normals = (float*)malloc(fileSize * sizeof(float));               // Allocate memory for the normals

		int triangle_index = 0;                                    // Set triangle index to zero
		int normal_index = 0;                                    // Set normal index to zero

		while (!objFile.eof())                                 // Start reading file data
		{
			getline(objFile, line);                                 // Get line from file

			if (line.c_str()[0] == 'v'&&line.c_str()[1] != 'n')                              // The first character is a v: on this line is a vertex stored.
			{
				line[0] = ' ';                                    // Set first character to 0. This will allow us to use sscanf

				sscanf(line.c_str(), "%f %f %f ",                     // Read floats from the line: v X Y Z
					&vertexBuffer[TotalConnectedPoints],
					&vertexBuffer[TotalConnectedPoints + 1],
					&vertexBuffer[TotalConnectedPoints + 2]);

				TotalConnectedPoints += POINTS_PER_VERTEX;               // Add 3 to the total connected points
			}
			if (line.c_str()[0] == 'v'&&line.c_str()[1] == 'n')
			{
				line[0] = ' '; line[1] = ' ';
				sscanf(line.c_str(), "%f %f %f",
					&normalBuffer[TotalConnectedVertexNormals],
					&normalBuffer[TotalConnectedVertexNormals + 1],
					&normalBuffer[TotalConnectedVertexNormals + 2]);

				TotalConnectedVertexNormals += POINTS_PER_VERTEX;
			}
			if(line.substr(0, 6) == "mtllib")
			{
				this->MtlFileName = line.substr(7);
			}
			if (line.substr(0, 6) == "usemtl")
			{
				string mat_name = line.substr(7);
				ifstream mtlFile(MtlFileName);
				if(mtlFile.is_open())
				{
					string mtlLine;
					mtlFile.seekg(0, ios::end);                              // Go to end of the file, 
					long mtlFileSize = mtlFile.tellg();                           // get file size
					mtlFile.seekg(0, ios::beg);									// we'll use this to register memory for our 3d model
					while (!mtlFile.eof())
					{
						getline(mtlFile, mtlLine);
						if (mtlLine.substr(0, 6) == "newmtl" && mtlLine.substr(7)==mat_name)
						{
							while (!mtlFile.eof())
							{
								getline(mtlFile, mtlLine);
								if (mtlLine.size() == 0)break;
								if (mtlLine.substr(0, 6) == "newmtl"&& mtlLine.substr(7) != mat_name)break;
								//material �Ӽ� �ο� 
							}
						}
					}
					mtlFile.close();
				}
				else
				{
					cout << "Unable to open mtl file";
				}
				
			}
			if (line.c_str()[0] == 'f')                              // The first character is an 'f': on this line is a point stored
			{
				line[0] = ' ';                                    // Set first character to 0. This will allow us to use sscanf
				
				int vertexNumber[4] = { 0, 0, 0 };
				int vertexNormalNumber[4] = { 0,0,0 };
				sscanf(line.c_str(), "%i//%i %i//%i %i//%i",                       // Read integers from the line:  f 1 2 3
					&vertexNumber[0],								// First point of our triangle. This is an 
					&vertexNormalNumber[0],                              // pointer to our vertexBuffer list
					&vertexNumber[1],
					&vertexNormalNumber[1],
					&vertexNumber[2],
					&vertexNormalNumber[2]);                              // each point represents an X,Y,Z.

				vertexNumber[0] -= 1;                              // OBJ file starts counting from 1
				vertexNumber[1] -= 1;                              // OBJ file starts counting from 1
				vertexNumber[2] -= 1;                              // OBJ file starts counting from 1
				vertexNormalNumber[0] -= 1;                              // OBJ file starts counting from 1
				vertexNormalNumber[1] -= 1;                              // OBJ file starts counting from 1
				vertexNormalNumber[2] -= 1;                              // OBJ file starts counting from 1


																   /********************************************************************
																   * Create triangles (f 1 2 3) from points: (v X Y Z) (v X Y Z) (v X Y Z).
																   * The vertexBuffer contains all verteces
																   * The triangles will be created using the verteces we read previously
																   */

				int tCounter = 0;
				for (int i = 0; i < VERTEX_PER_TRIANGLE; i++)
				{
					Faces_Triangles[triangle_index + tCounter] = vertexBuffer[3 * vertexNumber[i]];
					Faces_Triangles[triangle_index + tCounter + 1] = vertexBuffer[3 * vertexNumber[i] + 1];
					Faces_Triangles[triangle_index + tCounter + 2] = vertexBuffer[3 * vertexNumber[i] + 2];

					Faces_Triangles_vertex_normal[triangle_index + tCounter] = normalBuffer[3 * vertexNormalNumber[i]];
					Faces_Triangles_vertex_normal[triangle_index + tCounter + 1] = normalBuffer[3 * vertexNormalNumber[i] + 1];
					Faces_Triangles_vertex_normal[triangle_index + tCounter + 2] = normalBuffer[3 * vertexNormalNumber[i] + 2];
					tCounter += POINTS_PER_VERTEX;
				}
				/*********************************************************************
				* Calculate all normals, used for lighting
				*/
				float coord1[3] = { Faces_Triangles[triangle_index], Faces_Triangles[triangle_index + 1],Faces_Triangles[triangle_index + 2] };
				float coord2[3] = { Faces_Triangles[triangle_index + 3],Faces_Triangles[triangle_index + 4],Faces_Triangles[triangle_index + 5] };
				float coord3[3] = { Faces_Triangles[triangle_index + 6],Faces_Triangles[triangle_index + 7],Faces_Triangles[triangle_index + 8] };
				float *norm = this->calculateNormal(coord1, coord2, coord3);

				tCounter = 0;
				for (int i = 0; i < VERTEX_PER_TRIANGLE; i++)
				{
					normals[normal_index + tCounter] = norm[0];
					normals[normal_index + tCounter + 1] = norm[1];
					normals[normal_index + tCounter + 2] = norm[2];
					tCounter += POINTS_PER_VERTEX;
				}

				triangle_index += TOTAL_FLOATS_IN_TRIANGLE;
				normal_index += TOTAL_FLOATS_IN_TRIANGLE;
				TotalConnectedTriangles += TOTAL_FLOATS_IN_TRIANGLE;
			}
		}
		objFile.close();                                          // Close OBJ file
	}
	else
	{
		cout << "Unable to open obj file";
	}
	return 0;
}

void Model_OBJ::Draw()
{
	/*glEnableClientState(GL_VERTEX_ARRAY);                  // Enable vertex arrays
	glEnableClientState(GL_NORMAL_ARRAY);                  // Enable normal arrays
	glVertexPointer(3, GL_FLOAT, 0, Faces_Triangles);            // Vertex Pointer to triangle array
	glNormalPointer(GL_FLOAT, 0, normals);                  // Normal pointer to normal array
	glDrawArrays(GL_TRIANGLES, 0, TotalConnectedTriangles);      // Draw the triangles
	glDisableClientState(GL_VERTEX_ARRAY);                  // Disable vertex arrays
	glDisableClientState(GL_NORMAL_ARRAY);                 // Disable normal arrays
	*/
	glBegin(GL_TRIANGLES);
		for (int i = 0; i < TotalConnectedTriangles; i+=3) {
			//if (i % 3 == 0)glNormal3f(normals[i], normals[i + 1], normals[i + 2]);
			glNormal3f(Faces_Triangles_vertex_normal[i], Faces_Triangles_vertex_normal[i + 1], Faces_Triangles_vertex_normal[i + 2]);
			glVertex3f(Faces_Triangles[i], Faces_Triangles[i + 1], Faces_Triangles[i + 2]);
			}
	glEnd();
}

void Model_OBJ::Release()
{
	free(this->Faces_Triangles);
	free(this->normals);
	free(this->vertexBuffer);
}