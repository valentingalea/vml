#include <cstdio>
#include <cmath>

#include "vector.h"

typedef vmath::vector<float, 3> vec3;
typedef vmath::vector<float, 2> vec2;
typedef vmath::vector<int, 3> ivec3;
typedef vmath::vector<int, 2> ivec2;

template<int N>
void test_inout(vmath::vector<float, N> &in)
{
	in.x = 1000;
}

void test_funcs()
{
	vec3 a(vec2(1.f, 2.f), 3.f);
	vec3 b(4.f, vec2(5.f, 6.f));
	vec3 c = normalize(cross(a, b));
	vec2 i = a.xz;
	
	test_inout(c);

	printf ("%f %f %f\n", c.x, c.y, c.z);
	printf ("%f %f\n", i[0], i[1]);
}

void test_asm()
{
	ivec3 a, b, c;
	scanf("%i %i %i", &a.x, &b.y, &c.z);

	ivec3 s = a + b + c;

	printf("%i %i %i\n", s.x, s.y, s.z);
}

int main ()
{	
	test_funcs();

	return 0;
}