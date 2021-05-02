#pragma once

#include "hierarchy_mesh_drawable_node/hierarchy_mesh_drawable_node.hpp"

#include <map>
#include <vector>

namespace vcl
{
	struct hierarchy_mesh_drawable
	{
		std::map<std::string, int> name_map;
		std::vector<hierarchy_mesh_drawable_node> elements;

		// Add new node to the hierarchy
		// Note: Parent node is expected to be already present in the hierarchy
		// The name of each node must be unique in the hierarchy
		void add(hierarchy_mesh_drawable_node const& node);
		void add(mesh_drawable const& element, std::string const& name, std::string const& name_parent="global_frame", vec3 const& translate=vec3());
		void add(mesh_drawable const& element, std::string const& name, std::string const& name_parent, affine_rts const& transform);

		// Get node by name
		hierarchy_mesh_drawable_node& operator[](std::string const& name);
		hierarchy_mesh_drawable_node const& operator[](std::string const& name) const;

		// Fill global coordinates of the nodes - must be called before draw
		void update_local_to_global_coordinates();
	};


	template <typename SCENE>
	void draw(hierarchy_mesh_drawable const& hierarchy, SCENE const& scene)
	{
	    const size_t N = hierarchy.elements.size();
	    for(size_t k=0; k<N; ++k)
		{
			hierarchy_mesh_drawable_node const& node = hierarchy.elements[k];
			draw(node, scene);
		}
	}

	template <typename SCENE>
	void draw_wireframe(hierarchy_mesh_drawable const& hierarchy, SCENE const& scene, vec3 const color={0,0,1})
	{
	    const size_t N = hierarchy.elements.size();
	    for(size_t k=0; k<N; ++k)
		{
			hierarchy_mesh_drawable_node const& node = hierarchy.elements[k];
			draw_wireframe(node, scene, color);
		}
	}
}