#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <cassert>
#include <algorithm>

#include "Model.h"
#include "../utils/ComHelpers.h"

namespace renderer
{
    Model::Model()
        :m_importer(),
        m_scene(nullptr),
        m_numMeshes(0),
        m_totalVertices(0),
        m_totalNormals(0),
        m_totalTexCoords(0),
        m_totalIndices(0),
        m_numTris(0),
        m_meshes(),
        m_fileDir(),
        m_deviceRef(nullptr)
    {
    }

    Model::~Model()
    {
        m_meshes.clear();

		for (auto mat : m_materials)
		{
			delete mat;
		}
    }

    void Model::LoadModelAndParseData(IDirect3DDevice9* device, std::string filepath)
    {
        m_fileDir = filepath.substr(0, filepath.find_last_of("/") + 1);
        m_deviceRef = device;

        const auto flags = aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_ConvertToLeftHanded |
            aiProcess_JoinIdenticalVertices |
            aiProcess_RemoveRedundantMaterials | aiProcess_ValidateDataStructure |
            aiProcess_FlipUVs | aiProcess_OptimizeMeshes;

        m_scene = m_importer.ReadFile(filepath, flags);
        assert(m_scene != nullptr);
        m_numMeshes = m_scene->mNumMeshes;

        ProcessModelVertexIndex();
    }

    void Model::ProcessModelVertexIndex()
    {
        m_meshes.reserve(m_numMeshes);
        auto const meshes = m_scene->mMeshes;
        aiMaterial** materials = m_scene->mMaterials;
        uint32_t numMaterials = m_scene->mNumMaterials;

        int32_t totalVertices = 0;
        int32_t totalIndices = 0;

        this->ProcessModelMaterials(materials, numMaterials); //get the material list ready for ref-counting

        for (auto itr = 0; itr < m_numMeshes; ++itr)
        {
            std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

            totalVertices += meshes[itr]->mNumVertices;
            totalIndices += meshes[itr]->mNumFaces * 3;
            m_numTris += meshes[itr]->mNumFaces;

            mesh->SetNumVertices(meshes[itr]->mNumVertices);
            mesh->SetNumNormals(meshes[itr]->mNumVertices); //number of normals = number of vertices
            mesh->SetNumTexCoords(meshes[itr]->mNumVertices); //number of texcoord = number of vertices
            mesh->SetNumTangents(meshes[itr]->mNumVertices);
            mesh->SetNumBiTangents(meshes[itr]->mNumVertices);
            mesh->SetNumIndices(meshes[itr]->mNumFaces * 3);
            mesh->SetNumTris(meshes[itr]->mNumFaces);

            const auto numVert = meshes[itr]->mNumVertices;

            uint32_t iter = 0;
            for (iter = 0; iter < numVert; ++iter)
            {
                float x = meshes[itr]->mVertices[iter].x;
                float y = meshes[itr]->mVertices[iter].y;
                float z = meshes[itr]->mVertices[iter].z;
                mesh->AppendVertices(x, y, z);

                x = meshes[itr]->mNormals[iter].x;
                y = meshes[itr]->mNormals[iter].y;
                z = meshes[itr]->mNormals[iter].z;
                mesh->AppendNormals(x, y, z);

				x = meshes[itr]->HasTextureCoords(0) ? meshes[itr]->mTextureCoords[0][iter].x : 0.0f;
                y = meshes[itr]->HasTextureCoords(0) ? meshes[itr]->mTextureCoords[0][iter].y : 0.0f;
                mesh->AppendTexCoords(x, y);

                if (meshes[itr]->HasTangentsAndBitangents())
                {
                    x = meshes[itr]->mTangents[iter].x;
                    y = meshes[itr]->mTangents[iter].y;
                    z = meshes[itr]->mTangents[iter].z;
                    mesh->AppendTangents(x, y, z);

                    x = meshes[itr]->mBitangents[iter].x;
                    y = meshes[itr]->mBitangents[iter].y;
                    z = meshes[itr]->mBitangents[iter].z;
                    mesh->AppendBiTangents(x, y, z);
                }
                else
                {
                    x = y = z = 0.0f;
                    mesh->AppendTangents(x, y, z);
                    mesh->AppendBiTangents(x, y, z);
                }
            }

            const auto numFaces = meshes[itr]->mNumFaces;
            for (iter = 0; iter < numFaces; ++iter)
            {
                const uint32_t vertexA = meshes[itr]->mFaces[iter].mIndices[0];
                const uint32_t vertexB = meshes[itr]->mFaces[iter].mIndices[1];
                const uint32_t vertexC = meshes[itr]->mFaces[iter].mIndices[2];

                mesh->AppendIndices(vertexA, vertexB, vertexC);
            }
            mesh->SetName(meshes[itr]->mName.C_Str());
            mesh->SetMaterialIndex(meshes[itr]->mMaterialIndex);
			
            m_meshes.emplace_back(std::move(mesh));
        }

        //sort Meshlist
		std::sort(m_meshes.begin(), m_meshes.end(), [](std::shared_ptr<Mesh> a, std::shared_ptr<Mesh> b)
			{
				return a->GetMaterialIndex() < b->GetMaterialIndex();
			});

        m_totalVertices = totalVertices;
        m_totalNormals = totalVertices;
        m_totalIndices = totalIndices;
    }
    void Model::ProcessModelMaterials(aiMaterial** materials, uint32_t materialCount)
    {
        m_materials.reserve(materialCount);
        
        for (uint32_t itr = 0; itr < materialCount; ++itr)
        {
            Material* material = new Material();
            aiString path;
           
			if (materials[itr]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			{
				materials[itr]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
				std::string fullTexturePath = m_fileDir + path.C_Str();
				ComResult(D3DXCreateTextureFromFileA(m_deviceRef, fullTexturePath.c_str(), material->GetPtrToTextureOfType(Material::TextureType::Diffuse)));
			}
            else
            {
                std::string fullTexturePath = "data/DefaultTex/default_diffuse.png";
                ComResult(D3DXCreateTextureFromFileA(m_deviceRef, fullTexturePath.c_str(), material->GetPtrToTextureOfType(Material::TextureType::Diffuse)));
            }

			if (materials[itr]->GetTextureCount(aiTextureType_HEIGHT) > 0)
			{
				materials[itr]->GetTexture(aiTextureType_HEIGHT, 0, &path);
				std::string fullTexturePath = m_fileDir + path.C_Str();
				ComResult(D3DXCreateTextureFromFileA(m_deviceRef, fullTexturePath.c_str(), material->GetPtrToTextureOfType(Material::TextureType::Normal)));
			}
            else
            {
                std::string fullTexturePath = "data/DefaultTex/default_normal.png";
                ComResult(D3DXCreateTextureFromFileA(m_deviceRef, fullTexturePath.c_str(), material->GetPtrToTextureOfType(Material::TextureType::Normal)));
            }

			if (materials[itr]->GetTextureCount(aiTextureType_SHININESS) > 0)
			{
				materials[itr]->GetTexture(aiTextureType_SHININESS, 0, &path);
				std::string fullTexturePath = m_fileDir + path.C_Str();
				ComResult(D3DXCreateTextureFromFileA(m_deviceRef, fullTexturePath.c_str(), material->GetPtrToTextureOfType(Material::TextureType::Specular)));
			}
			else
			{
				std::string fullTexturePath = "data/DefaultTex/default_specular.png";
				ComResult(D3DXCreateTextureFromFileA(m_deviceRef, fullTexturePath.c_str(), material->GetPtrToTextureOfType(Material::TextureType::Specular)));
			}

			if (materials[itr]->GetTextureCount(aiTextureType_OPACITY) > 0)
			{
				materials[itr]->GetTexture(aiTextureType_OPACITY, 0, &path);
				std::string fullTexturePath = m_fileDir + path.C_Str();
				ComResult(D3DXCreateTextureFromFileA(m_deviceRef, fullTexturePath.c_str(), material->GetPtrToTextureOfType(Material::TextureType::Opacity)));
			}
			else
			{
				std::string fullTexturePath = "data/DefaultTex/default_opacity.png";
				ComResult(D3DXCreateTextureFromFileA(m_deviceRef, fullTexturePath.c_str(), material->GetPtrToTextureOfType(Material::TextureType::Opacity)));
			}
           m_materials.emplace_back(material);
        }
    }
}