#include "../vml/vector.h"

using  vec4 = vml::vector<float, 0, 1, 2, 3>;
using  vec3 = vml::vector<float, 0, 1, 2>;
using  vec2 = vml::vector<float, 0, 1>;

namespace ref
{
	typedef vec2& vec2;
	typedef vec3& vec3;
	typedef vec4& vec4;
}

namespace in
{
	typedef const ::vec2& vec2;
	typedef const ::vec3& vec3;
	typedef const ::vec4& vec4;
}

#define uniform
#define in
#define out ref::
#define inout ref::

// verbatim from Shadertoy
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)

/* >>>>>>>>>>>>> SHADER GOES HERE >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord / iResolution.xy;

	// Time varying pixel color
	vec3 col = 0.5 + 0.5 * cos(iTime + uv.xyx + vec3(0, 2, 4));

	// Output to screen
	fragColor = vec4(col, 1.0);
}
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

#undef in
#undef out
#undef inout
#undef uniform

int main()
{
}