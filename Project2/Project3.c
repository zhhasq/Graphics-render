#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#pragma warning(disable : 4996)  
#define PI 3.14159265

GLint window[2];
char *f1 = "bunny.obj";
char *f2 = "cactus.obj";
int renderMode = 0; //default render mode which is GL_TRIANGLE
int frontFace = 0;  // 0 for CCW and 1 for CW
int nv = 0;
int nf = 0;

float xmin = -0.1;
float xmax = 0.1;
float ymin = -0.1;
float ymax = 0.1;
float zmin = -0.1;
float zmax = 0.1;
float restorzmin;
float restorzmax;

float color[3] = { 1, 1, 1 };

float camu[3] = { 1,0,0 };  // camera axis 
float camv[3] = { 0,1,0 };
float camn[3] = { 0,0,1 };
float R[3][3];
float view[16] = { 1,0,0,0,
0,1,0,0,
0,0,1,0,
0,0,0,1 };
float proj[16] = { 0,0,0,0,
0,0,0,0,
0,0,0,0,
0,0,0,0 };
float newcv[3]; //camera position
float oneFaceNormal[4];

GLfloat *verB;
GLfloat *verC;
GLfloat *norB;
GLfloat *norC;
GLint *faceB;
GLint *faceC;

GLuint vsID, fsID, pID, VBO[2], VAO[2], EBO[2],NOR[2];
GLfloat * verVBO;
GLint *faceEBO;
GLfloat * normalForPipe;
void initBufferObject(void);
int isLightOn;
int globalLightMode;
int localLightMode;
int rgb;
int smooth;
int diffuse;
GLfloat globalAmbientLight[4];
GLfloat localAmbientLight[4];
GLfloat diffuseLight[4];
GLfloat specularLight[4];
GLfloat lightPosition[4];
int readForShader(char *s, float *vert, int *face) {
	FILE *fp = fopen(s, "r");
	char t[10];
	int line = 1;
	char buf[60];
	char *bufp = buf;
	int num[2];

	if (fp != NULL) {
		while (fgets(buf, 50, fp)) {
			if (buf[0] == 'v') {
				//       printf("line %d ", line);
				sscanf(bufp, "%s %f %f %f\n", t, &vert[3 * nv + 0], &vert[3 * nv + 1], &vert[3 * nv + 2]);
				nv++;

			}

			else if (buf[0] == 'f') {

				sscanf(bufp, "%s %d %d %d\n", t, &face[3 * nf + 0], &face[3 * nf + 1], &face[3 * nf + 2]);
				face[3 * nf + 0] = face[3 * nf + 0] - 1;
				face[3 * nf + 1] = face[3 * nf + 1] - 1;
				face[3 * nf + 2] = face[3 * nf + 2] - 1;
				nf++;

			}
			line++;
		}
	}
	fclose(fp);
}
int readdata(char *s) {
	FILE *fp = fopen(s, "r");

	char t[10];
	int line = 1;
	char buf[60];
	char *bufp = buf;
	int num[2];

	faceEBO = (int *)malloc(300000 * sizeof(int));
	if (!faceEBO) {
		printf("error to allocate space for faces\n");
		exit(1);
	}
	else if (faceEBO) {
		printf("success allocate space for faces, the address starts at %p\n", faceEBO);
	}
	verVBO = (GLfloat *)malloc(120000 * sizeof(float));
	if (!verVBO ) {
		printf("error to allocate space for vertices\n");
		exit(1);
	}
	else if (verVBO) {
		printf("success allocate space for vertices, the address starts at %p\n", verVBO);
	}
	normalForPipe = (GLfloat *)malloc(120000 * sizeof(float));
	if (!normalForPipe) {
		printf("error to allocate space for normal\n");
		exit(1);
	}
	else if (normalForPipe) {
		printf("success allocate space for normal, the address starts at %p\n", normalForPipe);
	}

	if (faceEBO == NULL)
		printf("malloc error for faceEBO");
	if (verVBO == NULL)
		printf("malloc error for verVBO");
	if (fp != NULL) {
		while (fgets(buf, 50, fp)) {
			if (buf[0] == 'v') {
				//       printf("line %d ", line);
				sscanf(bufp, "%s %f %f %f\n", t, &verVBO[3 * nv + 0], &verVBO[3 * nv + 1], &verVBO[3 * nv + 2]);

				/*if (i == nv - 1)
				printf("vert%d is %f %f %f\n", i, verVBO[3 * i + 0], verVBO[3 * i + 1], verVBO[3 * i + 2]);
				//printf("facep%d is %f %f %f\n", i, facep[i].f[0], facep[i].f[1], facep[i].f[2]);
				*/

				// printf("ver is %s, %f, %f, %f\n",t, ver[nv].v[0], ver[nv].v[1], ver[nv].v[2]);
				/**********************adjust camera projection boundary************************/
				if (verVBO[3 * nv + 0] < xmin) {
					xmin = 2 * verVBO[3 * nv + 0];
				}
				if (verVBO[3 * nv + 0] > xmax) {
					xmax = 2 * verVBO[3 * nv + 0];
				}
				if (verVBO[3 * nv + 1] < ymin) {
					ymin = 2 * verVBO[3 * nv + 1];
				}
				if (verVBO[3 * nv + 1] > xmax) {
					ymax = 2 * verVBO[3 * nv + 1];
				}
				if (verVBO[3 * nv + 2] < zmin) {
					zmin = 3 * verVBO[3 * nv + 2];
					restorzmin = zmin;
				}
				if (verVBO[3 * nv + 2]> zmax) {
					zmax = 3 * verVBO[3 * nv + 2];
					restorzmax = zmax;
				}
				nv++;
			}
			else if (buf[0] == 'f') {
				//	printf("line %d ", line);
				//sscanf(bufp, "%s %d %d %d\n", t, &facep[nf].f[0], &facep[nf].f[1], &facep[nf].f[2]);
				sscanf(bufp, "%s %d %d %d\n", t, &faceEBO[3 * nf + 0], &faceEBO[3 * nf + 1], &faceEBO[3 * nf + 2]);
				faceEBO[3 * nf + 0] = faceEBO[3 * nf + 0] - 1;
				faceEBO[3 * nf + 1] = faceEBO[3 * nf + 1] - 1;
				faceEBO[3 * nf + 2] = faceEBO[3 * nf + 2] - 1;

				//printf("faceEBO%d is %d %d %d\n", nf, faceEBO[3 * nf + 0], faceEBO[3 * nf + 1], faceEBO[3 * nf + 2]);

				//printf("face is %s, %d, %d, %d\n",t, facep[nf].f[0], facep[nf].f[1], facep[nf].f[2]);
				nf++;
			}
			line++;
		}
	}
	printf("total %d vertices, %d faces\n", nv, nf);

	fclose(fp);
}

void calculateOneFaceNormal(GLfloat *ver, GLint *face, int faceIndex) {

	//get all 3 points
	int fv1 = face[3 * faceIndex + 0];
	int fv2 = face[3 * faceIndex + 1];
	int fv3 = face[3 * faceIndex + 2];

	float v1[3] = { ver[3 * fv1 + 0], ver[3 * fv1 + 1], ver[3 * fv1 + 2] };
	float v2[3] = { ver[3 * fv2 + 0], ver[3 * fv2 + 1], ver[3 * fv2 + 2] };
	float v3[3] = { ver[3 * fv3 + 0], ver[3 * fv3 + 1], ver[3 * fv3 + 2] };

	float a[3] = { v2[0] - v1[0],v2[1] - v1[1],v2[2] - v1[2] };
	float b[3] = { v3[0] - v1[0],v3[1] - v1[1],v3[2] - v1[2] };
	//aybz-azby, azbx-axbz, axby-aybx
	//x--0 y--1 z--2
	//calculate normal
	float n[3] = { a[1] * b[2] - a[2] * b[1],
				   a[2] * b[0] - a[0] * b[2],
				   a[0] * b[1] - a[1] * b[0] };
	//calculate the magnitude of n which is also the 2*area of triangle
	oneFaceNormal[3] =  sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
	//normalize the normal 
	oneFaceNormal[0] =  n[0]/ oneFaceNormal[3];
	oneFaceNormal[1] =  n[1]/oneFaceNormal[3];
	oneFaceNormal[2] =  n[2]/oneFaceNormal[3];
	//printf("normal is %f,%f,%f,%f\n", oneFaceNormal[0], oneFaceNormal[1], oneFaceNormal[2], oneFaceNormal[3]);

}

void calculateNormal(GLfloat *ver, GLint *face, GLfloat *nor) {
	int *adjFaces;
	int adjFacesIndex = 0;
	float accNormal[3];
	float accArea;
	adjFaces = (int *)malloc(100 * sizeof(int));
	if (!adjFaces) {
		printf("error to allocate space for normal\n");
		exit(1);
	}
	else if (adjFaces) {
		printf("success allocate space for adjFaces, the address starts at %p\n", adjFaces);
	}
	//printf("faceEBO%d is %d %d %d\n", 0, face[0], face[1], face[2]);
	//printf("vertex%d is %f %f %f\n", 0, ver[0], ver[1], ver[2]);

	//printf("There are total %d vertices\n",nv);
	//find a normal for each vertex
	for (int nver = 0; nver < nv; nver++) {
		accNormal[0] = 0; accNormal[1] = 0; accNormal[2] = 0;
		accArea = 0;
		adjFacesIndex = 0;

		//1. find all the faces that adjacent to a vertex and put it into adjFaces[]
		for (int f = 0; f < nf; f++) {
			if (face[3 * f + 0] == nver || face[3 * f + 1] == nver || face[3 * f + 2] == nver) {
				adjFaces[adjFacesIndex] = f;
				adjFacesIndex++;
				//printf("faceEBO%d is %d %d %d\n", f, faceEBO[3 * f + 0], faceEBO[3 * f + 1], faceEBO[3 * f + 2]);
			}		
		}
		
		//printf("for ver %d, there are %d faces adjacent to it\n", nver, adjFacesIndex);

		for (int i=0; i< adjFacesIndex; i++) {
			//printf("The %d adjacent face is %d\n", i, adjFaces[i]);
			//the oneFaceNormal[4] is area of that triangle
			//function calculateOneFaceNormal will return normalized normal of each face
		    calculateOneFaceNormal(ver, face, adjFaces[i]);
			//the normal need to times weight which is the area of triangle 1/2|n|
			//to be simple just times the area of Parallelogram which is |n|
			accNormal[0] = accNormal[0] + oneFaceNormal[3] * oneFaceNormal[0];
			accNormal[1] = accNormal[1] + oneFaceNormal[3] * oneFaceNormal[1];
			accNormal[2] = accNormal[2] + oneFaceNormal[3] * oneFaceNormal[2];
			accArea = accArea + oneFaceNormal[3];
			//printf("%f,%f,%f\n", accNormal[0], accNormal[1], accNormal[2]);
		}
		//normal of a vertex is (a1*n1*+a2*n2+...+an*nn)/sum of area
		accNormal[0] = accNormal[0] / accArea;
		accNormal[1] = accNormal[1] / accArea;
		accNormal[2] = accNormal[2] / accArea;
		//normalize accNormal
		float mag = sqrt(accNormal[0]* accNormal[0] + accNormal[1] * accNormal[1] + accNormal[2] * accNormal[2]);

		nor[3 * nver + 0] = accNormal[0] / mag;
		nor[3 * nver + 1] = accNormal[1] / mag;
		nor[3 * nver + 2] = accNormal[2] / mag;
		//printf("final normal is %f,%f,%f\n", normalForPipe[0], normalForPipe[1], normalForPipe[2]);
	}
		free(adjFaces);

}
void setProj() {
	//glOrtho(xmin, xmax, ymin, ymax, zmin, zmax);
	/*************row 1 *****************/
	proj[0] = 2 / (xmax - xmin);
	proj[4] = 0;
	proj[8] = 0;
	proj[12] = -(xmax + xmin) / (xmax - xmin);

	/*************row 2 *****************/
	proj[1] = 0;
	proj[5] = 2 / (ymax - ymin);
	proj[9] = 0;
	proj[13] = -(ymax + ymin) / (ymax - ymin);

	/*************row 3 *****************/
	proj[2] = 0;
	proj[6] = 0;
	proj[10] = -2 / (zmax - zmin);
	proj[14] = -(zmax + zmin) / (zmax - zmin);

	/*************row 4 *****************/
	proj[3] = 0;
	proj[7] = 0;
	proj[11] = 0;
	proj[15] = 1;
	/*
	proj[3] = - (xmax + xmin) / (xmax - xmin);
	proj[5] = 2 / (ymax - ymin);
	proj[7] = -(ymax + ymin) / (ymax - ymin);
	proj[10] = -2 / (zmax - zmin);
	proj[11] = -(zmax + zmin) / (zmax - zmin);
	*/
}

void oneFace(int fv1, int fv2, int fv3) {
	float v1[3] = { verVBO[3*fv1 + 0], verVBO[3 * fv1 + 1], verVBO[3 * fv1 + 2] };
	float v2[3] = { verVBO[3 * fv2 + 0], verVBO[3 * fv2 + 1], verVBO[3 * fv2 + 2] };
	float v3[3] = { verVBO[3 * fv3 + 0], verVBO[3 * fv3 + 1], verVBO[3 * fv3 + 2] };

	float n1[3] = { normalForPipe[3 * fv1 + 0], normalForPipe[3 * fv1 + 1], normalForPipe[3 * fv1 + 2] };
	float n2[3] = { normalForPipe[3 * fv2 + 0], normalForPipe[3 * fv2 + 1], normalForPipe[3 * fv2 + 2] };
	float n3[3] = { normalForPipe[3 * fv3 + 0], normalForPipe[3 * fv3 + 1], normalForPipe[3 * fv3 + 2] };

	glBegin(GL_TRIANGLES);
	glNormal3fv(n1);
	glVertex3fv(v1);

	glNormal3fv(n2);
	glVertex3fv(v2);

	glNormal3fv(n3);
	glVertex3fv(v3);
	glEnd();
}

void renderTri() {

	for (int i = 0; i<nf; i++) {
		//fv1,fv2,fv3 are 3 points of the ith face
		int fv1 = faceEBO[3 * i + 0];
		int fv2 = faceEBO[3 * i + 1];
		int fv3 = faceEBO[3 * i + 2];
		oneFace(fv1, fv2, fv3);
	}
}


void renderFunction() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*drwaing code here */
	glColor3fv(color);

	glLoadIdentity();
	glMultMatrixf(view);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glIsEnabled(GL_NORMALIZE);
	glCullFace(GL_BACK);
	glEnable(GL_LIGHTING);
	
	switch (frontFace) {
	case 0:
		glFrontFace(GL_CCW);
		printf("pipeline CCW is front face\n");
		break;
	case 1:
		glFrontFace(GL_CW);
		printf("pipeline CW is front face\n");
		break;
	}
	switch (renderMode) {
	case 1:
		glPolygonMode(GL_FRONT, GL_POINT);
		printf("pipeline rendering mode is points\n");
		break;
	case 2:
		glPolygonMode(GL_FRONT, GL_LINE);
		printf("pipeline rendering mode is wireframe\n");
		break;

	default:
		glPolygonMode(GL_FRONT, GL_FILL);
		printf("pipeline rendering mode is fill\n");
		break;

	}
	if (smooth) {
		glShadeModel(GL_SMOOTH);
	}
	else if (!smooth) {
		glShadeModel(GL_FLAT);
	}
	

	
	
	GLfloat matSpecular[4] = {0.3,0.3,0.3,1.0 };
	GLfloat matShininess[1] = { 30.0 };
	GLfloat localView[1] = { 0.0 };
	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, localView);

	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

	glLightfv(GL_LIGHT0, GL_AMBIENT, localAmbientLight);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	
	if (isLightOn) {
		glEnable(GL_LIGHT0);
		printf("enable light0\n");
	}
	else if (!isLightOn) {
		glDisable(GL_LIGHT0);
		printf("disable light0\n");
	}
	
	renderTri();

	glFlush();
	glutSwapBuffers();

}
void refresh() {
	// Request redisplay of all three windows.
	for (int i = 0; i < 2; i++) {
		if (i == 0) {
			glutSetWindow(window[i]);
			glutPostRedisplay();
		}
		if (i == 1) {
			glutSetWindow(window[i]);
			glUseProgram(pID);
			GLint modeViewMatLocation = glGetUniformLocation(pID, "modelViewMatrix");
			GLint colorVectorLocation = glGetUniformLocation(pID, "colorVector");
			glUniformMatrix4fv(modeViewMatLocation, 1, GL_FALSE, view);
			glUniform3f(colorVectorLocation, color[0], color[1], color[2]);
			GLint projectionMatLocation = glGetUniformLocation(pID, "projectionMatrix");
			glUniformMatrix4fv(projectionMatLocation, 1, GL_FALSE, proj);


			GLint globalAmbientLocation = glGetUniformLocation(pID, "globalAmbientVector");
			glUniform4f(globalAmbientLocation, globalAmbientLight[0], globalAmbientLight[1], globalAmbientLight[2], globalAmbientLight[3]);

			GLint diffLightLocation = glGetUniformLocation(pID, "diffuseLight");
			glUniform4f(diffLightLocation, diffuseLight[0], diffuseLight[1], diffuseLight[2], diffuseLight[3]);

			GLint localAmbientLocation = glGetUniformLocation(pID, "localAmbientVector");
			glUniform4f(localAmbientLocation, localAmbientLight[0], localAmbientLight[1], localAmbientLight[2], localAmbientLight[3]);

			GLint specLightLocation = glGetUniformLocation(pID, "specularLight");
			glUniform4f(specLightLocation, specularLight[0], specularLight[1], specularLight[2], specularLight[3]);
			
			GLint cameraLocation = glGetUniformLocation(pID, "cameraVecor");
			glUniform4f(cameraLocation, newcv[0], newcv[1], newcv[2], 1);

			GLint lightSwitchLocation = glGetUniformLocation(pID, "isLocalLightOn");
			glUniform1i(lightSwitchLocation, isLightOn);

			GLint shadingSwitchLocation = glGetUniformLocation(pID, "isGouraudShading");
			glUniform1i(shadingSwitchLocation, smooth);

			glutPostRedisplay();
		}
	}
}
void refreshProj() {
	// Request redisplay of all three windows.
	for (int i = 0; i < 2; i++) {
		if (i == 0) {
			glutSetWindow(window[i]);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMultMatrixf(proj);
			glMatrixMode(GL_MODELVIEW);
			glutPostRedisplay();
		}
		if (i == 1) {
			printf("refresh shader window\n");
			glutSetWindow(window[i]);
			glUseProgram(pID);
			GLint projectionMatLocation = glGetUniformLocation(pID, "projectionMatrix");
			glUniformMatrix4fv(projectionMatLocation, 1, GL_FALSE, proj);
			glutPostRedisplay();
		}

	}
}

void reshapeFunction(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(proj);
	glMatrixMode(GL_MODELVIEW);

}


void rotation(float a, float *v) {
	float v1 = v[0];
	float v2 = v[1];
	float v3 = v[2];
	float cosIVVT[3][3];
	float sinVstar[3][3];
	float PT[3];
	float Up[3];
	float u[3];
	float cv[3];
	float n[3];

	/********************compute rotation matrix with Axis-angle ********************/
	float VVT[3][3] = { { v1*v1, v1*v2, v1*v3 },
	{ v2*v1, v2*v2, v2*v3 },
	{ v3*v1, v3*v2, v3*v3 }
	};

	float Vstar[3][3] = { { 0, -v3, v2 },
	{ v3, 0, -v1 },
	{ -v2, v1, 0 }
	};

	float IVVT[3][3] = { { 1 - v1*v1, -v1*v2, -v1*v3 },
	{ -v2*v1, 1 - v2*v2, -v2*v3 },
	{ -v3*v1, -v3*v2, 1 - v3*v3 }
	};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			cosIVVT[i][j] = cos(a*(PI / 180)) * IVVT[i][j];
			//printf("cosIVVT[%d][%d] is %f\n",i, j, cosIVVT[i][j]);
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			sinVstar[i][j] = sin(a*(PI / 180)) * Vstar[i][j];
			//printf("sinVstar[%d][%d] is %f\n",i, j, sinVstar[i][j]);
		}
	}


	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			R[i][j] = VVT[i][j] + cosIVVT[i][j] + sinVstar[i][j];
		}
	}

	/*****************apply rotation on camu[3]*********************/

	for (int i = 0; i <3; i++) {
		u[i] = R[i][0] * camu[0] + R[i][1] * camu[1] + R[i][2] * camu[2];
	}

	for (int i = 0; i <3; i++) {
		camu[i] = u[i];
		printf("camu[%d] is %f\n", i, camu[i]);
	}
	/*****************apply rotation on camv[3]*********************/

	for (int i = 0; i <3; i++) {
		cv[i] = R[i][0] * camv[0] + R[i][1] * camv[1] + R[i][2] * camv[2];
	}

	for (int i = 0; i <3; i++) {
		camv[i] = cv[i];
		printf("camv[%d] is %f\n", i, camv[i]);
	}

	/*****************apply rotation on camn[3]*********************/

	for (int i = 0; i <3; i++) {
		n[i] = R[i][0] * camn[0] + R[i][1] * camn[1] + R[i][2] * camn[2];
	}

	for (int i = 0; i <3; i++) {
		camn[i] = n[i];
		printf("camn[%d] is %f\n", i, camn[i]);
	}

}
void resetcam() {
	camu[0] = 1;
	camu[1] = 0;
	camu[2] = 0;

	camv[0] = 0;
	camv[1] = 1;
	camv[2] = 0;

	camn[0] = 0;
	camn[1] = 0;
	camn[2] = 1;

	newcv[0] = 0;
	newcv[1] = 0;
	newcv[2] = 0;

	zmin = restorzmin;
	zmax = restorzmax;

	proj[10] = -2 / (zmax - zmin);
	proj[14] = -(zmax + zmin) / (zmax - zmin);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(proj);
	glMatrixMode(GL_MODELVIEW);


	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i == j) {
				R[i][j] = 1;
			}
			else {
				R[i][j] = 0;
			}
		}

	}
}


void translation(float *coordi) {


	float T[3][3] = { { camu[0],camv[0], camn[0] },
	{ camu[1],camv[1], camn[1] },
	{ camu[2],camv[2], camn[2] },
	};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			newcv[i] += T[i][j] * coordi[j];
		}
		printf("newcv[%d] is %f\n", i, newcv[i]);
	}

}

void updateView() {
	/*************row 1 *****************/
	view[0] = camu[0];
	view[4] = camu[1];
	view[8] = camu[2];
	view[12] = -newcv[0];
	/*************row 2 *****************/
	view[1] = camv[0];
	view[5] = camv[1];
	view[9] = camv[2];
	view[13] = -newcv[1];
	/*************row 3 *****************/
	view[2] = camn[0];
	view[6] = camn[1];
	view[10] = camn[2];
	view[14] = -newcv[2];
	/*************row 4 *****************/
	view[3] = 0;
	view[7] = 0;
	view[11] = 0;
	view[15] = 1;
}

void keyFunction(unsigned char key, int x, int y) {
	float angle = 0;
	float speed = 1;
	float tspeed = 0.3;
	float tx[3] = { tspeed,0,0 };
	float txn[3] = { -tspeed, 0, 0 };
	float ty[3] = { 0,tspeed,0 };
	float tyn[3] = { 0,-tspeed, 0 };
	float tz[3] = { 0,0,tspeed };
	float tzn[3] = { 0,0,-tspeed };

	/************************Translation control**************************/
	if (key == 'a') {
		translation(txn);
	}
	if (key == 'd') {
		translation(tx);
	}
	if (key == 'w') {
		translation(tzn);
	}
	if (key == 's') {
		translation(tz);
	}
	if (key == 'q') {
		translation(ty);
	}
	if (key == 'z') {
		translation(tyn);
	}

	/************************rotation control**************************/

	if (key == 'n') {
		angle += speed;
		rotation(angle, camu);
	}
	if (key == 'm') {
		angle -= speed;
		rotation(angle, camu);
	}
	if (key == 'k') {
		angle += speed;
		rotation(angle, camv);
	}
	if (key == 'l') {
		angle -= speed;
		rotation(angle, camv);
	}
	if (key == 'o') {
		angle += speed;
		rotation(angle, camn);
	}
	if (key == 'p') {
		angle -= speed;
		rotation(angle, camn);
	}
	/************************Restore**************************/
	if (key == 'x') {
		color[0] = 1;
		color[1] = 1;
		color[2] = 1;
		globalAmbientLight[0] = 1; globalAmbientLight[1] = 1; globalAmbientLight[2] = 1; globalAmbientLight[3] = 1;
		localAmbientLight[0] = 1; localAmbientLight[1] = 1; localAmbientLight[2] = 1; localAmbientLight[3] = 1;
		diffuseLight[0] = 1; diffuseLight[1] = 1; diffuseLight[2] = 1; diffuseLight[3] = 1;
		specularLight[0] = 1; specularLight[1] = 1; specularLight[2] = 1; specularLight[3] = 1;
		resetcam();
	}
	/************************Change file**************************/
	if (key == '1') {

		nv = 0;
		nf = 0;
		resetcam();
		readdata(f1);
		normalForPipe = norB;
		//calculateNormal(verVBO, faceEBO, normalForPipe);


	}
	if (key == '2') {
		nv = 0;
		nf = 0;
		resetcam();
		readdata(f2);
		normalForPipe = norC;
		//calculateNormal(verVBO, faceEBO, normalForPipe);

	}
	/************************Change rendering mode**************************/
	if (key == '3') {
		renderMode += 1;
		if (renderMode > 2) {
			renderMode = 0;
		}
		resetcam();
	}
	/************************Change front face**************************/
	if (key == 'f') {
		frontFace = !frontFace;
	}
	/************************Change color**************************/
	if (key == 'c') {
		color[0] = (float)rand() / RAND_MAX;
		color[1] = (float)rand() / RAND_MAX;
		color[2] = (float)rand() / RAND_MAX;
	}
	/************************change shading mode **************************/
	if (key == 'e') {
		smooth = !smooth;
	}
	/************************Light switch**************************/
	if (key == '4') {
		isLightOn = !isLightOn;
		printf("isLighton is %d\n", isLightOn);
	}
	/************************switch to global light mode **************************/
	if (key == '5') {
		globalLightMode = 1;
		localLightMode = 0;
		printf("The current light mode is global\n");
	}
	/************************switch to local light mode **************************/
	if (key == '6') {
		globalLightMode = 0;
		localLightMode = 1;
		printf("The current light mode is local\n");
	}
	/************************choose the diffuse or specular **************************/
	if (key == 't') {
		diffuse = !diffuse;
		if (diffuse) {
			printf("edit diffuse component\n");
		}
		else if (!diffuse) {
			printf("edit specular component\n");
		}
	}


	/************************choose the color to be modified **************************/
	if (key == 'r') {
		rgb = 1;
		printf("change the red color\n");
	}
	if (key == 'g') {
		rgb = 2;
		printf("change the green color\n");
	}
	if (key == 'b') {
		rgb = 3;
		printf("change the blue color\n");
	}
	if (key == 'v') {
		rgb = 4;
		printf("change the alpha\n");
	}
	/************************change the color component of diffuse or specular **************************/

	if (key == '9') {
		if (diffuse) {
			switch (rgb) {
			case 1:
				diffuseLight[0] -= 0.1;
				break;
			case 2:
				diffuseLight[1] -= 0.1;
				break;
			case 3:
				diffuseLight[2] -= 0.1;
				break;
			case 4:
				diffuseLight[3] -= 0.1;
				break;
			}
			printf("diffuseLight is [%f,%f,%f,%f]\n", diffuseLight[0], diffuseLight[1], diffuseLight[2], diffuseLight[3]);
		}
		else if (!diffuse) {
			switch (rgb) {
			case 1:
				specularLight[0] -= 0.1;
				break;
			case 2:
				specularLight[1] -= 0.1;
				break;
			case 3:
				specularLight[2] -= 0.1;
				break;
			case 4:
				specularLight[3] -= 0.1;
				break;
			}
			printf("specularLight is [%f,%f,%f,%f]\n", specularLight[0], specularLight[1], specularLight[2], specularLight[3]);
		}

	}
	if (key == '0') {
		if (diffuse) {
			switch (rgb) {
			case 1:
				diffuseLight[0] += 0.1;
				break;
			case 2:
				diffuseLight[1] += 0.1;
				break;
			case 3:
				diffuseLight[2] += 0.1;
				break;
			case 4:
				diffuseLight[3] += 0.1;
				break;
			}
			printf("diffuseLight is [%f,%f,%f,%f]\n", diffuseLight[0], diffuseLight[1], diffuseLight[2], diffuseLight[3]);
		}
		else if (!diffuse) {
			switch (rgb) {
			case 1:
				specularLight[0] += 0.1;
				break;
			case 2:
				specularLight[1] += 0.1;
				break;
			case 3:
				specularLight[2] += 0.1;
				break;
			case 4:
				specularLight[3] += 0.1;
				break;
			}
			printf("specularLight is [%f,%f,%f,%f]\n", specularLight[0], specularLight[1], specularLight[2], specularLight[3]);
		}
	}
	/************************change the color value **************************/
	if (key == '-') {
		if (globalLightMode == 1) {
			switch (rgb) {
			case 1:
				globalAmbientLight[0] -= 0.1;
				break;
			case 2:
				globalAmbientLight[1] -= 0.1;
				break;
			case 3:
				globalAmbientLight[2] -= 0.1;
				break;
			case 4:
				globalAmbientLight[3] -= 0.1;
				break;
			}
			printf("globalAmbient is [%f,%f,%f,%f]\n", globalAmbientLight[0], globalAmbientLight[1], globalAmbientLight[2], globalAmbientLight[3]);
		}else if (localLightMode == 1) {
			switch (rgb) {
			case 1:
				localAmbientLight[0] -= 0.1;
				break;
			case 2:
				localAmbientLight[1] -= 0.1;
				break;
			case 3:
				localAmbientLight[2] -= 0.1;
				break;
			case 4:
				localAmbientLight[3] -= 0.1;
				break;
			}
			printf("localAmbientLight is [%f,%f,%f,%f]\n", localAmbientLight[0], localAmbientLight[1], localAmbientLight[2], localAmbientLight[3]);
		}
		
	}
	if (key == '=') {
		if (globalLightMode == 1) {
			switch (rgb) {
			case 1:
				globalAmbientLight[0] += 0.1;
				break;
			case 2:
				globalAmbientLight[1] += 0.1;
				break;
			case 3:
				globalAmbientLight[2] += 0.1;
				break;
			case 4:
				globalAmbientLight[3] += 0.1;
				break;
			}
			printf("globalAmbient is [%f,%f,%f,%f]\n", globalAmbientLight[0], globalAmbientLight[1], globalAmbientLight[2], globalAmbientLight[3]);

		}
		else if (localLightMode == 1) {
			switch (rgb) {
			case 1:
				localAmbientLight[0] += 0.1;
				break;
			case 2:
				localAmbientLight[1] += 0.1;
				break;
			case 3:
				localAmbientLight[2] += 0.1;
				break;
			case 4:
				localAmbientLight[3] += 0.1;
				break;
			}
			printf("localAmbientLight is [%f,%f,%f,%f]\n", localAmbientLight[0], localAmbientLight[1], localAmbientLight[2], localAmbientLight[3]);
		}
		
	}
	updateView();
	printf("\n\n\n");
	refresh();
}
/************************Change near clipping plane**************************/

void SpecialKey(GLint key, GLint x, GLint y) {
	if (key == GLUT_KEY_LEFT) {
		zmin -= 0.3;
		printf("near clipping plane is %f, far clipping plane is %f\n", zmin, zmax);
	}
	if (key == GLUT_KEY_RIGHT) {
		zmin += 0.3;
		printf("near clipping plane is %f, far clipping plane is %f\n", zmin, zmax);
	}
	/************************Change far clipping plane**************************/

	if (key == GLUT_KEY_UP) {
		zmax -= 0.3;
		printf("near clipping plane is %f, far clipping plane is %f\n", zmin, zmax);
	}
	if (key == GLUT_KEY_DOWN) {
		zmax += 0.3;
		printf("near clipping plane is %f, far clipping plane is %f\n", zmin, zmax);
	}

	//setProj();

	proj[10] = -2 / (zmax - zmin);
	proj[14] = -(zmax + zmin) / (zmax - zmin);

	refreshProj();

}


/*********************************Start shader function part *****************************************/

char *textFileRead(char *fn)
{
	FILE *fp;
	char *content = NULL;

	int count = 0;

	if (fn != NULL)
	{
		fp = fopen(fn, "rt");

		if (fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0)
			{
				content = (char *)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}


void initBufferObject(void)
{
	int size = 104505 * sizeof(float);
	int size2 = 4662 * sizeof(float);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	VAO[0] = 0; VBO[0] = 0; EBO[0] = 0; NOR[0] = 0;
	VAO[1] = 0; VBO[1] = 0; EBO[1] = 0;  NOR[1] = 0;
	
	printf("There are %d vertices\n", nv);
	printf("There are %d faces\n", nf);
	//printf("the first vertex is %f %f %f\n", verB[0], verB[1], verB[2]);
	//printf("the first face is %d %d %d\n", faceB[0], faceB[1], faceB[2]);

	glGenBuffers(2, &VBO);
	glGenBuffers(2, &EBO);
	glGenBuffers(2, &NOR);

	// setup VAO
	glGenVertexArrays(2, &VAO[0]);


	glBindVertexArray(VAO[0]);
	printf("start VAO\n");
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 2 * 104505 * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 104505 * sizeof(float), verB);
	glBufferSubData(GL_ARRAY_BUFFER, 104505 * sizeof(float), 104505 * sizeof(float), norB);
	//glBufferData(GL_ARRAY_BUFFER, 2* 104505 * sizeof(float), verB, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, 104505 * sizeof(float), norB, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const GLvoid*) size);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 208998 * sizeof(int), faceB, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	glBindVertexArray(VAO[1]);
	printf("start VAO\n");
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, 2 * 4662 * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4662 * sizeof(float), verC);
	glBufferSubData(GL_ARRAY_BUFFER, 4662 * sizeof(float), 4662 * sizeof(float), norC);
	//glBufferData(GL_ARRAY_BUFFER, 4662 * sizeof(float), verC, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const GLvoid*)size2);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//glBindBuffer(GL_ARRAY_BUFFER, NOR[1]);
	//glBufferData(GL_ARRAY_BUFFER, 4662 * sizeof(float), norC, GL_STATIC_DRAW);
	

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 9312 * sizeof(int), faceC, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// Use depth buffering for hidden surface elimination
	glEnable(GL_DEPTH_TEST);


	setProj();

	updateView();

}

void setShaders() {
	char *vs = NULL;
	char *fs = NULL;

	vsID = glCreateShader(GL_VERTEX_SHADER);
	fsID = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("vertexshader.txt");
	fs = textFileRead("fragshader.txt");

	printf("vs is %s\n", vs);
	printf("fs is %s\n", fs);
	const GLchar *vv = vs;
	const GLchar *fv = fs;

	glShaderSource(vsID, 1, &vv, NULL);
	glShaderSource(fsID, 1, &fv, NULL);

	glCompileShader(vsID);
	glCompileShader(fsID);

	GLint success1;
	GLint success2;

	glGetShaderiv(vsID, GL_COMPILE_STATUS, &success1);
	if (!success1) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(vsID, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", GL_VERTEX_SHADER, InfoLog);

	}

	glGetShaderiv(fsID, GL_COMPILE_STATUS, &success2);
	if (!success2) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(fsID, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", GL_FRAGMENT_SHADER, InfoLog);

	}

	pID = glCreateProgram();
	glAttachShader(pID, vsID);
	glAttachShader(pID, fsID);

	glLinkProgram(pID);
	glUseProgram(pID);

	// get uniform input 
	GLint modeViewMatLocation = glGetUniformLocation(pID, "modelViewMatrix");
	GLint projectionMatLocation = glGetUniformLocation(pID, "projectionMatrix");
	GLint colorVectorLocation = glGetUniformLocation(pID, "colorVector");
	GLint globalAmbientLocation = glGetUniformLocation(pID, "globalAmbientVector");
	GLint lightPosLocation = glGetUniformLocation(pID, "lightPos");
	GLint diffLightLocation = glGetUniformLocation(pID, "diffuseLight");
	GLint localAmbientLocation = glGetUniformLocation(pID, "localAmbientVector");
	GLint specLightLocation = glGetUniformLocation(pID, "specularLight");
	GLint cameraLocation = glGetUniformLocation(pID, "cameraVecor");
	GLint lightSwitchLocation = glGetUniformLocation(pID, "isLocalLightOn");
	GLint shadingSwitchLocation = glGetUniformLocation(pID, "isGouraudShading");

	glUniformMatrix4fv(modeViewMatLocation, 1, GL_FALSE, view);
	glUniformMatrix4fv(projectionMatLocation, 1, GL_FALSE, proj);
	glUniform4f(globalAmbientLocation, globalAmbientLight[0], globalAmbientLight[1], globalAmbientLight[2], globalAmbientLight[3]);
	glUniform4f(lightPosLocation, lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]);
	glUniform4f(diffLightLocation, diffuseLight[0], diffuseLight[1], diffuseLight[2], diffuseLight[3]);
	glUniform4f(specLightLocation, specularLight[0], specularLight[1], specularLight[2], specularLight[3]);
	glUniform4f(localAmbientLocation, localAmbientLight[0], localAmbientLight[1], localAmbientLight[2], localAmbientLight[3]);

	glUniform1i(lightSwitchLocation, isLightOn);
	glUniform1i(shadingSwitchLocation, smooth);


	glUniform4f(cameraLocation, newcv[0], newcv[1], newcv[2], 1);

	//glUniform3f(colorVectorLocation, color[0], color[1], color[2]);

	
	glDeleteShader(vsID);
	glDeleteShader(fsID);
}

void display(void){

	//glLoadIdentity();
	glUseProgram(pID);
	//glUseProgram(0);
	glValidateProgram(pID);
	GLint validate = 0;
	glGetProgramiv(pID, GL_VALIDATE_STATUS, &validate);


	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	switch (frontFace) {
	case 0:
		glFrontFace(GL_CCW);
		printf("shader CCW is front face\n");
		break;
	case 1:
		glFrontFace(GL_CW);
		printf("shaderCW is front face\n");
		break;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (nv == 34835) {
		printf("shader start draw bunny\n");
		glBindVertexArray(VAO[0]);
	}
	else if (nv == 1554) {
		printf("shader start draw cactus\n");
		glBindVertexArray(VAO[1]);
	}
		switch (renderMode) {
		case 1:
			glPolygonMode(GL_FRONT, GL_POINT);
			printf("shader rendering mode is points\n");
			//renderTri();
			break;
		case 2:
			glPolygonMode(GL_FRONT, GL_LINE);
			printf("shader rendering mode is wireframe\n");
			//renderTri();
			break;

		default:
			glPolygonMode(GL_FRONT, GL_FILL);
			printf("shader rendering mode is fill\n");
			//renderTri();
			break;

		}		
		glDrawElements(GL_TRIANGLES, 3 * nf, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	
	//glFlush();
	glutSwapBuffers();
}
/*********************************End shader function part  *****************************************/
int main(int argc, char** argv) {
	verB = (GLfloat *)malloc(120000 * sizeof(float));
	norB = (GLfloat *)malloc(120000 * sizeof(float));
	faceB = (int *)malloc(300000 * sizeof(int));

	verC = (GLfloat *)malloc(5000 * sizeof(float));
	norC = (GLfloat *)malloc(5000 * sizeof(float));
	faceC = (int *)malloc(10000 * sizeof(int));
	if (verB && norB && faceB && verC && norC && faceC) {
		printf("successfully allocate apace for shader data\n");
	}
	
	if (verB == NULL)
		printf("malloc error for verB");
	if (faceB == NULL)
		printf("malloc error for faceB");
	if (verC == NULL)
		printf("malloc error for verC");
	if (faceC == NULL)
		printf("malloc error for faceC");

	readForShader(f1, verB, faceB);
	printf("shader: there are %d vertices %d faces for bunny\n", nv,nf);
	calculateNormal(verB, faceB, norB);
	nv = 0;
	nf = 0;
	readForShader(f2, verC, faceC);
	printf("shader: there are %d vertices %d faces for cactus\n", nv, nf);
	calculateNormal(verC, faceC, norC);

	nv = 0;
	nf = 0;
	readdata(f1);
	calculateNormal(verVBO, faceEBO, normalForPipe);
	printf("pipe %f, %f,%f \n", normalForPipe[630], normalForPipe[631], normalForPipe[632] );
	printf("shader %f, %f,%f \n", norB[630], norB[631], norB[632]);
	printf("shader %f, %f,%f \n", norC[630], norC[631], norC[632]);


	/* start setting up the opengl */
	setProj();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	isLightOn = 1;
	globalLightMode = 0;
	localLightMode = 0;
	rgb = 0;
	smooth = 1;
	globalAmbientLight[0] = 1; globalAmbientLight[1] = 1; globalAmbientLight[2] = 1; globalAmbientLight[3] = 1;
	localAmbientLight[0] = 1; localAmbientLight[1] = 1; localAmbientLight[2] = 1; localAmbientLight[3] = 1;
	diffuseLight[0] = 1; diffuseLight[1] = 1; diffuseLight[2] = 1; diffuseLight[3] = 1;
	specularLight[0] = 1; specularLight[1] = 1; specularLight[2] = 1; specularLight[3] = 1;
	lightPosition[0] = 0.0; lightPosition[1] = 2.0; lightPosition[2] = 0.0; lightPosition[3] = 1.0;
	
	/* 1st Window */
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	window[0] = glutCreateWindow("Project 1");

	glutInitContextFlags(GLUT_COMPATIBILITY_PROFILE);
	glutDisplayFunc(renderFunction);
	glutReshapeFunc(reshapeFunction);

	glutKeyboardFunc(keyFunction);
	glutSpecialFunc(SpecialKey);


	/* 2st Window */
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(600, 100);
	//window[1] = glutCreateWindow("Project 2");

	window[1] = glutCreateWindow("Hellow GLSL");
	glewInit();
	glutDisplayFunc(display);

	initBufferObject();
	setShaders();

	//glEnable(GL_DEPTH_TEST);
	glutMainLoop();

	free(verB);
	free(faceB);
	free(verC);
	free(faceC);
	return 0;
}