#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

struct Item {
    int id;
    string nome;
    double preco;
};

// ----- estrutura global ----- //
map<string, vector<Item>> categorias;
vector<string> ordemCategorias;
int nextId = 1;
int cupomCounter = 1;

const string MENU_FILE = "menu.csv";
const string SETTINGS_FILE = "settings.txt";

// ---------- formatacao ---------- //

string capitalize(const string& s) {
    if (s.empty()) return s;
    string out = s;
    for (auto &c : out) c = tolower(c);
    out[0] = toupper(out[0]);
    return out;
}
string formatCupom(int n) {
    stringstream ss;
    ss << setw(3) << setfill('0') << n;
    return ss.str();
}

// ---------- leitura ---------- //

string readLine(const string& prompt, bool allowEmpty = false) {
    string s;
    cout << prompt;
    getline(cin, s);
    if (!allowEmpty) {
        while (s.empty()) {
            cout << "Entrada vazia. Tente novamente.\n" << prompt;
            getline(cin, s);
        }
    }
    return s;
}

int readInt(const string& prompt) {
    while (true) {
        string s = readLine(prompt, true);
        if (s.empty()) {
            cout << "Valor invalido. Tente novamente.\n";
            continue;
        }
        try {
            return stoi(s);
        } catch (...) {
            cout << "Valor invalido. Tente novamente.\n";
        }
    }
}
double readDoubleReq(const string& prompt) {
    while (true) {
        string s = readLine(prompt, true);
        try {
            return stod(s);
        } catch (...) {
            cout << "Valor invalido. Tente novamente.\n";
        }
    }
}

bool readDoubleOptional(const string& prompt, double &out) {
    cout << prompt;
    string s;
    getline(cin, s);
    if (s.empty()) return false;
    try {
        out = stod(s);
        return true;
    } catch (...) {
        cout << "Preco invalido. Mantendo valor anterior.\n";
        return false;
    }
}

void pause() {
    cout << "\nPressione ENTER para continuar...";
    string dummy;
    getline(cin, dummy);
}

// ---------- salvar / carregar ---------- //

void carregarMenu() {
    categorias.clear();
    ordemCategorias.clear();
    ifstream fin(MENU_FILE);
    if (!fin.is_open()) return;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token, categoria, nome;
        Item it;
        if (!getline(ss, token, ';')) continue;
        it.id = stoi(token);
        getline(ss, categoria, ';');
        getline(ss, nome, ';');
        it.nome = nome;
        if (!getline(ss, token, ';')) continue;
        it.preco = stod(token);
        if (categorias.find(categoria) == categorias.end()) {
            ordemCategorias.push_back(categoria);
        }
        categorias[categoria].push_back(it);
        if (it.id >= nextId) nextId = it.id + 1;
    }
    fin.close();
}

void salvarMenu() {
    ofstream fout(MENU_FILE);
    for (auto &cat : ordemCategorias) {
        for (auto &it : categorias[cat]) {
            fout << it.id << ";" << cat << ";" << it.nome << ";"
                 << fixed << setprecision(2) << it.preco << "\n";
        }
    }
    fout.close();
}

void carregarConfig() {
    ifstream fin(SETTINGS_FILE);
    if (!fin.is_open()) return;
    fin >> cupomCounter;
    fin.close();
}

void salvarConfig() {
    ofstream fout(SETTINGS_FILE);
    fout << cupomCounter << "\n";
    fout.close();
}

// ---------- operacoes ---------- //

map<int, pair<string,int>> listarMenu(bool mostrarId = false) {
    cout << "----------------------------------------\n";
    cout << "               [CARDAPIO]\n";
    cout << "========================================" << endl;
    map<int, pair<string,int>> mapaIds;
    int idExib = 1;
    bool tem = false;
    for (auto &cat : ordemCategorias) {
        if (categorias[cat].empty()) continue;
        tem = true;
        if (idExib > 1) cout << "----------------------------------------\n";
        cout << "[" << cat << "]\n";
        cout << "     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+" << endl;
        for (int i=0; i<(int)categorias[cat].size(); i++) {
            if (mostrarId) {
                cout << setw(3) << idExib << " - ";
                mapaIds[idExib] = {cat, i};
            } else {
                cout << "    ";
            }
            cout << left << setw(25) << categorias[cat][i].nome
                 << " R$ " << right << setw(6) << fixed << setprecision(2)
                 << categorias[cat][i].preco << "\n";
            idExib++;
        }
    }
    cout << "========================================" << endl;
    if (!tem) cout << "Nenhum item cadastrado.\n";
    return mapaIds;
}

string escolherCategoria(bool permitirVoltar = false) {
    if (ordemCategorias.empty()) {
        cout << "\nAinda nao ha categorias. Criando nova.\n";
        string nova = readLine("Nome da nova categoria: ");
        nova = capitalize(nova);
        categorias[nova] = {};
        ordemCategorias.push_back(nova);
        return nova;
    }
    cout << "\nEscolha a categoria:\n";
    int i = 1;
    for (auto &c : ordemCategorias) {
        cout << i << " - " << c << "\n";
        i++;
    }
    cout << i << " - Criar nova categoria\n";
    if (permitirVoltar) cout << "0 - Voltar\n";
    int opc = readInt("Opcao: ");
    if (permitirVoltar && opc == 0) return "";
    if (opc >= 1 && opc < i) {
        return ordemCategorias[opc - 1];
    } else if (opc == i) {
        string nova = readLine("Nome da nova categoria: ");
        nova = capitalize(nova);
        categorias[nova] = {};
        ordemCategorias.push_back(nova);
        return nova;
    } else {
        cout << "Opcao invalida.\n";
        return "";
    }
}

void adicionarItem() {
    string categoria = escolherCategoria(true);
    if (categoria.empty()) return;
    Item novo;
    novo.id = nextId++;
    novo.nome = readLine("Nome do item (ou ENTER para cancelar): ", true);
    if (novo.nome.empty()) {
        cout << "Operacao cancelada.\n";
        return;
    }
    novo.nome = capitalize(novo.nome);
    novo.preco = readDoubleReq("Preco: R$ ");
    categorias[categoria].push_back(novo);
    salvarMenu();
    cout << "Item adicionado com sucesso!\n";
}

void editarItemMenu(Item &it, vector<Item> &lista) {
    while (true) {
        cout << "\nEditando: " << it.nome << " (R$"
             << fixed << setprecision(2) << it.preco << ")\n";
        cout << "1 - Editar nome\n";
        cout << "2 - Editar preco\n";
        cout << "3 - Mover ordem\n";
        cout << "0 - Voltar\n";
        int opc = readInt("Opcao: ");
        if (opc == 1) {
            string novoNome = readLine("Novo nome (ENTER para manter): ", true);
            if (!novoNome.empty()) it.nome = capitalize(novoNome);
            salvarMenu();
        } else if (opc == 2) {
            double novoPreco;
            bool mudou = readDoubleOptional("Novo preco (ENTER para manter): R$ ", novoPreco);
            if (mudou && novoPreco > 0) it.preco = novoPreco;
            salvarMenu();
        } else if (opc == 3) {
            cout << "Mover para qual posicao (1-" << lista.size() << "): ";
            int pos = readInt("");
            if (pos >= 1 && pos <= (int)lista.size()) {
                auto itAtual = find_if(lista.begin(), lista.end(),
                                       [&](const Item& x){ return x.id == it.id; });
                if (itAtual != lista.end()) {
                    Item temp = it;
                    lista.erase(itAtual);
                    lista.insert(lista.begin() + (pos - 1), temp);
                    salvarMenu();
                }
            }
        } else if (opc == 0) {
            break;
        }
    }
}

void editarItem() {
    auto mapa = listarMenu(true);
    if (mapa.empty()) return;
    int id = readInt("\nDigite o numero do item para editar (0 cancela): ");
    if (id == 0 || mapa.find(id) == mapa.end()) return;
    string cat = mapa[id].first;
    int idx = mapa[id].second;
    editarItemMenu(categorias[cat][idx], categorias[cat]);
}

void editarCategoria() {
    if (ordemCategorias.empty()) {
        cout << "Nenhuma categoria para editar.\n";
        return;
    }
    cout << "\nCategorias:\n";
    for (int i=0; i<(int)ordemCategorias.size(); i++) {
        cout << i+1 << " - " << ordemCategorias[i] << "\n";
    }
    int opc = readInt("Qual categoria editar (0 cancela): ");
    if (opc <= 0 || opc > (int)ordemCategorias.size()) return;
    string cat = ordemCategorias[opc-1];
    while (true) {
        cout << "\nEditando categoria: " << cat << "\n";
        cout << "1 - Editar nome\n";
        cout << "2 - Mover ordem\n";
        cout << "0 - Voltar\n";
        int escolha = readInt("Opcao: ");
        if (escolha == 1) {
            string novo = readLine("Novo nome: ");
            novo = capitalize(novo);
            if (!novo.empty()) {
                categorias[novo] = categorias[cat];
                categorias.erase(cat);
                ordemCategorias[opc-1] = novo;
                cat = novo;
                salvarMenu();
            }
        } else if (escolha == 2) {
            cout << "Mover para qual posicao (1-" << ordemCategorias.size() << "): ";
            int pos = readInt("");
            if (pos >= 1 && pos <= (int)ordemCategorias.size()) {
                string temp = ordemCategorias[opc-1];
                ordemCategorias.erase(ordemCategorias.begin() + (opc-1));
                ordemCategorias.insert(ordemCategorias.begin() + (pos-1), temp);
                salvarMenu();
            }
        } else if (escolha == 0) {
            break;
        }
    }
}

// ---------- remover ---------- //

void removerItem() {
    auto mapa = listarMenu(true);
    if (mapa.empty()) return;
    int id = readInt("\nDigite o numero do item para remover (0 cancela): ");
    if (id == 0 || mapa.find(id) == mapa.end()) return;
    string cat = mapa[id].first;
    int idx = mapa[id].second;
    cout << "Removendo: " << categorias[cat][idx].nome << " da categoria " << cat << "\n";
    categorias[cat].erase(categorias[cat].begin() + idx);
    salvarMenu();
}
void removerCategoria() {
    if (ordemCategorias.empty()) {
        cout << "Nenhuma categoria para remover.\n";
        return;
    }
    cout << "\nCategorias existentes:\n";
    for (int i=0; i<(int)ordemCategorias.size(); i++) {
        cout << i+1 << " - " << ordemCategorias[i] << " (" << categorias[ordemCategorias[i]].size() << " itens)\n";
    }
    cout << "0 - Cancelar\n";
    int opc = readInt("Digite o numero da categoria para remover: ");
    if (opc > 0 && opc <= (int)ordemCategorias.size()) {
        cout << "Removendo categoria: " << ordemCategorias[opc-1] << "\n";
        categorias.erase(ordemCategorias[opc-1]);
        ordemCategorias.erase(ordemCategorias.begin() + (opc-1));
        salvarMenu();
    }
}

// ---------- menu cupom ---------- //

void menuCupom() {
    while (true) {
        cout << "\n===== MENU CUPOM =====\n";
        cout << "1 - Resetar para 001\n";
        cout << "2 - Definir novo valor manualmente\n";
        cout << "3 - Visualizar cupom atual\n";
        cout << "0 - Voltar\n";
        int opc = readInt("Opcao: ");
        switch (opc) {
            case 1:
                cupomCounter = 1;
                salvarConfig();
                cout << "Cupom resetado para 001.\n";
                pause();
                break;
            case 2: {
                int novo = readInt("Digite o novo valor do cupom: ");
                if (novo > 0) {
                    cupomCounter = novo;
                    salvarConfig();
                    cout << "Cupom atualizado para: " << formatCupom(cupomCounter) << "\n";
                } else {
                    cout << "Valor invalido.\n";
                }
                pause();
                break;
            }
            case 3:
                cout << "Cupom atual: " << formatCupom(cupomCounter) << "\n";
                pause();
                break;
            case 0:
                return;
            default:
                cout << "Opcao invalida.\n";
        }
    }
}

// ---------- submenus ---------- //

void menuEditar() {
    while (true) {
        cout << "\n===== EDITAR =====\n";
        cout << "1 - Editar item\n";
        cout << "2 - Editar categoria\n";
        cout << "3 - Remover item\n";
        cout << "4 - Remover categoria\n";
        cout << "0 - Voltar\n";
        int opc = readInt("Opcao: ");
        switch (opc) {
            case 1: editarItem(); pause(); break;
            case 2: editarCategoria(); pause(); break;
            case 3: removerItem(); pause(); break;
            case 4: removerCategoria(); pause(); break;
            case 0: return;
            default: cout << "Opcao invalida.\n";
        }
    }
}
void menu() {
    while (true) {
        cout << "\n==================== MENU ====================\n";
        cout << "1 - Listar cardapio\n";
        cout << "2 - Adicionar item\n";
        cout << "3 - Editar (itens/categorias)\n";
        cout << "4 - Menu de cupom\n";
        cout << "0 - Sair\n";
        int opc = readInt("Opcao: ");
        switch (opc) {
            case 1: listarMenu(); pause(); break;
            case 2: adicionarItem(); pause(); break;
            case 3: menuEditar(); break;
            case 4: menuCupom(); break;
            case 0: cout << "Saindo...\n"; return;
            default: cout << "Opcao invalida.\n";
        }
    }
}

// ---------- main ---------- //

int main() {
    carregarMenu();
    carregarConfig();
    menu();
    return 0;
}