#include "Voronoi.h"

Voronoi::Voronoi(int seed_, int num_nodes_, float w, float h,
	float r, float g, float b, bool type, bool rand_c) :
	num_nodes(num_nodes_), width(w), height(h), cell_r(r),
	cell_g(g), cell_b(b), shaded(type), random_colors(rand_c)
{
	this->seed = seed_;
	if (this->seed == 0)
	{
		random_device r;
		this->seed = r();
	}
}

Voronoi::~Voronoi(){}

vector<tuple<float, float, float>> Voronoi::gen()
{
	/* first generate random nodes */
	default_random_engine e(this->seed);
	uniform_int_distribution<> w_uid(0, this->width);
	uniform_int_distribution<> h_uid(0, this->height);
	uniform_int_distribution<> c_uid(0, 255);

	/*
		In addition to generating the node position,
		 a random r, g, and b value are genned for each node.
		Each region of pixels closest to this node will take on 
		 that genned color.
	*/
	vector<pair<vec2, tuple<float, float, float>>> nodes;
	for (int i = 0; i < this->num_nodes; i++)
	{
		float rand_width = w_uid(e);
		float rand_height = h_uid(e);

		/*
			In case I forget it:
			 Cool orange rgb = 226 57 20
		*/

		if (this->random_colors)
		{
			this->cell_r = c_uid(e);
			this->cell_g = c_uid(e);
			this->cell_b = c_uid(e);
		}

		vec2 node(rand_width, rand_height);
		tuple<float, float, float> t
		{ 
			this->cell_r, 
			this->cell_g, 
			this->cell_b 
		};
		nodes.push_back(pair<vec2, tuple<float, float, float>>{ node, t });
	}

	/*
		Next step is to go through all pixels and determine
		 the closest node.
		This will be slow as its a nearest neighbors search,
		 and could be sped up by a BVH, but I'm lazy lol
		There's also a lot of object copying being done 
		 but that's because I didn't want to deal with memory management.
	*/
	vector<tuple<float, float, float>> pixels;
	for (int i = 0; i < this->height; i++)
	{
		for (int j = 0; j < this->width; j++)
		{
			vec2 pixel(j, i);
			float min_dist = 9e15;
			int min_node_ind = -1;
			for (int k = 0; k < this->num_nodes; k++)
			{
				float dist = pixel.distanceTo(nodes[k].first);
				if (dist < min_dist)
				{
					min_node_ind = k;
					min_dist = dist;
				}
			}
			if (min_node_ind == -1)
			{
				cerr << "Something went wrong in voronoi node solving." << endl;
				exit(EXIT_FAILURE);
			}

			/* 
				At this point, the min node has been found.
				I want to make it so that the pixels closer to boundaries
				 are a darker color than the pixels closer to the node.
				This will make a nice gradient.
				The closeness to a boundary can be determined by distance
				 from the node. 
				The further from the node, the closer to a boundary.
			*/

			/*
				Okay so after some testing this method works pretty well,
				 however, some splotches are very dark.
				I figured this would happen I was just hoping I was wrong.
				What I really need is to figure out the proportion of the 
				 distance between the node and the edge which
				 I'm not quite sure off the top of my head how to do.
			*/

			/*
				Well after a bit more thinking, I can't come up with anything.
				I'd love to  put more time into this but uh I think 
				 I've gone overboard enough lol
			*/
			float r = get<0>(nodes[min_node_ind].second);
			float g = get<1>(nodes[min_node_ind].second);
			float b = get<2>(nodes[min_node_ind].second);

			if (this->shaded)
			{
				r -= 5 * min_dist;
				g -= min_dist;
				b -= min_dist;

				if (r < 0) r = 0.0;
				if (g < 0) g = 0.0;
				if (b < 0) b = 0.0;
			}

			tuple<float, float, float> t(r, g, b);

			/*pixels.push_back(nodes[min_node_ind].second);*/
			pixels.push_back(t);
		}
	}

	return pixels;
}

