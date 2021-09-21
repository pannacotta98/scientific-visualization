#include <modules/tnm067lab2/processors/marchingtetrahedra.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/network/networklock.h>
#include <modules/tnm067lab1/utils/interpolationmethods.h>

namespace inviwo {

size_t MarchingTetrahedra::HashFunc::max = 1;

const ProcessorInfo MarchingTetrahedra::processorInfo_{
    "org.inviwo.MarchingTetrahedra",  // Class identifier
    "Marching Tetrahedra",            // Display name
    "TNM067",                         // Category
    CodeState::Stable,                // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo MarchingTetrahedra::getProcessorInfo() const { return processorInfo_; }

MarchingTetrahedra::MarchingTetrahedra()
    : Processor()
    , volume_("volume")
    , mesh_("mesh")
    , isoValue_("isoValue", "ISO value", 0.5f, 0.0f, 1.0f) {

    addPort(volume_);
    addPort(mesh_);

    addProperty(isoValue_);

    isoValue_.setSerializationMode(PropertySerializationMode::All);

    volume_.onChange([&]() {
        if (!volume_.hasData()) {
            return;
        }
        NetworkLock lock(getNetwork());
        float iso = (isoValue_.get() - isoValue_.getMinValue()) /
                    (isoValue_.getMaxValue() - isoValue_.getMinValue());
        const auto vr = volume_.getData()->dataMap_.valueRange;
        isoValue_.setMinValue(static_cast<float>(vr.x));
        isoValue_.setMaxValue(static_cast<float>(vr.y));
        isoValue_.setIncrement(static_cast<float>(glm::abs(vr.y - vr.x) / 50.0));
        isoValue_.set(static_cast<float>(iso * (vr.y - vr.x) + vr.x));
        isoValue_.setCurrentStateAsDefault();
    });
}

void MarchingTetrahedra::process() {
    auto volume = volume_.getData()->getRepresentation<VolumeRAM>();
    MeshHelper mesh(volume_.getData());

    const auto& dims = volume->getDimensions();
    MarchingTetrahedra::HashFunc::max = dims.x * dims.y * dims.z;

    const float iso = isoValue_.get();

    util::IndexMapper3D indexInVolume(dims);

    const static size_t tetrahedraIds[6][4] = {{0, 1, 2, 5}, {1, 3, 2, 5}, {3, 2, 5, 7},
                                               {0, 2, 4, 5}, {6, 4, 2, 5}, {6, 7, 5, 2}};

    size3_t pos{};
    for (pos.z = 0; pos.z < dims.z - 1; ++pos.z) {
        for (pos.y = 0; pos.y < dims.y - 1; ++pos.y) {
            for (pos.x = 0; pos.x < dims.x - 1; ++pos.x) {
                // Step 1: create current cell

                // The DataPoint index should be the 1D-index for the DataPoint in the cell
                // Use volume->getAsDouble to query values from the volume
                // Spatial position should be between 0 and 1

                Cell c;

                for (int dz = 0; dz <= 1; ++dz) {
                    for (int dy = 0; dy <= 1; ++dy) {
                        for (int dx = 0; dx <= 1; ++dx) {
                            ivec3 offsetInCell{dx, dy, dz};

                            vec3 scaledPos = calculateDataPointPos(pos, offsetInCell, dims);
                            float value = volume->getAsDouble(pos + size3_t{offsetInCell});
                            size_t index = indexInVolume(pos + size3_t{offsetInCell});

                            c.dataPoints[calculateDataPointIndexInCell(offsetInCell)] =
                                MarchingTetrahedra::DataPoint{scaledPos, value, index};
                        }
                    }
                }

                // Step 2: Subdivide cell into tetrahedra (hint: use tetrahedraIds)
                std::vector<Tetrahedra> tetrahedras;
                for (auto const& ids : tetrahedraIds) {
                    Tetrahedra tempTetra;
                    for (std::size_t i = 0; i < std::size(ids); ++i)
                        tempTetra.dataPoints[i] = c.dataPoints[ids[i]];
                    tetrahedras.push_back(tempTetra);
                }

                auto interpolatePosFromIso = [iso](const DataPoint& d1, const DataPoint& d2) {
                    float t = (iso - d1.value) / (d2.value - d1.value);
                    return glm::lerp(d1.pos, d2.pos, t);
                };

                for (const Tetrahedra& tetrahedra : tetrahedras) {
                    // Step three: Calculate for tetra case index
                    int caseId = 0;
                    for (size_t i = 0; i < std::size(tetrahedra.dataPoints); ++i)
                        if (tetrahedra.dataPoints[i].value > iso) caseId += pow(2, i);

                    // step four: Extract triangles
                    switch (caseId) {
                        case 0:
                        case 15:
                            break;
                        case 1:
                        case 14: {
                            auto v1 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[0],
                                                      tetrahedra.dataPoints[1]),
                                tetrahedra.dataPoints[0].index, tetrahedra.dataPoints[1].index);
                            auto v2 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[0],
                                                      tetrahedra.dataPoints[3]),
                                tetrahedra.dataPoints[0].index, tetrahedra.dataPoints[3].index);
                            auto v3 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[0],
                                                      tetrahedra.dataPoints[2]),
                                tetrahedra.dataPoints[0].index, tetrahedra.dataPoints[2].index);
                            if (caseId == 14)
                                mesh.addTriangle(v1, v2, v3);
                            else
                                mesh.addTriangle(v1, v3, v2);
                            break;
                        }
                        case 2:
                        case 13: {
                            auto v1 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[1],
                                                      tetrahedra.dataPoints[0]),
                                tetrahedra.dataPoints[1].index, tetrahedra.dataPoints[0].index);
                            auto v2 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[1],
                                                      tetrahedra.dataPoints[2]),
                                tetrahedra.dataPoints[1].index, tetrahedra.dataPoints[2].index);
                            auto v3 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[1],
                                                      tetrahedra.dataPoints[3]),
                                tetrahedra.dataPoints[1].index, tetrahedra.dataPoints[3].index);
                            if (caseId == 13)
                                mesh.addTriangle(v1, v2, v3);
                            else
                                mesh.addTriangle(v1, v3, v2);
                            break;
                        }
                        case 3:
                        case 12: {
                            auto v1 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[1],
                                                      tetrahedra.dataPoints[3]),
                                tetrahedra.dataPoints[1].index, tetrahedra.dataPoints[3].index);
                            auto v2 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[0],
                                                      tetrahedra.dataPoints[3]),
                                tetrahedra.dataPoints[0].index, tetrahedra.dataPoints[3].index);
                            auto v3 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[0],
                                                      tetrahedra.dataPoints[2]),
                                tetrahedra.dataPoints[0].index, tetrahedra.dataPoints[2].index);
                            auto v4 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[1],
                                                      tetrahedra.dataPoints[2]),
                                tetrahedra.dataPoints[1].index, tetrahedra.dataPoints[2].index);
                            if (caseId == 12) {
                                mesh.addTriangle(v1, v2, v4);
                                mesh.addTriangle(v2, v3, v4);
                            } else {
                                mesh.addTriangle(v1, v4, v2);
                                mesh.addTriangle(v2, v4, v3);
                            }
                            break;
                        }
                        case 4:
                        case 11: {
                            auto v1 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[2],
                                                      tetrahedra.dataPoints[0]),
                                tetrahedra.dataPoints[2].index, tetrahedra.dataPoints[0].index);
                            auto v2 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[2],
                                                      tetrahedra.dataPoints[3]),
                                tetrahedra.dataPoints[2].index, tetrahedra.dataPoints[3].index);
                            auto v3 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[2],
                                                      tetrahedra.dataPoints[1]),
                                tetrahedra.dataPoints[2].index, tetrahedra.dataPoints[1].index);
                            if (caseId == 11)
                                mesh.addTriangle(v1, v2, v3);
                            else
                                mesh.addTriangle(v1, v3, v2);
                            break;
                        }
                        case 5:
                        case 10: {
                            auto v1 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[0],
                                                      tetrahedra.dataPoints[1]),
                                tetrahedra.dataPoints[0].index, tetrahedra.dataPoints[1].index);
                            auto v2 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[1],
                                                      tetrahedra.dataPoints[2]),
                                tetrahedra.dataPoints[1].index, tetrahedra.dataPoints[2].index);
                            auto v3 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[3],
                                                      tetrahedra.dataPoints[2]),
                                tetrahedra.dataPoints[3].index, tetrahedra.dataPoints[2].index);
                            auto v4 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[0],
                                                      tetrahedra.dataPoints[3]),
                                tetrahedra.dataPoints[0].index, tetrahedra.dataPoints[3].index);
                            if (caseId == 5) {
                                mesh.addTriangle(v1, v2, v4);
                                mesh.addTriangle(v2, v3, v4);
                            } else {
                                mesh.addTriangle(v1, v4, v2);
                                mesh.addTriangle(v2, v4, v3);
                            }
                            break;
                        }
                        case 6:
                        case 9: {
                            auto v1 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[0],
                                                      tetrahedra.dataPoints[1]),
                                tetrahedra.dataPoints[0].index, tetrahedra.dataPoints[1].index);
                            auto v2 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[1],
                                                      tetrahedra.dataPoints[3]),
                                tetrahedra.dataPoints[1].index, tetrahedra.dataPoints[3].index);
                            auto v3 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[3],
                                                      tetrahedra.dataPoints[2]),
                                tetrahedra.dataPoints[3].index, tetrahedra.dataPoints[2].index);
                            auto v4 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[2],
                                                      tetrahedra.dataPoints[0]),
                                tetrahedra.dataPoints[2].index, tetrahedra.dataPoints[0].index);
                            if (caseId == 6) {
                                mesh.addTriangle(v1, v2, v4);
                                mesh.addTriangle(v2, v3, v4);
                            } else {
                                mesh.addTriangle(v1, v4, v2);
                                mesh.addTriangle(v2, v4, v3);
                            }
                            break;
                        }
                        case 7:
                        case 8: {
                            auto v1 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[3],
                                                      tetrahedra.dataPoints[1]),
                                tetrahedra.dataPoints[3].index, tetrahedra.dataPoints[1].index);
                            auto v2 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[3],
                                                      tetrahedra.dataPoints[2]),
                                tetrahedra.dataPoints[3].index, tetrahedra.dataPoints[2].index);
                            auto v3 = mesh.addVertex(
                                interpolatePosFromIso(tetrahedra.dataPoints[3],
                                                      tetrahedra.dataPoints[0]),
                                tetrahedra.dataPoints[3].index, tetrahedra.dataPoints[0].index);
                            if (caseId == 7)
                                mesh.addTriangle(v1, v2, v3);
                            else
                                mesh.addTriangle(v1, v3, v2);
                            break;
                        }
                    }
                }
            }
        }
    }

    mesh_.setData(mesh.toBasicMesh());
}

int MarchingTetrahedra::calculateDataPointIndexInCell(ivec3 index3D) {
    return 1 * index3D.x + 2 * index3D.y + 4 * index3D.z;
}

vec3 MarchingTetrahedra::calculateDataPointPos(size3_t posVolume, ivec3 posCell, ivec3 dims) {
    vec3 posVolumeFloat{posVolume};
    vec3 posCellFloat{posCell};
    vec3 scale{dims - 1};

    return (posVolumeFloat + posCellFloat) / scale;
}

MarchingTetrahedra::MeshHelper::MeshHelper(std::shared_ptr<const Volume> vol)
    : edgeToVertex_()
    , vertices_()
    , mesh_(std::make_shared<BasicMesh>())
    , indexBuffer_(mesh_->addIndexBuffer(DrawType::Triangles, ConnectivityType::None)) {
    mesh_->setModelMatrix(vol->getModelMatrix());
    mesh_->setWorldMatrix(vol->getWorldMatrix());
}

void MarchingTetrahedra::MeshHelper::addTriangle(size_t i0, size_t i1, size_t i2) {
    IVW_ASSERT(i0 != i1, "i0 and i1 should not be the same value");
    IVW_ASSERT(i0 != i2, "i0 and i2 should not be the same value");
    IVW_ASSERT(i1 != i2, "i1 and i2 should not be the same value");

    indexBuffer_->add(static_cast<glm::uint32_t>(i0));
    indexBuffer_->add(static_cast<glm::uint32_t>(i1));
    indexBuffer_->add(static_cast<glm::uint32_t>(i2));

    const auto a = std::get<0>(vertices_[i0]);
    const auto b = std::get<0>(vertices_[i1]);
    const auto c = std::get<0>(vertices_[i2]);

    const vec3 n = glm::normalize(glm::cross(b - a, c - a));
    std::get<1>(vertices_[i0]) += n;
    std::get<1>(vertices_[i1]) += n;
    std::get<1>(vertices_[i2]) += n;
}

std::shared_ptr<BasicMesh> MarchingTetrahedra::MeshHelper::toBasicMesh() {
    for (auto& vertex : vertices_) {
        // Normalize the normal of the vertex
        std::get<1>(vertex) = glm::normalize(std::get<1>(vertex));
    }
    mesh_->addVertices(vertices_);
    return mesh_;
}

std::uint32_t MarchingTetrahedra::MeshHelper::addVertex(vec3 pos, size_t i, size_t j) {
    IVW_ASSERT(i != j, "i and j should not be the same value");
    if (j < i) std::swap(i, j);

    auto [edgeIt, inserted] = edgeToVertex_.try_emplace(std::make_pair(i, j), vertices_.size());
    if (inserted) {
        vertices_.push_back({pos, vec3(0, 0, 0), pos, vec4(0.7f, 0.7f, 0.7f, 1.0f)});
    }
    return static_cast<std::uint32_t>(edgeIt->second);
}

}  // namespace inviwo
