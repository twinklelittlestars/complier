#include "NFA2DFA.h"
#include <iostream>
#include <string>
#include <map>
#include <cstddef> // For size_t

using namespace std; 

string nodes_sets; // 结点集合
string final_sets; // 终结符集合
int edgenum; // NFA边数

void minimizeDFA(edge b[], int N, string &NODE, string &CHANGE, string &endnode, State_change t[], int &h) {
    // DFA最小化的代码逻辑
    int i, j, k, m, n, x, y, len = CHANGE.length();
    bool flag;
    string sta, ednode;
    vector<string> d(h); // Temporary storage for new state names

    // DFA最小化
    m = 2;
    sta.erase();
    flag = 0;
    // 划分终态和非终态
    for (i = 0; i < m; i++) {
        // 判断是否为终态
        for (k = 0; k < len; k++) {
            y = m;
            // 遍历当前状态的所有NFA状态
            for (j = 0; j < d[i].length(); j++) {
                // 遍历所有DFA状态
                for (n = 0; n < y; n++) {
                    // 如果当前DFA状态包含NFA的终态，则将其标记为DFA的终态
                    if (d[n].find(t[NODE.find(d[i][j])].sets[k]) < d[n].length() || t[NODE.find(d[i][j])].sets[k].length() == 0) {
                        x = (t[NODE.find(d[i][j])].sets[k].length() == 0) ? m : n;
                        // 如果当前DFA状态不是终态，则将其标记为非终态
                        if (sta.empty()) {
                            sta += x + '0';
                        } else if (sta[0] != x + '0') {// 如果当前DFA状态不是终态，则将其标记为非终态
                            d[m] += d[i][j];
                            flag = 1;
                            d[i].erase(j, 1);
                            j--;
                        }
                        break; // 跳出n
                    }
                } // n
            } // j
            if (flag) {
                m++;
                flag = 0;
            }
            sta.erase();
        } // k
    }

    cout << endl;

    // 状态重新命名
    State_change *md = new State_change[m];
    NODE.erase();
    for (i = 0; i < m; i++) {
        md[i].state_table = 'A' + i;
        NODE += md[i].state_table;
    }

    // 更新状态转换矩阵
    for (i = 0; i < m; i++) {
        // 遍历当前状态的所有NFA状态
        for (k = 0; k < len; k++) {
            // 遍历所有DFA状态
            for (j = 0; j < h; j++) {
                // 如果当前DFA状态包含NFA的终态，则将其标记为DFA的终态
                if (d[i][0] == t[j].state_table[0]) {
                    // 遍历所有DFA状态
                    for (n = 0; n < m; n++) {
                        // 如果当前DFA状态包含NFA的终态，则将其标记为DFA的终态
                        if (t[j].sets[k].empty()) {
                            break;
                        } else if (d[n].find(t[j].sets[k]) < d[n].length()) {// 如果当前DFA状态不是终态，则将其标记为非终态
                            md[i].sets[k] = md[n].state_table;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    map<string, char> stateMap;
    for (i = 0; i < m; i++) {
        stateMap[d[i]] = 'A' + i;
    }
    // 更新终态信息
    ednode.erase();
    for (i = 0; i < m; i++) {
        for (j = 0; j < endnode.length(); j++) {
            // 检查原始终态是否在新的状态集合中
            if (d[i].find(endnode[j]) != string::npos && ednode.find(md[i].state_table) == string::npos) {
                ednode += md[i].state_table;
            }
        }
    } 
    endnode = ednode;
}
// 输出空格
void cout_tab(int a) {
    int i;
    for (i = 0; i < a; i++)
        cout << ' ';
}

// 排序
void Sort(string &a) {
    int i, j;
    char b;
    // 冒泡排序
    for (j = 0; j < a.length(); j++)
        for (i = 0; i < a.length(); i++)
            if (nodes_sets.find(a[i]) > nodes_sets.find(a[i + 1])) {
                b = a[i];
                a[i] = a[i + 1];
                a[i + 1] = b;
            }
}

// 求e-clouse
void eclouse(char c, string &he, edge b[]) {
    int k;
    // 遍历所有边
    for (k = 0; k < edgenum; k++) {
        if (c == b[k].start[0])
            if (b[k].condition == "*") {
                if (he.find(b[k].end) > he.length())
                    he += b[k].end;
                eclouse(b[k].end[0], he, b);
            }
    }
}

// 求move(I,a)
void update(State_change &he, int m, edge b[]) {//
    int i, j, k, l;
    k = he.state_table.length();
    l = he.sets[m].length();
    // 遍历所有边
    for (i = 0; i < k; i++)
        for (j = 0; j < edgenum; j++)
        // 如果当前边的起点和条件与当前状态相同，则将其终点加入到当前状态的集合中
            if ((final_sets[m] == b[j].condition[0]) && (he.state_table[i] == b[j].start[0]))
                if (he.sets[m].find(b[j].end[0]) > he.sets[m].length())
                    he.sets[m] += b[j].end[0];

    // 遍历所有边
    for (i = 0; i < l; i++)
        for (j = 0; j < edgenum; j++)
        // 如果当前边的起点和条件与当前状态相同，则将其终点加入到当前状态的集合中
            if ((final_sets[m] == b[j].condition[0]) && (he.sets[m][i] == b[j].start[0]))
                if (he.sets[m].find(b[j].end[0]) > he.sets[m].length())
                    he.sets[m] += b[j].end[0];
}

// 格式化输出
void mycout(int len, int h, State_change *t) {
    int i, j, m;
    cout << " I       ";
    for (i = 0; i < len; i++)
        cout << 'I' << final_sets[i] << "      ";
    cout << endl << "-------------------------" << endl;
    for (i = 0; i < h; i++) {
        cout << ' ' << t[i].state_table;
        m = t[i].state_table.length();
        // 遍历所有集合
        for (j = 0; j < len; j++) {
            cout_tab(8 - m);
            m = t[i].sets[j].length();
            cout << t[i].sets[j];
        }
        cout << endl;
    }
}

int main() {
    edge *b = new edge[MAXS_states];
    int i, j, k, m, n, h, len;
    string endnode, sta;
    
    cout << "请输入NFA各边信息,分别为 起点 边(空为e) 终点最后以#结束: " << endl;
    for (i = 0; i < MAXS_states; i++) {
        cin >> b[i].start;
        if (b[i].start == "#") break;
        cin >> b[i].condition >> b[i].end;
    }
    edgenum = i;


    for (i = 0; i < edgenum; i++) {
        // 求结点集合
        if (nodes_sets.find(b[i].start) > nodes_sets.length()) nodes_sets += b[i].start;
        if (nodes_sets.find(b[i].end) > nodes_sets.length()) nodes_sets += b[i].end;// 求终结符集合
        if ((final_sets.find(b[i].condition) > final_sets.length()) && (b[i].condition != "*")) final_sets += b[i].condition;
    }
    len = final_sets.length();

    cout << "结点中属于终态的是: " << endl;
    cin >> endnode;

    // 求状态转换矩阵
    State_change *t = new State_change[MAXS_states];
    t[0].state_table = b[0].start;
    h = 1;
    eclouse(b[0].start[0], t[0].state_table, b); // 求e-clouse

    // 求move(I,a)
    for (i = 0; i < h; i++) {
        // 遍历所有终结符
        for (j = 0; j < t[i].state_table.length(); j++) {
            for (m = 0; m < len; m++) {
                eclouse(t[i].state_table[j], t[i].sets[m], b); // 求e-clouse
            }
        }
        // 遍历所有终结符
        for (k = 0; k < len; k++) {
            update(t[i], k, b); // 求move(I,a)
            for (j = 0; j < t[i].sets[k].length(); j++) {
                eclouse(t[i].sets[k][j], t[i].sets[k], b); // 求e-clouse
            }
        }
        // 遍历所有集合
        for (j = 0; j < len; j++) {
            Sort(t[i].sets[j]); // 对集合排序以便比较
            for (k = 0; k < h; k++) {
                if (t[k].state_table == t[i].sets[j]) break;
            }
            if (k == h && t[i].sets[j].length()) {
                t[h++].state_table = t[i].sets[j];
            }
        }
    }

    cout << endl << "状态转换矩阵如下: " << endl;
    mycout(len, h, t); // 输出状态转换矩阵

    // 状态重新命名
    string *d = new string[h];
    nodes_sets.erase();
    cout << endl << "重命名: " << endl;
    for (i = 0; i < h; i++) {
        sta = t[i].state_table;//当前DFA状态对应的NFA状态集合
        t[i].state_table.erase();
        t[i].state_table = 'A' + i;
        nodes_sets += t[i].state_table;
        cout << '{' << sta << "}=" << t[i].state_table << endl;
        for (j = 0; j < endnode.length(); j++) {
            // 如果当前DFA状态包含NFA的终态，则将其标记为DFA的终态
            if (sta.find(endnode[j]) < sta.length()) {
                d[i] = t[i].state_table;// d数组用于存储DFA的终态
                break;
            }
        }
        for (k = 0; k < h; k++) {
            for (m = 0; m < len; m++) {
                if (sta == t[k].state_table) { 
                }
            }
        }
    }

    cout << endl << "DFA如下: " << endl;
    mycout(len, h, t); // 输出DFA
    cout << "其中终态为: ";
    for (i = 0; i < h; i++) {
        if (!d[i].empty()) {
            cout << d[i] << " ";
        }
    }
    cout << endl;

    // DFA最小化
    string userChoice;
    cout << "是否要进行DFA最小化? (Yes/No): ";
    cin >> userChoice;

    // 执行DFA最小化
    if (userChoice == "Yes" || userChoice == "yes") {
        cout << "最小化DFA如下: " << endl;
        mycout(len, h, t); 
        cout << "其中终态为: " << endnode << endl;
    } else {
        cout << "未执行DFA最小化。" << endl;
    }


    // 清理动态分配的内存
    delete[] b;
    delete[] t;
    delete[] d;

    return 0;
}