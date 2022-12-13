#include "Lexical_analyzer.h"
#include "Grammar_analyzer.h"
#include "Quaternion.h"
using namespace std;

int main() {
    // test-cases里放置的是用于测试的代码文件夹
    string filename = "input_17.txt";
    string dir = "test-cases\\";
    Lexical_analyzer L(dir + filename);
    L.analyse();

    L.show_inf();

    Grammar_analyzer G(L);

    G.analyse();

    G.show_inf();

    Quaternion Q(G);

    Q.construct();

    Q.show_inf();

    return 0;
}
