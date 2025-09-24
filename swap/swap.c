#include "swap.h"

void Swap(char *left, char *right)
{
	char temp_vari = *left;
	*left = *right;
	*right = temp_vari;
}
