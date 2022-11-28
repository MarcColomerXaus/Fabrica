#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

const int infi = 100000000;

struct Data {
    int C;                              // nombre de cotxes
    int M;                              // nombre de millores
    int K;                              // nombre de classes
    vector<int> capacitat_finestra;     // v[i] indica el màxim nombre de cotxes que poden requerir la millora [i] en un conjunt de cotxes consecutius
    vector<int> mida_finestra;          // v[i] indica la mida del conjunt de cotxes consecutius per a la millora [i]
    vector<int> produccio;              // v[i] indica els cotxes que s'han de produïr de la classe [i]
    vector<vector<int>> classes;       // v[i][j] indica si la classe [i] requereix o no la millora [j]
};

void inicialitzar_dades(Data& dades, int C, int M, int K)
{
    dades.C = C;
    dades.M = M;
    dades.K = K;
    dades.capacitat_finestra = vector<int> (M);
    dades.mida_finestra = vector<int> (M);
    dades.produccio = vector<int> (K);
    dades.classes = vector<vector<int>> (K, vector<int>(M, false));
}


// Funció que llegeix les dades d'entrada, les guarda en  una variable de tipus Data i la retorna.
Data llegir_dades(char** argv)
{
    Data dades;
    ifstream in(argv[1]);
    int C, M, K;
    in >> C;
    in >> M;
    in >> K;
    inicialitzar_dades(dades, C, M, K);
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
    return dades;
}


// Funció que escriu un resultat del problema en el arxiu de sortida
void escriure_resultat(const vector<int>& sol, int cost, double temps, char** argv)
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


int calcular_cost_millora(const Data& dades, int millora, const vector<int>& cotxes_a_millorar)
{
    int mida = dades.mida_finestra[millora];
    int capacitat = dades.capacitat_finestra[millora];
    int n = cotxes_a_millorar.size();
    // Cas en el que el nº de cotxes és inferior  o igual a la capacitat de la millora i per tant mai hi haurà cap penalització
    if (n <= capacitat)
        return 0;
    // Els altres casos, on podem tenir penalitzacions:
    int final = n - 1;
    int inici;
    if (n <= mida)
        inici = 0;
    else
        inici = final - mida + 1;
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

// Funció que calcula la penalització total de la permutació, sumant les penalitzacions associades a cadascuna de les millores
int calcular_cost_total(const Data& dades, const vector<int>& sol, int n)
{
    // Vector que per a cada millora indica quins cotxes de la permutació requereixen aquesta millora
    vector<int> cotxes_a_millorar(n + 1);
    int penalitzacio_total = 0;
    for (int i = 0; i < dades.M; ++i) {
        for (int j = 0; j < n + 1; ++j) {
            if (dades.classes[sol[j]][i] == 1)
                cotxes_a_millorar[j] = true;
            else
                cotxes_a_millorar[j] = false;
        }
        penalitzacio_total += calcular_cost_millora(dades, i, cotxes_a_millorar);
    }
    return penalitzacio_total;
}

// Funció que calcula totes les permutacions de cotxes possibles de forma recursiva
void permutacions_rec(const Data& dades, int n, vector<int>& sol_parcial, int cost_parcial, vector<int>& no_produits, int& min_penalitzacio, char** argv, unsigned t0)
{
    if (n == dades.C) {
        if (cost_parcial < min_penalitzacio) {
            // Calculem el temps que ens ha tardat en trobar la solucio:
            unsigned t1 = clock();
            double temps = (double(t1 - t0) / CLOCKS_PER_SEC);
            // Escrivim el resultat en el doc de sortida
            escriure_resultat(sol_parcial, cost_parcial, temps, argv);
            min_penalitzacio = cost_parcial;
        }
    } else {
        for (int i = 0; i < dades.K; ++i) {
            if (no_produits[i] > 0) {
                sol_parcial[n] = i;
                --no_produits[i];
                int cost_parcial_aux = cost_parcial + calcular_cost_total(dades, sol_parcial, n);
                if (cost_parcial_aux < min_penalitzacio) {
                    permutacions_rec(dades, n + 1, sol_parcial, cost_parcial_aux, no_produits, min_penalitzacio, argv, t0);
                }
                ++no_produits[i];
            }
        }
    }
}


void permutacions(const Data& dades, char** argv, unsigned t0)
{
    vector<int> sol_parcial(dades.C);
    // Per controlar quants cotxes ens queden de cada classe a posar en la permutació, utilitzem el següent vector. La posició produits[i] idica quants cotxes de i encara podem posar.
    vector<int> no_produits = dades.produccio;
    int min_penalitzacio = infi;
    permutacions_rec(dades, 0, sol_parcial, 0, no_produits, min_penalitzacio, argv, t0);
}


void treure_dades(Data dades)
{
    cout << dades.C << ' ' << dades.M << ' ' << dades.K << endl;
    for (int x : dades.capacitat_finestra)
        cout << x << ' ';
    cout << endl;
    for (int x : dades.mida_finestra)
        cout << x << ' ';
    cout << endl;
    for (int x : dades.produccio)
        cout << x << ' ';
    cout << endl;
    for (int i = 0; i < dades.K; ++i) {
        for (int j = 0; j < dades.M; ++j) {
            cout << dades.classes[i][j];
        }
        cout << endl;
    }
    cout << endl;
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        cout << "Syntax: " << argv[0] << " input_file outputfile" << endl;
        exit(1);
    }
    unsigned t0 = clock();
    Data dades;
    dades = llegir_dades(argv);
    permutacions(dades, argv, t0);
}
