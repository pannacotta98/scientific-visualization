#include "utils/structs.glsl"

uniform sampler2D vfColor;
uniform ImageParameters vfParameters;
in vec3 texCoord_;

float passThrough(vec2 coord){
    return texture(vfColor,coord).x;
}

vec2 getdVdX(vec2 pos, vec2 pixelSize) {
    vec2 dXleftTerm = texture(vfColor, vec2(pos.x + pixelSize.x, pos.y)).xy;
    vec2 dXrightTerm = texture(vfColor, vec2(pos.x - pixelSize.x, pos.y)).xy;
    return (dXleftTerm - dXrightTerm) / (2*pixelSize.x);
}

vec2 getdVdY(vec2 pos, vec2 pixelSize) {
    vec2 dYleftTerm = texture(vfColor, vec2(pos.x, pos.y + pixelSize.y)).xy;
    vec2 dYrightTerm = texture(vfColor, vec2(pos.x, pos.y - pixelSize.y)).xy;
    return (dYleftTerm - dYrightTerm) / (2*pixelSize.y);
}

float magnitude( vec2 coord ){
    //TASK 1: find the magnitude of the vectorfield at the position coords
    vec2 velo = texture2D(vfColor,coord.xy).xy;
    return sqrt(pow(velo.x,2) + pow(velo.y, 2)); // Add velo.z for third dimension
}

float divergence(vec2 coord){
    //TASK 2: find the divergence of the vectorfield at the position coords
    vec2 pixelSize = vfParameters.reciprocalDimensions;

    // dV/dX
    vec2 dVdX = getdVdX(coord.xy, pixelSize);

    // dV/dY
    vec2 dVdY = getdVdY(coord.xy, pixelSize);

    // define dVdZ, add in result for third dimension

    // Black areas are where divergence < 0, it's a sink

    return dVdX.x + dVdY.y;
}

float rotation(vec2 coord){
    //TASK 3: find the curl of the vectorfield at the position coords
    vec2 pixelSize = vfParameters.reciprocalDimensions;

    // dV/dX
    vec2 dVdX = getdVdX(coord.xy, pixelSize);

    // dV/dY
    vec2 dVdY = getdVdY(coord.xy, pixelSize);

    // define dVdZ, the result becomes a vector and more complex

    return dVdX.y - dVdY.x; // Which way?
}

void main(void) {
    float v = OUTPUT(texCoord_.xy);
    FragData0 = vec4(v);
}
