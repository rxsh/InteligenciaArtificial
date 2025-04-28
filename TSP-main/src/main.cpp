#include <GL/freeglut.h>
#include <math.h>
#include <time.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "funciones.h"

using namespace std;
struct punto {
  int x, y;
  punto(int x, int y) {
    this->x = x;
    this->y = y;
  }
};
struct individuo {
  vector<int> camino;
  double funcion, pselec, valor_esp, valor_actual;
  individuo(vector<int> cam = {}, double f = 0, double p = 0, double v = 0, double a = 0) {
    this->camino = cam;
    this->funcion = f;
    this->pselec = p;
    this->valor_esp = v;
    this->valor_actual = a;
  }
};


vector<double> suma_fin, media_fin, max_min_fin;
int sep = 15;
int cantidad = 20;
int nodos = 10;
int inicio = 4;
int fin = 8;

vector<punto> puntos;
vector<individuo> poblacion;
vector<int> tmp;
vector<int> index;
individuo caminofin;

double funcion(vector<int> camino) {
  double dist = 0;
  for (int i = 0; i < camino.size() - 1; i++) {
    int j = (i+1) % camino.size();
    dist += sqrt(pow(puntos[camino[i]].x - puntos[camino[i + 1]].x, 2) +
                 pow(puntos[camino[i]].y - puntos[camino[i + 1]].y, 2));
  }

  dist += sqrt(pow(puntos[camino[camino.size() - 1]].x - puntos[camino[0]].x, 2) +
               pow(puntos[camino[camino.size() - 1]].y - puntos[camino[0]].y, 2));
  return dist;
}

double round1(double var) { return round(var * 100) / 100; }
void print2(vector<int> camino) {
  for (int i = 0; i < camino.size(); i++) {
    cout << camino[i] << " ";
  }
}
void print(vector<individuo> poblacion) {
  cout << "N" << setw(sep) << "Camino" << setw(sep) << "F" << setw(sep) << "Psel" << setw(sep)
       << "V_esp" << setw(sep) << "V_act" << endl;
  for (int i = 0; i < poblacion.size(); i++) {
    cout << i + 1 << setw(sep) << " ";
    print2(poblacion[i].camino);
    cout << setw(sep) << poblacion[i].funcion << setw(sep) << poblacion[i].pselec << setw(sep)
         << poblacion[i].valor_esp << setw(sep) << poblacion[i].valor_actual << endl;
  }
  cout << endl;
}
individuo ruleta(vector<individuo> poblacion) {
  double suma = 0;
  for (int i = 0; i < poblacion.size(); i++) {
    suma += poblacion[i].funcion;
  }

  double r = (double)rand() / RAND_MAX * suma;
  double acum = 0;
  for (int i = 0; i < poblacion.size(); i++) {
    acum += poblacion[i].funcion;
    if (r >= acum) {
      return poblacion[i];
    }
  }
  return poblacion[0];
}

individuo cruce(individuo &padre, individuo &madre) {
  int tam = padre.camino.size();
  int ini = rand() % tam;
  int fin = rand() % tam;
  if (ini > fin) {
    swap(ini, fin);
  }
  vector<int> hijo1;
  hijo1.assign(tam, -1);
  for (int i = ini; i <= fin; i++) {
    hijo1[i] = padre.camino[i];
  }
  int index = 0;
  for (int j = 0; j < tam; j++) {
    if (hijo1[j] == -1) {
      while (find(hijo1.begin(), hijo1.end(), madre.camino[index]) != hijo1.end()) {
        index++;
      }
      hijo1[j] = madre.camino[index];
    }
  }
  individuo hijo(hijo1);
  return hijo;
}
void mutar(individuo &hijo, double prob) {
  double prob_actual = (generacion_sin_mejora > 5) ? prob * 2 : prob;
  for (int i = 0; i < hijo.camino.size(); i++) {
    if ((double)rand() / RAND_MAX < prob_actual) {
      //int pos = rand() % (hijo.camino.size() - 2) + 1;
      //swap(hijo.camino[i], hijo.camino[pos]);
      swap(hijo.camino[rand()%hijo.camino.size()], 
             hijo.camino[rand()%hijo.camino.size()]);
    }
  }
}

void muta_ini(vector<individuo> &poblacion) {
  double suma = 0, media;
  double total = 0;
  double extr = 10000000;
  
  // Calcular suma total y mejor distancia
  for (int j = 0; j < poblacion.size(); j++) {
      double tmp5 = round1(funcion(poblacion[j].camino));
      poblacion[j].funcion = tmp5;
      total += tmp5;
      extr = min(extr, tmp5);
  }
  
  suma = total;
  media = total / poblacion.size();
  
  // Calcular Psel, V_esp y V_act
  for (int j = 0; j < poblacion.size(); j++) {
      poblacion[j].pselec = poblacion[j].funcion / suma;
      poblacion[j].valor_esp = poblacion[j].funcion / media;
      poblacion[j].valor_actual = round(poblacion[j].valor_esp);
  }
  
  suma_fin.push_back(suma);
  media_fin.push_back(media);
  max_min_fin.push_back(extr);
}

individuo muta_fin(vector<individuo> &poblacion) {
  generacion_actual = 0; 
  
  while(generacion_actual < max_generaciones) {
      muta_ini(poblacion);
      sort(poblacion.begin(), poblacion.end(),
           [](individuo a, individuo b) { return a.funcion < b.funcion; });
      
      // Mostrar información de la generación actual
      cout << "\n--- Generacion " << generacion_actual << " ---" << endl;
      cout << "Mejor distancia: " << max_min_fin.back() << endl;
      cout << "Distancia promedio: " << media_fin.back() << endl;
      print(poblacion);
      
      if (max_min_fin.back() < mejor_distancia_global - umbral_convergencia) {
        mejor_distancia_global = max_min_fin.back();
        generacion_sin_mejora = 0;
      } else {
          generacion_sin_mejora++;
      }
      
      if (generacion_sin_mejora >= 10) { 
          cout << "Deteniendo por convergencia en generación " << generacion_actual << endl;
          break;
      }
      
      // Operadores genéticos
      vector<individuo> hijos(poblacion.size());
      hijos[0] = poblacion[0]; // Elitismo
      
      for(int j = 1; j < poblacion.size(); j++) {
          individuo padre = ruleta(poblacion);
          individuo madre = ruleta(poblacion);
          individuo hijo = cruce(padre, madre);
          mutar(hijo, 0.2);
          hijos[j] = hijo;
      }
      
      poblacion = hijos;
      generacion_actual++;
  }
  
  return poblacion[0];
}

void iniciar() {
  srand(time(NULL));
  for (int i = 0; i < nodos; i++) {
    int x = rand() % 100;
    int y = rand() % 100;
    punto tmp(x, y);
    puntos.push_back(tmp);
  }
}

int windowWidth = 1900;
int windowHeight = 1050;

// Botones
int button1X = 50;
int button1Y = windowHeight / 2 - 150;
int button1Width = 200;
int button1Height = 100;
int button2X = 50;
int button2Y = windowHeight / 2 + 50;
int button2Width = 200;
int button2Height = 100;
int puntos_cant = 0;


float colors[6][3] = {{1.0f, 1.0f, 1.0f},   // Blanco
                      {0.0f, 1.0f, 0.0f},   // Verde
                      {1.0f, 0.0f, 0.0f},   // Rojo
                      {0.0f, 0.0f, 1.0f},   // Azul
                      {1.0f, 1.0f, 0.0f},   // Amarillo
                      {0.0f, 0.0f, 0.0f}};  // Negro
const char *button1Text = "Generar";
const char *button2Text = "TSP";
string percentageInput = "";

// Dibujar Texto
void drawText(float x, float y, string text, float color[3]) {
  glColor3fv(color);
  glRasterPos2f(x, y);
  for (char &c : text) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
  }
}
void keyboard(unsigned char key, int x, int y) {
  if (key == 13) {
    percentageInput += "\n";
  } else if (key == 8) {
    if (!percentageInput.empty()) {
      percentageInput.pop_back();
    }
  } else {
    percentageInput += key;
  }
  glutPostRedisplay();
}
int point_radius = 10.0f;
bool drawGraphC = false;

// Función para dibujar un punto
void drawPoint(float x, float y, float color[]) {
  glColor3fv(color);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(x, y);
  for (int i = 0; i <= 100; ++i) {
    float angle = 2.0f * 3.14159265358979323846f * float(i) / 100.0f;
    float dx = point_radius * cosf(angle);
    float dy = point_radius * sinf(angle);
    glVertex2f(x + dx, y + dy);
  }
  glEnd();
}

// Función para dibujar una línea entre dos puntos
void drawLine(float x1, float y1, float x2, float y2, float color[]) {
  glColor3fv(color);
  glBegin(GL_LINES);
  glVertex2f(x1, y1);
  glVertex2f(x2, y2);
  glEnd();
}

void drawCartesianPlane() {
  // Set color to white
  glColor3f(0.5f, 0.5f, 0.5f);

  // Draw y-axis
  glBegin(GL_LINES);
  glVertex2f(400, 0);
  glVertex2f(400, 1000);
  glEnd();

  // Draw x-axis
  glBegin(GL_LINES);
  glVertex2f(400, 1000);
  glVertex2f(1400, 1000);
  glEnd();

  // Draw marks on y-axis
  for (int i = 0; i <= 100; i += 10) {
    glBegin(GL_LINES);
    glVertex2f(390, 1000 - i * 10);
    glVertex2f(410, 1000 - i * 10);
    glEnd();
  }

  // Draw marks on x-axis
  for (int i = 0; i <= 100; i += 10) {
    glBegin(GL_LINES);
    glVertex2f(400 + i * 10, 990);
    glVertex2f(400 + i * 10, 1010);
    glEnd();
  }
}

void drawGraph(std::vector<punto> puntos, vector<int> camino) {
  // Dibujar todas las conexiones posibles (gris claro)
  glColor3f(0.7f, 0.7f, 0.7f); // Color más suave para las conexiones
  glLineWidth(1.0f);
  for (int i = 0; i < puntos.size(); i++) {
      int cord1X = puntos[i].x * 10 + 400;
      int cord1Y = 1000 - puntos[i].y * 10;
      for (int j = i + 1; j < puntos.size(); j++) {
          int cord2X = puntos[j].x * 10 + 400;
          int cord2Y = 1000 - puntos[j].y * 10;
          drawLine(cord1X, cord1Y, cord2X, cord2Y, colors[0]);
      }
  }

  if (drawGraphC && !camino.empty()) {
      glColor3f(1.0f, 0.0f, 0.0f); // Rojo
      glLineWidth(3.0f); // Más grueso para destacar
      
      for (int i = 0; i < camino.size(); i++) {
          int current = camino[i];
          int next = camino[(i + 1) % camino.size()]; 
          
          int cord1X = puntos[current].x * 10 + 400;
          int cord1Y = 1000 - puntos[current].y * 10;
          int cord2X = puntos[next].x * 10 + 400;
          int cord2Y = 1000 - puntos[next].y * 10;
          
          drawLine(cord1X, cord1Y, cord2X, cord2Y, colors[2]);
      }

      // Mostrar orden de los nodos en el camino
      int textX = windowWidth - 250;
      int textYStart = 50;
      int textXIncrement = 30;
      for (int i = 0; i < camino.size(); i++) {
          string text = to_string(camino[i]);
          if (camino[i] == inicio) {
              drawText(textX, textYStart, text, colors[1]); // Verde para inicio
          } else if (camino[i] == fin) {
              drawText(textX, textYStart, text, colors[3]); // Azul para fin
          } else {
              drawText(textX, textYStart, text, colors[2]); // Rojo para otros
          }
          textYStart += textXIncrement;
      }
  }

  // Dibujar los nodos (puntos)
  for (int i = 0; i < puntos.size(); i++) {
      int transformedX = puntos[i].x * 10 + 400;
      int transformedY = 1000 - puntos[i].y * 10;
      
      if (i == inicio) {
          drawPoint(transformedX, transformedY, colors[1]); // Verde para inicio
      } else if (i == fin) {
          drawPoint(transformedX, transformedY, colors[3]); // Azul para fin
      } else {
          drawPoint(transformedX, transformedY, colors[0]); // Blanco para otros
      }
  }

  int textX = windowWidth - 200;
  int textYStart = 50;
  int textYIncrement = 30;
  for (int i = 0; i < puntos.size(); i++) {
      string text = to_string(i) + " - X: " + to_string(puntos[i].x) + ", Y: " + to_string(puntos[i].y);
      drawText(textX, textYStart, text, colors[0]);
      textYStart += textYIncrement;
  }
}

void drawButton(int x, int y, int width, int height, const char *text, float color[3]) {
  glColor3f(color[0], color[1], color[2]);
  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(x + width, y);
  glVertex2f(x + width, y + height);
  glVertex2f(x, y + height);
  glEnd();

  // Draw text
  glColor3f(0, 0, 0);
  int textWidth = strlen(text) * 10;
  int textX = x + (width - textWidth) / 2;
  int textY = y + height / 2;
  glRasterPos2f(textX, textY);
  for (const char *c = text; *c != '\0'; c++) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
  }
}

void mouseClick(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    if (x >= button1X && x <= button1X + button1Width && y >= button1Y &&
        y <= button1Y + button1Height) {
      // Iniciar
      iniciar();
      glutPostRedisplay();

    } else if (x >= button2X && x <= button2X + button2Width && y >= button2Y &&
               y <= button2Y + button2Height) {
      drawGraphC = true;

      suma_fin.clear();
      media_fin.clear();
      max_min_fin.clear();
      generacion_actual = 0;
      // TSP

      tmp.assign(nodos, 0);
      for (int i = 0; i < nodos; i++) {
        index.push_back(i);
      }
      for (int i = 0; i < cantidad; i++) {
        vector<int> camino;
        shuffle(index.begin(), index.end(), default_random_engine(time(0)));
        for (int j = 0; j < nodos; j++) {
          if (index[j] != inicio && index[j] != fin) {
            camino.push_back(index[j]);
          }
        }
        do {
          shuffle(camino.begin(), camino.end(), default_random_engine(time(0)));
        } while (tmp == camino);
        tmp = camino;
        camino.insert(camino.begin(), inicio);
        camino.push_back(fin);
        individuo tmp(camino);
        poblacion.push_back(tmp);
      }
      caminofin = muta_fin(poblacion);
      glutPostRedisplay();
    }
  }
  if (puntos_cant < 3) {
    for (int i = 0; i < puntos.size(); i++) {
      int pointX = puntos[i].x * 10 + 400;
      int pointY = 1000 - puntos[i].y * 10;
      if (x >= pointX - point_radius && x <= pointX + point_radius && y >= pointY - point_radius &&
          y <= pointY + point_radius) {
        if (puntos_cant == 1) {
          inicio = i;
        } else {
          fin = i;
        }
        puntos_cant++;
      }
    }
  }
}

void drawConvergenceGraph() {
  if(media_fin.empty() || max_min_fin.empty()) return;

  int graphX = windowWidth - 500;
  int graphY = windowHeight - 300;
  int graphWidth = 450;
  int graphHeight = 250;

  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_LINE_LOOP);
  glVertex2f(graphX, graphY);
  glVertex2f(graphX + graphWidth, graphY);
  glVertex2f(graphX + graphWidth, graphY + graphHeight);
  glVertex2f(graphX, graphY + graphHeight);
  glEnd();

  double max_dist = *max_element(max_min_fin.begin(), max_min_fin.end());
  double min_dist = *min_element(max_min_fin.begin(), max_min_fin.end());
  double range = max_dist - min_dist;
  if(range == 0) range = 1; // Evitar división por cero

  glColor3f(0.5, 0.5, 0.5);
  glBegin(GL_LINES);
  glVertex2f(graphX, graphY + graphHeight);
  glVertex2f(graphX + graphWidth, graphY + graphHeight);
  glVertex2f(graphX, graphY);
  glVertex2f(graphX + graphWidth, graphY);
  glEnd();

  double stepX = graphWidth / (double)(media_fin.size() - 1);
  
  string info_gen = "Generacion: " + to_string(generacion_actual) + "/" + to_string(max_generaciones);
  drawText(graphX + 10, graphY - 20, info_gen, colors[0]);
  
  glColor3f(0.0, 0.0, 1.0);
  glBegin(GL_LINE_STRIP);
  for(int i = 0; i < media_fin.size(); i++) {
      double x = graphX + i * stepX;
      double y = graphY + graphHeight - ((media_fin[i] - min_dist) / range * graphHeight);
      glVertex2f(x, y);
  }
  glEnd();

  glColor3f(1.0, 0.0, 0.0);
  glBegin(GL_LINE_STRIP);
  for(int i = 0; i < max_min_fin.size(); i++) {
      double x = graphX + i * stepX;
      double y = graphY + graphHeight - ((max_min_fin[i] - min_dist) / range * graphHeight);
      glVertex2f(x, y);
  }
  glEnd();

  string mejor_str = "Mejor: " + to_string(max_min_fin.back());
  string media_str = "Media: " + to_string(media_fin.back());
    
  drawText(graphX + graphWidth - 150, graphY + 20, mejor_str, colors[2]); // Rojo
  drawText(graphX + graphWidth - 150, graphY + 50, media_str, colors[4]); // Azul

  // Dibujar leyenda
  //drawText(graphX + 10, graphY + 20, "Mejor: " + to_string(max_min_fin.back()), colors[2]);
  //drawText(graphX + 10, graphY + 50, "Media: " + to_string(media_fin.back()), colors[4]);
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  drawButton(button1X, button1Y, button1Width, button1Height, button1Text, colors[0]);
  drawButton(button2X, button2Y, button2Width, button2Height, button2Text, colors[0]);
  drawCartesianPlane();
  drawGraph(puntos, caminofin.camino);
  drawConvergenceGraph();
  glutSwapBuffers();
}

int main(int argc, char **argv) {
  cout << "=== ALGORITMO GENETICO ===" << endl;
  cout << "Ciudades: " << nodos << endl;
  cout << "Tamano poblacion: " << cantidad << endl;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("lab");
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, windowWidth, windowHeight, 0);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouseClick);
  glutMainLoop();
  return 0;
}