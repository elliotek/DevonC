#include "Compiler.h"
#include <iostream>
#include <time.h>

int main(const int argc, char* argv[])  // NOLINT(bugprone-exception-escape)
{
	if (argc > 1)
	{
		clock_t t = clock();

		DevonC::Compiler Compiler;
		Compiler.Compile(argv[1]);

		printf("Compiled in %fs.\n", float(clock() - t) / CLOCKS_PER_SEC);

		const int NbErr = Compiler.GetNbErrors();
		printf("%d error%s.", NbErr, NbErr>1?"s":"");

		Compiler.DumpDebug();
	}

	return 1;
}
