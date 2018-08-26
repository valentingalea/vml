// https://www.shadertoy.com/view/MdjSzt

float WardAnisotropy (in vec3 vNormal, in vec3 vDirection, in vec3 vEye, in vec3 vLight,in vec2 Roughness) 
{    
    vec3 A = normalize(vEye+vLight);
    
    float X = dot(A,normalize(cross(vNormal,vDirection)))/Roughness.x;
    
    float Y = dot(A,normalize(cross(vNormal,A)))/Roughness.y;
    
    float B = 1.0+dot(A,vNormal);
    
    float XY = -2.0*(pow(X,2.0)+pow(Y,2.0)) / B;
    
    float C = exp(XY) * (1.0/12.5664*Roughness.x*Roughness.y);
    
    return C * (1.0/sqrt(max(dot(vEye,vNormal),0.0)/max(dot(vLight,vNormal),0.0)));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.yy;
    
    vec2 uvl = fragCoord.xy / iResolution.xy-0.5;
    
    //Cones  
    float s1 = 1.0-clamp(length(uv*4.0-vec2(1.5,1.2)),0.0,1.0);
    
    float s2 = 1.0-clamp(length(uv*4.0-vec2(5.67,1.2)),0.0,1.0);
        
    float s3 = 1.0-clamp(length(uv*4.0-vec2(3.67,2.8)),0.0,1.0);
    
    float sph = s1+s2+s3;
    
    float spm = clamp((s1+s2+s3)*96.0,0.0,1.0);
    
    
    //Normals
    float dx = dFdx(sph)*iResolution.x/15.0*spm;
    
    float dy = dFdy(sph)*iResolution.x/15.0*spm;
     
    vec3 vNormal = normalize(vec3(dx,dy,sqrt(clamp(1.0-dx*dx-dy*dy,0.0,1.0))));

    
    //Shading
    vec2 Roughness = vec2(0.2,0.8);

	vec3 Dir1 = normalize(vec3(fract(uv.x*4.0)-0.5,fract(uv.y*4.0)-0.5,0.0));
    
	vec3 Dir2 = normalize(vec3(0.0,1.0,0.0));
    
    vec3 Dir3 = normalize(vec3(uv.x*4.0-5.67,uv.y*4.0-1.2,0.0));
    
    vec3 vLight = normalize(vec3(uvl.x+(0.5*sin(iGlobalTime)),uvl.y+(0.5*cos(iGlobalTime)),0.5));
    
    vec3 vEye = vec3(0.0,0.0,1.0);
    
    float sh = clamp(dot(vNormal,vLight),0.0,1.0);
    
    vec3 Dir = mix(mix(Dir1,Dir2,spm),Dir3,min(s1*48.0,1.0));
    
    vec3 sp = WardAnisotropy(vNormal,Dir,vEye,vLight,Roughness)*vec3(24.0);
    
    vec3 Color = vec3(0.15)+vec3(0.45)*sh+sp;
    
    fragColor = vec4(Color,1.0);
}