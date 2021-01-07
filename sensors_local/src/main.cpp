#include "homectl/Homectl.h"

static byte homectl[sizeof(Homectl)];

void setup() { (new (homectl) Homectl)->setup(); }
void loop() { reinterpret_cast<Homectl *>(homectl)->loop(); }
