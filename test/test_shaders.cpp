#include "c4droid.h"

#include "shader/sandbox.h"

 vec3 sandbox::iResolution = vec3(100, 100, 0);
float sandbox::iTime = 0.f;
float sandbox::iTimeDelta = 0.f;

int main()
{
	sandbox::fragment_shader ps;
	ps.main(ps.gl_FragColor, vec2(0));
}