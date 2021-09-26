uniform sampler2D vfColor;
uniform sampler2D noiseColor;

uniform int nSteps;
uniform float stepSize;

in vec3 texCoord_;

/*
* Traverse the vector field and sample the noise image
* @param posF Starting position
* @param stepSize length of each step
* @param nSteps the number of steps to traverse
* @param accVal the accumulated value from sampling the noise image
* @param nSamples the number of samples used for v
*/

void traverse(vec2 posF, float stepSize, int nSteps, inout float accVal, inout int nSamples) {
    // traverse the vectorfield staring at `posF` for `nSteps` using `stepSize` and sample the noiseColor texture for each position
    // store the accumulated value in `accVal` and the amount of samples in `nSamples`
    float boxFilter = 1.0 / (2.0*float(nSteps) - 1.0);

//    nSamples = 1;

    vec2 currentPos = posF;
    vec2 currentDir = vec2(0.0, 0.0);
    for (int i = 0; i < nSteps; ++i) {
        currentDir = texture(vfColor, currentPos).xy;



        currentPos = (currentPos + currentDir*stepSize) * boxFilter;

        accVal += length(texture(noiseColor, currentPos).rgb);
        nSamples++;
    }
}

void main(void) {
    float accVal = texture(noiseColor, texCoord_.xy).r;
    int nSamples = 1;
    
    //traverse the vector field both forward and backwards to calculate the output color
    traverse(texCoord_.xy, stepSize, nSteps, accVal, nSamples);
    traverse(texCoord_.xy, -1.0*stepSize, nSteps, accVal, nSamples);

    accVal /= nSamples;
    FragData0 = vec4(accVal, accVal, accVal, 1);
}
