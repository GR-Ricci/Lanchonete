#include <iostream>
#include <vector>
#include <iomanip>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <thread>
#include <chrono>

using namespace std;

// ---------- salvar pedidos ---------- //

const string PEDIDOS_FILE = "pedidos_abertos.csv";

// ---------- funcoes >> lanchonete ---------- //

struct Item {
    int id;
    string nome;
    double preco;
};

// ---------- estruturas ---------- //

map<string, vector<Item>> categorias;
vector<string> ordemCategorias;

int cupomCounter = 1;

const string MENU_FILE = "menu.csv";
const string SETTINGS_FILE = "settings.txt";

// ---------- formatacao ---------- //

string formatCupom(int n) {
    stringstream ss;
    ss << setw(3) << setfill('0') << n;
    return ss.str();
}

string trim(const string &s) {
    const string ws = " \t\n\r";
    size_t start = s.find_first_not_of(ws);
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// ---------- menu > lanchonete ---------- //

void carregarMenu() {
    categorias.clear();
    ordemCategorias.clear();
    ifstream fin(MENU_FILE);
    if (!fin.is_open()) {
        cout << "\n  Nao foi possivel carregar o cardapio\n";
        exit(1);
    }
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token, categoria, nome;
        Item it;
        if (!getline(ss, token, ';')) continue;
        token = trim(token);
        try {
            it.id = stoi(token);
        } catch (...) {
            continue; 
        }
        if (!getline(ss, categoria, ';')) continue;
        if (!getline(ss, nome, ';')) continue;
        if (!getline(ss, token, ';')) continue; 
        categoria = trim(categoria);
        nome = trim(nome);
        token = trim(token);
        if (categoria.empty() || nome.empty() || token.empty()) continue;
        try {
            it.preco = stod(token);
        } catch (...) {
            continue; 
        }
        it.nome = nome;
        if (categorias.find(categoria) == categorias.end()) {
            ordemCategorias.push_back(categoria);
        }
        categorias[categoria].push_back(it);
    }
    fin.close();
    ifstream cfg(SETTINGS_FILE);
    if (cfg.is_open()) {
        cfg >> cupomCounter;
        cfg.close();
    } else {
        cupomCounter = 1;
    }
    cout << "\n Cardapio carregado com sucesso!\n";
    cout << "Cupom atual: " << formatCupom(cupomCounter) << "\n";
}

void salvarCupom() {
    ofstream fout(SETTINGS_FILE);
    fout << cupomCounter << "\n";
    fout.close();
}
int gerarCupom() {
    int atual = cupomCounter;
    cupomCounter++;
    salvarCupom();
    return atual;
}

// ---------- estrutura pedido ---------- //

struct PedidoItem {
    string nome;
    double preco;
    string categoria; 
};

// ---------- linhas ---------- //

void linha_menu() { cout << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+"; }
void linha_menu_principal() { cout << "============================="; }
void separador() { cout << "-----------------------------"; }

// ---------- exibição ---------- //

void exibirPedido(const vector<PedidoItem>& pedido) {
    if (pedido.empty()) return;
    cout << "Pedido:" << endl;
    linha_menu_principal();
    cout << "\n";
    map<string, vector<string>> itensPorCategoria;
    for (const auto& item : pedido) {
        itensPorCategoria[item.categoria].push_back(item.nome);
    }
    for (const auto& cat : ordemCategorias) {
        if (itensPorCategoria.count(cat) && !itensPorCategoria[cat].empty()) {
            cout << "> " << cat << ":\n";
            for (const auto& nome : itensPorCategoria[cat]) {
                cout << "- " << nome << endl;
            }
        }
    }
}

void exibirPedidoComIndices(const vector<PedidoItem>& pedido) {
    if (pedido.empty()) return;
    int indice = 1;
    string categoriaAtual = "";
    for (const auto& item : pedido) {
        if (item.categoria != categoriaAtual) {
            cout << "> " << item.categoria << ":\n";
            categoriaAtual = item.categoria;
        }
        cout << indice << " - " << item.nome << endl;
        indice++;
    }
}

// ---------- funcoes menu ---------- //

void menuCategoria(const string& categoria, vector<PedidoItem>& pedido, double& total_pedido) {
    if (categorias.find(categoria) == categorias.end() || categorias[categoria].empty()) {
        cout << "\nNenhum item cadastrado na categoria " << categoria << ".\n";
        return;
    }
    int escolha;
    do {
        linha_menu();
        cout << "\n" << categoria << ":\n";
        for (size_t i = 0; i < categorias[categoria].size(); ++i) {
            cout << " " << i + 1 << " - " << categorias[categoria][i].nome
                 << " - R$: " << fixed << setprecision(2) << categorias[categoria][i].preco << "\n";
        }
        cout << " " << categorias[categoria].size() + 1 << " - Retornar" << endl;
        linha_menu();
        cout << "\n Adicionar: ";
        cin >> escolha;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Opcao Invalida" << endl;
            continue;
        }
        if (escolha >= 1 && escolha <= (int)categorias[categoria].size()) {
            const Item& itemEscolhido = categorias[categoria][escolha - 1];
            cout << "[" << itemEscolhido.nome << " Adicionado]" << endl;
            separador();
            PedidoItem novoItem = {itemEscolhido.nome, itemEscolhido.preco, categoria};
            pedido.push_back(novoItem);
            total_pedido += itemEscolhido.preco;
            cout << "\n";
            exibirPedido(pedido);
            separador();
            cout << "\n> Total Pedido: R$" << total_pedido << endl;
            cout << endl;
        } else if (escolha != (int)categorias[categoria].size() + 1) {
            cout << "Opcao Invalida" << endl;
        }
    } while (escolha != (int)categorias[categoria].size() + 1);
}

void salvarPedido(const string& nomeCliente, int cupom, double total, const vector<PedidoItem>& pedido) {
    ofstream fout(PEDIDOS_FILE, ios::app);
    if (!fout.is_open()) {
        cout << "\nERRO CRITICO: Nao foi possivel abrir o arquivo de pedidos para salvar.\n";
        return;
    }
    fout << formatCupom(cupom) << ";" << nomeCliente << ";" << fixed << setprecision(2) << total << ";";
    for (size_t i = 0; i < pedido.size(); ++i) {
        fout << pedido[i].nome << (i == pedido.size() - 1 ? "" : ",");
    }
    fout << "\n";
    fout.close();
}

void reiniciarSistema() {
    cout << "\nIniciando novo atendimento em ";
    for (int i = 10; i >= 1; --i) {
        cout << i << "... " << flush;
        this_thread::sleep_for(chrono::seconds(1));
    }
    cout << "\n\n========================================\n\n";
}

// ---------- main ---------- //

int main() {
    carregarMenu();
    while (true) {
        map<int, string> menuPrincipalMap;
        int menuIndex = 1;
        for (const string& cat : ordemCategorias) {
            menuPrincipalMap[menuIndex++] = cat;
        }
        if (menuPrincipalMap.empty()) {
            cout << "ERRO: Nenhuma categoria encontrada no menu.csv.\n";
            return 1;
        }
        double total_pedido = 0.00;
        int opcao = 0, opcao_final = 0;
        vector<PedidoItem> pedido;
        do {
            linha_menu_principal();
            cout << "\n| >  CARDAPIO LANCHONETE  < |" << endl;
            separador();
            for (const auto& pair : menuPrincipalMap) {
                cout << "\n" << pair.first << " - " << pair.second;
            }
            cout << "\n" << menuIndex << " - Conferir & Editar Pedido";
            cout << "\n" << menuIndex + 1 << " - Finalizar Pedido" << endl;
            linha_menu_principal();
            cout << "\nOpcao: ";
            cin >> opcao;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Opcao Invalida" << endl;
                continue;
            }
            if (menuPrincipalMap.count(opcao)) {
                string categoria = menuPrincipalMap[opcao];
                menuCategoria(categoria, pedido, total_pedido);
            } else if (opcao == menuIndex) {
                do {
                    separador();
                    cout << "\n";
                    exibirPedido(pedido);
                    if (!pedido.empty()) {
                        separador();
                        cout << "\n";
                    }
                    cout << "> Total Pedido: R$" << fixed << setprecision(2) << total_pedido << endl;
                    separador();
                    cout << "\n1 - Retornar\n2 - Finalizar pedido\n3 - Remover Itens";
                    cout << "\nSelecione: ";
                    cin >> opcao_final;
                    if (cin.fail()) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Opcao Invalida" << endl;
                        continue;
                    }
                    if (opcao_final == 1 || opcao_final == 2) break;
                    if (opcao_final == 3) {
                        int itemRemover;
                        while (true) {
                            int totalItens = pedido.size();
                            if (totalItens == 0) {
                                cout << "Nenhum item para remover\n";
                                break;
                            }
                            linha_menu();
                            cout << "\n        [Remover Itens]" << endl;
                            exibirPedidoComIndices(pedido);
                            separador();
                            cout << "\n> Total Pedido: R$" << fixed << setprecision(2) << total_pedido << endl;
                            linha_menu();
                            cout << "\n[Digite 0 para voltar]\nDigite o numero do item que deseja remover: ";
                            cin >> itemRemover;
                            if (cin.fail()) {
                                itemRemover = -1;
                                cin.clear();
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                cout << "Valor Invalido!\n";
                                continue;
                            }
                            if (itemRemover == 0) break;
                            if (itemRemover < 1 || itemRemover > totalItens) {
                                cout << "Numero invalido!\n";
                                continue;
                            }
                            int i = itemRemover - 1;
                            cout << "Removido: " << pedido[i].nome << endl;
                            total_pedido -= pedido[i].preco;
                            pedido.erase(pedido.begin() + i);
                            separador();
                            cout << "\n";
                        }
                    }
                } while (opcao_final != 1 && opcao_final != 2);
            } else if (opcao == menuIndex + 1) {
                opcao_final = 2;
            } else {
                cout << "Opcao Invalida" << endl;
            }
        } while (opcao_final != 2);
        if (total_pedido > 0) {
            string nomeCliente;
            separador();
            cout << "\nPara finalizar, digite seu nome: ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            getline(cin, nomeCliente);
            int cupomGerado = gerarCupom();
            salvarPedido(nomeCliente, cupomGerado, total_pedido, pedido);
            separador();
            cout << "\n";
            exibirPedido(pedido);
            linha_menu_principal();
            cout << "\n> Cliente: " << nomeCliente;
            cout << "\n> Total do Pedido: R$" << fixed << setprecision(2) << total_pedido << endl;
            separador();
            cout << "\nCupom: " << formatCupom(cupomGerado) << endl;
            cout << "Pedido salvo com sucesso!\n";
            cout << "Realize o pagamento no Caixa informando seu nome ou cupom.\nObrigado e volte sempre!" << endl;
            linha_menu_principal();
        } else {
            separador();
            cout << "\nNenhum pedido realizado\nObrigado e volte sempre!" << endl;
            linha_menu_principal();
        }
        reiniciarSistema();
    }
    return 0;
}