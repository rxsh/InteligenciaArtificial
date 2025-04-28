#include <time.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int max_generaciones = 100; 
int generacion_actual = 0;  
double umbral_convergencia = 0.01; 
int generacion_sin_mejora = 0;
double mejor_distancia_global = INFINITY;

struct nodo {
  int color;
};

struct par {
  float cant;
  int index;
};
int contar(vector<int> a, int n) {
  int cont1 = 0;
  for (int i = 0; i < n; i++) {
    if (a[i] == 1) {
      cont1++;
    }
  }
  return cont1;
}
void contar2(vector<vector<int>> aristas, vector<par>& index, int n) {
  for (int i = 0; i < n; i++) {
    index[i].cant = contar(aristas[i], n);
    index[i].index = i;
  }
}
bool compara(par a, par b) { return a.cant > b.cant; }
bool comprob(vector<nodo> nodos, nodo b, vector<int> aristas, int n) {
  for (int i = 0; i < n; i++) {
    if (aristas[i] && b.color == nodos[i].color) {
      return true;
    }
  }
  return false;
}

void var(vector<par>& indices, vector<nodo> nodos, vector<vector<int>> aristas, int color, int n) {
  for (int i = 0; i < n; i++) {
    vector<int> sw(color);
    indices[i].index = i;
    float divisor = color;
    float dividendo = 0;
    for (int j = 0; j < n; j++) {
      if (aristas[i][j]) {
        if (nodos[j].color != 0) {
          sw[nodos[j].color - 1] = 1;
        }
      }
    }
    for (int j = 0; j < color; j++) {
      dividendo += sw[j];
    }
    indices[i].cant = dividendo / divisor;
  }
  sort(indices.begin(), indices.end(), compara);
}
