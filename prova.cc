#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

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

void inicialitzar_dades(Data& dades_aux, int C, int M, int K)
{
    dades_aux.C = C;
    dades_aux.M = M;
    dades_aux.K = K;
    dades_aux.capacitat_finestra = vector<int> (M);
    dades_aux.mida_finestra = vector<int> (M);
    dades_aux.produccio = vector<int> (K);
    dades_aux.classes = vector<vector<int>> (K, vector<int>(M, false));
}


// Funció que llegeix les dades d'entrada, les guarda en  una variable de tipus Data i la retorn.
Data llegir_dades(char** argv)
{
    Data dades_aux;
    ifstream in(argv[1]);
    int C, M, K;
    in >> C;
    in >> M;
    in >> K;
    inicialitzar_dades(dades_aux, C, M, K);
    for (int i = 0; i < M; ++i)
        in >> dades_aux.capacitat_finestra[i];

    for (int i = 0; i < M; ++i)
        in >> dades_aux.mida_finestra[i];

    for (int i = 0; i < K; ++i) {
        int aux;
        in >> aux;
        in >> dades_aux.produccio[i];
        for (int j = 0; j < M; ++j) {
            in >> dades_aux.classes[i][j];
        }
    }
    in.close();
    return dades_aux;
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


int calcular_penalitzacio_afegida(const vector<int>& sol, int n, int max)
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
        if (penalitzacions >= max)
            break;
    }
    return penalitzacions;
}

// Funció que troba la penalització mínima usant permutacions.
void permutacions_rec(int n, vector<int>& sol_parcial, int penalitzacio, vector<int>& no_produits, int& penalitzacio_minima, char** argv, unsigned t0)
{
    if (n == dades.C) {
        if (penalitzacio < penalitzacio_minima) {
            // Calculem el temps que ens ha tardat en trobar la solucio:
            unsigned t1 = clock();
            double temps = (double(t1 - t0) / CLOCKS_PER_SEC);
            // Escrivim el resultat en el document de sortida:
            escriure_solucio(sol_parcial, penalitzacio, temps, argv);
            penalitzacio_minima = penalitzacio;
        }
    } else {
        for (int i = 0; i < dades.K; ++i) {
            if (no_produits[i] > 0) {
                sol_parcial[n] = i;
                --no_produits[i];
                int penalitzacio_aux = penalitzacio + calcular_penalitzacio_afegida(sol_parcial, n, penalitzacio_minima - penalitzacio);
                if (penalitzacio_aux < penalitzacio_minima) {
                    permutacions_rec(n + 1, sol_parcial, penalitzacio_aux, no_produits, penalitzacio_minima, argv, t0);
                }
                ++no_produits[i];
            }
        }
    }
}


void permutacions(char** argv, unsigned t0)
{
    vector<int> sol_parcial(dades.C);
    // Per controlar quants cotxes ens queden de cada classe a posar en la permutació, utilitzem el següent vector. La posició produits[i] idica quants cotxes de i encara podem posar.
    vector<int> no_produits = dades.produccio;
    int penalitzacio_minima = infinit;
    permutacions_rec(0, sol_parcial, 0, no_produits, penalitzacio_minima, argv, t0);
}


int main(int argc, char** argv)
{
    if (argc != 3) {
        cout << "Syntax: " << argv[0] << " input_file outputfile" << endl;
        exit(1);
    }
    unsigned t0 = clock();
    dades = llegir_dades(argv);
    permutacions(argv, t0);
}
