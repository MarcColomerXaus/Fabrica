#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

// pq minima penalitzacio = -1?
// explicacio principi i final calcul penalitzacions

using namespace std;

struct Data {
    int C; // nombre de cotxes
    int M; // nombre de millores
    int K; // nombre de classes
    vector<int> capacitat_finestra; // v[i] indica el màxim nombre de cotxes que poden requerir la millora [i] en un conjunt de cotxes consecutius
    vector<int> mida_finestra; // v[i] indica la mida del conjunt de cotxes consecutius per a la millora [i]
    vector<int> produccio; // v[i] indica els cotxes que s'han de produïr de la classe [i]
    vector<vector<bool>> classes; // v[i][j] indica si la classe [i] requereix o no la millora [j]
};

void inicialitzar_dades(Data& dades, int C, int M, int K)
{
    dades.C = C;
    dades.M = M;
    dades.K = K;
    vector<int> v(M);
    dades.capacitat_finestra = v;
    dades.mida_finestra = v;
    vector<int> u(K);
    dades.produccio = u;
    vector<vector<bool>> mat(K, vector<bool>(M, false));
    dades.classes = mat;
}

// Funció que llegeix les dades d'entrada i retorna una variable de tipus Data
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
            in >> aux;
            if (aux == 1)
                dades.classes[i][j] = true;
        }
    }
    return dades;
}

// FUNCIÓ PER A COMPROVAR QUE LLEGIM BÉ LES DADES
// void treure_dades(Data dades)
// {
//     cout << dades.C << ' ' << dades.M << ' ' << dades.K << endl;
//     for (int x : dades.capacitat_finestra)
//         cout << x << ' ';
//     cout << endl;
//     for (int x : dades.mida_finestra)
//         cout << x << ' ';
//     cout << endl;
//     for (int x : dades.produccio)
//         cout << x << ' ';
//     cout << endl;
//     for (int i = 0; i < dades.K; ++i) {
//         for (int j = 0; j < dades.M; ++j) {
//             cout << dades.classes[i][j];
//         }
//         cout << endl;
//     }
//     cout << endl;
// }

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

// Funció que calcula la penalització de la permutació associada a una millora concreta
int calcular_cost_millora(const Data& dades, int millora, vector<bool> cotxes_a_millorar)
{
    int mida = dades.mida_finestra[millora];
    int capacitat = dades.capacitat_finestra[millora];
    // final = inici + mida
    int penalitzacions = 0;
    int inici = 0;
    int final = capacitat;
    while (final < mida - 1) {
        int contador = 0;
        for (int i = inici; i <= final; ++i) {
            if (cotxes_a_millorar[i])
                ++contador;
        }
        if (contador > capacitat) {
            penalitzacions += contador - capacitat;
        }
        ++final;
    }
    while (final < dades.C) {
        int contador = 0;
        for (int i = inici; i <= final; ++i) {
            if (cotxes_a_millorar[i])
                ++contador;
        }
        if (contador > capacitat) {
            penalitzacions += contador - capacitat;
        }
        ++inici;
        ++final;
    }
    while (final - inici >= capacitat) {
        int contador = 0;
        for (int i = inici; i <= final; ++i) {
            if (cotxes_a_millorar[i])
                ++contador;
        }
        if (contador > capacitat) {
            penalitzacions += contador - capacitat;
        }
        ++inici;
    }
    return penalitzacions;
}

// Funció que calcula la penalització total de la permutació, sumant les penalitzacions associades a cadascuna de les millores
int calcular_cost_total(const Data& dades, const vector<int>& sol)
{
    vector<bool> cotxes_a_millorar(dades.C);
    int penalitzacio = 0;
    for (int i = 0; i < dades.M; ++i) { // per a cada millora, generem un vector que ens indica quins cotxes de la
        for (int j = 0; j < dades.C; ++j) { // permutació requereixen aquesta millora
            if (dades.classes[sol[j]][i] == 1)
                cotxes_a_millorar[j] = true;
            else
                cotxes_a_millorar[j] = false;
        }
        penalitzacio += calcular_cost_millora(dades, i, cotxes_a_millorar);
    }
    return penalitzacio;
}

// Funció que calcula totes les permutacions de cotxes possibles de forma recursiva
void permutacions_rec(const Data& dades, int n, vector<int>& sol_parcial, vector<int>& no_produits, int& min_penalitzacio, char** argv, unsigned t0)
{
    if (n == dades.C) {
        int cost = calcular_cost_total(dades, sol_parcial);
        if (min_penalitzacio == -1 or min_penalitzacio > cost) {
            unsigned t1 = clock();
            double temps = (double(t1 - t0) / CLOCKS_PER_SEC);
            escriure_resultat(sol_parcial, cost, temps, argv);
            min_penalitzacio = cost;
        }

    } else {
        for (int i = 0; i < dades.K; ++i) {
            if (no_produits[i] > 0) {
                sol_parcial[n] = i;
                --no_produits[i];
                permutacions_rec(dades, n + 1, sol_parcial, no_produits, min_penalitzacio, argv, t0);
                ++no_produits[i];
            }
        }
    }
}

void permutacions(const Data& dades, char** argv, unsigned t0)
{
    vector<int> sol_parcial(dades.C);
    vector<int> no_produits = dades.produccio; // v[i] indica el nombre de cotxes que encara s'han de produir de la classe i
    int min_penalitzacio = -1;
    permutacions_rec(dades, 0, sol_parcial, no_produits, min_penalitzacio, argv, t0);
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

