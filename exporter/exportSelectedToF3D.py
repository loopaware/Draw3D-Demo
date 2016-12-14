#############################################
#
#   Autor:  Fredrick Johansson
#   Project:	F3D Exporter
#   Purpose:	Exporting the vertices of meshes into a binary f3d-file.
#
#############################################

import bpy  # Blender Python API @UnresolvedImport
import struct   # Struct, for packing of binary data
from ctypes import c_ulonglong

# Say hello
print("F3D Exporter initializing...")

# Open file, binary mode, and write header
#file = open("testModel.entity", 'wb')
file = open("D:\\Projects\\LoopAware\\Workspaces\\FredrickJohansson\\draw3d-quick\\meshes\\mesh.f3d", 'wb')
file.write(struct.pack('<3I', 1, 0, 0))			# Filetype version: 1.0.0

# Fetch scene entities
for item in bpy.data.objects:
	print("Name: " + item.name)

	# Is it a mesh?
	if (item.type == 'MESH'):
		# Tesselating mesh (cache)
		print("Tesselating mesh (" + item.name + ")... ")
		
		item.data.update(calc_tessface=True)
		#item.data.calc_normals()
		#item.data.calc_tangents()
		
		# Tell user important info
		print("Exporting mesh (" + item.name + ")... ")
		 
		polygons	= [] # list of polygons
		vCoords  = [] # List of vertex coords/positions
		vTexCoords  = [] # List of texture coordinates
		vNormals	= [] # List of vertex normals
		
		#item.data.update(calc_tessface=1)
		for face_uv in item.data.tessface_uv_textures.active.data:
			for vert_uv in face_uv.uv:
				vTexCoords.append(tuple(vert_uv))
				#print(list(vert_uv))
				
		# vertexColors
		#print(item.data.tessface_vertex_colors)
		for poly_colors in item.data.tessface_vertex_colors:
			for vColor in poly_colors:
				pass#print(list(vColor))
		
		# Fetch vertices and normals
		for vertex in item.data.vertices:
			vCoords.append(tuple(vertex.co))
			vNormals.append(tuple(vertex.normal))
		   
		for poly in item.data.tessfaces:
			if len(tuple(poly.vertices)) == 3:
				polygons.append(tuple(poly.vertices))
			else:
				polygons.append((poly.vertices[0], poly.vertices[1], poly.vertices[2]))
				polygons.append((poly.vertices[0], poly.vertices[2], poly.vertices[3]))
			
			
			#polygons.append(tuple(poly.vertices))
			
			#for loop_index in range(face[0], face[0] + 3):
				#vertexNormals.append(item.data.loops[loop_index].normal)
				
				#print("	Vertex: %d" % me.loops[loop_index].vertex_index)
				#print(" Normal: " + str(item.data.loops[loop_index].normal))
				#print("	Tangent: " + str(me.loops[loop_index].tangent))
				#print("	Bitangent: " + str(me.loops[loop_index].bitangent))

			# Add same normal 3 times, (duplicate normals)
			#vertexNormals.append(item.data.loops[0 + i].normal)
			#vertexNormals.append(item.data.loops[1 + i].normal)
			#vertexNormals.append(item.data.loops[2 + i].normal)
			#i += 1
		
		file.write(struct.pack('<Q', len(polygons)*3))
		
		i = 0
		for poly in polygons:   
			for index in poly:
				#print(str(verts[index]))
				
				file.write(struct.pack('<fff', vCoords[index][0], vCoords[index][1], vCoords[index][2]))		# Write postition
				file.write(struct.pack('<ff', vTexCoords[i][0], vTexCoords[i][1]))					  # Write texCoord
				file.write(struct.pack('<fff', vNormals[index][0], vNormals[index][1], vNormals[index][2]))  # Write normal
				i += 1

		# Tell user important info
		print("Exported mesh (" + item.name + ") successfully!")
		
# Close the file
file.close()

# Say goodbye
print("F3D Exporter done!")
