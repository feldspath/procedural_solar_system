#include "icosphere.hpp"

using namespace vcl;

static std::vector<std::pair<int, mesh>> generatedIcospheres;

static void generateFaceVertex(vec3 v0, vec3 v1, vec3 v2, int division, vcl::buffer<vcl::vec3>& positions, vcl::buffer<vcl::uint3>& connectivity, int& count, int* base, int* lastVertices) {
	for (int j = 0; j < division + 1; j++) {
		double ky = (double)j / (division + 1);
		int offset = division - j;
		if (j > 0 || base == NULL) {
			for (int i = 0; i + j < division + 1; i++) {

				//connectivity
				if (j < division && i + j < division) {
					connectivity.push_back({ count, count + offset + 1, count + 1 });
				}
				if (i > 0 && i + j < division) {
					connectivity.push_back({ count, count + offset, count + offset + 1 });
				}

				//vertex
				double kx = (double)i / (division + 1);
				vec3 vertex = v0 + kx * (v1 - v0) + ky * (v2 - v0);
				vertex = normalize(vertex);
				positions[count++] = vertex;
			}
			connectivity.push_back({ count - 1, lastVertices[j + 1], lastVertices[j] });
			if (j < division) connectivity.push_back({ count - 1, count + offset - 1, lastVertices[j + 1] });
		}
		else {
			for (int i = 0; i < division + 1; i++) {
				connectivity.push_back({ base[i], i < division ? count + i : lastVertices[1], i < division ? base[i + 1] : lastVertices[0] });
				if (i > 0) connectivity.push_back({ base[i], count + i - 1, i < division ? count + i : lastVertices[1] });
			}
		}
	}
}

mesh mesh_icosphere(float r, unsigned int division) {

	for (int i = 0; i < generatedIcospheres.size(); i++) {
		if (generatedIcospheres[i].first == division)
			return generatedIcospheres[i].second;
	}

	mesh m;
	int size = 4 * (division + 1) * (division + 1) + 2;
	int upVertexCount = 2 * (division + 1) * (division + 2);
	m.position.resize(size);

	float c = 1 / sqrt(2.0f);

	const vec3 v0 = { 0.0f, 0.0f, 1.0f };
	const vec3 v1 = { c, c, 0.0f };
	const vec3 v2 = { c, -c, 0.0f };

	m.position[0] = v0;
	m.position[1] = -v0;
	int count = 2;

	int* lastVertices = new int[division + 2];
	int offset = (division + 1) * (division + 2) / 2;
	lastVertices[0] = offset + count;
	lastVertices[division + 1] = 0;

	for (int j = 1; j < division + 1; j++) lastVertices[j] = lastVertices[j - 1] + division - j + 2;
	generateFaceVertex(-v2, v1, v0, division, m.position, m.connectivity, count, NULL, lastVertices);

	for (int j = 0; j < division + 1; j++) lastVertices[j] += offset;
	generateFaceVertex(v1, v2, v0, division, m.position, m.connectivity, count, NULL, lastVertices);

	for (int j = 0; j < division + 1; j++) lastVertices[j] += offset;
	generateFaceVertex(v2, -v1, v0, division, m.position, m.connectivity, count, NULL, lastVertices);

	for (int j = 0; j < division + 1; j++) lastVertices[j] -= 3 * offset;
	generateFaceVertex(-v1, -v2, v0, division, m.position, m.connectivity, count, NULL, lastVertices);

	lastVertices[0] = 2;
	offset -= division + 1;
	lastVertices[1] = offset + count;
	lastVertices[division + 1] = 1;

	int* base = new int[division + 1];
	base[0] = 2 + offset + division + 1;
	for (int i = 1; i < division + 1; i++) base[i] = 3 + division - i;
	for (int j = 2; j < division + 1; j++) lastVertices[j] = lastVertices[j - 1] + division - j + 2;

	generateFaceVertex(v1, -v2, -v0, division, m.position, m.connectivity, count, base, lastVertices);

	for (int j = 1; j < division + 1; j++) lastVertices[j] += offset;
	for (int i = 0; i < division + 1; i++) base[i] += 3 * (offset + division + 1);
	base[0] -= upVertexCount;
	lastVertices[0] += 3 * (offset + division + 1);
	generateFaceVertex(-v2, -v1, -v0, division, m.position, m.connectivity, count, base, lastVertices);
	base[0] += upVertexCount;

	for (int j = 1; j < division + 1; j++) lastVertices[j] += offset;
	for (int i = 0; i < division + 1; i++) base[i] -= offset + division + 1;
	lastVertices[0] -= offset + division + 1;
	generateFaceVertex(-v1, v2, -v0, division, m.position, m.connectivity, count, base, lastVertices);
	 
	for (int j = 1; j < division + 1; j++) lastVertices[j] -= 3 * offset;
	for (int i = 0; i < division + 1; i++) base[i] -= (offset + division + 1);
	lastVertices[0] -= offset + division + 1;
	generateFaceVertex(v2, v1, -v0, division, m.position, m.connectivity, count, base, lastVertices);

	assert_vcl(count == size, "Wrong vertex count creating the sphere");

	m.fill_empty_field();

	generatedIcospheres.push_back(std::pair<int, mesh>(division, m));
	return m;
}