#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_set>
#include <map>
#include <algorithm> // Para std::find

using namespace sf;
using namespace std;

int cont = 0;
RenderWindow window(VideoMode(800, 700), "SFML Pathfinding Example");
Event event;

struct Circle {
    CircleShape cir;
    pair<int, int> cord;
    Circle(int x = 0, int y = 0) {
        cir.setRadius(2.5);
        cir.setFillColor(Color::Red);
        cord = { x, y };
    }
    void set(int x, int y) {
        cord = { x, y };
        cir.setPosition(x, y);
    }
    void draw() { window.draw(cir); }
    bool clicked(int x, int y) {
        FloatRect bounds = cir.getGlobalBounds();
        return bounds.contains(x, y);
    }
};

template <class T> struct node;
template <class T>
struct edge {
    float v;
    vector<node<T>*> nodes;
    edge(float _v, node<T>* n1, node<T>* n2) {
        v = _v;
        nodes.push_back(n1);
        nodes.push_back(n2);
    }
};
template <class T>
struct node {
    T v;
    vector<edge<T>*> edges;
    Circle cir;
    pair<int, int> cord;
    node(T _v, int x = 0, int y = 0) : v(_v) {}
    void set(int x, int y) {
        cord = { x, y };
        cir.set(cord.first, cord.second);
    }
};
template <class T>
struct graph {
    vector<edge<T>*> edges;
    vector<node<T>*> nodes;
    vector<node<T>*> pares;
    graph() { pares.resize(2, nullptr); }
    void ins_node(T v) {
        node<T>* nod = new node<T>(v);
        nodes.push_back(nod);
    }
    node<T>* find(T v) {
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i]->v == v)
                return nodes[i];
        }
        return nullptr;
    }
    void ins_edge(T a, T b, float v = 1) {
        node<T>* n1 = find(a);
        node<T>* n2 = find(b);
        if (n1 && n2) {
            edge<T>* tmp = new edge<T>(v, n1, n2);
            edges.push_back(tmp);
            n1->edges.push_back(tmp);
            n2->edges.push_back(tmp);
        }
    }
    void setpos(T a, int x, int y) {
        node<T>* tmp = find(a);
        if (tmp) tmp->set(x, y);
    }
    void drawe() {
        for (int i = 0; i < edges.size(); i++) {
            VertexArray line(LineStrip);
            line.append(Vertex(Vector2f(edges[i]->nodes[0]->cord.first + 2.5f, edges[i]->nodes[0]->cord.second + 2.5f), Color::Red));
            line.append(Vertex(Vector2f(edges[i]->nodes[1]->cord.first + 2.5f, edges[i]->nodes[1]->cord.second + 2.5f), Color::Red));
            window.draw(line);
        }
    }
    void drawn() {
        for (int i = 0; i < nodes.size(); i++) {
            nodes[i]->cir.draw();
        }
    }
    void clicked() {
        if (event.type == Event::MouseButtonPressed) {
            if (event.mouseButton.button == Mouse::Left) {
                // Convertir las coordenadas del pixel a coordenadas del mundo (view)
                Vector2i pixelPos = { event.mouseButton.x, event.mouseButton.y };
                Vector2f worldPos = window.mapPixelToCoords(pixelPos);
                for (int i = 0; i < nodes.size(); i++) {
                    if (nodes[i]->cir.clicked(worldPos.x, worldPos.y)) {
                        if (pares[0] == nullptr) {
                            pares[0] = nodes[i];
                            nodes[i]->cir.cir.setFillColor(Color::Green);
                        }
                        else if (pares[1] == nullptr && nodes[i] != pares[0]) {
                            pares[1] = nodes[i];
                            nodes[i]->cir.cir.setFillColor(Color::Blue);
                        }
                        break;
                    }
                }
            }
        }
    }

    void color_path(const vector<node<T>*>& path) {
        for (int i = 0; i < path.size() - 1; i++) {
            VertexArray line(LineStrip);
            line.append(Vertex(Vector2f(path[i]->cord.first + 2.5f, path[i]->cord.second + 2.5f), Color::Green));
            line.append(Vertex(Vector2f(path[i + 1]->cord.first + 2.5f, path[i + 1]->cord.second + 2.5f), Color::Green));
            window.draw(line);
        }
    }
    bool ciclo(vector<node<T>*>& v, node<T>* n) {
        for (int i = 0; i < v.size(); i++) {
            if (v[i] == n) return true;
        }
        return false;
    }

    template <class T_HC>
    vector<node<T_HC>*> Hill_Climbing_Rev3(T_HC a, T_HC b) {
        node<T_HC>* ini = find(a);
        node<T_HC>* fin = find(b);

        if (!ini || !fin) {
            cout << "Error HillClimbing: Nodo inicial o final no encontrado." << endl;
            return {};
        }

        vector<node<T_HC>*> path;                
        std::unordered_set<node<T_HC>*> path_set;

        // Funciones auxiliares para mantener sincronizados el vector y el conjunto
        auto push_node = [&](node<T_HC>* n) {
            path.push_back(n);
            path_set.insert(n);
            };
        auto pop_node = [&]() {
            if (!path.empty()) {
                path_set.erase(path.back()); 
                path.pop_back();
            }
            };

        push_node(ini); 

        // Distancia al cuadrado
        auto distance_sq = [](node<T_HC>* n1, node<T_HC>* n2) -> float {
            float dx = static_cast<float>(n2->cord.first - n1->cord.first);
            float dy = static_cast<float>(n2->cord.second - n1->cord.second);
            return dx * dx + dy * dy;
            };

        int backtrackCount = 0;
        const int maxBacktrack = 2500; // Límite de retroceso aumentado considerablemente
        int sidewaysMovesCount = 0;
        const int maxSidewaysMoves = 10; // Permitir más movimientos laterales
        unordered_set<node<T_HC>*> backtracked_from_nodes; // Nodos de los que retrocedimos

        while (!path.empty() && path.back() != fin) {
            node<T_HC>* current = path.back();

            if (path.size() > 800) { // Ajusta este límite si es necesario
                cout << "Hill Climbing: Camino explorado demasiado largo (>800 nodos)." << endl;
                return {};
            }


            float currentDistSq = distance_sq(current, fin);
            node<T_HC>* bestNeighbor = nullptr;
            float bestDistSq = currentDistSq;
            node<T_HC>* sidewaysNeighbor = nullptr;

            for (auto edge : current->edges) {
                node<T_HC>* neighbor = (edge->nodes[0] == current) ? edge->nodes[1] : edge->nodes[0];

                if (path_set.count(neighbor)) {
                    continue; 
                }

                // Evitar re-explorar inmediatamente nodos desde los que acabamos de retroceder
                if (backtracked_from_nodes.count(neighbor)) {
                    continue;
                }

                float neighborDistSq = distance_sq(neighbor, fin);

                // Comprobar si es mejor vecino
                if (neighborDistSq < bestDistSq) {
                    bestDistSq = neighborDistSq;
                    bestNeighbor = neighbor;
                    sidewaysNeighbor = nullptr; 
                }

                // Comprobar si es un movimiento lateral válido (si no hay uno mejor)
                else if (bestNeighbor == nullptr && neighborDistSq == currentDistSq && sidewaysMovesCount < maxSidewaysMoves) {
                    if (sidewaysNeighbor == nullptr) {
                        sidewaysNeighbor = neighbor;
                    }
                }
            } 

            if (bestNeighbor) {
                push_node(bestNeighbor);
                backtrackCount = 0;
                sidewaysMovesCount = 0;
                backtracked_from_nodes.clear(); 
            }
            else if (sidewaysNeighbor) {
                push_node(sidewaysNeighbor);
                backtrackCount = 0;
                sidewaysMovesCount++;
                backtracked_from_nodes.clear(); 
            }
            else {

                // backtrack
                if (path.size() > 1) {
                    backtracked_from_nodes.insert(current); // retrocedemos desde aqui
                    pop_node(); 
                    backtrackCount++;
                    sidewaysMovesCount = 0; // reset contador lateral
                }
                else {
                    cout << "Hill Climbing Rev3 falló: Atascado en el inicio." << endl;
                    return {};
                }
                // comprobar límite de retroceso
                if (backtrackCount > maxBacktrack) {
                    cout << "Hill Climbing Rev3 falló: Límite máximo de retrocesos (" << maxBacktrack << ") alcanzado." << endl;
                    return {};
                }
            }
        } 

        if (!path.empty() && path.back() == fin) {
            return path; // Éxito
        }
        else {
            if (!path.empty()) {
                cout << "Hill Climbing Rev3 terminó pero no llegó al destino." << endl;
            }
            return {}; 
        }
    }

    
    vector<node<T>*> Astar(T a, T b) {
        node<T>* start = find(a);
        node<T>* goal = find(b);

        // Estructuras para reconstruir el camino y guardar los costos
        map<node<T>*, node<T>*> came_from;
        map<node<T>*, float> cost_so_far;

        // Cola de prioridad (min-heap) para los nodos abiertos
        auto cmp = [](const pair<float, node<T>*>& a, const pair<float, node<T>*>& b) {
            return a.first > b.first;
            };
        priority_queue<pair<float, node<T>*>, vector<pair<float, node<T>*>>, decltype(cmp)> open(cmp);

        open.push({ 0, start });
        came_from[start] = nullptr;
        cost_so_far[start] = 0;

        while (!open.empty()) {
            node<T>* current = open.top().second;
            open.pop();

            if (current == goal)
                break;

            // Recorremos vecinos
            for (auto edge : current->edges) {
                node<T>* neighbor = (edge->nodes[0] == current) ? edge->nodes[1] : edge->nodes[0];
                float new_cost = cost_so_far[current] + edge->v;
                if (cost_so_far.find(neighbor) == cost_so_far.end() || new_cost < cost_so_far[neighbor]) {
                    cost_so_far[neighbor] = new_cost;
                    float heuristic = sqrt(pow(goal->cord.first - neighbor->cord.first, 2) +
                        pow(goal->cord.second - neighbor->cord.second, 2));
                    float priority = new_cost + heuristic;
                    open.push({ priority, neighbor });
                    came_from[neighbor] = current;
                }
            }
        }

        // Reconstrucción del camino
        vector<node<T>*> path;
        if (came_from.find(goal) == came_from.end())
            return path; // no se encontró camino

        for (node<T>* cur = goal; cur != nullptr; cur = came_from[cur]) {
            path.push_back(cur);
        }
        reverse(path.begin(), path.end());
        return path;
    }

    vector<node<T>*> BP(T a, T b) {
        stack<vector<node<T>*>> s;
        node<T>* ini = find(a);
        node<T>* fin = find(b);
        s.push(vector<node<T>*>{ini});
        while (s.top()[0] != fin) {
            vector<node<T>*> aux = s.top();
            s.pop();
            if (aux[0]->edges.size() > 0) {
                for (auto& it : aux[0]->edges) {
                    vector<node<T>*> tmp = aux;
                    if (it->nodes[0] == aux[0] && !ciclo(tmp, it->nodes[1])) {
                        tmp.insert(tmp.begin(), it->nodes[1]);
                        s.push(tmp);
                    }
                    else if (it->nodes[1] == aux[0] && !ciclo(tmp, it->nodes[0])) {
                        tmp.insert(tmp.begin(), it->nodes[0]);
                        s.push(tmp);
                    }
                }
            }
        }
        return s.top();
    }
    
    template <class T_BFS> 
    vector<node<T_BFS>*> BFS_Optimized(T_BFS a, T_BFS b) {
        node<T_BFS>* start_node = find(a);
        node<T_BFS>* goal_node = find(b);

        if (!start_node || !goal_node) {
            cout << "Error BFS: Nodo inicial o final no encontrado." << endl;
            return {};
        }

        std::queue<node<T_BFS>*> q;                    
        std::unordered_set<node<T_BFS>*> visited;     
        std::map<node<T_BFS>*, node<T_BFS>*> parent_map; 

        q.push(start_node);
        visited.insert(start_node);
        parent_map[start_node] = nullptr; // El nodo inicial no tiene padre

        bool found = false;

        while (!q.empty()) {
            node<T_BFS>* current_node = q.front();
            q.pop();

            if (current_node == goal_node) {
                found = true;
                break; 
            }

            // Explorar vecinos del nodo actual
            for (auto& edge : current_node->edges) {
                node<T_BFS>* neighbor = nullptr;
                if (edge->nodes[0] == current_node) {
                    neighbor = edge->nodes[1];
                }
                else if (edge->nodes[1] == current_node) {
                    neighbor = edge->nodes[0];
                }
                else continue; 

                // Si el vecino NO ha sido visitado aún
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);          // Marcar como visitado
                    parent_map[neighbor] = current_node; // Guardar quién es su padre en el camino
                    q.push(neighbor);                  // Añadir a la cola para explorar después
                }
            }
        } 

        vector<node<T_BFS>*> path;
        if (found) {
            node<T_BFS>* crawl = goal_node;
            // Retroceder desde el nodo final hasta el inicial usando el mapa de padres
            while (crawl != nullptr) {
                path.push_back(crawl);
                // Moverse al padre del nodo actual
                 // Comprobación de seguridad por si el mapa está incompleto (no debería pasar si found=true)
                if (parent_map.count(crawl)) {
                    crawl = parent_map[crawl];
                }
                else {
                   
                    if (crawl != start_node) {
                        cout << "Error BFS: Fallo en reconstrucción de camino." << endl;
                        return {}; 
                    }
                    crawl = nullptr; 
                }
            }
            // El camino se reconstruyó de fin -> inicio, así que lo invertimos
            std::reverse(path.begin(), path.end());
        }
        else {
            cout << "BFS_Optimized: No se encontró camino." << endl;
        }

        return path; 
    }


    void del_node(T a) {
        node<T>* target = find(a);
        if (!target) return;
        for (auto it = edges.begin(); it != edges.end();) {
            if ((*it)->nodes[0] == target || (*it)->nodes[1] == target) {
                edge<T>* temp = *it;
                it = edges.erase(it);
                for (auto n : nodes) {
                    n->edges.erase(remove(n->edges.begin(), n->edges.end(), temp), n->edges.end());
                }
                delete temp;
            }
            else ++it;
        }
        nodes.erase(remove(nodes.begin(), nodes.end(), target), nodes.end());
        if (pares[0] == target) pares[0] = nullptr;
        if (pares[1] == target) pares[1] = nullptr;
        delete target;
    }
    void del_random(int r) {
        srand(time(NULL));
        for (int i = 0; i < r; i++) {
            if (nodes.empty()) break;
            int index = rand() % nodes.size();
            T v = nodes[index]->v;
            del_node(v);
        }
    }
};

int main() {
    int nodos = 20;
    graph<int> G;
    for (int i = 0; i < nodos * nodos; i++) {
        G.ins_node(i);
    }
    for (int j = 0; j < nodos; j++) {
        for (int i = 0; i < nodos; i++) {
            G.setpos(i + (j * nodos), 10 + (i * 10), 10 + (j * nodos));
        }
    }
    for (int j = 0; j < nodos - 1; j++) {
        for (int i = 0; i < nodos - 1; i++) {
            G.ins_edge(i + (j * nodos), (j * nodos) + i + 1, 10);
            G.ins_edge(i + (j * nodos), ((j + 1) * nodos) + i, 10);
            if ((i + j) % 2 == 0) {
                G.ins_edge(i + (j * nodos), ((j + 1) * nodos) + i + 1, 14.1421f);
            }
        }
    }
    G.del_random(100);
    bool pathCalculated = false;
    vector<node<int>*> gr;
    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            G.clicked();
        }
        window.clear();
        G.drawe();
        G.drawn();
        if (G.pares[0] && G.pares[1] && !pathCalculated) {
            //gr = G.Hill_Climbing_Rev3(G.pares[0]->v, G.pares[1]->v);
            //gr = G.Astar(G.pares[0]->v, G.pares[1]->v);
            //gr = G.BP(G.pares[0]->v, G.pares[1]->v);
            //gr = G.BFS_Optimized(G.pares[0]->v, G.pares[1]->v);
            cout << "Path: ";
            for (auto n : gr)
                cout << n->v << " -> ";
            cout << "FIN" << endl;
            pathCalculated = true;
        }
        if (!gr.empty()) G.color_path(gr);
        window.display();
    }
    return 0;
}
