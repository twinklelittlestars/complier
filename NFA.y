//构建一个正则表达式转NFA的yacc程序
//使用方法：./yacc <正则表达式> <输出文件名>
%{

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
int yylex();
extern int yyparse();
FILE* yyin;
void yyerror(const char* s);
// 这里可以定义辅助函数和全局变量
typedef struct Transition {
    char symbol; // 输入符号
    struct State *to_state; // 指向下一个状态的指针
} Transition;

typedef struct State {
    int stateID;          // 唯一标识符
    int is_final;         // 是否是接受状态
    Transition *transitions; // 状态的转换列表
    int num_transitions;  // 转换的数量
} State;


typedef struct NFA {
    State *start_state; // NFA的起始状态
} NFA;
NFA EndNFA;
int next_state_id = 0;
// Function prototypes
NFA unionNFA(NFA first, NFA second);
NFA concatenateNFA(NFA first, NFA second);
NFA singleCharNFA(char c);
NFA kleeneStarNFA(NFA nfa);

%}

%union {
    NFA nfa;
    char character;
}

%token OR STAR LPAREN RPAREN
%token <character> CHARACTER
%type <nfa> expression term factor


%%

expression
    : expression OR term { $$ = unionNFA($1, $3); EndNFA = $$;}
    | term               { $$ = $1;EndNFA = $$; }
    ;

term
    : term factor        { $$ = concatenateNFA($1, $2); }
    | factor             { $$ = $1; }
    ;

factor
    : CHARACTER               { $$ = singleCharNFA($1); }
    | factor STAR         { $$ = kleeneStarNFA($1); }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

%%

// 创建一个新的NFA状态
State *newState(int is_final) {
    State *state = (State *)malloc(sizeof(State));
    if (state == NULL) {
        // 处理内存分配失败的情况
        perror("Unable to allocate memory for new state.");
        exit(EXIT_FAILURE);
    }
    state->stateID = next_state_id++; // 分配一个新的ID并递增ID计数器
    state->is_final = is_final;
    state->transitions = NULL;
    state->num_transitions = 0;
    return state;
}

// 创建一个单字符的NFA
NFA singleCharNFA(char c) {
    State *start = newState(0);
    State *end = newState(1);

    Transition transition = {c, end};
    start->transitions = (Transition *)malloc(sizeof(Transition));
    start->transitions[0] = transition;
    start->num_transitions = 1;

    NFA nfa = {start};
    return nfa;
}

// 添加一个转换到状态
void addTransition(State *state, Transition transition) {
    state->transitions = (Transition *)realloc(state->transitions, (state->num_transitions + 1) * sizeof(Transition));
    state->transitions[state->num_transitions] = transition;
    state->num_transitions++;
}

// 连接两个NFA
NFA concatenateNFA(NFA first, NFA second) {
    // 将第一个NFA的所有接受状态连接到第二个NFA的起始状态
    for (int i = 0; i < first.start_state->num_transitions; i++) {
        if (first.start_state->transitions[i].to_state->is_final) {
            addTransition(first.start_state->transitions[i].to_state, (Transition){'e', second.start_state});
            first.start_state->transitions[i].to_state->is_final = 0;
        }
    }

    NFA nfa = {first.start_state};
    return nfa;
}

NFA unionNFA(NFA first, NFA second) {
    State *start = newState(0); // 新的起始状态
    State *end = newState(1);   // 新的终态

    // 添加空转换从新起始状态到两个输入 NFA 的起始状态
    addTransition(start, (Transition){'e', first.start_state});
    addTransition(start, (Transition){'e', second.start_state});

    // 遍历 first NFA 的所有状态，将终态的转换指向新的终态
    for (int i = 0; i < first.start_state->num_transitions; i++) {
        if (first.start_state->transitions[i].to_state->is_final) {
            first.start_state->transitions[i].to_state->is_final = 0;
            addTransition(first.start_state->transitions[i].to_state, (Transition){'e', end});
        }
    }

    // 遍历 second NFA 的所有状态，将终态的转换指向新的终态
    for (int i = 0; i < second.start_state->num_transitions; i++) {
        if (second.start_state->transitions[i].to_state->is_final) {
            second.start_state->transitions[i].to_state->is_final = 0;
            addTransition(second.start_state->transitions[i].to_state, (Transition){'e', end});
        }
    }

    NFA nfa = {start};
    return nfa;
}


NFA kleeneStarNFA(NFA nfa) {
    // 创建新的起始状态，它不是终态
    State *start = newState(0);
    // 创建新的接受状态，它是终态
    State *accept = newState(1);

    // 从新的起始状态添加空转换（epsilon transition）到原始 NFA 的起始状态
    addTransition(start, (Transition){'e', nfa.start_state});
    // 为原始 NFA 的每个终态添加空转换回到原始 NFA 的起始状态
    for (int i = 0; i < nfa.start_state->num_transitions; i++) {
        if (nfa.start_state->transitions[i].to_state->is_final) {
            addTransition(nfa.start_state->transitions[i].to_state, (Transition){'e', nfa.start_state});
            // 将原始终态的转换指向新的接受状态
            addTransition(nfa.start_state->transitions[i].to_state, (Transition){'e', accept});
            // 确保原始终态不再是终态
            nfa.start_state->transitions[i].to_state->is_final = 0;
        }
    }

    // 从新的起始状态添加空转换到新的接受状态
    addTransition(start, (Transition){'e', accept});

    // 设置新的起始状态为终态，以接受空字符串
    start->is_final = 1;

    // 返回新的 NFA，它的起始状态是新创建的起始状态
    NFA kleene = {start};
    return kleene;
}




void printNFA(NFA nfa, FILE *output) {
    // 创建一个队列存放待访问的状态
    State* queue[1000]; // 假设最多有1000个状态，你可以根据需要增减
    int front = 0, rear = 0;

    // 创建一个标记数组来记录哪些状态已被访问
    bool visited[1000] = {false}; // 这里的大小应该与状态的最大数量相匹配

    // 开始状态入队
    queue[rear++] = nfa.start_state;
    visited[nfa.start_state->stateID] = true;

    while (front != rear) { // 当队列不为空时
        State* currentState = queue[front++];

        fprintf(output, "State: %d, isFinal: %d\n", currentState->stateID, currentState->is_final);
        
        for (int i = 0; i < currentState->num_transitions; ++i) {
            Transition currentTrans = currentState->transitions[i];
            fprintf(output, "\tTransition on '%c' to state %d\n", currentTrans.symbol, currentTrans.to_state->stateID);
            
            // 如果目的状态未被访问，则将其添加到队列中
            if (!visited[currentTrans.to_state->stateID]) {
                queue[rear++] = currentTrans.to_state;
                visited[currentTrans.to_state->stateID] = true;
            }
        }
    }
}
void printNFAAsDot(NFA nfa, FILE *output) {
    fprintf(output, "digraph NFA {\n");
    fprintf(output, "    rankdir=LR;\n");
    fprintf(output, "    node [shape = circle];\n");

    // 创建一个队列存放待访问的状态
    State* queue[1000]; // 假设最多有1000个状态，你可以根据需要增减
    int front = 0, rear = 0;

    // 创建一个标记数组来记录哪些状态已被访问
    bool visited[1000] = {false}; // 这里的大小应该与状态的最大数量相匹配

    // 开始状态入队
    queue[rear++] = nfa.start_state;
    visited[nfa.start_state->stateID] = true;

    while (front != rear) { // 当队列不为空时
        State* currentState = queue[front++];

        // 如果是终态，使用双圆圈表示
        if (currentState->is_final) {
            fprintf(output, "    %d [shape = doublecircle];\n", currentState->stateID);
        } else {
            fprintf(output, "    %d;\n", currentState->stateID);
        }

        for (int i = 0; i < currentState->num_transitions; ++i) {
            Transition currentTrans = currentState->transitions[i];
            // 打印转换，对于空转换使用 'e' 而不是 'ε'
            fprintf(output, "    %d -> %d [label = \"%c\"];\n",
                    currentState->stateID,
                    currentTrans.to_state->stateID,
                    currentTrans.symbol == '\0' ? 'e' : currentTrans.symbol);

            // 如果目的状态未被访问，则将其添加到队列中
            if (!visited[currentTrans.to_state->stateID]) {
                queue[rear++] = currentTrans.to_state;
                visited[currentTrans.to_state->stateID] = true;
            }
        }
    }

    fprintf(output, "}\n");
}



int yylex()
{
    char token = getchar();
    switch(token) {
        case '|': return OR;
        case '*': return STAR;
        case '(': return LPAREN;
        case ')': return RPAREN;
        case '\n':
        case ' ':
        case '\t': return yylex(); // 忽略空白字符
        default:
            if (isalnum(token)) {
                yylval.character = token;
                return CHARACTER;
            }
            break;
    }
    return token;
}


int main(void)
{
    yyin=stdin;
    do{
        yyparse();
    }while(!feof(yyin));
    FILE* file = fopen("Output", "w");
    if (file) {
        printNFAAsDot(EndNFA, file);
        fclose(file);
    } else {
        fprintf(stderr, "Unable to open output file.\n");
    }

    return 0;
}
void yyerror(const char* s){
    fprintf(stderr,"Parse error: %s\n",s);
    exit(1);
}