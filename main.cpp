#include <cstdio>
#include <cmath>

#include "vector.h"

typedef vmath::vector<float, 4> vec4;
typedef vmath::vector<float, 3> vec3;
typedef vmath::vector<float, 2> vec2;

void test_inout(vec3 &in)
{
	in.x = 1000;
}

int main ()
{	
	vec3 a(vec2(1.f, 2.f), 3.f);
	vec3 b(4.f, vec2(5.f, 6.f));
	vec3 c = a - b; //normalize(cross(a, b));
	vec2 i = a.xz;
	
	test_inout(c);

	printf ("%f %f %f\n", c.x, c.y, c.z);
	printf ("%f %f\n", i[0], i[1]);
	
	return 0;
}