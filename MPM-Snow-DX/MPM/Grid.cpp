#include "pch.h"
#include "Grid.h"

Grid::Grid(Eigen::Vector3d pos, Eigen::Vector3d dims, Eigen::Vector3d cells, PointCloud* object)
{
	point_cloud = object;
	origin = pos;
	cellsize = division(dims, cells);
	size = add_const(cells, 1);
	nodes_length = product(size);
	nodes = new GridNode[nodes_length];
	node_volume = product(cellsize);
	
	// APIC: initialize grid node position
	nodes_position = new Eigen::Vector3d[nodes_length];
	for (int y = 0, idx = 0; y < size[1]; y++)
	{
		for (int z = 0; z < size[2]; z++)
		{
			for (int x = 0; x < size[0]; x++, idx++)
			{
				nodes_position[idx] = Eigen::Vector3d(
					x * cellsize(0) + cellsize(0) / 2.0,
					y * cellsize(1) + cellsize(1) / 2.0,
					z * cellsize(2) + cellsize(2) / 2.0);
			}
		}
	}
}

// Copy constructor
Grid::Grid(const Grid& orig) {}

Grid::~Grid()
{
	delete[] nodes;
	delete[] nodes_position;
}

// Maps mass to the grid
void Grid::initializeMass()
{
	// Reset the grid
	// If the grid is sparsely filled, it may be better to reset individual nodes
	memset(nodes, 0, sizeof(GridNode)*nodes_length);

	// Map particle data to grid
	for (int i = 0; i < point_cloud->size; i++)
	{
		Particle& p = point_cloud->particles[i];
		// Particle position to grid coordinates
		// This will give errors if the particle is outside the grid bounds
		p.grid_position = division(p.position - origin, cellsize);
		double ox = p.grid_position[0], oy = p.grid_position[1], oz = p.grid_position[2];

		// Shape function gives a blending radius of two;
		// so we do computations within a 2x2x2 cube for each particle
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++)
		{
			// Y-dimension interpolation
			double y_pos = oy - y,
				wy = Grid::B_Spline(y_pos),
				dy = Grid::B_SplineSlope(y_pos);

			for (int z = oz - 1, z_end = z + 3; z <= z_end; z++)
			{
				// Z-dimension interpolation
				double z_pos = oz - z,
					wz = Grid::B_Spline(z_pos),
					dz = Grid::B_SplineSlope(z_pos);

				for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++)
				{
					// X-dimension interpolation
					double x_pos = ox - x,
						wx = Grid::B_Spline(x_pos),
						dx = Grid::B_SplineSlope(x_pos);

					// Final weight is dyadic product of weights in each dimension
					double weight = wx * wy * wz;
					p.weights[idx] = weight;

					// Weight gradient is a vector of partial derivatives
					setData(p.weight_gradient[idx], dx*wy*wz / cellsize(0), dy*wx*wz / cellsize(1), dz*wx*wz / cellsize(2));

					// Interpolate mass
					nodes[(int)(y*size[0]*size[2] + z*size[0] + x)].mass += weight * p.mass;
				}
			}
		}
	}
}

// APIC: initialize the inertia-like tensor matrix D(n, p) in the particles
void Grid::initializeInertiaTensor()
{
	for (int i = 0; i < point_cloud->size; i++)
	{
		Particle& p = point_cloud->particles[i];

		p.inertia_tensor_reverse.setZero();

		int ox = p.grid_position[0],
			oy = p.grid_position[1],
			oz = p.grid_position[2];

		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++)
		{
			for (int z = oz - 1, z_end = z + 3; z <= z_end; z++)
			{
				for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++)
				{
					double w = p.weights[idx];
					if (w > BSPLINE_EPSILON)
					{
						int n = y * size[0] * size[2] + z * size[0] + x;
						p.inertia_tensor_reverse += w * (nodes_position[n] - p.position) * ((nodes_position[n] - p.position).transpose());
					}
				}
			}
		}

		// Save the reverse result
		p.inertia_tensor_reverse.reverseInPlace();
	}
}

// Maps velocity to the grid
void Grid::initializeVelocities()
{
	// Interpolate velocity after mass, to conserve momentum
	for (int i = 0; i < point_cloud->size; i++)
	{
		Particle& p = point_cloud->particles[i];

		int ox = p.grid_position[0],
			oy = p.grid_position[1],
			oz = p.grid_position[2];

		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++)
		{
			for (int z = oz - 1, z_end = z + 3; z <= z_end; z++)
			{
				for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++)
				{
					double w = p.weights[idx];
					if (w > BSPLINE_EPSILON)
					{
						// APIC: transfer from particles to grid is motivated analogously to the piecewise rigid case
						int n = (int)(y*size[0]*size[2] + z*size[0] + x);
						nodes[n].velocity += w * p.mass * (p.velocity + p.affine_state * p.inertia_tensor_reverse * (nodes_position[n] - p.position));
						nodes[n].active = true;
					}
				}
			}
		}
	}

	for (int i = 0; i < nodes_length; i++)
	{
		GridNode &node = nodes[i];
		if (node.active)
		{
			node.velocity /= node.mass;
		}
	}

	collisionGrid();
}

// Maps volume from the grid to particles
// This should only be called once, at the beginning of the simulation
void Grid::calculateVolumes() const
{
	// Estimate each particles volume (for force calculations)
	for (int i = 0; i < point_cloud->size; i++)
	{
		Particle& p = point_cloud->particles[i];

		int ox = p.grid_position[0],
			oy = p.grid_position[1],
			oz = p.grid_position[2];

		// First compute particle density
		p.density = 0;
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++)
		{
			for (int z = oz - 1, z_end = z + 3; z <= z_end; z++)
			{
				for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++)
				{
					double w = p.weights[idx];
					if (w > BSPLINE_EPSILON)
					{
						// Node density is trivial
						p.density += w * nodes[(int)(y*size[0]*size[2] + z*size[0] + x)].mass;
					}
				}
			}
		}

		p.density /= node_volume;

		// Volume for each particle can be found from density
		p.volume = p.mass / p.density;

		// First time step initialize the affine state matrix
		p.affine_state.setZero();
	}
}

// Calculate next timestep velocities for use in implicit integration
void Grid::explicitVelocities(const Eigen::Vector3d& gravity)
{
	// First, compute the forces
	// We store force in velocity_new, since we're not using that variable at the moment
	for (int i = 0; i < point_cloud->size; i++)
	{
		Particle& p = point_cloud->particles[i];

		// Solve for grid internal forces
		Eigen::Matrix3d energy = p.energyDerivative();

		int ox = p.grid_position[0],
			oy = p.grid_position[1],
			oz = p.grid_position[2];

		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++)
		{
			for (int z = oz - 1, z_end = z + 3; z <= z_end; z++)
			{
				for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++)
				{
					double w = p.weights[idx];
					if (w > BSPLINE_EPSILON)
					{
						// Weight the force onto nodes
						int n = (int)(y*size[0]*size[2] + z*size[0] + x);
						nodes[n].velocity_new += energy * p.weight_gradient[idx];
					}
				}
			}
		}
	}

	// Compute velocities (euler integration)
	for (int i = 0; i < nodes_length; i++)
	{
		GridNode &node = nodes[i];
		if (node.active)
		{
			node.velocity_new = node.velocity + TIMESTEP * (gravity - node.velocity_new / node.mass);
		}
	}

	collisionGrid();
}

// APIC: Update the B(n, p) affine state matrix in patticles
void Grid::updateAffineState() const
{
	for (int i = 0; i < point_cloud->size; i++)
	{
		Particle& p = point_cloud->particles[i];

		p.affine_state.setZero();

		int ox = p.grid_position[0],
			oy = p.grid_position[1],
			oz = p.grid_position[2];

		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++)
		{
			for (int z = oz - 1, z_end = z + 3; z <= z_end; z++)
			{
				for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++)
				{
					double w = p.weights[idx];
					if (w > BSPLINE_EPSILON)
					{
						int n = (y*size[0] * size[2] + z * size[0] + x);
						// This is calculated for the next time step
						p.affine_state += w * nodes[n].velocity_new * (nodes_position[n] - p.position).transpose();
					}
				}
			}
		}
	}
}

// Map grid velocities back to particles
void Grid::updateVelocities() const
{
	for (int i = 0; i < point_cloud->size; i++)
	{
		Particle& p = point_cloud->particles[i];
		// Reset velocity
		p.velocity.setZero();
		// Also keep track of velocity gradient
		Eigen::Matrix3d& grad = p.velocity_gradient;
		setData(grad, 0.0);
		// VISUALIZATION PURPOSES ONLY:
		// Recompute density
		p.density = 0;

		int ox = p.grid_position[0],
			oy = p.grid_position[1],
			oz = p.grid_position[2];

		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++)
		{
			for (int z = oz - 1, z_end = z + 3; z <= z_end; z++)
			{
				for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++)
				{
					double w = p.weights[idx];
					if (w > BSPLINE_EPSILON)
					{
						GridNode &node = nodes[(int)(y*size[0]*size[2] + z*size[0] + x)];
						// Affine Particle-In-Cell
						p.velocity += w * node.velocity_new;
						// Velocity gradient
						grad += outerProduct(node.velocity_new, p.weight_gradient[idx]);
						// VISUALIZATION ONLY: Update density
						p.density += w * node.mass;
					}
				}
			}
		}

		// VISUALIZATION: Update density
		p.density /= node_volume;
	}

	collisionParticles();
}

// Collision detection on grid
void Grid::collisionGrid()
{
	Eigen::Vector3d delta_scale = Eigen::Vector3d(TIMESTEP, TIMESTEP, TIMESTEP);
	delta_scale = division(delta_scale, cellsize);

	for (int y = 0, idx = 0; y < size[1]; y++)
	{
		for (int z = 0; z < size[2]; z++)
		{
			for (int x = 0; x < size[0]; x++, idx++)
			{
				// Get grid node (equivalent to (y*size[0]*size[2] + z*size[0] + x))
				GridNode &node = nodes[idx];
				// Check to see if this node needs to be computed
				if (node.active)
				{
					// Collision response
					// TODO: make this work for arbitrary collision geometry
					Eigen::Vector3d new_pos = dot(node.velocity_new, delta_scale) + Eigen::Vector3d(x, y, z);
					// Left border, right border
					if (new_pos[0] < BSPLINE_RADIUS || new_pos[0] > size[0] - BSPLINE_RADIUS - 1)
					{
						node.velocity_new[0] = 0;
						node.velocity_new[1] *= STICKY;
						node.velocity_new[2] *= STICKY;
					}
					// Bottom border, top border
					if (new_pos[1] < BSPLINE_RADIUS || new_pos[1] > size[1] - BSPLINE_RADIUS - 1)
					{
						node.velocity_new[0] *= STICKY;
						node.velocity_new[1] = 0;
						node.velocity_new[2] *= STICKY;
					}
					// Front border, back border
					if (new_pos[2] < BSPLINE_RADIUS || new_pos[2] > size[2] - BSPLINE_RADIUS - 1)
					{
						node.velocity_new[0] *= STICKY;
						node.velocity_new[1] *= STICKY;
						node.velocity_new[2] = 0;
					}
				}
			}
		}
	}
}

// Collision detection on particle
void Grid::collisionParticles() const
{
	for (int i = 0; i < point_cloud->size; i++)
	{
		Particle& p = point_cloud->particles[i];
		Eigen::Vector3d new_pos = p.grid_position + TIMESTEP * division(p.velocity, cellsize);
		// Left border, right border
		if (new_pos[0] < BSPLINE_RADIUS - 1 || new_pos[0] > size[0] - BSPLINE_RADIUS)
		{
			p.velocity[0] = -STICKY * p.velocity[0];
		}
		// Bottom border, top border
		if (new_pos[1] < BSPLINE_RADIUS - 1 || new_pos[1] > size[1] - BSPLINE_RADIUS)
		{
			p.velocity[1] = -STICKY * p.velocity[1];
		}
		// Front border, back border
		if (new_pos[2] < BSPLINE_RADIUS - 1 || new_pos[2] > size[2] - BSPLINE_RADIUS)
		{
			p.velocity[2] = -STICKY * p.velocity[2];
		}
	}
}