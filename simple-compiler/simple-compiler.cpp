#include "Lexical_analyzer.h"
#include "Grammar_analyzer.h"
#include "Quaternion.h"
using namespace std;

int main() {
    // 手动添加测试代码文件的路径，
    // 我本地测的时候不知为何相对路径不行，只能是绝对路径
    Lexical_analyzer L("input_1.txt");
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
