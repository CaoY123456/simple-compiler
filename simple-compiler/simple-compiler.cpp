#include "Lexical_analyzer.h"
#include "Grammar_analyzer.h"
#include "Quaternion.h"
using namespace std;

int main() {
    Lexical_analyzer L("input_17.txt");
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
