#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <list>

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

void criteri_1(list<int>& candidats, const vector<int>& no_produits) {
    // Posem tots els cotxes que s'han de fabricar de cada classe candidata:
    vector<int> cotxes;
    for (int x : candidats)
        cotxes.push_back(no_produits[x]);
    // Trobem el mínim:
    int maxim = 0;
    for (int x : cotxes)
        if (x > maxim) maxim = x;
    // Trobem totes les classes que no tenen aquest mínim i les traiem de candidats:
    list<int>::iterator it = candidats.begin();
    int i = 0;
    while(it != candidats.end()) {
        if (cotxes[i] != maxim) {
            list<int>::iterator aux = it;
            ++it;
            candidats.erase(aux);
        }
        else ++it;
        ++i;
    }
}

void criteri_2(list<int>& candidats) {
  // Posem tots els cotxes que s'han de fabricar de cada classe candidata:
  vector<int> raons;
  for (int x : candidats) {
      int rao = 0;
      for (int i = 0; i < dades.M; ++i) {
          if (dades.classes[x][i] == 0)
              rao += 1;
          else {
              int rao =  dades.capacitat_finestra[i] / dades.mida_finestra[i];
              rao += rao;
          }
      }
      raons.push_back(rao);
  }
  // Trobem el minim
  int minim = infinit;
  for (int x : raons)
      if (x < minim) minim = x;
  // Trobem totes les classes que no tenen aquest mínim i les traiem de candidats:
  list<int>::iterator it = candidats.begin();
  int i = 0;
  while(it != candidats.end()) {
      if (raons[i] != minim) {
          list<int>::iterator aux = it;
          ++it;
          candidats.erase(aux);
      }
      else ++it;
      ++i;
  }
}


void criteri_3(list<int>& candidats) {
    // Posem el nombre de millores necessàries de cada classe candidata:
    vector<int> millores;
    for (int x : candidats) {
        int num_millores = 0;
        for (int i = 0; i < dades.M; ++i) {
            if (dades.classes[x][i] == 1)
                ++num_millores;
        }
        millores.push_back(num_millores);
    }
    // Trobem el mínim:
    int minim = infinit;
    for (int x : millores)
        if (x < minim) minim = x;
    // Trobem totes les classes que no tenen aquest mínim i les treiem de candidats:
    list<int>::iterator it = candidats.begin();
    int i = 0;
    while(it != candidats.end()) {
        if (millores[i] != minim) {
            list<int>::iterator aux = it;
            ++it;
            candidats.erase(aux);
        }
        else ++it;
        ++i;
    }
}



// Funció que retorna la classe següent en la línia de producció següint els següents criteris:
// CRITERI 1: Retornarem la classe que tingui més cotxes a fabricar encara.
// CRITERI 2: en cas d'empat, ens quedarem amb la classe que tingui el menor sumatori de quocients entre capacitats i mides de les millores necessàries (si una millora no la necessita, se li suma 1)
// CRITERI 3: en cas d'empat, ens quedarem amb ela classe que necessiti més millores. En cas d'un altre empat, ens quedarem amb la primera que ho compleixi.
int decidir_classe_seguent(list<int>& candidats, vector<int> no_produits) {
    if (candidats.size() == 1)
        return *candidats.begin();
    criteri_1(candidats, no_produits);
    if (candidats.size() == 1)
        return *candidats.begin();
    criteri_2(candidats);
    if (candidats.size() == 1)
        return *candidats.begin();
    criteri_3(candidats);
    return *candidats.begin();
}


void greedy(char** argv, unsigned t0)
{
    vector<int> solucio(dades.C);
    vector<int> no_produits = dades.produccio;
    list<int> candidats_inici;
    for(int i = 0; i < dades.K; ++i)
        candidats_inici.push_back(i);
    // Primer trobem el primer cotxe de la línia de producció:
    solucio[0] = decidir_classe_seguent(candidats_inici, dades.produccio);
    --no_produits[solucio[0]];
    // Després fem les altres:
    int penalitzacio_tot = 0;
    for (int i = 1; i < dades.C; ++i) {
        vector<int> penalitzacions(dades.K, infinit);
        // El primer criteri que tenim en compte es la penalització afegida que ens dona cada classe:
        // Calculem la penalització afegida de cada classe que encara quede cotxes per afegir:
        for (int j = 0; j < dades.K; ++j) {
            if (no_produits[j] > 0) {
                solucio[i] = j;
                penalitzacions[j] = calcular_penalitzacio_afegida(solucio, i, infinit);
            }
        }
        // Trobem el mínim:
        int minim = infinit;
        for (int x : penalitzacions)
            if (x < minim) minim = x;
        penalitzacio_tot += minim;
        // Guardem en una llista totes les classes que tenen aquest mínim (menys la classe de solucio[i - 1]):
        list<int> candidats;
        for (int k = 0; k < dades.K; ++k)
            if (penalitzacions[k] == minim) {
                if (k != solucio[i - 1])
                    candidats.push_back(k);
            }
        // Cas en el que només hi ha com a candidat la classe anterior (classe de solucio[i - 1]):
        if (candidats.size() == 0)
            solucio[i] = solucio[i - 1];
        // En els altres casos apliquem els 3 criteris que hem aplicat per el primer cotxe:
        else solucio[i] = decidir_classe_seguent(candidats, no_produits);
        --no_produits[solucio[i]];
    }
    cout << argv[1] << ' ' << penalitzacio_tot << endl;
    // Finalment, escrivim el resutltat:
    escriure_solucio(solucio, penalitzacio_tot, t0, argv);
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
