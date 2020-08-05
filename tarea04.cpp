#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>

using namespace std;

/////////////////////////////////////////////

class User {
    public:
        string name;
        list<User*> relatives;
        bool tested;
};

/////////////////////////////////////////////

class Graph {


    private:

        unordered_map<string, User*> _user_indexes; // tabla hash con punteros a los usuarios
        list<User> _users;                          // lista de usuarios

    public:

        void Add(string, string);
        string Find(string);
        vector<vector<string>> Clique(void);
        void Bron_Kerbosch(vector<string>, vector<string>, vector<string>, vector<vector<string>>*);
        void Compact(void);
        void Follow(int);
        int dfs_rec(User*, string);
        void print_graph(void);

};

/////////////////////////////////////////////

void Graph::Add(string user1, string user2){

    User temp;

    // si user1 no existe se crea
    if(_user_indexes.count(user1) == 0) {
        temp = User();
        temp.name = user1;
        _users.push_back(temp); // se agrega a la lista de usuarios
        _user_indexes[user1] = &(_users.back()); // se agrega el puntero a la tabla hash
    }

    // si user2 no existe se crea
    if(_user_indexes.count(user2) == 0) {
        temp = User();
        temp.name = user2;
        _users.push_back(temp); // se agrega a la lista de usuarios
        _user_indexes[user2] = &(_users.back()); // se agrega el puntero a la tabla hash
    }

    // si no existe, se incluye la relacion para ambos usuarios
    list<User*>::iterator it;
    it = find(_user_indexes[user1]->relatives.begin(),
              _user_indexes[user1]->relatives.end(),
              _user_indexes[user2]);

    if(it == _user_indexes[user1]->relatives.end()){
        _user_indexes[user1]->relatives.push_back(_user_indexes[user2]);
        _user_indexes[user2]->relatives.push_back(_user_indexes[user1]);
    }

}

/////////////////////////////////////////////

// Busca si el usuario es o no parte de la red social.
// Utilizar uno de los métodos de recorrido para grafos no dirigidos.
string Graph::Find(string userName){
    string out = "No";
    // primero se marcan todos los nodos como no revisados
    list<User>::iterator iter;
    for(iter = _users.begin(); iter!=_users.end(); iter++){
        iter->tested = false;
    }
    // se aplica DFS a la lista completa para considerar
    // distintos nodos de inicio, asi se prueban nodos no conexos
    for(iter = _users.begin(); iter!=_users.end(); iter++){
        // depth first search recursivo
        if(dfs_rec(&(*iter),userName)) {
            out = "Yes";
            break;
        }
    }
    return(out);
}

// Algoritmo de Busqueda por Profundad
int Graph::dfs_rec(User* user, string userName){
    // solo se trabaja sobre vertices no marcados
    if(!user->tested){
        user->tested = true;        // se marca el vertice recorrido
        if(user->name == userName)  // si se encuentra el vertice buscado se retorna 1
            return(1);
        // si no se a encontrado el vertice se continua la busqueda en los nodos vecinos
        list<User*>::iterator iter;
        for(iter=user->relatives.begin(); iter!=user->relatives.end(); iter++){
            if(dfs_rec(*iter,userName)) {
                return(1);
            }
        }
    }
    return(0);
}

/////////////////////////////////////////////

// Bron–Kerbosch
void Graph::Bron_Kerbosch(vector<string> R, vector<string> P, vector<string> X, vector<vector<string>> *output) {

    // se retorna Clique Maximal
    if(P.empty() && X.empty() && R.size() > 2 && R.size() < 6){
        output->push_back(R);
    }

    vector<string>::iterator iter;
    while(!P.empty()){      // mientras existan vértices por probar

        vector<string> v;   // {v}
        vector<string> Nv;  // {vecinos de v}
        vector<string> R2;
        vector<string> P2;
        vector<string> X2;
        vector<string> temp;

        iter = P.begin();
        v.push_back(*iter);

        list<User*>::iterator iter2;
        for(iter2 = _user_indexes[*iter]->relatives.begin(); iter2 != _user_indexes[*iter]->relatives.end(); iter2++){
            Nv.push_back((*iter2)->name);
        }
        sort(Nv.begin(),Nv.end());

        // se calculan las uniones e intersecciones de los conjuntos
        set_union(R.begin(), R.end(), v.begin(), v.end(), back_inserter(R2));          // R2 = R U {v}
        set_intersection(P.begin(), P.end(), Nv.begin(), Nv.end(), back_inserter(P2)); // P2 = P inter N{v}
        set_intersection(X.begin(), X.end(), Nv.begin(), Nv.end(), back_inserter(X2)); // X2 = X inter N{v}

        // llamada recursiva a Bron_Kerbosch
        Bron_Kerbosch(R2, P2, X2, output);

        //set_difference(P.begin(), P.end(), v.begin(), v.end(), back_inserter(temp));
        //P = temp;
        P.erase(iter);
        set_union(X.begin(), X.end(), v.begin(), v.end(), back_inserter(temp));
        X = temp;
        //X.push_back(*iter);
        //sort(X.begin(),X.end());
    }
}

// busqueda de cliques maximales
vector<vector<string>> Graph::Clique(void){

        vector<string> R, P, X;
        vector<vector<string>> output;

        list<User>::iterator iter;
        for(iter=_users.begin(); iter!=_users.end(); iter++){
            P.push_back(iter->name);
        }
        sort(P.begin(),P.end());

        Bron_Kerbosch(R, P, X, &output);

        return(output);

}

/////////////////////////////////////////////

void Graph::Compact(void){

    // se calculan los cliques del grafo
    vector<vector<string>> cliques;
    cliques = Clique();

    // se guardan los resultados de clique en una tabla hash para acelerar la busqueda de nodos virtuales
    unordered_map<string, string> _virtual;     // tabla hash de pertenencia a nodos virtuales
    vector<vector<string>>::iterator iter1;
    vector<string>::iterator iter2;
    int componentCtr = 1;
    for(iter1=cliques.begin(); iter1!=cliques.end();iter1++){
        for(iter2=iter1->begin(); iter2!=iter1->end(); iter2++){
            _virtual[*iter2] = "Component" + to_string(componentCtr);
        }
        componentCtr++;
    }

    // se crea un grafo donde se copian los nodos renombrados segun la tabla de nombres virtuales
    Graph graph;
    list<User>::iterator iter3;
    list<User*>::iterator iter4;
    string userName;
    string relativeName;

    for(iter3=_users.begin(); iter3!=_users.end(); iter3++){
        for(iter4=iter3->relatives.begin(); iter4!=iter3->relatives.end(); iter4++){

            // se busca si el usuario y sus relativos se encuentran en alguno de los vectores de cliques
            userName = iter3->name;
            relativeName = (*iter4)->name;

            if(_virtual.count(iter3->name)) // si el usuario es parte del click
                userName = _virtual[iter3->name];

            if(_virtual.count((*iter4)->name)) // si el usuario relativo es parte del click
                relativeName = _virtual[(*iter4)->name];

            if(userName != relativeName)
                graph.Add(userName, relativeName);
        }
    }

    // se imprimen los nodos y sus relaciones
    graph.print_graph();

}

/////////////////////////////////////////////

// funcion de comparacion de relaciones
bool cmpr(User x, User y) {
    if(x.relatives.size() > y.relatives.size())
        return(true);
    else
        return(false);
}

// Follow entrega los n usuarios con mayor cantidad de relaciones
void Graph::Follow(int n){
    _users.sort(cmpr);  // se ordena la lista de usuarios usando sort y funcion cmpr
    list<User>::iterator iter;
    int ctr = 0;
    // se imprimen los n usuarios con mayor cantidad de relaciones
    for(iter = _users.begin(); iter != _users.end() && ctr < n ; iter++) {
        cout << iter->name << endl;
        ctr++;
    }
}

/////////////////////////////////////////////

void Graph::print_graph(void){

    list<User>::iterator iter1;
    list<User*>::iterator iter2;
    for(iter1=_users.begin(); iter1!=_users.end(); iter1++){
        for(iter2=iter1->relatives.begin(); iter2!=iter1->relatives.end(); iter2++){
            cout << "(" << iter1->name << ", " << (*iter2)->name << ")" << endl;
        }
    }
}


/////////////////////////////////////////////

int main (int argc, char** argv) {

    Graph graph;                // grafo correspondiente a la red de usuarios
    string A, B, word;          // variables temporales para lectura
    ifstream cin("input.txt");  // archivo de entrada

    // loop de lectura de comandos
    while(cin >> word ){

        // Add: se agrega la relacion de usuarios al grafo
        if(word == "Add") {
            cin >> A;
            cin >> B;
            graph.Add(A,B);

        // Find: realiza busqueda del usuario usando DFS
        } else if(word == "Find") {
            cin >> A;
            cout << graph.Find(A) << endl;

        // Clique: devuelve los clique maximales
        } else if(word == "Clique") {
            vector<vector<string>> output;
            vector<vector<string>>::iterator iter;
            vector<string>::iterator iter2;
            output = graph.Clique();
            for(iter=output.begin(); iter!=output.end();iter++){
                for(iter2=iter->begin(); iter2!=iter->end(); iter2++){
                    cout << *iter2 << " ";
                }
                cout << endl;
            }

        // Follow: devuelve n usuarios con el mayor numero de seguidores
        } else if(word == "Follow") {
            cin >> A;
            stringstream followers(A);
            int x = 0;
            followers >> x;
            graph.Follow(x);

        // Compact: entrega una representacion compacta del grafo
        } else if(word == "Compact") {
            graph.Compact();
        }

    }

    return(0);

}
