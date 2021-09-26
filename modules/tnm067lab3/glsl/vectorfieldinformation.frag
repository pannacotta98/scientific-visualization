#include "utils/structs.glsl"

uniform sampler2D vfColor;
uniform ImageParameters vfParameters;
in vec3 texCoord_;

float passThrough(vec2 coord){
    return texture(vfColor,coord).x;
}

vec2 getdVdX(vec2 velo, vec2 pixelSize) {
    vec2 dXleftTerm = vec2(velo.x + pixelSize.x, velo.y);
    vec2 dXrightTerm = vec2(velo.x - pixelSize.x, velo.y);
    return (dXleftTerm + dXrightTerm) / (2*pixelSize.x);
}

vec2 getdVdY(vec2 velo, vec2 pixelSize) {
    vec2 dYleftTerm = vec2(velo.x, velo.y + pixelSize.y);
    vec2 dYrightTerm = vec2(velo.x, velo.y - pixelSize.y);
    return (dYleftTerm + dYrightTerm) / (2*pixelSize.y);
}

float magnitude( vec2 coord ){
    //TASK 1: find the magnitude of the vectorfield at the position coords
    vec2 velo = texture2D(vfColor,coord.xy).xy;
    return sqrt(pow(velo.x,2) + pow(velo.y, 2));
}

float divergence(vec2 coord){
    //TASK 2: find the divergence of the vectorfield at the position coords
    vec2 pixelSize = vfParameters.reciprocalDimensions;
    vec2 velo = texture2D(vfColor,coord.xy).xy;

    // dV/dX
    vec2 dVdX = getdVdX(velo, pixelSize);

    // dV/dY
    vec2 dVdY = getdVdY(velo, pixelSize);

    return dVdX.x + dVdY.y;
}

float rotation(vec2 coord){
    //TASK 3: find the curl of the vectorfield at the position coords
    vec2 pixelSize = vfParameters.reciprocalDimensions;
    vec2 velo = texture2D(vfColor,coord.xy).xy;

    // dV/dX
    vec2 dVdX = getdVdX(velo, pixelSize);

    // dV/dY
    vec2 dVdY = getdVdY(velo, pixelSize);

    return dVdY.x - dVdX.y;
}

void main(void) {
    float v = OUTPUT(texCoord_.xy);
    FragData0 = vec4(v);
}
