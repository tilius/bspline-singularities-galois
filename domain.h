//
// Created by bj on 05.01.16.
//

#ifndef BSPLINE_SINGULARITIES_GALOIS_DOMAIN_H
#define BSPLINE_SINGULARITIES_GALOIS_DOMAIN_H

#include "node.h"
#include "bspline.h"
#include "bspline-non-rect.h"

enum MeshType {
	UNEDGED,
	EDGED_4,
	EDGED_8
};

enum MeshShape {
	QUADRATIC,
	RECTANGULAR
};

struct BsplineChoice {
	Bspline* regular = nullptr;
	GnomonBspline* gnomon = nullptr;
	Cube get_support() const {
		if (regular)
			return regular->get_support();
		return gnomon->get_support_as_cube();
	}
};

// -1, 0 or +1 depending on the sign of val
template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

class Domain {
public:
	Domain(const Cube &box);

	bool is_middle_element(const Cube &cube, Coord middle) const;

	bool is_horizontal_side_element(const Cube &cube, Coord middle) const;

	bool is_vertical_side_element(const Cube &cube, Coord middle) const;

	bool overlaps_with_any_other(const Cube &that) const;

	void split_eight_side_elements_within_box_2D(Cube cube);

	void split_elements_within_box_into_4_2D(const Cube &box);

	void split_elements_within_box_into_6_2D(const Cube &box);

	void split_all_elements_into_4_2D();

	void split_all_elements_into_6_2D();

	void remove_all_elements_not_contained_in(const Cube &box);

	void add_edge_2D(int dim, const Cube &box, Coord coord, int count, bool edged_8);

	void add_corner_vertices_2D(const Cube &box);

	void print_elements_within_box(const Cube &box) const;

	void print_elements_level_and_id_within_box(const Node *node) const;

	void print_all_elements() const;

	void println_non_empty_elements_count() const;

	void print_tree_nodes_count() const;

	void print_galois_output() const;

	void print_tree_postorder(const Node*, vector<bool>* bspline_printed) const;

	static void print_line(Coord x1, Coord y1, Coord x2, Coord y2);

	static void print_tabs(int cnt);

	bool cubes_are_adjacent(const Cube &that, const Cube &other, int bound_no, bool looseened_conds) const;

	void print_all_neighbors() const;

	void compute_neighbors(Cube &that, Coord size);

	void compute_all_neighbors(Coord size);

	void tree_process_box_2D(const Cube &box);

	vector<Cube> get_cut_off_boxes() const;

	int count_elements_within_box(const Cube &cube) const;

	Node *add_tree_node(Cube cube, Node *parent);

	void tree_process_cut_off_box(int dim, Node *node, bool toggle_dim);

	const vector<Node *> &get_tree_nodes() const;

	void print_elements_per_tree_nodes() const;

	void print_tree_size() const;

	void print_tree_for_draw() const;

	void print_node_children(const Node *node) const;

	void print_elements_count_within_node(const Node *node) const;

	void tweak_bounds();

	void untweak_bounds();

	void compute_bsplines_supports(MeshType type, int order);

	void compute_bspline_support(MeshType type, int order, Cube &e, int original_bspline_num);

	void print_support_for_each_bspline() const;

	void print_knots_for_each_bspline() const;

	void print_bsplines_per_elements() const;

	void print_bsplines_line_by_line() const;

	int count_non_empty_elements() const;

	int compute_level(const Cube &cube) const;

	void enumerate_all_elements();

	void allocate_elements_count_by_level_vector(int depth);

private:

	void add_vertex_2D(Coord x, Coord y);

	void add_element_2D(Coord left, Coord right, Coord up, Coord down);

	void add_element(const Cube &e);

	int get_e_num_per_level_and_inc(int level) const;
	Cube original_box;
	vector<Cube> elements;
	vector<Cube> cut_off_boxes;
	vector<Node *> tree_nodes;
	vector<BsplineChoice> bsplines;

	mutable vector<int> elements_count_by_level;

	int tree_node_id = 0;

	Cube compute_not_defined_cube(const Cube &e, const Cube &support_cube) const;
};

#endif //BSPLINE_SINGULARITIES_GALOIS_DOMAIN_H
