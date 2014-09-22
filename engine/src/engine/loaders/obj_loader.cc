/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Wavefront mesh loader.
 *
 * @todo		This is hideously slow. readLine() reads individual
 *			characters at a time, and there's a metric fuckton of
 *			vector reallocations going on.
 * @todo		Make use of index buffers. Hash lookup for unique vertex
 *			combinations. If non-existant, add new to shared vertex
 *			buffer then index to that in submesh (do comparison on
 *			whether it results in a reduction in size).
 */

#include "engine/asset_loader.h"
#include "engine/mesh.h"

#include "gpu/gpu.h"

#include <sstream>

/** Wavefront .obj mesh loader. */
class OBJLoader : public AssetLoader {
public:
	OBJLoader();

	AssetPtr load() override;
private:
	/** Vertex imported from a .obj file. */
	struct Vertex {
		float x, y, z, _pad1;
		float nx, ny, nz, _pad2;
		float u, v, _pad3, _pad4;
	public:
		Vertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec2 &texcoord) :
			x(pos.x), y(pos.y), z(pos.z),
			nx(normal.x), ny(normal.y), nz(normal.z),
			u(texcoord.x), v(texcoord.y)
		{}

		Vertex() {}
	};
private:
	/** Triple of indices in the order (position, texcoord, normal). */
	typedef std::array<uint16_t, 3> VertexTriple;

	/** Submesh descriptor. */
	struct SubMeshDesc {
		std::string material;		/**< Material name. */

		/** Array of vertices. */
		std::vector<VertexTriple> vertices;
	public:
		explicit SubMeshDesc(const std::string &inMaterial) : material(inMaterial) {}
	};
private:
	template <typename T> bool addVertex(const std::vector<std::string> &tokens, std::vector<T> &array);
	bool addFace(const std::vector<std::string> &tokens);
	bool setMaterial(const std::vector<std::string> &tokens);
	bool setGroup(const std::vector<std::string> &tokens);
private:
	/** Geometry information. */
	std::vector<glm::vec3> m_positions;	/**< Positions ("v" declarations). */
	std::vector<glm::vec2> m_texcoords;	/**< UVs ("vt" declarations). */
	std::vector<glm::vec3> m_normals;	/**< Normals ("vn" declarations). */

	/** List of submeshes. */
	std::list<SubMeshDesc> m_subMeshes;

	/** Parser state. */
	size_t m_currentLine;			/**< Current line of the file (for error messages). */
	std::string m_currentMaterial;		/**< Current material name. */
	SubMeshDesc *m_currentSubMesh;		/**< Current submesh. */

	static GlobalGPUResource<VertexFormat> m_vertexFormat;
};

IMPLEMENT_ASSET_LOADER(OBJLoader, "obj");

/** .obj file vertex format. */
GlobalGPUResource<VertexFormat> OBJLoader::m_vertexFormat;

/** Initialize the OBJ loader. */
OBJLoader::OBJLoader() :
	m_currentLine(0),
	m_currentMaterial("default"),
	m_currentSubMesh(nullptr)
{}

/** Load an OBJ file.
 * @return		Pointer to loaded asset, null on failure. */
AssetPtr OBJLoader::load() {
	std::string line;
	while(m_data->readLine(line)) {
		m_currentLine++;

		std::vector<std::string> tokens;
		util::tokenize(line, tokens, " \r", true);
		if(!tokens.size())
			continue;

		bool ret = true;

		if(tokens[0] == "v") {
			ret = addVertex(tokens, m_positions);
		} else if(tokens[0] == "vt") {
			ret = addVertex(tokens, m_texcoords);
		} else if(tokens[0] == "vn") {
			ret = addVertex(tokens, m_normals);
		} else if(tokens[0] == "f") {
			ret = addFace(tokens);
		} else if(tokens[0] == "usemtl") {
			ret = setMaterial(tokens);
		} else if(tokens[0] == "g") {
			ret = setGroup(tokens);
		}

		if(!ret)
			return nullptr;
	}

	if(!m_subMeshes.size()) {
		orionLog(LogLevel::kError, "%s: No faces defined", m_path);
		return nullptr;
	}

	MeshPtr mesh(new Mesh());

	/* Create the vertex format if we don't already have it. FIXME: thread
	 * safety. Also this should just be some globally defined format. */
	if(!m_vertexFormat) {
		m_vertexFormat() = g_gpu->createVertexFormat();
		m_vertexFormat->addBuffer(0, sizeof(Vertex));
		m_vertexFormat->addAttribute(
			VertexAttribute::kPositionSemantic, 0,
			VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, x));
		m_vertexFormat->addAttribute(
			VertexAttribute::kNormalSemantic, 0,
			VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, nx));
		m_vertexFormat->addAttribute(
			VertexAttribute::kTexCoordSemantic, 0,
			VertexAttribute::kFloatType, 2, 0, offsetof(Vertex, u));
		m_vertexFormat->finalize();
	}

	size_t j = 0;

	/* Register all submeshes. */
	for(const SubMeshDesc &desc : m_subMeshes) {
		orionLog(LogLevel::kDebug, "%s: Submesh %u: %u vertices", m_path, j++, desc.vertices.size());

		SubMesh *subMesh = mesh->addSubMesh();

		/* Add the material slot. If this name has already been added
		 * the existing index is returned. */
		subMesh->material = mesh->addMaterial(desc.material);

		GPUBufferPtr buffer = g_gpu->createBuffer(
			GPUBuffer::kVertexBuffer,
			GPUBuffer::kStaticDrawUsage,
			desc.vertices.size() * sizeof(Vertex));

		subMesh->vertices = g_gpu->createVertexData(desc.vertices.size());
		subMesh->vertices->setFormat(m_vertexFormat);
		subMesh->vertices->setBuffer(0, buffer);
		subMesh->vertices->finalize();

		GPUBufferMapper<Vertex> data(buffer, GPUBuffer::kMapInvalidate, GPUBuffer::kWriteAccess);

		for(size_t i = 0; i < desc.vertices.size(); i++) {
			new(&data[i]) Vertex(
				m_positions[desc.vertices[i][0]],
				m_normals[desc.vertices[i][2]],
				m_texcoords[desc.vertices[i][1]]);
		}
	}

	orionLog(LogLevel::kDebug,
		"%s: %u submeshes, %u materials",
		m_path, mesh->numSubMeshes(), mesh->numMaterials());

	return mesh;
}

/** Handle a vertex declaration.
 * @param tokens	Tokens from the current line.
 * @param array		Array to add to.
 * @return		Whether the declaration was valid. */
template <typename T>
bool OBJLoader::addVertex(const std::vector<std::string> &tokens, std::vector<T> &array) {
	T value;

	if(tokens.size() < value.length() + 1) {
		orionLog(LogLevel::kError, "%s: %u: Expected %d values", m_path, m_currentLine, value.length());
		return false;
	}

	for(size_t i = 0; i < value.length(); i++) {
		const char *str = tokens[i + 1].c_str(), *end;
		value[i] = strtof(str, const_cast<char **>(&end));
		if(end != str + tokens[i + 1].length()) {
			orionLog(LogLevel::kError, "%s: %u: Expected float value", m_path, m_currentLine);
			return false;
		}
	}

	array.push_back(value);
	return true;
}

/** Handle a face declaration.
 * @param tokens	Tokens from the current line.
 * @return		Whether the declaration was valid. */
bool OBJLoader::addFace(const std::vector<std::string> &tokens) {
	/* If we don't have a current submesh, we must begin a new one. */
	if(!m_currentSubMesh) {
		m_subMeshes.emplace_back(m_currentMaterial);
		m_currentSubMesh = &m_subMeshes.back();
	}

	size_t numVertices = tokens.size() - 1;

	if(numVertices != 3 && numVertices != 4) {
		orionLog(LogLevel::kError, "%s: %u: Expected 3 or 4 vertices", m_path, m_currentLine);
		return false;
	}

	VertexTriple vertices[numVertices];
	for(size_t i = 0; i < numVertices; i++) {
		std::vector<std::string> subTokens;
		util::tokenize(tokens[i + 1], subTokens, "/", false);
		if(subTokens.size() != 3) {
			orionLog(LogLevel::kError, "%s: %u: Expected v/vt/vn", m_path, m_currentLine);
			return false;
		}

		for(size_t j = 0; j < 3; j++) {
			const char *str = subTokens[j].c_str(), *end;
			vertices[i][j] = strtoul(str, const_cast<char **>(&end), 10);
			if(end != str + subTokens[j].length()) {
				orionLog(LogLevel::kError, "%s: %u: Expected integer value", m_path, m_currentLine);
				return false;
			}

			/* Indices are 1 based. */
			vertices[i][j] -= 1;
		}
	}

	/* Add the vertices. If there's 3 it's a triangle, it's a quad so add
	 * it as 2 triangles. */
	m_currentSubMesh->vertices.push_back(vertices[0]);
	m_currentSubMesh->vertices.push_back(vertices[1]);
	m_currentSubMesh->vertices.push_back(vertices[2]);
	if(numVertices == 4) {
		m_currentSubMesh->vertices.push_back(vertices[2]);
		m_currentSubMesh->vertices.push_back(vertices[3]);
		m_currentSubMesh->vertices.push_back(vertices[0]);
	}

	return true;
}

/** Handle a material declaration.
 * @param tokens	Tokens from the current line.
 * @return		Whether the declaration was valid. */
bool OBJLoader::setMaterial(const std::vector<std::string> &tokens) {
	if(tokens.size() != 2) {
		orionLog(LogLevel::kError, "%s: %u: Expected single material name", m_path, m_currentLine);
		return false;
	}

	if(tokens[1] != m_currentMaterial) {
		/* Begin a new submesh. */
		m_currentMaterial = tokens[1];
		m_currentSubMesh = nullptr;
	}

	return true;
}

/** Handle a group declaration.
 * @param tokens	Tokens from the current line.
 * @return		Whether the declaration was valid. */
bool OBJLoader::setGroup(const std::vector<std::string> &tokens) {
	if(tokens.size() != 2) {
		/* Note multiple group names can actually be specified to give
		 * shared elements between groups but we don't support this
		 * now. */
		orionLog(LogLevel::kError, "%s: %u: Expected single group name", m_path, m_currentLine);
		return false;
	}

	/* Begin a new submesh. TODO: Should we bother trying to handle
	 * duplicate group names and bundling them together? Probably not worth
	 * the effort. */
	m_currentSubMesh = nullptr;

	return true;
}
