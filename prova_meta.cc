#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <cmath>

using namespace std;
const int infinit = 100000000;

struct Data {
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



typedef pair<int, int> parella;

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

void simulated_annealing(const vector<int> &solucio_inicial, int &min_penalitzacio, char **argv, unsigned t0)
{
    vector<int> solucio_actual = solucio_inicial;
    int penalitzacio_actual = calcular_penalitzacio_total(solucio_actual);
    double alpha = 0.95;
    const double e = 2.718281828;
    for (double T = 100; T > 0.0000008; T *= alpha) 
    {
        // Per no haver de calcular els mateixos veins més d'ún cop:
        bool canviat = false;
        vector<vector<int>> veins = trobar_veins(solucio_actual);
        while (not canviat)
        {
            // Afegim un veí al atzar:
            int vei_triat = rand() % veins.size();
            vector<int> nova_solucio = veins[vei_triat];
            int penalitzacio_nova_solucio = calcular_penalitzacio_total(nova_solucio);
            if (penalitzacio_nova_solucio < penalitzacio_actual)
            {
                canviat = true;
                solucio_actual = nova_solucio;
                penalitzacio_actual = penalitzacio_nova_solucio;
                if (penalitzacio_actual < min_penalitzacio)
                {
                    min_penalitzacio = penalitzacio_actual;
                    cout << min_penalitzacio << endl;
                    unsigned t1 = clock(); // Rellotge que marca el temps de finalització del programa
                    double temps = (double(t1 - t0) / CLOCKS_PER_SEC);
                    escriure_solucio(solucio_actual, min_penalitzacio, temps, argv);
                }
            }
            else
            {
                double prob = pow(e, -(penalitzacio_nova_solucio - penalitzacio_actual) / T); 
                if ((rand() / (double)RAND_MAX) <= prob)
                {
                    canviat = true;
                    solucio_actual = nova_solucio;
                    penalitzacio_actual = penalitzacio_nova_solucio;
                }
            }
        }
    }
}


void metaheuristica(char** argv, unsigned t0)
{
    int min_penalitzacio = infinit;
    while (min_penalitzacio != 0) {
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
