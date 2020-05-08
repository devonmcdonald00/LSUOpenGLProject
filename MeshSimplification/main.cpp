//FINAL
#include "mesh.h"
#include "simplification.h"
#include <windows.h> 
#include <chrono>
#include <ctime>
#include <string>
#include <iostream>
#include <thread>
#include <future>

using namespace chrono;
using namespace std;


ofstream outputFile;
std::vector<std::future<float>> fut_vec;
int i = 0;
int counter = 0;
bool reduceAll = 0;
int nFacesReduced = 0;
auto startTime = high_resolution_clock::now();
auto stopTime = high_resolution_clock::now();
auto startTimeAll = high_resolution_clock::now();
auto stopTimeAll = high_resolution_clock::now();
auto accumTime = duration_cast<milliseconds>(stopTime - startTime);
auto accumTimeAll = milliseconds(0);
auto accumTimeallPrevious = milliseconds(10);
bool cPressed = 0;
bool Ent = 0;
bool zoom = 0;
bool parallel = 1;
bool translate = 0;
float camera_zoom = 0.0f;


Mesh mesh;
Simplification simplification;

GLfloat startx, starty;
GLfloat model_angle1 = 0.0, model_angle2 = 0.0, scale = 1.0, eye[3] = { 0.0, 0.0, 10.0 };
GLfloat window_width = 800, window_height = 800;

int Key;
int toggle = 0;
bool left_click = 0, right_click = 0;

bool doEdgeCollapse = false, doVertexSplit = false, doLOD = false;
int  step = 0;



void plotCSV() {
    ofstream executeFile;
    executeFile.open("executeFile.ps1");
    string powershell;
    if (reduceAll == 1) {
        if (parallel) {
            powershell = "python collapse_analysis_graph.py all p";
        }
        else {
            powershell = "python collapse_analysis_graph.py all s";
        }
    }
    else {
        if (parallel) {
            powershell = "python collapse_analysis_graph.py individual p";
        }
        else {
            powershell = "python collapse_analysis_graph.py individual s";
        }
    }
    executeFile << powershell << endl;
    executeFile.close();
    system("powershell -ExecutionPolicy Bypass -F executeFile.ps1");

}

void mouse(int button, int state, int x, int y)
{
    if ( button == GLUT_LEFT_BUTTON ) {
        if ( state == GLUT_DOWN ) {
            left_click = true;
            startx   = x;
            starty   = y;
        }else if (state == GLUT_UP) {
            left_click = false;
        }
    }else{ // button == GLUT_RIGHT_BUTTON
        if ( state == GLUT_DOWN ) {  
            right_click = true;
            startx   = x;
            starty   = y;
        }else if (state == GLUT_UP) {
            right_click = false;
        }
    }

}

void motion( int x, int y )
{
    if ( left_click && !right_click ) {       // rotation
        model_angle1 += (x - startx);
        model_angle2 += (y - starty);
    }else if( !left_click && right_click ){   // translating
        eye[0] -= (x - startx) / (window_width *0.25);
        eye[1] += (y - starty) / (window_height*0.25);
    }
    
    else{ // if( left_click && right_click ) // scaling
        scale -= (y - starty) * 0.01;
    }
    if (zoom) {
        camera_zoom += 100*((y - starty) / (window_height * 0.25));
    }

    startx = x;
    starty = y;

    glutPostRedisplay();
}


void keyboard( unsigned char c, int x, int y )
{
    switch(c){
    case 'a':
        nFacesReduced = simplification.facesCollapsed();
        outputFile << nFacesReduced / (accumTime.count() / 1000) << ", "<<  i + 1<< endl;
        i++;
        cout << "\nThe number of collapsed faces so far is " << nFacesReduced << " in " << accumTime.count()/1000 << " s" << endl;
        
        break;
    case 'b':
        //reduce all faces and do analysis
        reduceAll = !reduceAll;
        startTimeAll = high_resolution_clock::now();
        accumTimeAll = milliseconds(1000);
        break;
    case 'r':
        //reset analysis
        outputFile.close();
        outputFile.open("collapse_analysis.csv", ofstream::out | ofstream::trunc);
        outputFile.close();
        outputFile.open("collapse_analysis.csv", ios::out | ios::trunc);
        reduceAll = 0;
        break;
    case 'q':
        exit(0);
    case 'c':
        cPressed = 1;
        if (!Ent) {
            Ent = 1;
            startTime = high_resolution_clock::now();
        }
        doEdgeCollapse = true;  break;
    case 's':   
        doVertexSplit  = true;  break;
    case 'z':
        if(step < 200){
            step++; 
            doLOD = true;
        }
            break;
    case 'x':
        if(step > 0){
            step--;
            doLOD = true;
        }
        break;
    case 't':
        toggle = !toggle;
        break;
    case 'l':
        zoom = !zoom;
        break;
    case 'm':
        translate = !translate;
        break;
    case 'p':
        plotCSV();
        break;
    case 'o':
        parallel = !parallel;
        break;
    default:
        break;
    }
    
    glutPostRedisplay();
}


void display()
{
    counter++;
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glTranslatef(0.0f, 0.0f, -camera_zoom);

    glScalef(scale, scale, scale);
    gluLookAt(eye[0], eye[1], 10,
        eye[0], eye[1], 0,
        0.0, 1.0, 0.0);

    if (translate) {
        glRotatef(model_angle1, 0, 1, 0);
        glRotatef(model_angle2, 1, 0, 0);
    }

    glPushMatrix();

    if (!cPressed && Ent) {
        stopTime = high_resolution_clock::now();
        accumTime += duration_cast<milliseconds>(stopTime - startTime);
        //cout << accumTime.count()/1000 << endl;
        Ent = 0;
    }
    cPressed = 0;
    if (doEdgeCollapse || reduceAll) {
        
        //THIS IS PARALLEL
        if (parallel) {
            for (int i = 0; i <= 50; i++) {
                future<bool> returned = async(launch::async, &Simplification::EdgeCollapse, &simplification);
                if (i == 50) {
                    stopTimeAll = high_resolution_clock::now();
                    accumTimeAll += duration_cast<milliseconds>(stopTimeAll - startTimeAll);
                    outputFile << simplification.facesCollapsed() << "," << accumTimeAll.count() - 1000 << endl;
                    startTimeAll = high_resolution_clock::now();
                }
            }
        }
        //THIS IS SEQUENTIAL

        else {
            stopTimeAll = high_resolution_clock::now();
            accumTimeallPrevious = accumTimeAll;
            accumTimeAll += duration_cast<milliseconds>(stopTimeAll - startTimeAll);
            simplification.EdgeCollapse();
            if (accumTimeAll != accumTimeallPrevious && counter % 50 == 0) {
                outputFile << simplification.facesCollapsed() << "," << accumTimeAll.count() - 1000 << endl;
            }
            startTimeAll = high_resolution_clock::now();
        }

        doEdgeCollapse = false;
    }
    

    if(doVertexSplit){
        simplification.VertexSplit();
        doVertexSplit = false;
    }

    if(doLOD){
        simplification.ControlLevelOfDetail(step);
        doLOD = false;
    }

    mesh.Display(toggle);
    glPopMatrix();
    glutSwapBuffers();
    
    
    if (reduceAll == 1 && simplification.facesCollapsed() <= mesh.n_faces-3) {
        glutPostRedisplay();
    }
    else {
        //Was all the way reduced
        reduceAll = 0;
    }
}


int main(int argc, char *argv[])
{
    outputFile.open("collapse_analysis.csv", ios::out | ios::trunc);
    outputFile << "Faces Reduced" << ","<< "Time(ms)" << endl;
    outputFile << 0 << "," << 0 << endl;

    if( argc != 2 || mesh.ConstructMeshDataStructure(argv[1]) == false ){
        cerr << "usage: meshSimplification.exe *.off\n";
        exit(0);
    }
    simplification.InitSimplification(&mesh);
    cout << "\nTo collapse the mesh completely press 'b'... Then to plot the data press 'p'\n";
    cout << "\nAutomatically the mesh will be reduced in parallel...\n";
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow("Mesh Simplification");

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    GLInit();
    glutMainLoop();
    outputFile.close();
    return 1;
}

