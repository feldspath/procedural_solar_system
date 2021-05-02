#include "vcl/base/base.hpp"
#include "hierarchy_mesh_drawable_node.hpp"


namespace vcl {

hierarchy_mesh_drawable_node::hierarchy_mesh_drawable_node()
    :element(), name("undefined"), name_parent("global_frame"), transform(), global_transform()
{}

hierarchy_mesh_drawable_node::hierarchy_mesh_drawable_node(mesh_drawable const& element_arg,
							                            std::string const& name_arg,
							                            std::string const& name_parent_arg,
							                            affine_rts const& transform_arg)
:element(element_arg), name(name_arg), name_parent(name_parent_arg), transform(transform_arg), global_transform()
{}





}