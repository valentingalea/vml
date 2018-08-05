#include <cstdio>

constexpr static auto str_type = "swizzler";
constexpr static auto str_xyzw = "xyzw";
constexpr static auto str_rgba = "rgba";
constexpr static auto str_stpq = "stpq";
	
void gen_perm_for_2_swizzle(int V)
{
	for (int j = 0; j < V; j++) {
		for (int i = 0; i < V; i++) {
			printf("%s<T, %d, %d, %d> %c%c, %c%c, %c%c;\n",
				str_type, V,
				j, i,
				str_xyzw[j], str_xyzw[i],
				str_rgba[j], str_rgba[i],
				str_stpq[j], str_stpq[i]
			);
		}
	}
}

void gen_perm_for_3_swizzle(int V)
{
	for (int k = 0; k < V; k++) {
		for (int j = 0; j < V; j++) {
			for (int i = 0; i < V; i++) {
				printf("%s<T, %d, %d, %d, %d> %c%c%c, %c%c%c, %c%c%c;\n",
					str_type, V,
					k, j, i,
					str_xyzw[k], str_xyzw[j], str_xyzw[i],
					str_rgba[k], str_rgba[j], str_rgba[i],
					str_stpq[k], str_stpq[j], str_stpq[i]
				);
			}
		}
	}
}

void gen_perm_for_4_swizzle(int V)
{
	for (int l = 0; l < V; l++) {
		for (int k = 0; k < V; k++) {
			for (int j = 0; j < V; j++) {
				for (int i = 0; i < V; i++) {
					printf("%s<T, %d, %d, %d, %d, %d> %c%c%c%c, %c%c%c%c, %c%c%c%c;\n",
						str_type, V,
						l, k, j, i,
						str_xyzw[l], str_xyzw[k], str_xyzw[j], str_xyzw[i],
						str_rgba[l], str_rgba[k], str_rgba[j], str_rgba[i],
						str_stpq[l], str_stpq[k], str_stpq[j], str_stpq[i]
					);
				}
			}
		}
	}
}

int main()
{
#if 1
	printf("vec2\n");
	gen_perm_for_2_swizzle(2); // vec2 by itself
	gen_perm_for_3_swizzle(2); // vec3 swizzles of vec2
	gen_perm_for_4_swizzle(2); // vec3 swizzles of vec2
	printf("\n");
#endif

#if 1
	printf("vec3\n");
	gen_perm_for_2_swizzle(3); // vec2 swizzles of vec3
	gen_perm_for_3_swizzle(3); // vec3 itself
	gen_perm_for_4_swizzle(3); // vec4 swizzles of vec3
	printf("\n");
#endif

#if 1
	printf("vec4\n");
	gen_perm_for_2_swizzle(4); // vec2 swizzles of vec4
	gen_perm_for_3_swizzle(4); // vec3 swizzles of vec3
	gen_perm_for_4_swizzle(4); // vec4 itself
	printf("\n");
#endif

	return 0;
}