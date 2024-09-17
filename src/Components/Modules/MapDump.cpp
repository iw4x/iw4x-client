#include <STDInclude.hpp>
#include "MapDump.hpp"

namespace Components
{
	class MapDumper
	{
	public:
		MapDumper(Game::GfxWorld* world) : world_(world)
		{
		}

		void dump()
		{
			if (!this->world_) return;

			Logger::Print("Exporting '{}'...\n", this->world_->baseName);

			this->parseVertices();
			this->parseFaces();
			this->parseStaticModels();

			this->write();
		}

	private:
		struct Vertex
		{
			Game::vec3_t coordinate;
			Game::vec2_t texture;
			Game::vec3_t normal;
		};

		struct Face
		{
			int a{};
			int b{};
			int c{};
		};

		struct FaceList
		{
			std::vector<Face> indices{};
		};

		class File
		{
		public:
			File() {}

			File(const std::string& file)
			{
				Utils::IO::WriteFile(file, {});
				this->stream_ = std::ofstream(file, std::ofstream::out);
			}

			void append(const std::string& str)
			{
				this->stream_.write(str.data(), str.size());
			}

		private:
			std::ofstream stream_{};
		};

		Game::GfxWorld* world_{};
		std::vector<Vertex> vertices_{};
		std::unordered_map<Game::Material*, FaceList> faces_{};
		std::vector<Game::Material*> facesOrder_{};

		File object_{};
		File material_{};

		void transformAxes(Game::vec3_t& vec) const
		{
			std::swap(vec[0], vec[1]);
			std::swap(vec[1], vec[2]);
		}

		void parseVertices()
		{
			Logger::Print("Parsing vertices...\n");

			for (unsigned int i = 0; i < this->world_->draw.vertexCount; ++i)
			{
				const auto* vertex = &this->world_->draw.vd.vertices[i];

				Vertex v{};

				v.coordinate[0] = vertex->xyz[0];
				v.coordinate[1] = vertex->xyz[1];
				v.coordinate[2] = vertex->xyz[2];
				this->transformAxes(v.coordinate);

				v.texture[0] = vertex->texCoord[0];
				v.texture[1] = -vertex->texCoord[1];

				Game::Vec3UnpackUnitVec(vertex->normal, &v.normal);
				this->transformAxes(v.normal);

				this->vertices_.push_back(v);
			}
		}

		void parseFaces()
		{
			Logger::Print("Parsing faces...\n");

			for (unsigned int i = 0; i < this->world_->dpvs.staticSurfaceCount; ++i)
			{
				const auto* surface = &this->world_->dpvs.surfaces[i];

				const unsigned int vertOffset = surface->tris.firstVertex + 1;
				const unsigned int indexOffset = surface->tris.baseIndex;

				// Fuck cube maps for now
				if(this->findImage(surface->material, "colorMap")->mapType == 5) continue;

				auto& f = this->getFaceList(surface->material);

				for (unsigned short j = 0; j < surface->tris.triCount; ++j)
				{
					Face face{};
					face.a = this->world_->draw.indices[indexOffset + j * 3 + 0] + vertOffset;
					face.b = this->world_->draw.indices[indexOffset + j * 3 + 1] + vertOffset;
					face.c = this->world_->draw.indices[indexOffset + j * 3 + 2] + vertOffset;

					f.indices.push_back(face);
				}
			}
		}

		FaceList& getFaceList(Game::Material* material)
		{
			auto& faceList = this->faces_[material];

			if (this->facesOrder_.size() < this->faces_.size())
			{
				this->facesOrder_.push_back(material);
			}

			return faceList;
		}

		void performWorldTransformation(const Game::GfxPackedPlacement& placement, Vertex& v) const
		{
			Game::MatrixVecMultiply(placement.axis, v.normal, v.normal);
			Game::Vec3Normalize(v.normal);

			Game::MatrixVecMultiply(placement.axis, v.coordinate, v.coordinate);
			v.coordinate[0] = v.coordinate[0] * placement.scale + placement.origin[0];
			v.coordinate[1] = v.coordinate[1] * placement.scale + placement.origin[1];
			v.coordinate[2] = v.coordinate[2] * placement.scale + placement.origin[2];
		}

		std::vector<Vertex> parseSurfaceVertices(const Game::XSurface* surface, const Game::GfxPackedPlacement& placement)
		{
			std::vector<Vertex> vertices;

			for (unsigned short j = 0; j < surface->vertCount; j++)
			{
				const auto *vertex = &surface->verts0[j];

				Vertex v{};

				v.coordinate[0] = vertex->xyz[0];
				v.coordinate[1] = vertex->xyz[1];
				v.coordinate[2] = vertex->xyz[2];

				// Why...
				Game::Vec2UnpackTexCoords(vertex->texCoord, &v.texture);
				std::swap(v.texture[0], v.texture[1]);
				v.texture[1] *= -1;

				Game::Vec3UnpackUnitVec(vertex->normal, &v.normal);

				this->performWorldTransformation(placement, v);
				this->transformAxes(v.coordinate);
				this->transformAxes(v.normal);

				vertices.push_back(v);
			}

			return vertices;
		}

		std::vector<Face> parseSurfaceFaces(const Game::XSurface* surface) const
		{
			std::vector<Face> faces;

			for (unsigned short j = 0; j < surface->triCount; ++j)
			{
				Face face{};
				face.a = surface->triIndices[j * 3 + 0];
				face.b = surface->triIndices[j * 3 + 1];
				face.c = surface->triIndices[j * 3 + 2];

				faces.push_back(face);
			}

			return faces;
		}

		void removeVertex(const int index, std::vector<Face>& faces, std::vector<Vertex>& vertices) const
		{
			vertices.erase(vertices.begin() + index);

			for (auto &face : faces)
			{
				if (face.a > index) --face.a;
				if (face.b > index) --face.b;
				if (face.c > index) --face.c;
			}
		}

		void filterSurfaceVertices(std::vector<Face>& faces, std::vector<Vertex>& vertices) const
		{
			for (auto i = 0; i < int(vertices.size()); ++i)
			{
				auto referenced = false;

				for (const auto &face : faces)
				{
					if (face.a == i || face.b == i || face.c == i)
					{
						referenced = true;
						break;
					}
				}

				if (!referenced)
				{
					this->removeVertex(i--, faces, vertices);
				}
			}
		}

		void parseStaticModel(Game::GfxStaticModelDrawInst* model)
		{
			for (unsigned char i = 0; i < model->model->numsurfs; ++i)
			{
				this->getFaceList(model->model->materialHandles[i]);
			}

			const auto* lod = &model->model->lodInfo[model->model->numLods - 1];

			const auto baseIndex = this->vertices_.size() + 1;
			const auto surfIndex = lod->surfIndex;

			assert(lod->modelSurfs->numsurfs <= model->model->numsurfs);

			for (unsigned short i = 0; i < lod->modelSurfs->numsurfs; ++i)
			{
				// TODO: Something is still wrong about the models. Probably baseTriIndex and baseVertIndex might help

				const auto* surface = &lod->modelSurfs->surfs[i];
				auto faces = this->parseSurfaceFaces(surface);
				auto vertices = this->parseSurfaceVertices(surface, model->placement);
				this->filterSurfaceVertices(faces, vertices);

				auto& f = this->getFaceList(model->model->materialHandles[i + surfIndex]);

				for (const auto& vertex : vertices)
				{
					this->vertices_.push_back(vertex);
				}

				for (auto face : faces)
				{
					face.a += baseIndex;
					face.b += baseIndex;
					face.c += baseIndex;
					f.indices.push_back(std::move(face));
				}
			}
		}

		void parseStaticModels()
		{
			Logger::Print("Parsing static models...\n");

			for (unsigned i = 0u; i < this->world_->dpvs.smodelCount; ++i)
			{
				this->parseStaticModel(this->world_->dpvs.smodelDrawInsts + i);
			}
		}

		void write()
		{
			this->object_ = File(Utils::String::VA("raw/mapdump/%s/%s.obj", this->world_->baseName, this->world_->baseName));
			this->material_ = File(Utils::String::VA("raw/mapdump/%s/%s.mtl", this->world_->baseName, this->world_->baseName));

			this->object_.append("# Generated by IW4x\n");
			this->object_.append("# Credit to SE2Dev for his D3DBSP Tool\n");
			this->object_.append(Utils::String::VA("o %s\n", this->world_->baseName));
			this->object_.append(Utils::String::VA("mtllib %s.mtl\n\n", this->world_->baseName));

			this->material_.append("# IW4x MTL File\n");
			this->material_.append("# Credit to SE2Dev for his D3DBSP Tool\n");

			this->writeVertices();
			this->writeFaces();

			Logger::Print("Writing files...\n");

			this->object_ = {};
			this->material_ = {};
		}

		void writeVertices()
		{
			Logger::Print("Writing vertices...\n");
			this->object_.append("# Vertices\n");

			for (const auto& vertex : this->vertices_)
			{
				this->object_.append(Utils::String::VA("v %.6f %.6f %.6f\n", vertex.coordinate[0], vertex.coordinate[1], vertex.coordinate[2]));
			}

			Logger::Print("Writing texture coordinates...\n");
			this->object_.append("\n# Texture coordinates\n");

			for (const auto& vertex : this->vertices_)
			{
				this->object_.append(Utils::String::VA("vt %.6f %.6f\n", vertex.texture[0], vertex.texture[1]));
			}

			Logger::Print("Writing normals...\n");
			this->object_.append("\n# Normals\n");

			for (const auto& vertex : this->vertices_)
			{
				this->object_.append(Utils::String::VA("vn %.6f %.6f %.6f\n", vertex.normal[0], vertex.normal[1], vertex.normal[2]));
			}

			this->object_.append("\n");
		}

		Game::GfxImage* findImage(Game::Material* material, const std::string& type) const
		{
			Game::GfxImage* image = nullptr;

			const auto hash = Game::R_HashString(type.data());

			for (char l = 0; l < material->textureCount; ++l)
			{
				if (material->textureTable[l].nameHash == hash)
				{
					image = material->textureTable[l].u.image; // Hopefully our map
					break;
				}
			}

			return image;
		}

		Game::GfxImage* extractImage(Game::Material* material, const std::string& type) const
		{
			auto* image = this->findImage(material, type);

			if (!image)
			{
				return image;
			}

			std::string _name = Utils::String::VA("raw/mapdump/%s/textures/%s.png", this->world_->baseName, image->name);
			D3DXSaveTextureToFileA(_name.data(), D3DXIFF_PNG, image->texture.map, nullptr);

			return image;
		}

		void writeMaterial(Game::Material* material)
		{
			std::string name = material->info.name;

			const auto pos = name.find_last_of('/');
			if (pos != std::string::npos)
			{
				name = name.substr(pos + 1);
			}

			this->object_.append(Utils::String::VA("usemtl %s\n", name.data()));
			this->object_.append("s off\n");

			auto* colorMap = this->extractImage(material, "colorMap");
			auto* normalMap = this->extractImage(material, "normalMap");
			auto* specularMap = this->extractImage(material, "specularMap");

			this->material_.append(Utils::String::VA("\nnewmtl %s\n", name.data()));
			this->material_.append("Ka 1.0000 1.0000 1.0000\n");
			this->material_.append("Kd 1.0000 1.0000 1.0000\n");
			this->material_.append("illum 1\n");
			this->material_.append(Utils::String::VA("map_Ka textures/%s.png\n", colorMap->name));
			this->material_.append(Utils::String::VA("map_Kd textures/%s.png\n", colorMap->name));

			if (specularMap)
			{
				this->material_.append(Utils::String::VA("map_Ks textures/%s.png\n", specularMap->name));
			}

			if (normalMap)
			{
				this->material_.append(Utils::String::VA("bump textures/%s.png\n", normalMap->name));
			}
		}

		void writeFaces()
		{
			Logger::Print("Writing faces...\n");
			Utils::IO::CreateDir(Utils::String::VA("raw/mapdump/%s/textures", this->world_->baseName));

			this->material_.append(Utils::String::VA("# Material count: %d\n", this->faces_.size()));

			this->object_.append("# Faces\n");

			for (const auto& material : this->facesOrder_)
			{
				this->writeMaterial(material);

				const auto& faces = this->getFaceList(material);
				for (const auto& index : faces.indices)
				{
					const int a = index.a;
					const int b = index.b;
					const int c = index.c;

					this->object_.append(Utils::String::VA("f %d/%d/%d %d/%d/%d %d/%d/%d\n", a, a, a, b, b, b, c, c, c));
				}

				this->object_.append("\n");
			}
		}
	};

	MapDump::MapDump()
	{
		Command::Add("dumpmap", []()
		{
			if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
			{
				Logger::Print("DirectX needs to be enabled, please start a client to use this command!\n");
				return;
			}

			Game::GfxWorld* world = nullptr;
			Game::DB_EnumXAssets(Game::XAssetType::ASSET_TYPE_GFXWORLD, [](Game::XAssetHeader header, void* world)
			{
				*reinterpret_cast<Game::GfxWorld**>(world) = header.gfxWorld;
			}, &world, false);

			if (world)
			{
				MapDumper dumper(world);
				dumper.dump();

				Logger::Print("Map '{}' exported!\n", world->baseName);
			}
			else
			{
				Logger::Print("No map loaded, unable to dump anything!\n");
			}
		});
	}
}
