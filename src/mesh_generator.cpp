#include "mesh_generator.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

using namespace godot;

void MeshGeneratorExtension::_bind_methods() {
    ClassDB::bind_method(D_METHOD("generate", "values"), &MeshGeneratorExtension::generate);
    ClassDB::bind_method(D_METHOD("fillNoise", "size", "noise"), &MeshGeneratorExtension::fillNoise);
    ClassDB::bind_method(D_METHOD("fillSphere", "size"), &MeshGeneratorExtension::fillSphere);
}

MeshGeneratorExtension::MeshGeneratorExtension() {
	// Initialize any variables here.
}

MeshGeneratorExtension::~MeshGeneratorExtension() {
	// Add your cleanup here.
}

PackedFloat32Array MeshGeneratorExtension::fillSphere(const int size)
{
    PackedFloat32Array values;
    values.resize(size * size * size);

    Vector3 center = Vector3(size / 2, size / 2, size / 2);
    real_t radius = center.length();

    for (int x = 0; x < size ; ++x)
    {
        for (int y = 0; y < size ; ++y)
        {        
            for (int z = 0; z < size ; ++z)
            {
                values.set(x * size * size + y * size + z, UtilityFunctions::remap((Vector3(x, y, z) - center).length(), 0, radius, -1, 1) );
            }
        }
    }

    return values;
}

PackedFloat32Array MeshGeneratorExtension::fillNoise(const int size, const Ref<FastNoiseLite> noise)
{
    PackedFloat32Array values;
    values.resize(size * size * size);

    Vector3 center = Vector3(size / 2, size / 2, size / 2);
    real_t radius = center.length();

    for (int x = 0; x < size ; ++x)
    {
        for (int y = 0; y < size ; ++y)
        {        
            for (int z = 0; z < size ; ++z)
            {
                values.set(x * size * size + y * size + z, noise->get_noise_3d(x, y, z));
            }
        }
    }

    return values;
}

Ref<ArrayMesh> MeshGeneratorExtension::generate(const int size, const float resolution, const PackedFloat32Array values)
{
    int offset_x = size * size;
	int offset_y = size;
	int offset_z = 1;

    SurfaceTool st;
    st.begin(Mesh::PRIMITIVE_TRIANGLES);

    for (int x = 0; x < size - 1 ; ++x)
    {
        for (int y = 0; y < size - 1 ; ++y)
        {        
            for (int z = 0; z < size - 1 ; ++z)
            {
                int index = 0;
                int v = x * offset_x + y * offset_y + z;

                if (values[v] <= 0.0) index += 1;
                if (values[v + offset_x] <= 0.0) index += 2;
                if (values[v + offset_x + offset_y] <= 0.0) index += 4;
                if (values[v + offset_y] <= 0.0) index += 8;
                if (values[v + offset_z] <= 0.0) index += 16;
                if (values[v + offset_x + offset_z] <= 0.0) index += 32;
                if (values[v + offset_x + offset_y + offset_z] <= 0.0) index += 64;
                if (values[v + offset_y + offset_z] <= 0.0) index += 128;
                
                if (index == 0 || index == 255) continue;

                for (int tri = 0; tri < 5; ++tri)
                {
                    if (triangleTable[index][3 * tri] < 0) break;

                    for (int tri_corner = 0; tri_corner < 3; ++tri_corner)
                    {
                        int ei = triangleTable[index][3 * tri + tri_corner];
						Vector3 edge1I = edgeVertexOffsets[ei][0];
						Vector3 edge2I = edgeVertexOffsets[ei][1];
						Vector3 e1 = edge1I * resolution;
						Vector3 e2 = edge2I * resolution;

                        // Vector3 mp = (e1 + e2) * 0.5;
                        
						float_t s1 = values[ (x + edge1I.x) * offset_x + (y + edge1I.y) * offset_y + (z + edge1I.z) ];
						float_t delta = s1 - values[ (x + edge2I.x) * offset_x + (y + edge2I.y) * offset_y + (z + edge2I.z) ];
                        float_t ofst;
						if (delta == 0.0) { 
                            ofst = 0.5;
                        } else {
                            ofst = s1 / delta;
                        }
						Vector3 mp = e1 + ofst * (e2 - e1);

                        st.add_vertex(Vector3(x * resolution, y * resolution, z * resolution) + mp);
                    }
                }
            }
        }
    }

    st.generate_normals();
    return st.commit();
}
