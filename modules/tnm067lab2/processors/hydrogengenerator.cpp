#include <modules/tnm067lab2/processors/hydrogengenerator.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/volumeramutils.h>
#include <modules/base/algorithm/dataminmax.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

const ProcessorInfo HydrogenGenerator::processorInfo_{
    "org.inviwo.HydrogenGenerator",  // Class identifier
    "Hydrogen Generator",            // Display name
    "TNM067",                        // Category
    CodeState::Stable,               // Code state
    Tags::CPU,                       // Tags
};

const ProcessorInfo HydrogenGenerator::getProcessorInfo() const { return processorInfo_; }

HydrogenGenerator::HydrogenGenerator()
    : Processor(), volume_("volume"), size_("size_", "Volume Size", 16, 4, 256) {
    addPort(volume_);
    addProperty(size_);
}

void HydrogenGenerator::process() {
    auto vol = std::make_shared<Volume>(size3_t(size_), DataFloat32::get());

    auto ram = vol->getEditableRepresentation<VolumeRAM>();
    auto data = static_cast<float*>(ram->getData());
    util::IndexMapper3D index(ram->getDimensions());

    util::forEachVoxel(*ram, [&](const size3_t& pos) {
        vec3 cartesian = idTOCartesian(pos);
        data[index(pos)] = static_cast<float>(eval(cartesian));
    });

    auto minMax = util::volumeMinMax(ram);
    vol->dataMap_.dataRange = vol->dataMap_.valueRange = dvec2(minMax.first.x, minMax.second.x);

    volume_.setData(vol);
}

vec3 HydrogenGenerator::cartesianToSpherical(vec3 cartesian) {
    vec3 sph{0};

    sph.x = glm::length(cartesian);

    if (abs(sph.x) < 0.00001) return sph;

    sph.y = acos(cartesian.z / sph.x);
    sph.z = atan2(cartesian.y, cartesian.x);

    return sph;
}

double HydrogenGenerator::eval(vec3 cartesian) {
    const auto spherical = cartesianToSpherical(cartesian);

    double yellow = 1.0 / (81.0 * sqrt(6 * glm::pi<double>()));
    double red = 1.0;
    double blue = pow(spherical.x, 2);
    double green = exp(-1.0 * spherical.x / 3.0);
    double magenta = 3 * pow(cos(spherical.y), 2) - 1;

    return pow(yellow*red*blue*green*magenta, 2.0);
}

vec3 HydrogenGenerator::idTOCartesian(size3_t pos) {
    vec3 p(pos);
    p /= size_ - 1;
    return p * (36.0f) - 18.0f;
}

}  // namespace inviwo
