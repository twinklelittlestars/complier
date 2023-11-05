#ifndef NFA2DFA_H
#define NFA2DFA_H

#include <string>
#include <vector>

#define MAXS_states 100
using namespace std;

// 声明边的结构体
struct edge {
    std::string start;
    std::string condition;
    std::string end;
};

// 声明状态转换的结构体
struct State_change {
    std::string state_table;
    std::string sets[MAXS_states];
};

// 函数声明
//NFA转DFA函数声明
void convertNFAtoDFA(edge b[], int N, std::string &endnode);
// DFA最小化函数声明
void minimizeDFA(edge b[], int N, std::string &NODE, std::string &CHANGE, std::string &endnode, State_change t[], int &h);
void cout_tab(int a);
void Sort(std::string &a);
void eclouse(char c, std::string &he, edge b[]);
void update(State_change &he, int m, edge b[]);
void mycout(int len, int h, State_change *t);
#endif // NFA2DFA_H