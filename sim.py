import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from stl import mesh

# --- Load your STL ---
your_mesh = mesh.Mesh.from_file('Wind_Turbine.stl')
vectors = your_mesh.vectors  # shape (n_triangles, 3, 3)

# --- Rotation center (hub position in STL coords) ---
C = np.array([0.0, 0.0, 0.0])  # adjust to your model

# --- Set up the figure ---
fig = plt.figure(figsize=(10, 8))
ax = fig.add_subplot(111, projection='3d')

# Collect all triangles into one Poly3DCollection
def make_collection(verts):
    return Poly3DCollection(verts, facecolor='lightgray',
                            edgecolor='none', alpha=1.0)

poly = make_collection(vectors)
ax.add_collection3d(poly)

# Set axis limits based on the mesh
pts = vectors.reshape(-1, 3)
ax.set_xlim(pts[:,0].min(), pts[:,0].max())
ax.set_ylim(pts[:,1].min(), pts[:,1].max())
ax.set_zlim(pts[:,2].min(), pts[:,2].max())
ax.set_box_aspect([1,1,1])
ax.set_axis_off()

# --- Animation loop ---
def rotation_matrix(axis, theta):
    axis = axis / np.linalg.norm(axis)
    a = np.cos(theta/2); b,c,d = -axis*np.sin(theta/2)
    return np.array([
        [a*a+b*b-c*c-d*d, 2*(b*c+a*d),     2*(b*d-a*c)],
        [2*(b*c-a*d),     a*a+c*c-b*b-d*d, 2*(c*d+a*b)],
        [2*(b*d+a*c),     2*(c*d-a*b),     a*a+d*d-b*b-c*c]
    ])

fps = 30
frames = 300
omega_vis = 0.15  # rad/frame -- purely visual

for f in range(frames):
    R = rotation_matrix(np.array([0,0,1]), omega_vis)  # spin around z

    # Rotate around C: translate -> rotate -> translate back
    new_vectors = np.empty_like(vectors)
    for i in range(vectors.shape[0]):
        tri = vectors[i] - C
        tri = tri @ R.T
        new_vectors[i] = tri + C

    poly.set_verts(new_vectors)
    plt.pause(1/fps)

plt.show()