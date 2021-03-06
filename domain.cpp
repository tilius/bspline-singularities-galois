#include <iostream>
#include <cmath>
//#include "vector"
#include "node.h"
#include "domain.h"
#include <chrono>
#include <thread>


using namespace std;

int CORRECTNESS_ACCURACY = 10;

Domain::Domain(const Cube &box) {
	add_element(box);
	original_box = box;  // preserve for the object lifetime
}


/*** CHECKS ***/

bool Domain::is_middle_element(const Cube &cube, Coord middle) const {
	return (cube.right() == middle || cube.left() == middle) &&
		(cube.up() == middle || cube.down() == middle);
}

bool Domain::is_horizontal_side_element(const Cube &cube, Coord middle) const {
	return (cube.down() == middle) || (cube.up() == middle);
}

bool Domain::is_vertical_side_element(const Cube &cube, Coord middle) const {
	return (cube.right() == middle) || (cube.left() == middle);
}

bool Domain::overlaps_with_any_other(const Cube &that) const {
	for (const auto& other: elements)
		if (&that != &other && that.overlaps_with(other))
			return true;
	return false;
}

/*** SPLIT ELEMENTS ***/

void Domain::split_eight_side_elements_within_box_2D(Cube cube) {
	Coord mid = cube.get_middle(X_DIM);
	vector<Cube> old_elements;
	elements.swap(old_elements);
	for (const auto& e: old_elements) {
		if (e.non_empty() && e.contained_in_box(cube) && !is_middle_element(e, mid)) {
			if (is_horizontal_side_element(e, mid)) {
				Cube el1, el2;
				e.split_halves(Y_DIM, &el1, &el2);
				add_element(el1);
				add_element(el2);
			} else if(is_vertical_side_element(e, mid)) {
				Cube el1, el2;
				e.split_halves(X_DIM, &el1, &el2);
				add_element(el1);
				add_element(el2);
			} else {
				add_element(e);
			}
		} else {
			add_element(e);
		}
	}
}

// Splits each element within the given box into 4 smaller ones.
void Domain::split_elements_within_box_into_4_2D(const Cube &box) {
	vector<Cube> old_elements;
	elements.swap(old_elements);
	for (const auto& e: old_elements) {
		if (e.non_empty() && e.contained_in_box(box)) {
			Cube el, er;
			e.split_halves(X_DIM, &el, &er);
			Cube el1, el2, er1, er2;
			el.split_halves(Y_DIM, &el1, &el2);
			er.split_halves(Y_DIM, &er1, &er2);
			add_element(el1);
			add_element(el2);
			add_element(er1);
			add_element(er2);
		} else {
			add_element(e);
		}
	}
}

// Splits each element within the given box into 6 smaller ones.
void Domain::split_elements_within_box_into_6_2D(const Cube &box) {
	vector<Cube> old_elements;
	elements.swap(old_elements);
	for (const auto &e: old_elements) {
		if (e.non_empty() && e.contained_in_box(box)) {
			Cube e1, e2, e3;
			e.split_thirds(X_DIM, &e1, &e2, &e3);
			Cube e11, e12, e21, e22, e31, e32;
			e1.split_halves(Y_DIM, &e11, &e12);
			e2.split_halves(Y_DIM, &e21, &e22);
			e3.split_halves(Y_DIM, &e31, &e32);
			add_element(e11);
			add_element(e12);
			add_element(e21);
			add_element(e22);
			add_element(e31);
			add_element(e32);
		} else {
			add_element(e);
		}
	}
}

// Splits each element into 4 smaller ones.
void Domain::split_all_elements_into_4_2D() {
	split_elements_within_box_into_4_2D(original_box);
}

// Splits each element into 6 smaller ones.
void Domain::split_all_elements_into_6_2D() {
	split_elements_within_box_into_6_2D(original_box);
}


void Domain::remove_all_elements_not_contained_in(const Cube &box) {
	vector<Cube> old_elements;
	elements.swap(old_elements);
	for (const auto &e: old_elements) {
		if (e.contained_in_box(box)) {
			add_element(e);
		}
	}
}



/*** ADD ELEMENTS ***/

// Inserts `count' of edge elements parallel to the given dimension's axis,
// spanning from one side of the `box' to the other in the given dimension.
// The other dimension is fixed at `coord'.
void Domain::add_edge_2D(int dim, const Cube &box, Coord coord, int count, bool edged_8) {
	Coord from = box.get_from(dim);
	Coord to = box.get_to(dim);
	Coord element_size = (to - from) / count;
	for (int i = 0; i < count; i++) {
		Coord element_from = from + element_size * i;
		Coord element_to = element_from + element_size;

		//needs a flag, only for the new mesh
		if (edged_8) {
			if (i == count / 2 || i == (count / 2 - 1)) {
				Cube e1(2), e2(2);
				Coord mid = (element_from + element_to) / 2;
				e1.set_bounds(dim, element_from, mid);
				e1.set_bounds(dim ^ 1, coord, coord);  // the other dimension
				elements.push_back(e1);
				e2.set_bounds(dim, mid, element_to);
				e2.set_bounds(dim ^ 1, coord, coord);  // the other dimension
				elements.push_back(e2);
				if(dim == X_DIM) {
					add_vertex_2D(mid, coord);
				} else {
					add_vertex_2D(coord, mid);
				}
			} else {
				Cube e(2);
				e.set_bounds(dim, element_from, element_to);
				e.set_bounds(dim ^ 1, coord, coord);  // the other dimension
				elements.push_back(e);
			}
		} else {
			Cube e(2);
			e.set_bounds(dim, element_from, element_to);
			e.set_bounds(dim ^ 1, coord, coord);  // the other dimension
			elements.push_back(e);
		}
	}
}

// For each of the corners of the given box, inserts an infinitely small
// "vertex" element.
void Domain::add_corner_vertices_2D(const Cube &box) {
	add_vertex_2D(box.left(), box.up());
	add_vertex_2D(box.left(), box.down());
	add_vertex_2D(box.right(), box.up());
	add_vertex_2D(box.right(), box.down());
}


/*** PRINT ELEMENTS ***/

void Domain::print_elements_within_box(const Cube &box) const {
	cout << elements.size() << endl;
	for (const auto& e: elements) {
		if (e.contained_in_box(box)) {
			e.print_bounds();
			cout << endl;
		}
	}
}

void Domain::print_elements_level_and_id_within_box(const Node *node) const {
	for (const auto& e: elements) {
		if (e.non_empty() && e.contained_in_box(node->get_cube())) {
			cout << e.get_level() << " " << e.get_id_within_level() << " ";
		}
	}
}

void Domain::print_all_elements() const {
	print_elements_within_box(original_box);
}

void Domain::println_non_empty_elements_count() const {
	cout << count_non_empty_elements() << endl;
}

void Domain::print_tree_nodes_count() const {
	cout << get_tree_nodes().size() << endl;
}

void Domain::print_galois_output() const {
	print_bsplines_line_by_line();
	print_bsplines_per_elements();
	print_elements_per_tree_nodes();
}

void Domain::print_line(Coord x1, Coord y1, Coord x2, Coord y2) {
	cout << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
}

void Domain::print_tabs(int cnt) {
	for (int i = 0; i < cnt; i++)
		cout << "  ";
}


/*** NEIGHBORS ***/

bool Domain::cubes_are_adjacent(const Cube &that, const Cube &other, int bound_no, bool looseened_conds) const {
	int given_dim_no = bound_no >> 1;
	int opposite_bound_no = bound_no ^ 1;
	if (that.get_bound(bound_no) != other.get_bound(opposite_bound_no))
		return false;

	for (int dim_no = 0; dim_no < that.get_dim_cnt(); dim_no++) {
		if (dim_no == given_dim_no)
			continue;

		Coord overlap = that.get_overlapping_part(other, dim_no);
		if (!looseened_conds) {
			if (overlap != that.get_size(dim_no) && overlap != other.get_size(dim_no))
				return false;
		} else {
			if (overlap == 0)
				return false;
		}
	}
	return true;
}

void Domain::print_all_neighbors() const {
	int total_cnt = 0;
	for (const Cube& e: elements)
		total_cnt += e.get_neighbor_count();
	cout << total_cnt << endl;
	for (const Cube& that: elements) {
		for (int bound_no = 0; bound_no < that.get_dim_cnt() * 2; bound_no++) {
			Cube* other = that.get_neighbor(bound_no);
			if (other != nullptr) {
				print_line(
						that.get_middle(X_DIM), that.get_middle(Y_DIM),
						other->get_middle(X_DIM), other->get_middle(Y_DIM)
						);
			}
		}
	}
}

void Domain::compute_neighbors(Cube &that, Coord size) {
	// Check regular neighbors.
	for (auto& other: elements)
		for (int bound_no = 0; bound_no < that.get_dim_cnt() * 2; bound_no++)
			if (cubes_are_adjacent(that, other, bound_no, false))
				that.set_neighbor(bound_no, &other);
	// Check extra neigbors if regular are not found.
	for (int bound_no = 0; bound_no < that.get_dim_cnt() * 2; bound_no++) {
		if(that.get_neighbor(bound_no) == nullptr &&
				that.get_bound(bound_no) != 0 && that.get_bound(bound_no) != size) {
			for (auto& other: elements) {
				if (cubes_are_adjacent(that, other, bound_no, true))
					that.set_neighbor(bound_no, &other);
			}
		}
	}
}

void Domain::compute_all_neighbors(Coord size) {
	for (auto& e: elements)
		compute_neighbors(e, size);
}


/*** TREE ***/

void Domain::tree_process_box_2D(const Cube &box) {
	cut_off_boxes.push_back(box);
}

vector<Cube> Domain::get_cut_off_boxes() const {
	return cut_off_boxes;
}

int Domain::count_elements_within_box(const Cube &cube) const {
	int count = 0;
	for (const auto& e: elements)
		if (e.non_empty() && e.contained_in_box(cube))
			count++;
	return count;
}

Node *Domain::add_tree_node(Cube cube, Node *parent) {
	Node* node = new Node(cube, tree_node_id++);
	if (parent) {
		parent->add_child(node);
	}
	tree_nodes.push_back(node);
	return node;
}

void Domain::tree_process_cut_off_box(int dim, Node *node, bool toggle_dim) {
	int elements_cnt = count_elements_within_box(node->get_cube());
	//cout << "tree process cut off box " << elements_cnt << endl;
	if (elements_cnt == 1) // leaf
		return;

	else if (elements_cnt > 1 && elements_cnt % 2 == 0) { //even num, we split into halves
		//cout << "even" << endl;
		Cube cut_off_cube = node->get_cube();
		Cube first_half, second_half;
		cut_off_cube.split_halves(dim, &first_half, &second_half);

		Node *first_half_node = this->add_tree_node(first_half, node);
		Node *second_half_node = this->add_tree_node(second_half, node);

		if (toggle_dim)
			dim ^= 1;

		tree_process_cut_off_box(dim, first_half_node, toggle_dim);
		tree_process_cut_off_box(dim, second_half_node, toggle_dim);
	} else if (elements_cnt > 1) {
		//cout << "odd" << endl;
		//cut_off_box from rectangular mesh has odd cnt, it starts with 4,
		//then next level is 4*2 - 2 = 6, so it is 6 -> 3 + 3
		//6*2 - 2 = 10 -> 5 + 5
		Cube cut_off_cube = node->get_cube();
		Cube first_half, second_half;
		Coord size = cut_off_cube.get_size(dim);
		Coord where_to_split = cut_off_cube.get_from(dim) + size * (elements_cnt / 2) / elements_cnt;
		// let's assume we have 5 elements in cut_off_box and cut_off_box is from 2 to 7 ->
		// where_to_split equals 4 then
		cut_off_cube.split(dim, where_to_split, &first_half, &second_half);
		Node *first_half_node = this->add_tree_node(first_half, node);
		Node *second_half_node = this->add_tree_node(second_half, node);
		if (toggle_dim)
			dim ^= 1;

		tree_process_cut_off_box(dim, first_half_node, toggle_dim);
		tree_process_cut_off_box(dim, second_half_node, toggle_dim);
	}
}

const vector<Node *>& Domain::get_tree_nodes() const {
	return tree_nodes;
}

void Domain::print_elements_per_tree_nodes() const {
	print_tree_nodes_count();
	for (const Node* node: get_tree_nodes()) {
		//cout << "   tree node\n   ";
		//node->get_cube().print_bounds();
		//cout << endl;
		node->print_num();
		print_elements_count_within_node(node);
		print_elements_level_and_id_within_box(node);
		print_node_children(node);
	}

	// compute the supports here
	// print_tree_postorder(tree_nodes[0], new vector<bool>(bsplines.size(), false));
}

void Domain::print_tree_postorder(const Node* node, vector<bool>* bspline_printed) const {
	for (const Node* n: node->get_children())
		print_tree_postorder(n, bspline_printed);
	//cout << node->get_num() << endl;

	// Now print all the bsplines that:
	// as of now, their support is fully enclosed in already traversed elements
	// It is enough just to lookup get_cube of the node!
	for (unsigned i = 0; i < bsplines.size(); i++) {
		if (!(*bspline_printed)[i] && bsplines[i].get_support().contained_in_box(node->get_cube())) {
			cout << i << endl;
			(*bspline_printed)[i] = true;
		}
	}
}

void Domain::print_tree_size() const {
	cout << get_cut_off_boxes().size() << endl;
}

void Domain::print_tree_for_draw() const {
	print_tree_size();
	for (const Cube& box: get_cut_off_boxes()) {
		box.print_full();
	}
}

void Domain::print_node_children(const Node *node) const {
	for (const Node* n: node->get_children()) {
		cout << n->get_num() + 1 << " ";
	}
	cout << endl;
}

void Domain::print_elements_count_within_node(const Node *node) const {
	cout << count_elements_within_box(node->get_cube()) << " ";
}


/*** BOUNDARY TWEAKING ***/

void Domain::tweak_bounds() {
	for (auto& e: elements)
		e.back_up_bounds();
	original_box.back_up_bounds();

	// Scale up all elements.
	for (auto& e: elements)
		e.scale_up(8);
	original_box.scale_up(8);

	// Squeeze non-empty dims, pump up empty dims.
	for (auto& e: elements)
		e.pump_or_squeeze(2);

	// Try pump back non-empty dims (only if they won't overlap with now pumped-up empty dims).
	for (auto& e: elements) {
		for (int bound_no = 0; bound_no < 2 * e.get_dim_cnt(); bound_no++) {
			if (e.get_size(bound_no / 2) == 2)
				continue;  // skip empty dims
			e.spread(bound_no, 2);
			if (overlaps_with_any_other(e)) {
				// Roll back the pumping if it interferes with any other element.
				e.spread(bound_no, -2);
			}
		}
	}
	original_box.spread(2);
}

void Domain::untweak_bounds() {
	for (auto& e: elements)
		e.restore_bounds();
	original_box.restore_bounds();
}


/*** B-SPLINES ***/

void Domain::compute_bsplines_supports(MeshType type, int order) {
	for (auto& e: elements)
		compute_bspline_support(type, order, e, e.get_num());
}

// Computes support for B-spline centered at the element `e'.
void Domain::compute_bspline_support(MeshType type, int order, Cube &e, int original_bspline_num) {
	vector<Coord> support_bounds = e.compute_bspline_support_2D();
	Cube support_cube(support_bounds[0], support_bounds[1], support_bounds[2], support_bounds[3]);

	BsplineChoice choice;

	for (auto &support_candidate: elements) {
		if (support_candidate.non_empty() && support_candidate.contained_in_box(support_cube)) {
			if (type == EDGED_4 && e.is_point_2D()) {
				Coord min_el_size = e.get_neighbor(0)->get_size(0) / 2;
				if (min_el_size > 1 && support_candidate.get_size(0) < min_el_size) {
					// We just detected a gnomon-shaped B-spline!

					/*cerr << "e.x = " << e.get_from(X_DIM) << ", e.y = " << e.get_from(Y_DIM)
						<< ", min_el_size = " << min_el_size
						<< ", support_candidate.get_size(X_DIM) = " << support_candidate.get_size(X_DIM) << endl;*/
					
					Coord x_mid = e.left();  // can be right() as well, nvm
					// -1 or +1, depending on where the rejected candidate element lies
					int shift_x_sign = sign(support_candidate.left() - e.left());
					// The actual shift needed to define a GnomonBspline
					Coord shift_x = shift_x_sign * min_el_size;

					Coord y_mid = e.down();
					int shift_y_sign = sign(support_candidate.down() - e.down());
					Coord shift_y = shift_y_sign * min_el_size;

					if (choice.gnomon == nullptr)
						choice.gnomon = new GnomonBspline(x_mid, y_mid, shift_x, shift_y);
					continue;
				}
			}
			if (!support_candidate.is_bspline_duplicated(original_bspline_num)) {
				support_candidate.add_bspline(original_bspline_num);
			}
			if (order > 2) {
				compute_bspline_support(type, order - 1, support_candidate, original_bspline_num);
			}
		}
	}

	if (choice.gnomon == nullptr) {
		// Most common case, the B-spline is regular and not gmonon.
		vector<double> x_knots = e.get_dim_knots(support_cube, X_DIM);
		vector<double> y_knots = e.get_dim_knots(support_cube, Y_DIM);
		choice.regular = new Bspline(x_knots, y_knots);
	}

	bsplines.push_back(choice);
}

Cube Domain::compute_not_defined_cube(const Cube &e, const Cube &support_cube) const {
	Coord middle = original_box.get_size(X_DIM) / 2;

	//{l, u, r, d}
	vector<Coord> not_defined_vector;

	if (e.get_bound(0) < middle) {
		if (e.get_bound(2) < middle) {
			not_defined_vector = {support_cube.get_middle(X_DIM),
				support_cube.get_middle(Y_DIM),
				support_cube.get_bound(1),
				support_cube.get_bound(3)};
		} else {
			not_defined_vector = {support_cube.get_middle(X_DIM),
				support_cube.get_bound(2),
				support_cube.get_bound(1),
				support_cube.get_middle(Y_DIM)};
		}
	} else {
		if (e.get_bound(2) < middle) {
			not_defined_vector = {support_cube.get_bound(0),
				support_cube.get_middle(Y_DIM),
				support_cube.get_middle(X_DIM),
				support_cube.get_bound(3)};
		} else {
			not_defined_vector = {support_cube.get_bound(0),
				support_cube.get_bound(2),
				support_cube.get_middle(X_DIM),
				support_cube.get_middle(Y_DIM)};
		}
	}

	Cube not_defined_cube(not_defined_vector[0], not_defined_vector[2], not_defined_vector[1], not_defined_vector[3]);
	return not_defined_cube;
}

void Domain::print_support_for_each_bspline() const {
	vector<vector<Coord>> supports(elements.size());
	for (const auto& e: elements) {
		for (Coord bspline: e.get_bsplines()) {
			supports[bspline].push_back(e.get_num());
		}
	}
	cout << elements.size() << endl;
	for (unsigned i = 0; i < elements.size(); i++) {
		const auto& e = elements[i];
		cout << e.get_middle(X_DIM) << " " << e.get_middle(Y_DIM) << " ";

		const auto& support = supports[i];
		cout << support.size() << " ";
		for (auto& s: support)
			cout << s << " ";
		cout << endl;
	}
}

void Domain::print_knots_for_each_bspline() const {
	cout << bsplines.size() << endl;
	for (const BsplineChoice& choice: bsplines) {
		if (choice.regular != nullptr) {
			cout << "Regular ";
			for (auto coord: choice.regular->get_x_knots())
				cout << coord << " ";
			for (auto coord: choice.regular->get_y_knots())
				cout << coord << " ";
			cout << endl;
		} else {
			const GnomonBspline& gb = *choice.gnomon;
			cout << "Gnomon "
				<< gb.get_x_mid() << " " << gb.get_y_mid() << " "
				<< gb.get_shift_x() << " " << gb.get_shift_y()
				<< endl;
		}
	}
}

void Domain::print_bsplines_per_elements() const {
	println_non_empty_elements_count();
	for(auto& e : elements)
		if (e.non_empty()) {
			e.print_level_id_and_bsplines(e.get_id_within_level());
		}
}

void Domain::print_bsplines_line_by_line() const {
	cout << elements.size() << endl;
	for (const auto& e: elements)
		cout << e.get_num() + 1 << " " << 1 << endl;
}


/*** UTILS ***/

int Domain::count_non_empty_elements() const {
	int count = 0;
	for (const auto& e: elements)
		if (e.non_empty())
			count++;
	return count;
}

int Domain::compute_level(const Cube &cube) const {
	Coord size = max(cube.get_size(0), cube.get_size(1));
	return (int) log2((original_box.get_size(0) / size)) - 1;
}

void Domain::enumerate_all_elements() {
	vector<Cube> old_elements;
	elements.swap(old_elements);
	int i = 0;
	for (const auto& e: old_elements) {
		if (e.non_empty()) {
			int level = compute_level(e);
			int id = get_e_num_per_level_and_inc(level);
			elements.push_back(Cube(e, i++, level, id, -1));
		} else {
			elements.push_back(Cube(e, i++, -1, -1, -1));
		}
	}
}

void Domain::add_vertex_2D(Coord x, Coord y) {
	add_element_2D(x, x, y, y);
}

void Domain::add_element_2D(Coord left, Coord right, Coord up, Coord down) {
	add_element(Cube(left, right, up, down));
}

void Domain::add_element(const Cube &e) {
	elements.push_back(e);
}


void Domain::allocate_elements_count_by_level_vector(int depth) {
	elements_count_by_level.resize(depth + 1);
}

int Domain::get_e_num_per_level_and_inc(int level) const {
	return ++elements_count_by_level[level];
}

