#include "homectl/Homectl.h"
#include "homectl/unittest.h"

static byte homectl[sizeof(Homectl)];

void setup() { (new (homectl) Homectl)->setup(); }
void loop() { reinterpret_cast<Homectl *>(homectl)->loop(); }
