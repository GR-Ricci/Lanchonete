#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <ctime>

using namespace std;

// ---------- estruturas ---------- //

const string PEDIDOS_ABERTOS_FILE = "pedidos_abertos.csv";
const string PEDIDOS_PAGOS_FILE = "pedidos_pagos.csv";
const string LUCROS_MES_FILE = "lucros_mes.csv";
const string LUCROS_ANO_FILE = "lucros_ano.csv";

struct Pedido {
    string cupom;
    string nomeCliente;
    double total;
    string itens;
};

// ---------- funcoes ---------- //

string trim(const string &s) {
    const string ws = " \t\n\r";
    size_t start = s.find_first_not_of(ws);
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

string formatCupom(int n) {
    stringstream ss;
    ss << setw(3) << setfill('0') << n;
    return ss.str();
}

// ---------- linhas ---------- //

void linha_menu() { cout << "========================================\n"; }
void separador() { cout << "-------------------------------------------\n"; }

// ---------- formatacao ----------

string dataAtual() {
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    stringstream ss;
    ss << setw(2) << setfill('0') << lt->tm_mday << "/"
       << setw(2) << setfill('0') << lt->tm_mon + 1 << "/"
       << 1900 + lt->tm_year;
    return ss.str();
}
string toUpper(const string &s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(), ::toupper);
    return out;
}

// ---------- arquivos ---------- //

vector<Pedido> carregarPedidos(const string& filename) {
    vector<Pedido> pedidos;
    ifstream fin(filename);
    if (!fin.is_open()) return pedidos;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token;
        Pedido p;
        if (!getline(ss, token, ';')) continue;
        p.cupom = trim(token);
        if (!getline(ss, token, ';')) continue;
        p.nomeCliente = trim(token);
        if (!getline(ss, token, ';')) continue;
        try { p.total = stod(trim(token)); } catch (...) { continue; }
        if (!getline(ss, token, ';')) continue;
        p.itens = trim(token);
        pedidos.push_back(p);
    }
    fin.close();
    return pedidos;
}

void salvarPedidos(const string& filename, const vector<Pedido>& pedidos) {
    ofstream fout(filename);
    for (const auto& p : pedidos) {
        fout << p.cupom << ";" << p.nomeCliente << ";" << fixed << setprecision(2)
             << p.total << ";" << p.itens << "\n";
    }
    fout.close();
}

// ---------- lucro ---------- //

double calcularLucroDoDia(const vector<Pedido>& pagos) {
    double total = 0.0;
    for (const auto& p : pagos) total += p.total;
    return total;
}

double carregarLucro(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) return 0.0;
    double total = 0.0;
    fin >> total;
    fin.close();
    return total;
}

void salvarLucro(const string& filename, double total) {
    ofstream fout(filename);
    fout << fixed << setprecision(2) << total;
    fout.close();
}

// ---------- relatorio ---------- //

void relatorioDia() {
    vector<Pedido> pagos = carregarPedidos(PEDIDOS_PAGOS_FILE);
    double lucroDia = calcularLucroDoDia(pagos);
    linha_menu();
    cout << "        RELATORIO DE LUCRO DO DIA\n";
    linha_menu();
    cout << "Data: " << dataAtual() << "\n";
    cout << "Pedidos pagos: " << pagos.size() << "\n";
    cout << "Lucro do dia: R$ " << fixed << setprecision(2) << lucroDia << "\n";
    linha_menu();
}

void relatorioMes() {
    double totalMes = carregarLucro(LUCROS_MES_FILE);
    linha_menu();
    cout << "        RELATORIO DE LUCRO MENSAL\n";
    linha_menu();
    cout << "Lucro acumulado no mes: R$ " << fixed << setprecision(2) << totalMes << "\n";
    linha_menu();
}

void relatorioAno() {
    double totalAno = carregarLucro(LUCROS_ANO_FILE);
    linha_menu();
    cout << "        RELATORIO DE LUCRO ANUAL\n";
    linha_menu();
    cout << "Lucro acumulado no ano: R$ " << fixed << setprecision(2) << totalAno << "\n";
    linha_menu();
}

// ---------- reset ---------- //

void resetarDia() {
    separador();
    cout << "\n  --DAR BAIXA NO DIA--\n";
    cout << "Digite CONFIRMAR para prosseguir\nOu 0 para cancelar\n> ";
    string confirm;
    cin >> confirm;
    confirm = toUpper(confirm);
    if (confirm == "0") {
        cout << "Operacao cancelada.\n";
        return;
    }
    if (confirm == "CONFIRMAR") {
        vector<Pedido> pagos = carregarPedidos(PEDIDOS_PAGOS_FILE);
        double lucroDia = calcularLucroDoDia(pagos);
        double lucroMes = carregarLucro(LUCROS_MES_FILE) + lucroDia;
        double lucroAno = carregarLucro(LUCROS_ANO_FILE) + lucroDia;
        salvarLucro(LUCROS_MES_FILE, lucroMes);
        salvarLucro(LUCROS_ANO_FILE, lucroAno);
        ofstream(PEDIDOS_ABERTOS_FILE, ios::trunc).close();
        ofstream(PEDIDOS_PAGOS_FILE, ios::trunc).close();
        cout << "\n Lucro do dia (R$ " << fixed << setprecision(2)
             << lucroDia << ") adicionado ao mes e ano.\n";
        cout << " Relatorio do dia resetado com sucesso!\n";
    } else {
        cout << "Entrada invalida. Operacao cancelada.\n";
    }
}

void resetarMes() {
    separador();
    cout << "\n  --RESETAR RELATORIO DO MES--\n";
    cout << "Digite CONFIRMAR para prosseguir\nOu 0 para cancelar\n> ";
    string confirm;
    cin >> confirm;
    confirm = toUpper(confirm);
    if (confirm == "0") {
        cout << "Operacao cancelada.\n";
        return;
    }
    if (confirm == "CONFIRMAR") {
        ofstream(LUCROS_MES_FILE, ios::trunc).close();
        cout << "\n Relatorio mensal resetado com sucesso!\n";
    } else {
        cout << "Entrada invalida. Operacao cancelada.\n";
    }
}

void resetarAno() {
    separador();
    cout << "  --RESETAR RELATORIO DO ANO--\n";
    cout << "Digite CONFIRMAR para prosseguir\nOu 0 para cancelar\n> ";
    string confirm;
    cin >> confirm;
    confirm = toUpper(confirm);
    if (confirm == "0") {
        cout << "Operacao cancelada.\n";
        return;
    }
    if (confirm == "CONFIRMAR") {
        ofstream(LUCROS_ANO_FILE, ios::trunc).close();
        cout << "\n Relatorio anual resetado com sucesso!\n";
    } else {
        cout << "Entrada invalida. Operacao cancelada.\n";
    }
}

// ---------- exibição ---------- //

void exibirPedidos(const vector<Pedido>& pedidos) {
    if (pedidos.empty()) {
        cout << "Nenhum pedido pendente no momento.\n";
        return;
    }
    cout << "CUPOM | CLIENTE" << setw(25) << " | TOTAL" << endl;
    separador();
    for (const auto& p : pedidos) {
        cout << p.cupom << " | " << setw(25) << left << p.nomeCliente
             << " | R$ " << fixed << setprecision(2) << p.total << "\n";
    }
    separador();
}

void exibirDetalhesPedido(const Pedido& p) {
    separador();
    cout << "Cupom: " << p.cupom << "\n";
    cout << "Cliente: " << p.nomeCliente << "\n";
    cout << "Total: R$ " << fixed << setprecision(2) << p.total << "\n";
    cout << "Itens: " << p.itens << "\n";
    separador();
}

void processarPagamento() {
    vector<Pedido> abertos = carregarPedidos(PEDIDOS_ABERTOS_FILE);
    vector<Pedido> pagos = carregarPedidos(PEDIDOS_PAGOS_FILE);
    if (abertos.empty()) {
        cout << "\nNao ha pedidos pendentes.\n";
        return;
    }
    int opcao;
    string termo;
    cout << "\nBuscar pedido por:\n1 - Cupom\n2 - Nome do Cliente\nOpcao: ";
    cin >> opcao;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Digite o termo de busca: ";
    getline(cin, termo);
    transform(termo.begin(), termo.end(), termo.begin(), ::tolower);
    auto it = abertos.end();
    if (opcao == 1) {
        string cupomFormatado = termo;
        try { cupomFormatado = formatCupom(stoi(termo)); } catch (...) {}
        it = find_if(abertos.begin(), abertos.end(), [&](const Pedido& p) {
            return p.cupom == cupomFormatado;
        });
    } else if (opcao == 2) {
        it = find_if(abertos.begin(), abertos.end(), [&](const Pedido& p) {
            string nome = p.nomeCliente;
            transform(nome.begin(), nome.end(), nome.begin(), ::tolower);
            return nome.find(termo) != string::npos;
        });
    }
    if (it == abertos.end()) {
        cout << "\nPedido nao encontrado.\n";
        return;
    }
    exibirDetalhesPedido(*it);
    cout << "Confirmar pagamento (S/N)? ";
    char c;
    cin >> c;
    if (tolower(c) == 's') {
        pagos.push_back(*it);
        abertos.erase(it);
        salvarPedidos(PEDIDOS_ABERTOS_FILE, abertos);
        salvarPedidos(PEDIDOS_PAGOS_FILE, pagos);
        cout << "\n Pagamento processado com sucesso!\n";
    } else {
        cout << "\nPagamento cancelado.\n";
    }
}

// ---------- menus ---------- //

void menuRelatorios() {
    while (true) {
        linha_menu();
        cout << "      MENU RELATORIOS DE LUCRO\n";
        linha_menu();
        cout << "1 - Relatorio do dia\n";
        cout << "2 - Relatorio do mes\n";
        cout << "3 - Relatorio do ano\n";
        cout << "0 - Voltar\n";
        linha_menu();
        cout << "Opcao: ";
        int opc;
        cin >> opc;
        switch (opc) {
            case 1: relatorioDia(); break;
            case 2: relatorioMes(); break;
            case 3: relatorioAno(); break;
            case 0: return;
            default: cout << "Opcao invalida.\n"; break;
        }
    }
}

void menuResetarRelatorios() {
    while (true) {
        linha_menu();
        cout << "      MENU RESETAR RELATORIOS\n";
        linha_menu();
        cout << "1 - Resetar relatorio do dia\n";
        cout << "2 - Resetar relatorio do mes\n";
        cout << "3 - Resetar relatorio do ano\n";
        cout << "0 - Voltar\n";
        linha_menu();
        cout << "Opcao: ";
        int opc;
        cin >> opc;
        switch (opc) {
            case 1: resetarDia(); break;
            case 2: resetarMes(); break;
            case 3: resetarAno(); break;
            case 0: return;
            default: cout << "Opcao invalida.\n"; break;
        }
    }
}

// ---------- main ---------- //

int main() {
    int opcao;
    do {
        linha_menu();
        cout << "           SISTEMA DE CAIXA           \n";
        linha_menu();
        cout << "1 - Listar Pedidos Pendentes\n";
        cout << "2 - Processar Pagamento\n";
        cout << "3 - Relatorios de Lucros\n";
        cout << "4 - Resetar Relatorios\n";
        cout << "0 - Sair\n";
        linha_menu();
        cout << "Opcao: ";
        cin >> opcao;
        switch (opcao) {
            case 1: {
                cout << "\n        PEDIDOS PENDENTES\n";
                separador();
                exibirPedidos(carregarPedidos(PEDIDOS_ABERTOS_FILE));
                break;
            }
            case 2: processarPagamento(); break;
            case 3: menuRelatorios(); break;
            case 4: menuResetarRelatorios(); break;
            case 0: cout << "\nFechando o sistema de caixa.\n"; break;
            default: cout << "Opcao invalida.\n"; break;
        }
        cout << "\n";
    } while (opcao != 0);
    return 0;
}