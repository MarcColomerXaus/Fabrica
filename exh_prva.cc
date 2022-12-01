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


void greedy(char** argv, unsigned t0)
{
    // Començem decidint el primer cotxe de la solució tenint en compte les raons entre la capacitat i la mida de la finestra:
    vector<double> rao(dades.M);
    int classe_preferent = 0;
    for (int i = 0; i < dades.M; ++i) {
        rao[i] = dades.capacitat_finestra[i] / dades.mida_finestra[i];
        if (rao[i] > rao[classe_preferent]) {
            classe_preferent = i;
        }
    }
    vector<int> solucio(dades.C);
    // Per anar controlant els cotxes que ens queden utilitzem el següent vector, on no_produits[i] indica quants cotxes de la classse i queden per fabricar
    vector<int> no_produits = dades.produccio;
    solucio[0] = classe_preferent;
    --no_produits[classe_preferent];
    // A partir d'aquí decidirem quin cotxe afegirem tenint en compte quin és el que dona un cost mínim al afegir-lo a la solució:
    int cost_solucio = 0;
    for (int i = 1; i < dades.C; ++i) {
        int cost_minim = infi;
        for (int j = 0; j < dades.K; ++j) {
            solucio[i] = j;
            int cost_aux = calcular_penalitzacio_afegida(solucio, i, cost_minim);
            if (cost_aux < cost_minim) {
                classe_preferent = j;
                cost_minim = cost_aux;
            }
            solucio[i] = classe_preferent;
            cost_solucio += cost_minim;
        }
    }
    escriure_solucio(solucio, cost_solucio, t0, argv);
}


int main(int argc, char** argv)
{
    if (argc != 3) {
        cout << "Syntax: " << argv[0] << " input_file outputfile" << endl;
        exit(1);
    }
    unsigned t0 = clock();
    dades = llegir_dades(argv);
    greedy(argv, t0);
}
