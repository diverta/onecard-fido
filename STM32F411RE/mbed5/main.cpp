#include "mbed.h"

void application_initialize(void);
bool application_main(void);

int main(void)
{
    // 初期化処理
    application_initialize();

    // 主処理
    while (application_main());
}
