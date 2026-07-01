#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <unordered_map>
#include <list>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iomanip>

using namespace std;
using namespace std::chrono;

const int MX = 500005;

// 1. 고정 배열
struct ArrayTrie { int nxt[MX][26]; ArrayTrie() { for(int i=0; i<MX; i++) for(int j=0; j<26; j++) nxt[i][j] = -1; } void insert(const string& s) { int cur = 0; for(char c : s) { if(nxt[cur][c-'a'] == -1) nxt[cur][c-'a'] = 1; cur = nxt[cur][c-'a']; } } };
// 2. 동적 배열
struct VectorTrie { vector<pair<char, int>> nxt[MX]; void insert(const string& s) { int cur = 0; for(char c : s) { bool f=false; for(auto& p:nxt[cur]) if(p.first==c) {cur=p.second; f=true; break;} if(!f) {nxt[cur].push_back({c, 1}); cur=1;} } } };
// 3. 연결 리스트
struct ListTrie { list<pair<char, int>> nxt[MX]; void insert(const string& s) { int cur = 0; for(char c : s) { bool f=false; for(auto& p:nxt[cur]) if(p.first==c) {cur=p.second; f=true; break;} if(!f) {nxt[cur].push_back({c, 1}); cur=1;} } } };
// 4. 이진 검색 트리
struct MapTrie { map<char, int> nxt[MX]; void insert(const string& s) { int cur = 0; for(char c : s) { if(nxt[cur].find(c)==nxt[cur].end()) nxt[cur][c]=1; cur=nxt[cur][c]; } } };
// 5. 해시 (unordered_map)
struct HashTrie { unordered_map<char, int> nxt[MX]; void insert(const string& s) { int cur = 0; for(char c : s) { if(nxt[cur].find(c)==nxt[cur].end()) nxt[cur][c]=1; cur=nxt[cur][c]; } } };

template <typename T>
void run_and_print(string name) {
    pid_t pid = fork();
    if (pid == 0) {
        int N = 200000; // 프로세스별 독립 실행을 위해 20만으로 조정
        vector<string> data(N, "abcdefghij");
        T* t = new T();
        auto start = steady_clock::now();
        for(auto& s : data) t->insert(s);
        auto end = steady_clock::now();
        
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        
        cout << "| " << left << setw(14) << name 
             << " | " << setw(10) << duration_cast<milliseconds>(end - start).count() << "ms"
             << " | " << setw(10) << usage.ru_maxrss << " KB |" << endl;
        exit(0);
    } else {
        wait(NULL);
    }
}

int main() {
    cout << "| " << left << setw(15) << "구조체  이름" << " | " << setw(10) << "시  간" << "     | " << setw(10) << "메 모 리" << "      |" << endl;
    cout << "|--------------|--------------|---------------|" << endl;
    run_and_print<ArrayTrie>("고정   배열 ");
    run_and_print<VectorTrie>("동적   배열 ");
    run_and_print<ListTrie>("연결 리스트 ");
    run_and_print<MapTrie>("이진검색트리");
    run_and_print<HashTrie>("해시");
    return 0;
}