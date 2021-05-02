#pragma once

#include "vcl/display/drawable/mesh_drawable/mesh_drawable.hpp"
#include "vcl/math/affine/affine_rts/affine_rts.hpp"

namespace vcl
{
struct hierarchy_mesh_drawable_node
{
	hierarchy_mesh_drawable_node();

	hierarchy_mesh_drawable_node(const mesh_drawable& element,
									const std::string& name,
									const std::string& name_parent="global_frame",
									const affine_rts& transform = affine_rts());



	mesh_drawable element; // the visual element

	std::string name;        // name of the current node
	std::string name_parent; // name of the parent node

	// Affine transform express in relative coordinates with respect to the parent node frame
	affine_rts transform;
	// Affine transform expressed in global coordinates (supposed to be computed automatically)
	affine_rts global_transform; 
};

template <typename SCENE>
void draw(hierarchy_mesh_drawable_node const& node, SCENE const& scene)
{
	// copy of the mesh drawable (lightweight element) - to preserve its uniform parameters
	mesh_drawable visual_element = node.element;

	// Update local uniform values (and combine them with uniform already stored in the mesh)
	visual_element.transform = node.global_transform * visual_element.transform;

	if(visual_element.shader!=0)
		draw(visual_element, scene);
}

template <typename SCENE>
void draw_wireframe(hierarchy_mesh_drawable_node const& node, SCENE const& scene, vec3 const& color={0,0,1})
{
	// copy of the mesh drawable (lightweight element) - to preserve its uniform parameters
	mesh_drawable visual_element = node.element;

	// Update local uniform values (and combine them with uniform already stored in the mesh)
	visual_element.transform = node.global_transform * visual_element.transform;

	if(visual_element.shader!=0)
		draw_wireframe(visual_element, scene, color);
}

}
