#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

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
    dades.capacitat_finestra = vector<int>(M);
    dades.mida_finestra = vector<int>(M);
    dades.produccio = vector<int>(K);
    dades.classes = vector<vector<bool>>(K, vector<bool>(M, false));
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

int calcular_cost_millora(const Data& dades, int millora, const vector<bool>& cotxes_a_millorar)
{
    int mida = dades.mida_finestra[millora];
    int capacitat = dades.capacitat_finestra[millora];
    int n = cotxes_a_millorar.size();
    int contador = 0;

    if (n <= capacitat)
        return 0;

    if (n < mida) {
        int inici = 0;
        for (int i = inici; i < n; ++i) {
            if (cotxes_a_millorar[i])
                ++contador;
        }
        if (contador > capacitat)
            return contador - capacitat;
        return 0;
    }

    int penalitzacio = 0;
    if (n >= mida) {
        int inici = n - mida;
        for (int i = inici; i < n; ++i) {
            if (cotxes_a_millorar[i])
                ++contador;
        }
        if (contador > capacitat) {
            penalitzacio = contador - capacitat;
        }
    }

    if (n == dades.C) {
        int inici = n - mida + 1;
        while (inici < n - capacitat) {
            for (int i = inici; i < n; ++i) {
                if (cotxes_a_millorar[i])
                    ++contador;
            }
            if (contador > capacitat) {
                penalitzacio += contador - capacitat;
            }
            ++inici;
        }
    }
    return penalitzacio;
}

int calcular_cost_afegit(const Data& dades, const vector<int>& sol, int n)
{
    vector<bool> cotxes_a_millorar(n);
    int penalitzacio = 0;
    for (int i = 0; i < dades.M; ++i) { // per a cada millora, generem un vector que ens indica quins cotxes de la
        for (int j = 0; j < n; ++j) { // permutació requereixen aquesta millora
            if (dades.classes[sol[j]][i] == 1)
                cotxes_a_millorar[j] = true;
            else
                cotxes_a_millorar[j] = false;
        }
        penalitzacio += calcular_cost_millora(dades, i, cotxes_a_millorar);
    }
    return penalitzacio;
}

void greedy(Data& dades)
{
    vector<double> rao(dades.M);
    int millora_preferent = 0;
    for (int i = 0; i < dades.M; ++i) {
        v[i] = dades.capacitat_finestra[i] / dades.mida_finestra[i];
        if (rao[millora_preferent] > rao[i]) {
            millora_preferent = i;
        }
    }
    vector<int> solucio(dades.C);
    vector<int> no_produits = dades.produccio;
    solucio[0] = millora_preferent;
    --no_produits[millora_preferent];
    int cost_solucio = 0;
    for (int i = 1; i < dades.C; ++i) {
        cost_minim = 100000000;
        for (int j = 0; j < dades.K; ++j) {
            solucio[i] = j;
            int cost_aux = calcular_cost_afegit(dades, solucio, i);
            if (cost_aux < cost_minim) {
                cost_minim = cost_aux;
                classe_preferent = j;
            }
            solucio[i] = classe_preferent;
            cost_solucio += cost_minim;
        }
    }
    escriure_resultat(solucio, cost_solucio, double temps, char** argv);
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
    greedy(dades);
}
