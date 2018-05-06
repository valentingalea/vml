#include <cstdio>

constexpr static auto *indices = "xyzw";
	
void gen_vec2(int N)
{
	for (int j = 0; j < N; j++) {
		for (int i = 0; i < N; i++) {
			printf("swizzler<T, 2, %d, %d> %c%c;\n",
				j, i, indices[j], indices[i]);
		}
	}
}

void gen_vec3()
{
	for (int k = 0; k < 3; k++) {
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 3; i++) {
				printf("swizzler<T, 3, %d, %d, %d> %c%c%c;\n",
				k, j, i, indices[k], indices[j], indices[i]);
			}
		}
	}
}

int main()
{
	gen_vec2(2); // for pure vec2
	printf("\n");

	gen_vec2(3); // for vec2 swizzles of vec3
	printf("\n");

	gen_vec3();
	printf("\n");

	return 0;
}