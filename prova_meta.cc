#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <queue>
#include <stdlib.h>

using namespace std;
const int infinit = 100000000;
typedef pair<int, int> parella;


struct Data
{
    int C;                              // nombre de cotxes
    int M;                              // nombre de millores
    int K;                              // nombre de classes
    vector<int> capacitat_finestra;     // v[i] indica el màxim nombre de cotxes que poden requerir la millora [i] en un conjunt de cotxes consecutius
    vector<int> mida_finestra;          // v[i] indica la mida del conjunt de cotxes consecutius per a la millora [i]
    vector<int> produccio;              // v[i] indica els cotxes que s'han de produïr de la classe [i]
    vector<vector<int>> classes;       // v[i][j] indica si la classe [i] requereix o no la millora [j]
};

Data dades;


void inicialitzar_dades(int C, int M, int K)
{
    dades.C = C;
    dades.M = M;
    dades.K = K;
    dades.capacitat_finestra = vector<int> (M);
    dades.mida_finestra = vector<int> (M);
    dades.produccio = vector<int> (K);
    dades.classes = vector<vector<int>> (K, vector<int>(M, false));
}


// Funció que llegeix les dades d'entrada, les guarda en  una variable de tipus Data i la retorn.
void llegir_dades(char** argv)
{
    ifstream in(argv[1]);
    int C, M, K;
    in >> C;
    in >> M;
    in >> K;
    inicialitzar_dades(C, M, K);
    for (int i = 0; i < M; ++i)
        in >> dades.capacitat_finestra[i];

    for (int i = 0; i < M; ++i)
        in >> dades.mida_finestra[i];

    for (int i = 0; i < K; ++i) {
        int aux;
        in >> aux;
        in >> dades.produccio[i];
        for (int j = 0; j < M; ++j) {
            in >> dades.classes[i][j];
        }
    }
    in.close();
}


// Funció que escriu un resultat del problema en el arxiu de sortida
void escriure_solucio(const vector<int>& sol, int cost, double temps, char** argv)
{
    ofstream out(argv[2]);
    out << cost << ' ' << fixed << setprecision(1) << temps << endl;
    out << sol[0];
    int n = sol.size();
    for (int i = 1; i < n; ++i)
        out << ' ' << sol[i];
    out << endl;
    out.close();
}


int calcular_penalitzacio_millora_total(int millora, const vector<int>& cotxes_a_millorar)
{
    int mida = dades.mida_finestra[millora];
    int capacitat = dades.capacitat_finestra[millora];
    int inici = 0;
    int final = capacitat;
    int penalitzacions = 0;
    // Penalitzacions del principi:
    while (final < mida - 1) {
        int contador = 0;
        for (int i = inici; i <= final; ++i) {
            if (cotxes_a_millorar[i] == 1)
                ++contador;
        }
        if (contador > capacitat)
            penalitzacions += contador - capacitat;
        ++final;
    }
    // Penalitzacions del mig:
    while (final < dades.C) {
        int contador = 0;
        for (int i = inici; i <= final; ++i) {
            if (cotxes_a_millorar[i] == 1)
                ++contador;
        }
        if (contador > capacitat)
            penalitzacions += contador - capacitat;
        ++inici;
        ++final;
    }
    final = dades.C - 1;
    // Penalitzacions del final:
    while (final - inici >= capacitat) {
        int contador = 0;
        for (int i = inici; i <= final; ++i) {
            if (cotxes_a_millorar[i] == 1)
                ++contador;
        }
        if (contador > capacitat)
            penalitzacions += contador - capacitat;
        ++inici;
    }
    return penalitzacions;
}

// Funció que donada una solució calcula la penalització total d'aquesta.
int calcular_penalitzacio_total(const vector<int>& sol)
{
    // Vector que per a cada millora indica quins cotxes de la permutació requereixen aquesta millora
    vector<int> cotxes_a_millorar(dades.C);
    int penalitzacions = 0;
    for (int i = 0; i < dades.M; ++i) {
        for (int j = 0; j < dades.C; ++j) {
            if (dades.classes[sol[j]][i] == 1)
                cotxes_a_millorar[j] = true;
            else
                cotxes_a_millorar[j] = false;
        }
        penalitzacions += calcular_penalitzacio_millora_total(i, cotxes_a_millorar);
    }
    return penalitzacions;
}


int calcular_penalitzacio_millora(int millora, const vector<int>& cotxes_a_millorar)
{
    int mida = dades.mida_finestra[millora];
    int capacitat = dades.capacitat_finestra[millora];
    int n = cotxes_a_millorar.size();
    // Cas en el que el nº de cotxes és inferior  o igual a la capacitat de la millora i per tant mai hi haurà cap penalització
    if (n <= capacitat)
        return 0;
    // Els altres casos, on podem tenir penalitzacions:
    int inici;
    if (n <= mida)
        inici = 0;
    else
        inici = n - mida;
    int final = n - 1;
    int penalitzacions = 0;
    int contador = 0;
    for (int i = inici; i <= final; ++i) {
        if (cotxes_a_millorar[i] == 1)
            ++contador;
    }
    if (contador > capacitat)
        penalitzacions += contador - capacitat;
    // Si n és igual al nº de cotxes, vol dir que ja és l'últim cotxe de la cadena de muntatge i que per tant hem de revisar que no hi hagin penalitzacions al final:
    if (n == dades.C) {
        ++inici;
        while (final - inici >= capacitat) {
            contador = 0;
            for (int i = inici; i <= final; ++i) {
                if (cotxes_a_millorar[i] == 1)
                    ++contador;
            }
            if (contador > capacitat) {
                penalitzacions += contador - capacitat;
            }
            ++inici;
        }
    }
    return penalitzacions;
}

// Funció que donada una solució i una posició de la solució calcula les penalitzacions que apareixen en la solució al afegir-hi la posició n.
int calcular_penalitzacio_afegida(const vector<int>& sol, int n)
{
    // Vector que per a cada millora indica quins cotxes de la permutació requereixen aquesta millora
    vector<int> cotxes_a_millorar(n + 1);
    int penalitzacions = 0;
    for (int i = 0; i < dades.M; ++i) {
        for (int j = 0; j < n + 1; ++j) {
            if (dades.classes[sol[j]][i] == 1)
                cotxes_a_millorar[j] = true;
            else
                cotxes_a_millorar[j] = false;
        }
        penalitzacions += calcular_penalitzacio_millora(i, cotxes_a_millorar);
    }
    return penalitzacions;
}


// Funció que construeix una solució aleatòria tenint en compte les penalitzacions afegides i la probabilitat alpha.
// FUNCIONA BÉ
vector<int> construir_solucio_aleatoria() {
    vector<int> solucio(dades.C);
    vector<int> no_usats = dades.produccio;
    // Agafem una alpha aleatòria entre 1 i dades.K:
    int alpha = rand() % dades.K + 1;
    // Posem el primer número aleatòriament:
    solucio[0] = rand() % dades.K;
    --no_usats[solucio[0]];
    // A continuació posem els altres números restants tenint en compte la penalització afegida:
    for (int i = 1; i < dades.C; ++i) {
        // Anem posant en la cua de prioritats totes les penalitzacions junt amb les classes (per després triar els millors):
        // Per defecte la cua s'ordena tenint en compte el primer element
        priority_queue<parella, vector<parella>, greater<parella>> candidats;
        for(int j = 0; j < dades.K; ++j) {
            if (no_usats[j] > 0) {
                int penalitzacio = calcular_penalitzacio_afegida(solucio, j);
                candidats.push({penalitzacio, j});
                
            }
        }
        // Triem un nombre al atzar de la cua entre els alpha primers números:
        int guanyador;
        int num_candidats = candidats.size();
        if (num_candidats < alpha)
            guanyador = rand() % candidats.size();
        else guanyador = rand() % alpha;
        for(int i = 0; i < guanyador; ++i) {
            candidats.pop();
        }
        solucio[i] = candidats.top().second;
        --no_usats[solucio[i]];
    }
    return solucio;
}

// Funció que donada una solució retorna un vector amb tots els veïns de la solució, entenent un veí com qualssevol permutació
// de la solució obtinguda intercanviant dues posicions qualssevols de la solució (si tenen valors diferents).
// FUNCIONA BÉ
vector<vector<int>> trobar_veins(const vector<int>& solucio) {
    vector<vector<int>> veins;
    for (int i = 0; i < dades.C; ++i) {
        for (int j = i; j < dades.C; ++j) {
            if (solucio[i] != solucio[j]) {
                vector<int> candidat = solucio;
                swap(candidat[i], candidat[j]);
                veins.push_back(candidat);
            }
        }
    }
    return veins;
}


// FALTA IMPLEMENTAR LO DE LA PROBABILITAT DE BOLTZMAN DISTRIBUTION
void simulated_annealing(const vector<int>& sol_inicial, int& min_penalitzacio, char** argv, unsigned t0) {
    vector<int> x = sol_inicial;
    int penalitzacio_x = calcular_penalitzacio_total(x);
    while (true) {
        vector<vector<int>> veins = trobar_veins(x);
        // Afegim un veí al atzar:
        int guanyador = rand() % veins.size();
        vector<int> y = veins[guanyador];
        int penalitzacio_y = calcular_penalitzacio_total(y);
        if (penalitzacio_y < penalitzacio_x) {
            x = y;
            penalitzacio_x = penalitzacio_y;
            if (penalitzacio_x < min_penalitzacio)
            {
                min_penalitzacio = penalitzacio_x;
                escriure_solucio(x, penalitzacio_x, t0, argv);
            }
        }   
        // Amb una certa probabiltat, ens quedem amb el veí:
        //else if (probabilitat > 1) x = y;
    }
    
}


void metaheuristica(char** argv, unsigned t0)
{
    int min_penalitzacio = infinit;
    while (true and min_penalitzacio != 0) {
        vector<int> solucio = construir_solucio_aleatoria();
        simulated_annealing(solucio, min_penalitzacio, argv, t0);
    }
}


int main(int argc, char** argv)
{
    if (argc != 3) {
        cout << "Syntax: " << argv[0] << " input_file outputfile" << endl;
        exit(1);
    }
    unsigned t0 = clock();
    llegir_dades(argv);
    metaheuristica(argv, t0);
}
